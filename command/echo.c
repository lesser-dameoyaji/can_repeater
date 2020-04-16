#include <stdio.h>
#include "common.h"

void cmd_echo(int id, int argc, void** argv)
{
	int i;
	char** buf = (char**)argv;
	
	if(argc > 1)
	{
		i = 1;
		printf("%s",buf[i++]); 
		
		for(; i < argc; i++)
		{
			printf(" %s", buf[i]);
		}
		printf("\n");
	}
}
