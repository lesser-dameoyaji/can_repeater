#include <stdio.h>
#include <string.h>

#include "rpt.h"
#include "common.h"

void cmd_thread(int id, int argc, void** argv)
{
	char* command;
	
	if(argc < 2)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];

	if(strcmp("create", command)==0)
	{
		// create child thread
		if(self.child_thread_handle == -1)
		{
			// yet
			self.child_thread_handle = thread_start(id+1);
			if(self.child_thread_handle == -1)
			{
				printf("faile to create child thread%d\n", id+1);
			}
			return;
		}
		else
		{
			// 
			printf("thread%d has child thread\n", id);
		}
	}
	else if(strcmp("status", command)==0)
	{
		char buf[32];

		if(self.status >= STATUS_RUN)
		{
			sprintf(buf, "thread create");
		}
		else
		{
			sprintf(buf, "");
		}
		printf("%s\n", buf);
		
		if((argc > 3) && (strcmp("save", (char*)argv[2])==0))
		{
			config_write(buf, strlen(buf));
		}
	}

}

