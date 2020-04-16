#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>

#include "common.h"
#include "command.h"

#include "cansock.h"
#include <arpa/inet.h>

// runtime
#define BUF_SIZE	256			// テンポラリバッファサイズ

// 
static void* thread_main(void* arg);

//
// SOCKET
//
static int open_server_socket(int port)
{
	int fd;
	struct sockaddr_in addr;
	
	// create command socket
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		printf("create server socket error\n");
		return fd;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	
	bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	
	return fd;
}

static int open_client_socket(void)
{
	int fd;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		printf("create server socket error\n");
	}
	return fd;
}

int send_command(int id, char* data, int len)
{
	if(sendto(self_cli_handle, data, len, 0, (struct sockaddr *)&self.server_addr, sizeof(self.server_addr)) < 0)
	{
		return -1;
	}
	return 0;
}

int send_frame(int dst_id, canid_t can_id, unsigned char dlc, unsigned char* data)
{
	char cmd_buf[32];
	
	switch(dlc)
	{
	case 1:
		sprintf(cmd_buf, "send %03x %02x", can_id, data[0]);
		break;
	case 2:
		sprintf(cmd_buf, "send %03x %02x%02x", can_id, data[0], data[1]);
		break;
	case 3:
		sprintf(cmd_buf, "send %03x %02x%02x%02x", can_id, data[0], data[1], data[2]);
		break;
	case 4:
		sprintf(cmd_buf, "send %03x %02x%02x%02x%02x", can_id, data[0], data[1], data[2], data[3]);
		break;
	case 5:
		sprintf(cmd_buf, "send %03x %02x%02x%02x%02x%02x", can_id, data[0], data[1], data[2], data[3], data[4]);
		break;
	case 6:
		sprintf(cmd_buf, "send %03x %02x%02x%02x%02x%02x%02x", can_id, data[0], data[1], data[2], data[3], data[4], data[5]);
		break;
	case 7:
		sprintf(cmd_buf, "send %03x %02x%02x%02x%02x%02x%02x%02x", can_id, data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
		break;
	case 8:
		sprintf(cmd_buf, "send %03x %02x%02x%02x%02x%02x%02x%02x%02x", can_id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		break;
	default:
		return -1;
	}
	if(send_command(dst_id, cmd_buf, strlen(cmd_buf)) < 0)
	{
		return -1;
	}
	return dlc;
}

//
// thread
//
int thread_start(int id)
{
	int err;
	pthread_attr_t thread_attr;
	pthread_t thread_handle;
	
	self.id = id;
	self.cmd_port = CMD_SOCKET_BASE + id;
	self.exit_request = 0;
	
	err = pthread_attr_init(&thread_attr);
	err = pthread_create(&thread_handle, &thread_attr, thread_main, (void*)id);
	
	if(err < 0)
	{
		printf("thread start error");
		return -1;
	}
	
	return thread_handle;
}

static void* thread_main(void* arg)
{
	int id = (int)arg;
	struct sockaddr_in from;
	int ret, l, from_len, i;
	char buf[BUF_SIZE];
	struct can_frame frame;
	
	// initialize
	self.child_thread_handle = -1;
	self_can_handle.fd = -1;
	
	// create command server socket
	self_svr_handle.fd = open_server_socket(self.cmd_port);
	self_svr_handle.events = POLLIN;
	if (self_svr_handle.fd < 0)
	{
		printf("create socket error\n");
		return (void*)self_svr_handle.fd;
	}
	self.nfd = 1;
	
	// create command client socket
	self_cli_handle = open_client_socket();
	self.server_addr.sin_family = AF_INET;
	self.server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	self.server_addr.sin_port = htons(self.cmd_port);
	
	// routing table initialize
	for(i=0; i < ROUTING_TABLE_SIZE; i++)
	{
		self.routing_table[i].id = CAN_ID_INVALID;
		self.routing_table[i].dst_id = -1;
	}
	self.routing_table[0].id = CAN_ID_MAX;
	self.routing_table[0].id_mask = 0;
	if((id + 1) < THREAD_ID_MAX)
		self.routing_table[0].dst_id = id + 1;
	else
		self.routing_table[0].dst_id = 0;
	self.routing_table_count = 1;
	
	printf("thread%d enter loop..\n", id);
	while(self.exit_request==0)
	{
		ret = poll(self.server_handles, self.nfd, 500);
		if(ret > 0)
		{
			if(self_svr_handle.revents & POLLIN)
			{
				// command received
				from_len = sizeof(from);
				l = recvfrom(self_svr_handle.fd, buf, BUF_SIZE, 0, (struct sockaddr *)&from, &from_len);
				buf[l] = 0;
/*				if(l < 0)
				{
					printf("%d:%d:errno=%s:%s\n", id, l, strerror(errno), buf);
				}
*/				commander(id, buf);
			}
			if((self.nfd > 1) && (self_can_handle.revents & POLLIN))
			{
				// can frame  received
				l = csock_recv(self_can_handle.fd, &frame);
				
				printf("%d recv %04x:%02x:", id,  frame.can_id, frame.can_dlc);
				for(i=0; i<frame.can_dlc; i++)
				{
					printf("%02x ", frame.data[i]);
				}
				printf("\n");
				
				// テーブル末尾から検索 (先頭にはデフォルトルートがある)
				for(i=self.routing_table_count; i >= 0;i--)
				{
					// can_id が無効の場合、テーブル無視
					if(self.routing_table[i].id > CAN_ID_MAX)
						continue;
						
					if( (frame.can_id & self.routing_table[i].id_mask) == (self.routing_table[i].id & self.routing_table[i].id_mask) )
					{
						// 宛先無効の場合、受信フレームをドロップ
						if( (self.routing_table[i].dst_id < 0) || (self.routing_table[i].dst_id >= THREAD_ID_MAX) )
							printf("%d drop\n", id);
						else
						{
							if(send_frame(self.routing_table[i].dst_id, frame.can_id, frame.can_dlc, &frame.data[0]) < 0)
								printf("send frame failed\n");
						}
					}
				}
				
				
			}
		}
		else if (ret==0)
		{
			// timeout
		}
		else
		{
			// error
		}
	}
	
	if(self_svr_handle.fd > 0)
		close(self_svr_handle.fd);
	printf("thread%d exit\n", id);
}

