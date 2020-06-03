#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "command.h"

#define RESULT_EOF		-1
#define RESULT_FAIL		-2

static int fd = -1;

static int read_line(int fh, char* buf, int max_len)
{
	int idx;
	char c;
	size_t l;
	int ret = 0;
	
	for(idx=0; idx < max_len; idx++)
	{
		l = read(fh, &c, 1);
		/* 末尾だったら終了 */
		if (l == 0) {
			/* 何も読み出せなかったら、戻り値でEOFを通知 */
			if(ret == 0) {
				ret = RESULT_EOF;
			}
			break;
		}
		/* エラーだったら終了 */
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


int config_open(char *fname)
{
	if(fd >= 0)
	{
		printf("already open\n");
		return 0;
	}
	fd = open("./config.ini", O_RDONLY);
	if(fd < 0)
	{
		perror("read open config");
		return -1;
	}
	return 0;
}

int config_write(char *buf, int length)
{
	return write(fd, buf, length);
}

int config_read(char *buf, int buf_size)
{
	return read_line(fd, buf, buf_size);
}

int config_close(void)
{
	close(fd);
}

// menu(main thread)からの呼び出しを想定
// note.
//   将来的にレスポンスが返るようになったらmain thread以外からの呼び出しはNGになる
//   20/5/21現在どのthreadから呼び出しても実質問題なし
void configure(void)
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
	// 一行ずつ読みだして、スレッドへ渡す
	while (1)
	{
		len = read_line(fd, buf, sizeof(buf));
		if(len == 0)
			continue;
		if(len == RESULT_EOF)
		{
			// 終端まできたのでclose
			printf("EOF\n");
			break;
		}
		if(len < 0)
		{
			// エラー なのでclose
			printf("config read error\n");
			break;
		}
		printf("config: %d: %s\n", len, buf);
		send_command(0, 0, buf, strlen(buf));
	}
	close(fd);
}

