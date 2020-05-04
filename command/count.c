#include <stdio.h>
#include <string.h>
#include "common.h"

void cmd_count_reset_txrx(int id)
{
	// todo. ‘¼ƒ{[ƒh‚Ö‚Ì“]‘—
	self.tx_count = 0;
	self.rx_count = 0;
}

void cmd_count_reset_tx(int id)
{
	self.tx_count = 0;
}
void cmd_count_reset_rx(int id)
{
	self.rx_count = 0;
}

void cmd_count(int id, int argc, void** argv)
{
	int i;
	char* command;
	
	if(argc < 3)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];
	if (strcmp("reset", command)==0)
	{
		if(strcmp("txrx", (char*)argv[2]) == 0)
		{
			cmd_count_reset_txrx(id);
		}
		else if(strcmp("tx", (char*)argv[2]) == 0)
		{
			cmd_count_reset_tx(id);
		}
		else if(strcmp("rx", (char*)argv[2]) == 0)
		{
			cmd_count_reset_rx(id);
		}
		else if(strcmp("all", (char*)argv[2]) == 0)
		{
			for(i=0; i<GLOBAL_CH_MAX; i++)
				cmd_count_reset_txrx(i);
		}
	}

}
