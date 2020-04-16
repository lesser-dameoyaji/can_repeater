#include <stdio.h>

#include "common.h"
#include "rpt.h"

self_t selfs[2];

void main(void)
{
	int id;
	pthread_t thread_handle;
	// initialize
	id = 0;
	
	// thread start
	thread_handle = thread_start(id);
	if (thread_handle > 0)
	{
		printf("waiting\n");
		pthread_join(thread_handle, NULL);
	}
	printf("exit main\n");
}
