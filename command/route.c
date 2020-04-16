#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

void cmd_route(int id, int argc, void** argv)
{
	char *command;
	int i;
	unsigned int can_id;
	unsigned int id_mask;
	int dst_id;
	
	if(argc < 2)
	{
		printf("no command\n");
		return;
	}
	
		command = (char*)argv[1];
	
	if (strcmp("set", command)==0)
	{
		if(argc < 5)
		{
			printf("too few argument to set routing table\n");
			return;
		}
		
		can_id = strtol((char*)argv[2], NULL, 16);
		id_mask = strtol((char*)argv[3], NULL, 16);
		dst_id = strtol((char*)argv[4], NULL, 16);

		if(can_id > CAN_ID_MAX)
		{
			printf("invalid id\n");
			return;
		}
		for(i=1; i<=self.routing_table_count; i++)
		{
			if(self.routing_table[i].id == can_id)
				break;
		}
		if(i > self.routing_table_count)
			self.routing_table_count = i;
			
		self.routing_table[i].id = can_id;
		self.routing_table[i].id_mask = id_mask;
		self.routing_table[i].dst_id = dst_id;
	}
	else if(strcmp("del", command)==0)
	{
		if(argc < 3)
		{
			printf("too few argument to delete routing table\n");
			return;
		}
		
		can_id = strtol((char*)argv[2], NULL, 16);
		if(can_id > CAN_ID_MAX)
		{
			printf("invalid id\n");
			return;
		}
		for(i=1; i<=self.routing_table_count; i++)
		{
			if(self.routing_table[i].id == can_id)
				break;
		}
		if(i > self.routing_table_count)
		{
			printf("invalid id %d\n", i);
			return;
		}
		
		self.routing_table[i].id = CAN_ID_INVALID;
		self.routing_table[i].dst_id = -1;
		
		// garbage collection
		for(can_id=i+1; can_id < self.routing_table_count; i++, can_id++)
		{
			self.routing_table[i] = self.routing_table[can_id];
		}
		// update count
		for(i=ROUTING_TABLE_SIZE; i >= 0 ;i--)
		{
			if(self.routing_table[i].id != CAN_ID_INVALID)
			{
				self.routing_table_count = i;
				break;
			}
		}
	}
	else if(strcmp("enable", command)==0)
	{
		
	}
	else if(strcmp("disable", command)==0)
	{
	}
	else if(strcmp("stat", command)==0)
	{
	}
}
