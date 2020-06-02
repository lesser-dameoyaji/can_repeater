#include "common.h"
#include "command.h"
#include "cansock.h"

void cmd_interface_up(int id)
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

void cmd_interface_down(int id)
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


void cmd_interface(int id, int argc, void** argv)
{
	char* command;
	
	if(argc < 2)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];
	
	if(strcmp("up", command)==0)
	{
		cmd_interface_up(id);
	}
	else if(strcmp("down", command)==0)
	{
		cmd_interface_down(id);
	}
	else if(strcmp("stat", command)==0)
	{
		char buf[32];
		
		if(self.status >= STATUS_RUN_IFUP)
		{
			sprintf(buf, "interface up");
		}
		else
		{
			sprintf(buf, "interface down");
		}
		
		// send response
		if(is_need_response(id, "interface")==true)
		{
			sprintf(buf, "%s\r", buf);
			send_response(id, buf, strlen(buf));
		}
		else
		{
			printf("%s\n", buf);
		}
	}
	return;
}

