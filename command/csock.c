#include "common.h"
#include "cansock.h"

void cmd_csock(int id, int argc, void** argv)
{
	char buf[63];
	int fd;
	char* ifname;
	char* command;
	
	if(argc < 2)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];
	
	if (strcmp("open", command)==0)
	{
		if(self_can_handle.fd >= 0)
		{
			printf("can socket already created %d\n", self_can_handle.fd);
			return;
		}
		sprintf(buf, "can%d", id);
		
		// create can socket
		self_can_handle.fd = csock_open(buf);
		if(self_can_handle.fd < 0)
		{
			printf("%s socket create failed\n", buf);
			return;
		}
		printf("%s socket create success\n", buf);
		self_can_handle.events = POLLIN;
		self.nfd++;
	}
	else if(strcmp("close", command)==0)
	{
		if(self_can_handle.fd < 0)
		{
			printf("can socket have not been created\n");
			return;
		}
		close(self_can_handle.fd);
		self_can_handle.fd = -1;
		self.nfd--;
		
		printf("can socket closed\n");
	}
	else if(strcmp("ifup", command)==0)
	{
		sprintf(buf, "sudo ip link set can%d up type can bitrate 500000 restart-ms 100", id);
		if(system(buf) < 0)
		{
			printf("can%d up fail\n", id);
		}
		else
		{
			printf("can%d up success\n", id);
		}
	}
	else if(strcmp("ifdown", command)==0)
	{
		sprintf(buf, "sudo ip link set can%d down", id);
		if(system(buf) < 0)
		{
			printf("can%d down fail\n", id);
		}
		else
		{
			printf("can%d down success\n", id);
		}
	}
	
	return;
}

