#include "cansock.h"
#include <errno.h>

int csock_open(char* ifname)
{
	struct sockaddr_can can_addr;
	struct ifreq ifr;
	int fd, loopback;

	fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(fd < 0)
	{
		printf("socket");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, strlen(ifname));
	ifr.ifr_name[strlen(ifname)] = 0;
	ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
	if(ifr.ifr_ifindex == 0)
	{
		printf("if_nametoindex error %d\n", errno);
		return -1;
	}
	
	memset(&can_addr, 0, sizeof(can_addr));
	can_addr.can_family = AF_CAN;
	can_addr.can_ifindex= ifr.ifr_ifindex;
	
	// disable loopback
	loopback = 0;					/* 0 = disabled, 1 = enabled(default) */
    setsockopt(fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
    
	if(bind(fd, (struct sockaddr *)&can_addr, sizeof(can_addr)) < 0)
	{
		printf("bind failed");
		return  -1;
	}
	return fd;
	
}

int csock_send(int csock_fd, canid_t canid, unsigned char* data, int data_len)
{
	struct canfd_frame can_frame;
	int l;
	
	can_frame.can_id = canid;
	can_frame.len = data_len;
	can_frame.flags = 0;
	memcpy(&can_frame.data[0], data, data_len);

	if((l=write(csock_fd, &can_frame, CAN_MTU)) != CAN_MTU)
	{
		printf("write %d\n", l);
		return -1;
	}

	return data_len;
}

int csock_recv(int csock_fd, struct can_frame* frame)
{
	int len;

	len = read(csock_fd, frame, sizeof(struct can_frame));
	if(len < 0)
	{
		printf("recv");
		return -1;
	}

	return len;
}

void csock_close(int fd)
{
	close(fd);
}

