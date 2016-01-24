#ifndef __MEMORY__H
#define __MEMORY__H

#define assert(x) if (!(x)){printf("assertion on line %i\n",__LINE__);exit(1);}

void memcopy(char* source, char* dest, int length);

void memcopy(char* source, char* dest, int length)
{
	int total=0;
	
	assert(length > -1);

	while(total != length)
	{
		dest[total] =  source[total];
		total++;
	}	
}
#endif
