#include "common.h"
#include "cansock.h"

void cmd_csock_open(int id)
{
	char buf[63];
	
	if(self_can_handle.fd >= 0)
	{
		printf("can%d socket already created %d\n", id, self_can_handle.fd);
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
	
	self.status = STATUS_RUN_IFOPN;
}

void cmd_csock_close(int id)
{
	char buf[63];
	
	if(self_can_handle.fd < 0)
	{
		printf("can socket have not been created\n");
		return;
	}
	close(self_can_handle.fd);
	self_can_handle.fd = -1;
	self.nfd--;
	
	printf("can%d socket closed\n", id);
	
	if(self.status == STATUS_RUN_IFOPN)
		self.status = STATUS_RUN_IFUP;
}

void cmd_csock_ifup(int id)
{
	char buf[63];
	
	if(self.status == STATUS_RUN)
	{
		sprintf(buf, "sudo ip link set can%d up type can bitrate 500000 restart-ms 100", id);
		if(system(buf) < 0)
		{
			printf("can%d up fail\n", id);
		}
		else
		{
			printf("can%d up success\n", id);
			self.status = STATUS_RUN_IFUP;
		}
	}
}

void cmd_csock_ifdown(int id)
{
	char buf[63];
	
	if(self.status >= STATUS_RUN_IFUP)
	{
		sprintf(buf, "sudo ip link set can%d down", id);
		if(system(buf) < 0)
		{
			printf("can%d down fail\n", id);
		}
		else
		{
			printf("can%d down success\n", id);
			self.status = STATUS_RUN;
		}
	}
}


void cmd_csock(int id, int argc, void** argv)
{
	char* command;
	
	if(argc < 2)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];
	
	if (strcmp("open", command)==0)
	{
		cmd_csock_open(id);
	}
	else if(strcmp("close", command)==0)
	{
		cmd_csock_close(id);
	}
	else if(strcmp("ifup", command)==0)
	{
		cmd_csock_ifup(id);
	}
	else if(strcmp("ifdown", command)==0)
	{
		cmd_csock_ifdown(id);
	}
	
	return;
}

