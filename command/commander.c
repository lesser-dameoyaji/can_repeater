#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "command.h"
#include "rpt.h"

void cmd_csock(int id, int argc, void** argv);
void cmd_interface(int id, int argc, void** argv);
void cmd_id(int id, int argc, void** argv);
void cmd_send(int id, int argc, void** argv);
void cmd_route(int id, int argc, void** argv);
void cmd_echo(int id, int argc, void** argv);
void cmd_count(int id, int argc, void** argv);
void cmd_thread(int id, int argc, void** argv);
void cmd_reset(int id, int argc, void** argv);
void cmd_load(int id, int argc, void** argv);
void cmd_save(int id, int argc, void** argv);
void cmd_exit(int id, int argc, void** argv);

typedef struct {
	char* cmd_str;
	cmd_func* func;
	bool save;
} cmd_table_t;

//
static cmd_table_t cmd_table[] = {
	{"thread", cmd_thread, false},
	{"interface", cmd_interface, true},
	{"csock", cmd_csock, true},
	{"echo", cmd_echo, false},
	{"id", cmd_id, false},
	{"send", cmd_send, false},
	{"route", cmd_route, true},
	{"count", cmd_count, false},
	{"reset", cmd_reset, false},
	{"load", cmd_load, false},
	{"save", cmd_save, false},
	{"exit", cmd_exit, false},
	{NULL, NULL}
};
#define cmd_table_size (sizeof(cmd_table)/sizeof(cmd_table_t)-1)


// 文字列から文字を検索する
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

// 文字列から文字列を検索する
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


char cmd_crlf[] = {'\r', '\n', 0};
char cmd_separater1[] = {' ', 0};

// パーサー
//  末尾のCRLFを除去
//  スペースを0
//  文字列先頭アドレスを配列へ格納
void parser(int id, char* buf)
{
	char* crlf;
	
	// remove CRLF
	crlf = strchrs(buf, cmd_crlf);
	if(crlf != NULL)
		*crlf = 0;

	self_argc = 0;
	self_argv[self_argc] = (void*)buf;
	if(self_argv[self_argc] == NULL)
	{
		return;
	}
	self_argc++;
	
	while((self_argc < ARGUMENT_MAX) && (*buf != 0))
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
			self_argv[self_argc] = buf;
			self_argc++;
		}
	}
}

static int wakeup_thread(int id)
{
	pthread_t thread_handle;
	
	if(self.status < STATUS_RUN)
	{
		// thread start
		thread_handle = thread_start(id);
	
		if (thread_handle < 0)
		{
			printf("faile to create self thread %d\n", id);
			return -1;
		}
		// wait for thread boot
		while(true)
		{
			usleep(100*1000);
			if(self.status >= STATUS_RUN)
				break;
		}
		printf("detect thread %d start\n", id);
	}
	return 0;
}

int send_command(int id, int dst_id, char* data, int len)
{
	if(wakeup_thread(id) < 0)
	{
		return -1;
	}
	if(wakeup_thread(dst_id) < 0)
	{
		return -1;
	}
	
	if(sendto(self_cli_handle, data, len, 0, (struct sockaddr *)&selfs[dst_id].server_addr, sizeof(selfs[dst_id].server_addr)) < 0)
	{
		return -1;
	}
	return 0;
}

int send_response(int id, char* data, int len)
{
//	printf("send resp: %x,%d\n", self_from_addr.sin_addr.s_addr, self_from_addr.sin_port);
	if(sendto(self_cli_handle, data, len, 0, (struct sockaddr *)&self_from_addr, sizeof(self_from_addr)) < 0)
	{
		return -1;
	}
	return 0;
}

// caution need GLOBAL_CH_MAX < 10
bool is_need_response(int id, char* cmd_str)
{
	char* cmd = (char*)self_argv[0];
	
	if(cmd[strlen(cmd_str)] == 'r')
		return true;
	else
		return false;
}

int get_command_table_size(void)
{
	return cmd_table_size;
}

char* get_command_name(int func_id)
{
	if(cmd_table_size <= func_id)
	{
		printf("range over id\n");
		return NULL;
	}
	return cmd_table[func_id].cmd_str;
}

cmd_func* get_command_func(int func_id)
{
	if(cmd_table_size <= func_id)
	{
		printf("range over id\n");
		return NULL;
	}
	return cmd_table[func_id].func;
}

bool is_need_save_command(int func_id)
{
	if(cmd_table_size <= func_id)
	{
		printf("range over id\n");
		return false;
	}
	return cmd_table[func_id].save;
}

//
// command
//
void commander(int id, char* buf)
{
	int idx;
	int dbgi;
	int cmd_str_len;
	int len;
	int src_id;
	char* postfix;

	// search command from command table, and exec
	for(idx=0; cmd_table[idx].cmd_str != NULL; idx++)
	{
		cmd_str_len = strlen(cmd_table[idx].cmd_str);
		if(strncmp(cmd_table[idx].cmd_str, buf, cmd_str_len) == 0)
		{
			
		 	if(cmd_table[idx].func == NULL)
		 	{
		 		printf("no func\n");
		 		continue;
		 	}
			parser(id, buf);
			
			len = strlen((char*)self_argv[0]);
			if(len != cmd_str_len)
			{
				// detect postfix
				postfix = (char*)self_argv[0];
				postfix = &postfix[cmd_str_len];
				if(strncmp(postfix, "rm", 2) == 0)
				{
					self_from_addr.sin_port = htons(CMD_SOCKET_BASE-1);
				}
				else if(postfix[0] == 'r')
				{
					if(postfix[1] > '0')
					{
						src_id = postfix[1] - '0';
						if((0 <= src_id) && (src_id < GLOBAL_CH_MAX))
						{
							self_from_addr.sin_port = htons(selfs[src_id].cmd_port);
							printf("command:%s, response:%d\n", (char*)self_argv[0], selfs[src_id].cmd_port);
						}
					}
					else
					{
						self_from_addr.sin_port += htons(1);
					}
				}
			}
			cmd_table[idx].func(id, self_argc, (void**)self_argv);
		}
	}
}

