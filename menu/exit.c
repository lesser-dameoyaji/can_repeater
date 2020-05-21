#include "common.h"
#include "command.h"

int menu_exit(int event)
{
	send_command(0, 0, "exit", 5);
	return -1;
}
