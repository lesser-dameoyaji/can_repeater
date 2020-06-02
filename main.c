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
	// initialize
	id = 0;
	
	menu_init();
	
	// configuration
	configure();
	
	menu_change(0);
	menu();
	
	// wait thread end.
	pthread_join(self.thread_handle, NULL);
	printf("exit main\n");
}
