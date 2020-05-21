#include <stdio.h>
#include <string.h>

#include "common.h"
#include "command.h"

int menu_load(int event)
{
	printf("load\n");
	send_command(0, 0, "resetrm", strlen("resetrm"));
}
