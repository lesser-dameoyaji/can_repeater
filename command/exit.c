#include "common.h"
#include "rpt.h"


// thread exit command
void cmd_exit(int id, int argc, void** argv)
{
	// exit all thread
	if(self.child_thread_handle != -1)
	{
		send_command(id+1, "exit", 5);
		pthread_join(self.child_thread_handle, NULL);
		
	}
	self.exit_request++;
	return;
}

