#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "rpt.h"
#include "menu.h"

extern void configure(void);

self_t selfs[GLOBAL_CH_MAX];

void main(void)
{
	int id;
	pthread_t thread_handle;
	// initialize
	id = 0;
	
	menu_init();
	
	// thread start
	thread_handle = thread_start(id);
	if (thread_handle > 0)
	{
		// wait for thread boot
		while(true)
		{
			usleep(100*1000);
			if(self.status >= STATUS_RUN)
				break;
		}
		printf("detect thread %d start\n", id);
		
		// configuration
		configure();
		
		menu_change(0);
		menu();
		
		// wait thread end.
		pthread_join(thread_handle, NULL);
	}
	printf("exit main\n");
}
