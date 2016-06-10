#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include  <wchar.h>
#include <commCtrl.h>
#include "string.h"
#include "resource.h"

#define debug 0

#ifdef debug 1
#define assert(x) if(!(x)){ MessageBoxA(NULL,"Assert hit","Assert hit",MB_OK);int *i;*i=1; }
#else
#define assert(x) if (!(x)) { MessageBoxA(NULL,"An Error has occured","An Error has occured", MB_OK); }
#endif

enum {
COL1,COL2
};

bool done=false;
HINSTANCE hi=0;
HWND processHwnd=0;
HWND hwndListView=0;
HWND hDlg=0;

BOOL EnableDebugPrivilege();
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProcessCallback(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam);

void ProcessWindow();
void GetProcesses();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	BOOL ret;

	hi = hInstance;

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


void ProcessWindow()
{
	WNDCLASS wClass = {};
	wClass.hInstance =  hi;
	wClass.lpszClassName = L"ProcessWindow";
	wClass.lpfnWndProc = ProcessCallback;

	if (RegisterClass(&wClass))
	{
		processHwnd = CreateWindowEx(0,wClass.lpszClassName,L"Current Open Processes", WS_VISIBLE |  /*WS_OVERLAPPEDWINDOW |*/ WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT ,CW_USEDEFAULT,310,350,0,0,hi,0);

		if (processHwnd)
		{
				MSG msg={};
				while(GetMessage(&msg,NULL,0,0))
				{
					if (done == false)
					{
						break;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
		}
	}
}


LRESULT CALLBACK ProcessCallback(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result=0;

	switch(msg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case 23:
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

					int i=SendMessage(hwndListView,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED );
					if ( i != -1)
					{
						TCHAR buff2[256]={};
						LVITEM SelectedItem2={};
						SelectedItem2.iItem = i;
						SelectedItem2.state =  LVIS_SELECTED ;
						SelectedItem2.mask = LVIF_TEXT;
						SelectedItem2.stateMask = LVIS_SELECTED ;
						SelectedItem2.iSubItem = COL1;
						SelectedItem2.pszText = buff2;
						SelectedItem2.cchTextMax = 256;
						ListView_GetItem(hwndListView, (LVITEM*)&SelectedItem2);

						pid =  _wtoi( buff2);
						/*
						char testi[256] = {};
						sprintf(testi,"fuck windows %i", pid);
						MessageBoxA(NULL,testi,testi,MB_OK);
						*/
						TCHAR buff[256]={};
						LVITEM SelectedItem={};
						SelectedItem.iItem = i;
						SelectedItem.state =  LVIS_SELECTED ;
						SelectedItem.mask = LVIF_TEXT;
						SelectedItem.stateMask = LVIS_SELECTED ;
						SelectedItem.iSubItem = COL2;
						SelectedItem.pszText = buff;
						SelectedItem.cchTextMax = 256;
						ListView_GetItem(hwndListView, (LVITEM*)&SelectedItem);
						
						EnableDebugPrivilege();

						pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

						if (pHandle)
						{
							dllName = (char*) VirtualAlloc(NULL,length+2,MEM_COMMIT,PAGE_READWRITE);
							SendMessageA(hEditBox, WM_GETTEXT, (WPARAM)length+2, (LPARAM)dllName);
							length = strlen(dllName);

							procAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
							assert(procAddr != NULL);
							processdllName = (LPVOID) VirtualAllocEx(pHandle, NULL, length+2, MEM_COMMIT, PAGE_READWRITE);
							assert(processdllName !=NULL);
							
							if (processdllName == NULL)
							{
								MessageBoxA(NULL,"Error", "Process not found. Either the process stopped or doesn't exist. Please refresh the list",MB_OK);
								ShowWindow(processHwnd,SW_HIDE);
								break;
							}
							//char dname[256]={};
							//wcstombs(dname, buff, length );

							///wcstombs(dllName, buff, length );

							bool sess = WriteProcessMemory(pHandle,(LPVOID) processdllName, dllName, length+2, NULL);
							
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
						ShowWindow(processHwnd,SW_HIDE);

					}
					break;
				}

				case 24:
				{
					GetProcesses();
					break;
				}
			}

			break;
		}
		case WM_CREATE:
		{
			hwndListView = CreateWindowEx(0, WC_LISTVIEW,L"ListView",WS_BORDER | WS_VISIBLE| WS_CHILD | LVS_REPORT,10,80,280,200,hwnd,(HMENU)0,0,0);
			LVCOLUMNA lvcol;
			lvcol.mask=LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT |  LVIS_FOCUSED | LVIS_SELECTED;
			lvcol.fmt=LVCFMT_LEFT;
			lvcol.cx=80;
			lvcol.pszText = "PID";
			SendMessage(hwndListView, LVM_INSERTCOLUMNA,COL1,(WPARAM) &lvcol);

			lvcol.cx = 200;
			lvcol.pszText = "Executable name";
			SendMessage(hwndListView, LVM_INSERTCOLUMNA,COL2,(WPARAM) &lvcol);

			ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS | LVS_EX_GRIDLINES | LVIS_FOCUSED  );
			//ListView_SetExtendedListViewStyle(hwndListView, LVS_SHOWSELALWAYS);
			GetProcesses();
			CreateWindow(TEXT("BUTTON"), TEXT("Choose"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,180,290,100,20,hwnd,(HMENU) 23, 0, NULL);
			CreateWindow(TEXT("BUTTON"), TEXT("Refresh"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,180,50,100,20,hwnd,(HMENU) 24, 0, NULL);

			break;
		}
		case WM_CLOSE:
		{
			ShowWindow(processHwnd,SW_HIDE);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps={0};
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW));

			EndPaint(hwnd, &ps);
			
			break;
		}
		case WM_DESTROY:
		{
			done = true;
			DestroyWindow(hwnd);
			break;
		}
		default:
		{
			result = DefWindowProc(hwnd,msg,wParam,lParam);
			break;
		}
	}

	return result;
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
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
					if (processHwnd == 0)
					{
						ProcessWindow();
					} else {
						ShowWindow(processHwnd,SW_SHOW);
					}

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
			if (processHwnd != 0)
			{
				DestroyWindow(processHwnd);
			}
			return TRUE;
			break;
		}
	}
	return FALSE;
}


void GetProcesses()
{

	ListView_DeleteAllItems(hwndListView);

	HANDLE hProcessSnap=0;
	PROCESSENTRY32 pe32={};
	int count=0;			
	HWND comboBox1=0;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	assert(hProcessSnap != INVALID_HANDLE_VALUE);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hProcessSnap, &pe32);

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
			LVITEMA item;
			item.mask=LVIF_TEXT;
			item.iItem=0;
			item.iSubItem=COL1;
			item.pszText = buffer;
			SendMessage(hwndListView,LVM_INSERTITEMA,NULL,(WPARAM) &item);

			item.iSubItem =COL2;
			item.pszText = exeName;
			SendMessage(hwndListView,LVM_SETITEMA,NULL,(WPARAM) &item);

		}
	}

	CloseHandle(hProcessSnap);
}
