#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "command.h"
#include "rpt.h"

void cmd_thread(int id, int argc, void** argv)
{
	char* command;
	
	if(argc < 2)
	{
		printf("too few argument\n");
		return;
	}
	command = (char*)argv[1];

	printf("%d:%s\n", id, command);

	if(strcmp("create", command)==0)
	{
		pthread_t thread_handle;
		
		if(self.status < STATUS_RUN)
		{
			// thread start
			thread_handle = thread_start(id);
		
			if (thread_handle < 0)
			{
				printf("faile to create self thread %d\n", id);
				return;
			}
			// wait for thread boot
			while(true)
			{
				usleep(100*1000);
				if(self.status >= STATUS_RUN)
					break;
			}
			printf("detect thread %d start\n", id);
		}
		// create child thread
		else if(self.child_thread_handle == -1)
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
	else if(strcmp("stat", command)==0)
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
		
		// send response
		if(is_need_response(id, "thread")==true)
		{
			sprintf(buf, "%s\r", buf);
			send_response(id, buf, strlen(buf));
		}
		else
		{
			printf("%s\n", buf);
		}
		
	}

}

