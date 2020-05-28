#include <stdio.h>

#include "common.h"
#include "command.h"
#include "rpt.h"

extern void cmd_csock_ifdown(int id);
extern void cmd_csock_close(int id);

// thread reset command
void cmd_reset(int id, int argc, void** argv)
{
	// exit all thread
	if(self.child_thread_handle != -1)
	{
		send_command(id, id+1, "exit", 5);
		pthread_join(self.child_thread_handle, NULL);
		
	}
	if(self.status == STATUS_RUN_IFOPN)
	{
		cmd_csock_close(id);
		self.status = STATUS_RUN_IFUP;
	}
	if(self.status == STATUS_RUN_IFUP)
	{
		cmd_csock_ifdown(id);
		self.status = STATUS_RUN;
	}
	
	// send response
	if(is_need_response(id, "reset") == true)
	{
		send_response(id, "reset complete", 2);
	}
	return;
}

