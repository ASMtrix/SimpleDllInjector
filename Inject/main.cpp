#include <Windows.h>
#include <TlHelp32.h>
#include "string.h"
#include "resource.h"

#define assert(x) if(!(x)){ MessageBoxA(NULL,"Assert hit","Assert hit",MB_OK);int *i;*i=1; }

bool done=false;


BOOL EnableDebugPrivilege();
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND hDlg;
	MSG msg;
	BOOL ret;

	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);

	assert(hDlg != NULL);

	ShowWindow(hDlg, nShowCmd);

	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) 
	{
		if (done == true)
		{
			break;
		}
		if(ret == -1)
		{
			return -1;
		}

		if(!IsDialogMessage(hDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON3:
				{
					char pFilename[_MAX_PATH]={};
					HWND hComboBox=0;
					HWND hEditBox=0;
					int nIndex=0;
					struct String comboText={};
					struct String dashStr={};
					struct StringSearchResults stringResults={};
					struct String processIDString={};
					DWORD pid=0;
					HANDLE pHandle=0;
					LPVOID procAddr=0;
					int length=0;
					char* dllName=NULL;
					LPVOID processdllName=0;
					HANDLE newThread=0;
					
					EnableDebugPrivilege();
					hEditBox = GetDlgItem(hDlg, IDC_EDIT1); 
					length = SendMessage(hEditBox,WM_GETTEXTLENGTH,0,0);

					if (length == 0)
					{
						MessageBoxA(NULL,"Please Enter a DLL to inject","Error",MB_OK);
						return 0;
					}
					 
					dllName = (char*) VirtualAlloc(NULL,length+2,MEM_COMMIT,PAGE_READWRITE);

					SendMessageA(hEditBox, WM_GETTEXT, (WPARAM)length+2, (LPARAM)dllName);

					hComboBox =  GetDlgItem(hDlg, IDC_COMBO1);


					nIndex = SendMessage( hComboBox , CB_GETCURSEL, 0, 0 );
					
					if (nIndex != -1)
					{
						SendMessageA( hComboBox , CB_GETLBTEXT, nIndex, (LPARAM)pFilename );
						CreateString("-",&dashStr);
						CreateString(pFilename,&comboText);

						FindString(&dashStr,&comboText,&stringResults);

						if (stringResults.count > 0)
						{
							MidString(&comboText,0,(int) stringResults.results.items[0], &processIDString);
							
							pid =  atoi(GetString(&processIDString)); 

							pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

							if (pHandle)
							{
								procAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
								assert(procAddr != NULL);
								processdllName = (LPVOID) VirtualAllocEx(pHandle, NULL, length+2, MEM_COMMIT, PAGE_READWRITE);
								assert(processdllName !=NULL);
								bool sess = WriteProcessMemory(pHandle,(LPVOID) processdllName, dllName, length+2, NULL);
							
								//MessageBoxA(NULL, (char*)  processdllName, (char*)  dllName,MB_OK);

								newThread = CreateRemoteThread(pHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)procAddr,(LPVOID) processdllName, 0, NULL);
								//WaitForSingleObject(newThread, INFINITE);
								if (newThread)
								{
									MessageBoxA(NULL,"DLL Injected!","DLL Injected!",MB_OK);	
								} else {
									MessageBoxA(NULL,"Error Could not Inject DLL","Error could not inject DLL",MB_OK);
								}
							
							} else {
								MessageBoxA(NULL,"Error couldn't open process. You might need to run this under Admin.","Error",MB_OK);
							}
						}
					}
					break;
				}
				case IDC_BUTTON2:
				{
					char filename[MAX_PATH]={};
					OPENFILENAMEA ofn={};
					HWND textboxHandle=0;

					ZeroMemory(&filename,sizeof(filename));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFilter ="DLL Files\0*.dll\0";
					ofn.hwndOwner = NULL;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = "Select Dll to inject";

					assert(GetOpenFileNameA( &ofn ));
					
					textboxHandle = GetDlgItem(hDlg, IDC_EDIT1); 
						
					assert(textboxHandle != NULL);

					SendMessageA(textboxHandle, WM_SETTEXT, 0, (LPARAM) filename);
					

					break;
				}
				case IDC_BUTTON1:
				{
					HANDLE hProcessSnap=0;
					PROCESSENTRY32 pe32={};
					int count=0;			
					HWND comboBox1=0;

					comboBox1 = GetDlgItem(hDlg, IDC_COMBO1); 
					assert(comboBox1 != NULL);

					SendDlgItemMessage(hDlg, IDC_COMBO1, CB_RESETCONTENT, 0, 0);

					hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

					assert(hProcessSnap != INVALID_HANDLE_VALUE);

					pe32.dwSize = sizeof(PROCESSENTRY32);

					Process32First(hProcessSnap, &pe32);

					SendMessage(comboBox1, CB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);

					while (Process32Next(hProcessSnap, &pe32)) 
					{ 
						struct StringSearchResults stringResults={};

						struct String exefile={};
						char exeName[MAX_PATH]={};

						//jump though some hoops thanks to Microsoft.
						wcstombs(exeName,pe32.szExeFile,MAX_PATH);
						CreateString(exeName,&exefile);

						String exeExt={};
						CreateString(".exe", &exeExt);

						FindString(&exeExt,&exefile,&stringResults);
						
						if (stringResults.count > 0)
						{
							char buffer[256]={};
							struct String newString={};
							
							itoa (pe32.th32ProcessID,buffer,10);

							CreateString(buffer,&newString);
							CatString("-",&newString);
							CatString(exeName,&newString);

							SendMessageA(comboBox1, CB_ADDSTRING, 0, (LPARAM)GetString(&newString));
						}
					}

					CloseHandle(hProcessSnap);
					
					break;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			done = true;
			DestroyWindow(hDlg);
			return TRUE;
		}

		case WM_DESTROY:
		{
			done = true;
			DestroyWindow(hDlg);
			return TRUE;
			break;
		}
	}
	return FALSE;
}

BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    if(!OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ))
    {
        return FALSE;
    }

    if(!LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luid ))
    {
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(!AdjustTokenPrivileges( hToken, false, &tkp, sizeof( tkp ), NULL, NULL ))
    {
        return FALSE;
    }

    if(!CloseHandle( hToken ))
    {
        return FALSE;
    }

    return TRUE;
}