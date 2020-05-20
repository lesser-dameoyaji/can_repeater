#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "rpt.h"
#include "menu.h"

#define RESULT_EOF		-1
#define RESULT_FAIL		-2

self_t selfs[GLOBAL_CH_MAX];

static int read_line(int fh, char* buf, int max_len)
{
	int idx;
	char c;
	size_t l;
	int ret = 0;
	
	for(idx=0; idx < max_len; idx++)
	{
		l = read(fh, &c, 1);
		/* ������������I�� */
		if (l == 0) {
			/* �����ǂݏo���Ȃ�������A�߂�l��EOF��ʒm */
			if(ret == 0) {
				ret = RESULT_EOF;
			}
			break;
		}
		/* �G���[��������I�� */
		if (l < 0) {
			ret = RESULT_FAIL;
			break;
		}
		if (c == '\r') {
			break;
		}
		if (c == '\n') {
			break;
		}
		buf[idx] = c;
		ret++;
	}
	buf[idx] = '\0';
	return ret;
}

static void configure(void)
{
	int fd;
	int len;
	char buf[256];
	
	fd = open("./config.ini", O_RDONLY);
	if(fd < 0)
	{
		perror("read open config");
		return ;
	}
	// ��s���ǂ݂����āA�X���b�h�֓n��
	while (1)
	{
		len = read_line(fd, buf, sizeof(buf));
		if(len == 0)
			continue;
		if(len == RESULT_EOF)
		{
			// �I�[�܂ł����̂�close
			printf("EOF\n");
			break;
		}
		if(len < 0)
		{
			// �G���[ �Ȃ̂�close
			printf("config read error\n");
			break;
		}
		printf("config: %d: %s\n", len, buf);
		send_command(0, 0, buf, strlen(buf));
	}
	close(fd);
}


void main(void)
{
	int id;
	pthread_t thread_handle;
	// initialize
	id = 0;
	
	menu_init();
	
	// thread start
	thread_handle = thread_start(id);
	if (thread_handle > 0)
	{
		// wait for thread boot
		while(true)
		{
			usleep(100*1000);
			if(self.status >= STATUS_RUN)
				break;
		}
		printf("detect thread %d start\n", id);
		
		// configuration
		configure();
		
		menu_change(0);
		while(true)
		{
			usleep(MENU_CYCLIC*1000);
			if(menu() < 0)
				break;
		}
		
		// wait thread end.
		pthread_join(thread_handle, NULL);
	}
	printf("exit main\n");
}
