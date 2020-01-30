#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <poll.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "cansock.h"

#include "descriptors.h"
#include "uds.h"
#include "menu.h"
#include "filter.h"

#define DIR_CAN_RX	0
#define DIR_CAN_TX	1

// thread
thread_descriptor_t thread_desc[4];

int start_thread(thread_descriptor_t* thread_desc);

static void* canrep_thread(void* arg);
static int svr_handler(void* arg);
static int cs_handler(void* arg);
static int cr_handler(void* arg);

// etc
frame_descriptor_t frame_desc[4];
framefilter_que_t framefilter_que[2];
int exit_process_request = 0;

int configure(char** argv)
{
	int ch, i;
	int index;
	bool onoff;
	char buf[8];
	
	// check param name
	if(argv[0] == NULL)
		return -1;
	if(strncmp(argv[0], "frame", 5) == 0)
	{
		index = atoi(&argv[0][5]);
		if((0>index) || (index>1))
			return 2;
		// frame ch
		frame_desc[index].ch = atoi(&argv[1][0]);
		
		// frame ID
		sprintf(buf, "0x%s", argv[2]);
	 	frame_desc[index].frame.can_id	= strtol(buf, NULL, 16); 
		
		// frame dlc
		frame_desc[index].frame.can_dlc	= atoi(&argv[3][0]);
		
		// frame data
		for(i=0; i<8; i++)
		{
			sprintf(buf, "0x%c%c", argv[4][i*2],  argv[4][i*2+1]);
			frame_desc[index].frame.data[i] = strtol(buf, NULL, 16);
		}
	}
	if(strncmp(argv[0], "bridge", 6) == 0)
	{
		ch = atoi(&argv[0][6]);
		if(ch == 0)
		{
			printf("CH0:");
			ch = CH0;
		}
		else if(ch == 1)
		{
			printf("CH1:");
			ch = CH1;
		}
		else
		{
			return 2;
		}
		
		if(strncmp(argv[1], "ON", 2) == 0)
		{
			printf("ON\n");
			onoff = true;
		}
		else if(strncmp(argv[1], "OFF", 3) == 0)
		{
			printf("OFF\n");
			onoff = false;
		}
		else
		{
			printf("null\n");
			return 2;
		}
		
		thread_desc[ch].bridge_enable = onoff;
	}
}

int main(int argc, char ** argv)
{
	int i;

	if(system("sudo ip link set can0 up type can bitrate 500000")<0)
	{
		printf("can0 up fail\n");
		return -1;
	}
	if(system("sudo ip link set can1 up type can bitrate 500000")<0)
	{
		printf("can1 up fail\n");
		return -1;
	}

	for(i=0; i<4; i++)
	{
		thread_desc[CH0].handlers[i] = NULL;
		thread_desc[CH1].handlers[i] = NULL;
	}

	menu_init();
	
	//read_config(CONFIG_FILE_NAME);
	// thread configuration
	thread_desc[CH0].id				= CH0;
	thread_desc[CH0].svr_fd			= -1;
	thread_desc[CH0].cli_fd			= -1;
	thread_desc[CH0].can_tx_fd		= -1;
	thread_desc[CH0].can_rx_fd		= -1;
	thread_desc[CH0].fds_num		= 0;
	thread_desc[CH0].bridge_thread	= 1;
	strcpy(thread_desc[CH0].can_rx_name, "can0");
	strcpy(thread_desc[CH0].can_tx_name, "can1");
	strcpy(thread_desc[CH0].uds_path, "/tmp/can_repeater_thread0");
	thread_desc[CH0].framefilter_que = &framefilter_que[0];
	thread_desc[CH0].bridge_enable_default = false;
	
	thread_desc[CH1].id				= CH1;
	thread_desc[CH1].svr_fd			= -1;
	thread_desc[CH1].cli_fd			= -1;
	thread_desc[CH1].can_tx_fd		= -1;
	thread_desc[CH1].can_rx_fd		= -1;
	thread_desc[CH1].fds_num		= 0;
	thread_desc[CH1].bridge_thread	= 1;
	strcpy(thread_desc[CH1].can_rx_name, "can1");
	strcpy(thread_desc[CH1].can_tx_name, "can0");
	strcpy(thread_desc[CH1].uds_path, "/tmp/can_repeater_thread1");
	thread_desc[CH1].framefilter_que = &framefilter_que[1];
	thread_desc[CH1].bridge_enable_default = false;

	start_thread(&thread_desc[CH0]);
	start_thread(&thread_desc[CH1]);
#if 0
	thread_desc[TX0].id			= 0;
	thread_desc[TX0].dir		= DIR_CAN_TX;
	thread_desc[TX0].svr_fd		= -1;
	thread_desc[TX0].can_fd		= -1;
	thread_desc[TX0].fds_num	= 0;
	thread_desc[TX0].bridge_thread	= -1;
	strcpy(thread_desc[TX0].can_name, "can0");
	strcpy(thread_desc[TX0].uds_path, "/tmp/can_repeater_threadTX0");
	thread_desc[TX0].bridge_enable_default = false;
	thread_desc[TX0].framefilter_que = &framefilter_que[0];

	thread_desc[RX0].id			= 1;
	thread_desc[RX0].dir		= DIR_CAN_RX;
	thread_desc[RX0].svr_fd		= -1;
	thread_desc[RX0].can_fd		= -1;
	thread_desc[RX0].fds_num	= 0;
	thread_desc[RX0].bridge_thread	= TX1;
	strcpy(thread_desc[RX0].can_name, "can0");
	strcpy(thread_desc[RX0].uds_path, "/tmp/can_repeater_threadRX0");
	thread_desc[RX0].bridge_enable_default = false;
	thread_desc[RX0].framefilter_que = &framefilter_que[0];

	thread_desc[TX1].id			= 2;
	thread_desc[TX1].dir		= DIR_CAN_TX;
	thread_desc[TX1].svr_fd		= -1;
	thread_desc[TX1].can_fd		= -1;
	thread_desc[TX1].fds_num	= 0;
	thread_desc[TX1].bridge_thread	= -1;
	strcpy(thread_desc[TX1].can_name, "can1");
	strcpy(thread_desc[TX1].uds_path, "/tmp/can_repeater_threadTX1");
	thread_desc[TX1].bridge_enable_default = false;
	thread_desc[TX1].framefilter_que = &framefilter_que[1];

	thread_desc[RX1].id			= 3;
	thread_desc[RX1].dir		= DIR_CAN_RX;
	thread_desc[RX1].svr_fd		= -1;
	thread_desc[RX1].can_fd		= -1;
	thread_desc[RX1].fds_num	= 0;
	thread_desc[RX1].bridge_thread	= TX0;
	strcpy(thread_desc[RX1].can_name, "can1");
	strcpy(thread_desc[RX1].uds_path, "/tmp/can_repeater_threadRX1");
	thread_desc[RX1].bridge_enable_default = false;
	thread_desc[RX1].framefilter_que = &framefilter_que[1];
	
	start_thread(&thread_desc[TX0]);
	start_thread(&thread_desc[TX1]);
	start_thread(&thread_desc[RX0]);
	start_thread(&thread_desc[RX1]);
#endif

	// main loop
	while(exit_process_request == 0)
	{
		menu();

		// 50msec interval
		usleep(50000);
	}
	// wait thread stop
	pthread_join(thread_desc[0].thread_handle, NULL);
	pthread_join(thread_desc[1].thread_handle, NULL);
	pthread_join(thread_desc[2].thread_handle, NULL);
	pthread_join(thread_desc[3].thread_handle, NULL);
	return 0;
}

// threads
int start_thread(thread_descriptor_t* thread_desc)
{
	int err;
	pthread_attr_t thread_attr;


	err = pthread_attr_init(&thread_attr);
	err = pthread_create(&thread_desc->thread_handle
			, &thread_attr, canrep_thread, (void*)thread_desc);
	
	if(err < 0)
	{
		printf("thread start");
		return -1;
	}
}

static void* canrep_thread(void* arg)
{
	int i, ret, len, loop = 0;

	// thread argument
	thread_descriptor_t* tdesc = (thread_descriptor_t*)arg;
	
	
	//
	// UNIX DOMAIN socket open
	//
	tdesc->svr_fd = uds_open_svr(tdesc->uds_path);
	if(tdesc->svr_fd < 0)
	{
		printf("socket open error %d:%d\n", tdesc->id, tdesc->svr_fd);
		return NULL;
	}
	// polling setup
	tdesc->fds[tdesc->fds_num].fd		= tdesc->svr_fd;
	tdesc->fds[tdesc->fds_num].events	= POLLIN;
	tdesc->handlers[tdesc->fds_num]		= svr_handler;
	tdesc->fds_num++;

	// 
	tdesc->bridge_enable = tdesc->bridge_enable_default;
	framefilter_init(tdesc->framefilter_que);
	
	//
	// CAN RX socket open
	//
	tdesc->can_rx_fd = csock_open(tdesc->can_rx_name);
	if(tdesc->can_rx_fd < 0)
	{
		printf("can rx socket open error %d:%d\n", tdesc->id, tdesc->can_rx_fd);
		return NULL;
	}
		
	// polling setup
	tdesc->fds[tdesc->fds_num].fd		= tdesc->can_rx_fd;
	tdesc->fds[tdesc->fds_num].events	= POLLIN;
	tdesc->handlers[tdesc->fds_num]		= cr_handler;
	tdesc->fds_num++;
	
	//
	// CAN TX socket open
	//
	tdesc->can_tx_fd = csock_open(tdesc->can_tx_name);
	if(tdesc->can_tx_fd < 0)
	{
		printf("can tx socket open error %d:%d\n", tdesc->id, tdesc->can_tx_fd);
		return NULL;
	}
	
	if(tdesc->bridge_thread < 0)
	{
		tdesc->brg_fd = NULL;
	}
	else
	{
		tdesc->brg_fd = &(thread_desc[tdesc->bridge_thread].cli_fd);
	}
	

	printf("start thread:%d\n", tdesc->id);
	// main loop
	while(exit_process_request == 0)
	{
		ret = poll(tdesc->fds, tdesc->fds_num, 500);
		if(ret > 0)
		{
			for(i=0; i < tdesc->fds_num; i++)
			{
				if(tdesc->fds[i].revents & POLLIN)
				{
					loop = tdesc->handlers[i]((void*)tdesc);
				}
			}
		}
       	else if(ret == 0)
		{
			// timeout
			if(tdesc->cli_fd < 0)
			{
				tdesc->cli_fd = uds_open_cli(tdesc->uds_path);
			}
		}
		else
		{
			// error
		}
	}
	uds_close(tdesc->svr_fd);
}

// 
//
//
static int svr_handler(void* arg)
{
	thread_descriptor_t* desc = (thread_descriptor_t*)arg;
	struct sockaddr_un client_addr;
	socklen_t client_addr_len;

	memset(&client_addr, 0, sizeof(struct sockaddr_un));
	client_addr_len = sizeof(struct sockaddr_un);
	desc->acptd_fd = -1;

	desc->acptd_fd = accept(desc->svr_fd, (struct sockaddr*)&client_addr, &client_addr_len);
	if(desc->acptd_fd < 0)
	{
		printf("accept error %d\n", thread_desc->id);
		return -1;
	}
	printf("accepted %d\n", desc->id);
	desc->fds[desc->fds_num].fd = desc->acptd_fd;
	desc->fds[desc->fds_num].events = POLLIN;
	desc->handlers[desc->fds_num] = cs_handler;
	desc->fds_num++;
	
	return 0;
}

static int cs_handler(void* arg)
{
	struct can_frame frame;

	thread_descriptor_t* desc = (thread_descriptor_t*)arg;
	int len, i;

	// client socket event
	len = read(desc->acptd_fd, &frame, sizeof(struct can_frame));
	if(len <= 0)
	{
		return 0;
	}
	
	if(frame.can_id < 0x800)
	{
		// frame receive
		printf("cs_handler: send %d:%04x:%02x:", desc->id, frame.can_id, frame.can_dlc);
		for(i=0; i<frame.can_dlc; i++)
		{
			printf("%02x ", frame.data[i]);
		}
		printf("\n");
		
		framefilter_add(desc->framefilter_que, &frame);

		len = csock_send(desc->can_tx_fd, frame.can_id, frame.data, (int)frame.can_dlc);
		if(len < 0)
		{
			printf(" canrep_send fail\n");
		}
		else
		{
			if(desc->can_tx_count < 10000)
				desc->can_tx_count++;
			else
				desc->can_tx_count = 0;
		}
	}
	
	return 0;
}

static int cr_handler(void* arg)
{
	thread_descriptor_t* desc = (thread_descriptor_t*)arg;
	int ret, i, j;
	struct can_frame frame;

	ret = csock_recv(desc->can_rx_fd, &frame);
	
	printf("cr_handler: recv %d:%04x:%02x:", desc->id, frame.can_id, frame.can_dlc);
	for(i=0; i<frame.can_dlc; i++)
	{
		printf("%02x ", frame.data[i]);
	}

	if(is_framefilter_block(desc->framefilter_que, &frame) == true)
	{
		printf(":filtered\n");
		return 0;
	}
	
	if(desc->can_rx_count < 10000)
		desc->can_rx_count++;
	else
		desc->can_rx_count = 0;


	if(desc->bridge_enable == true)
	{
		send(*desc->brg_fd, &frame, sizeof(struct can_frame), 0);
	}
	else
	{
		printf(":drop");
	}

	printf("\n");
	return 0;
}

