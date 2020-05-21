#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "command.h"

// 転送コマンド

void cmd_id(int id, int argc, void** argv)
{
	int dst_id;
	char* data;
	
	if(argc < 2)
	{
		printf("no id\n");
		return;
	}
	if(argc < 3)
	{
		printf("no data\n");
		return;
	}
	dst_id = strtol((char*)argv[1], NULL, 10);
	data = (char*)argv[2];
	
	// reverse parser. exchange \0 to SPC
	for(;data < (char*)argv[argc-1]; data++)
	{
		if(*data == '\0')
			*data = ' ';
	}
	
	data = (char*)argv[2];
	
	send_command(id, dst_id, data, strlen(data));
}
