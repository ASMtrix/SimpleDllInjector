#ifndef STRING__H
#define STRING__H

#include <stdio.h>
#include <windows.h>
#include "memory.h"

#ifndef  __ARRAY__
#include "array.h"
#endif


/*
	Example:
	struct String testing={0};
	struct String resultRef={0};

	CreateString("I like ", &testing);
	CatString("hamburgers.",&testing);
	CatStringAt("to eat ", &testing,7,&resultRef);
	CatStringAt(" not ", &resultRef,7,&resultRef);
	CatStringFront("hello,",&resultRef);


	printf("%s\n", GetString(&resultRef));
	printf("%s\n", GetString(&testing));
*/

struct StringSearchResults
{
	int max;
	struct Array results;
	int count;
};

struct String
{
	char* text;	
};

void CreateString(char* text, struct String* ref);
int LenString(char* text);
void CatString(char* appendString,struct String* ref);
void CatStringFront(char* appendString,struct String* ref);
char* GetString(struct String* ref);
char GetCharFromString(struct String* source, int pos);
void MidString(struct String* source, int from, int to,struct String* resultRef);
void CatStringAt(char* addedText,struct String* source,int location,struct String* resultRef);
void FindString(struct String* stringToFind, struct String* sourceString, struct StringSearchResults* rString);

void CatStringAt(char* addedText,struct String* source,int location,struct String* resultRef)
{
	int i=0;
	int totalLen  = 0;
	struct String tempStart={0};
	struct String tempEnd={0};
	
	assert(location <= LenString(source->text));
	assert(location > 1);
	
	totalLen = LenString(source->text) - location;

	MidString(source,0,location,&tempStart);
	MidString(source,location, LenString(source->text), &tempEnd);
	/*
	if (resultRef->text == NULL)
	{
	} else {
		CatString(GetString(&tempStart),resultRef);
	}	
	*/
	CreateString(GetString(&tempStart),resultRef);

	//CatString(GetString(&tempStart), resultRef);
	CatString(addedText, resultRef);
	CatString(GetString(&tempEnd), resultRef);

}

void MidString(struct String* source, int from, int to,struct String* resultRef)
{
	int len = to - from;
	int i=0;
	char a[3]={0};
	char c='\0';

	assert(from > -1);
	assert(len > -1);

	if (LenString(source->text) < to)
	{
		assert(1 < 1);
	}

	if (resultRef->text == NULL)
	{
		CreateString("",resultRef);
	}

 	for (i=from;i<to;i++)
	{
		c = GetCharFromString(source,i);
		a[0] = c;	
		a[1] = '\0';
		CatString(a,resultRef);
	}
	
}

char GetCharFromString(struct String* source, int pos)
{
	char s = '\0';
	s = source->text[pos];

	return s;
}

char* GetString(struct String* ref)
{
	return ref->text;
}

void CreateString(char* text, struct String* ref)
{
	//ref->text =(char*)  malloc(LenString(text)+1);
	ref->text = (char*) VirtualAlloc(NULL,LenString(text)+1,MEM_COMMIT,PAGE_READWRITE);

	assert(ref->text != NULL);

	memcopy(text,ref->text, LenString(text)+1);
}

void CatStringFront(char* appendString,struct String* ref)
{
	int strLen = LenString(ref->text);
	char *textPtr=NULL;
	char* temp = NULL;// (char*) malloc(strlen+1);

	assert(appendString != NULL);
	assert(strLen > 1);

	temp = (char*) VirtualAlloc(NULL,strLen+1,MEM_COMMIT,PAGE_READWRITE);

	assert(temp != NULL);
	memcopy(ref->text,temp,strLen);

	/*
	if (ref->text)
	{
		free(ref->text);
		ref->text=NULL;
	}*/
	ref->text = NULL;
	ref->text = (char*) VirtualAlloc(NULL,strLen+LenString(appendString)+10,MEM_COMMIT,PAGE_READWRITE);
	assert(ref->text != NULL);
	//ref->text = (char*) malloc(strlen+LenString(appendString)+2);

	memcopy(appendString,ref->text,LenString(appendString));
	
	textPtr = ref->text;
	textPtr += LenString(appendString);
	
	memcopy(temp,textPtr,strLen+1);
	/*
	if (temp)
	{
		free(temp);
		temp=NULL;
	}
*/
}

void CatString(char* appendString,struct String* ref)
{
	#if 1
	int strlen = LenString(ref->text);
	char *textPtr=NULL;
	char* temp = NULL;//(char*) malloc(strlen+1);
	
	
	temp = (char*) VirtualAlloc(NULL,strlen+1,MEM_COMMIT,PAGE_READWRITE);
	assert(temp != NULL); 
	memcopy(ref->text,temp,strlen);
	/*
	if (ref->text)
	{
		free(ref->text);
		ref->text=NULL;
	}*/
	
	//ref->text = (char*) malloc(strlen+LenString(appendString)+2);
	ref->text = (char*) VirtualAlloc(NULL,strlen+LenString(appendString)+10,MEM_COMMIT,PAGE_READWRITE);

	assert(ref->text != NULL); 

	memcopy(temp,ref->text,strlen);
	textPtr = ref->text;

	textPtr+=strlen;

	memcopy(appendString,textPtr,LenString(appendString)+1);
	/*
	if (temp)
	{
		free(temp);
		temp=NULL;
	}*/
	#endif
}

int LenString(char* text)
{
	char* iter = text;
	int length=0;

	while (iter[0] != '\0')
	{
		iter++;
		length++;
	}
	return length;
}




void FindString(struct String* stringToFind, struct String* sourceString, struct StringSearchResults* rString)
{
	int i=0,k=0;
	int found=0;
	int start=0;

	assert(stringToFind->text != NULL);
	assert(sourceString->text != NULL);


	CreateArray(&rString->results);		



	for (i = 0;i<LenString(sourceString->text);i++)
	{
		
		if (GetCharFromString(stringToFind,0) ==  GetCharFromString(sourceString,i) && found == 0)
		{
			start = i;
			found=1;
		}
		
		if (found == 1)
		{
			if (GetCharFromString(stringToFind,k) == GetCharFromString(sourceString,i))
			{
				k++;
				if (GetCharFromString(stringToFind,k) == '\0')
				{
					int* postion = (int*) VirtualAlloc(NULL,sizeof(int),MEM_COMMIT,PAGE_READWRITE);
					postion = (int*) start;
					AddToArray(&rString->results,postion);
					k=0;
					found=0;
					start=0;
					rString->count++;
				}
			} else {
				k=0;
				found=0;
			}
		}
			
	}
}

#endif
