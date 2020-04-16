#include <string.h>
#include <errno.h>
#include "common.h"
#include "cansock.h"

void cmd_send(int id, int argc, void** argv)
{
	struct canfd_frame can_frame;
	char data[8], c[3];
	int i, dlc;
	
	if(argc < 3)
	{
		printf("too few argument to send can frame\n");
		return;
	}
	
	dlc = strlen((char*) argv[2]);
	if((dlc == 0) || (dlc > 16) || ((dlc & 0x01) >0))
	{
		printf("wrong length\n");
		return;
	}
	dlc /= 2;
	
	can_frame.can_id = strtol((char*) argv[1], NULL, 16);
	for(i=0; i < dlc ; i++)
	{
		c[0] = ((char*)(argv[2]))[i*2 + 0];
		c[1] = ((char*)(argv[2]))[i*2 + 1];
		c[2] = 0;
		can_frame.data[i] = strtol(c, NULL, 16);
	}
	
	can_frame.len = dlc;
	can_frame.flags = 0;

	if(write(self_can_handle.fd, &can_frame, CAN_MTU) != CAN_MTU)
	{
		printf("write error %d: %x %d", errno, can_frame.can_id, dlc);
		for(i=0; i < dlc ; i++)
		{
			printf(" %x", can_frame.data[i]);
		}
		printf("\n");
		return;
	}

	return ;
}
