#include <stdio.h>
#include <string.h>

#include "common.h"
#include "command.h"
#include "menu.h"
#include "lcd.h"

int menu_load_enter(int event)
{
	menu_change(MENU_LOAD);
}

int menu_load(int event)
{
	switch(event)
	{
	case ENTER:
		lcd_clear();
		lcd_cursor(0, 0, false);
		lcd_printf("loading...");
		lcd_flush();
		send_command(0, 0, "resetrm", strlen("resetrm"));
		break;
	case RECEIVE:
		menu_timer_start(0, 50, false);
		if(strncmp("reset complete", menu_socket_buf, 14)==0)
		{
			lcd_cursor(0, 0, false);
			lcd_printf("reconfig");
			lcd_flush();
			configure();
		}
		break;
	case TIMEOUT0:
		menu_change(MENU_MAIN_LIST);
		break;
	}
}
