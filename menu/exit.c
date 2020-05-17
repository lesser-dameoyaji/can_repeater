#include "common.h"
#include "rpt.h"

int menu_exit(unsigned int event)
{
	send_command(0, "exit", 5);
	return -1;
}
