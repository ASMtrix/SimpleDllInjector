#ifndef __ARRAY__
#define __ARRAY__
#include <Windows.h>
#include "memory.h"

struct Array
{
	int max;
	int total;
	int* items;
};

void MemoryZero(int* pointer,int size);
void CreateArray(struct Array* array);
void AddToArray(struct Array* array, int* pointer);

void CreateArray(struct Array* array)
{
	array->total=0;
	array->max = 3;
	array->items = (int*) VirtualAlloc(NULL,(sizeof(int)*array->max),MEM_COMMIT,PAGE_READWRITE);
	MemoryZero(array->items,sizeof(int)*array->max);
}


void AddToArray(struct Array* array, int* pointer)
{
	if (array->max == array->total)
	{
		int oldMax=array->max;
		int* tempItems = (int*) VirtualAlloc(NULL,(sizeof(int)*array->max),MEM_COMMIT,PAGE_READWRITE);
		
		MemoryZero((int*)tempItems,sizeof(int)*array->max);


		memcopy((char*)array->items,(char*)tempItems,sizeof(int)*array->max);

		array->max *=20;
		array->items = (int*) VirtualAlloc(NULL,(sizeof(int)*array->max),MEM_COMMIT,PAGE_READWRITE);
		MemoryZero((int*)array->items,sizeof(int)*array->max);
		memcopy((char*)tempItems, (char*)array->items,sizeof(int)*oldMax);
	}

	array->items[array->total] = (int) pointer;
	array->total++;
}



void MemoryZero(int* pointer,int size)
{
	int i=0;

	for (i=0;i<size;i++)
	{
		pointer[i] = 0;
	}
}



#endif

