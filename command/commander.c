#include <stdio.h>
#include <string.h>

#include "common.h"
#include "command.h"

void cmd_csock(int id, int argc, void** argv);
void cmd_id(int id, int argc, void** argv);
void cmd_send(int id, int argc, void** argv);
void cmd_route(int id, int argc, void** argv);
void cmd_echo(int id, int argc, void** argv);
void cmd_count(int id, int argc, void** argv);
void cmd_thread(int id, int argc, void** argv);
void cmd_reset(int id, int argc, void** argv);
void cmd_exit(int id, int argc, void** argv);

typedef struct {
	char* cmd_str;
	cmd_func* func;
} cmd_table_t;

//
static cmd_table_t cmd_table[] = {
	{"echo", cmd_echo},
	{"id", cmd_id},
	{"send", cmd_send},
	{"route", cmd_route},
	{"csock", cmd_csock},
	{"count", cmd_count},
	{"thread", cmd_thread},
	{"reset", cmd_reset},
	{"exit", cmd_exit},
	{NULL, NULL}
};

int chrchrs(char c, char* cs)
{
	int ic;
	
	for(ic=0; cs[ic] != 0; ic++)
	{
		// compare
		if(c == cs[ic])
		{
			return 0;
		}
	}
	return -1;
}

char* strchrs(char* buf, char* cs)
{
	int ib;
	
	// until buffer terminator
	for(ib=0; buf[ib] != 0; ib++)
	{
		if(chrchrs(buf[ib], cs) == 0)
			return &buf[ib];
	}
	return NULL;
}


void* cmd_argv[ARGUMENT_MAX];
int cmd_argc;
char cmd_crlf[] = {'\r', '\n', 0};
char cmd_separater1[] = {' ', 0};

void parser(char* buf)
{
	char* crlf;
	
	// remove CRLF
	crlf = strchrs(buf, cmd_crlf);
	if(crlf != NULL)
		*crlf = 0;

	cmd_argc = 0;
	cmd_argv[cmd_argc] = (void*)buf;
	if(cmd_argv[cmd_argc] == NULL)
	{
		return;
	}
	cmd_argc++;
	
	while((cmd_argc < ARGUMENT_MAX) && (*buf != 0))
	{
		buf = strchrs(buf, cmd_separater1);
		if(buf == NULL)
		{
			return;
		}
		
		// over write separater with 0
		do
		{
			*buf = 0;
			buf++;
		}
		while(chrchrs(*buf, cmd_separater1) == 0);
		
		if(*buf != 0)
		{
			cmd_argv[cmd_argc] = buf;
			cmd_argc++;
		}
	}
}

//
// command
//
void commander(int id, char* buf)
{
	int idx;
	int dbgi;
	
	for(idx=0; cmd_table[idx].cmd_str != NULL; idx++)
	{
		if(strncmp(cmd_table[idx].cmd_str, buf, strlen(cmd_table[idx].cmd_str)) == 0)
		{
		 	if(cmd_table[idx].func == NULL)
		 	{
		 		printf("no func\n");
		 		continue;
		 	}
			parser(buf);
/*			if(cmd_argc > 0)
			{
				printf("argc=%d ", cmd_argc);
				for(dbgi=0; dbgi < cmd_argc; dbgi++)
				printf("arg%d:%s,", dbgi, (char*)cmd_argv[dbgi]);
			}
			printf("\n");
*/			cmd_table[idx].func(id, cmd_argc, (void**)cmd_argv);
		}
	}
}

