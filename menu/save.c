#include <stdio.h>

#include "common.h"
#include "command.h"
#include "menu.h"
#include "lcd.h"

int menu_save_enter(int event)
{
	menu_change(MENU_SAVE);
}

int menu_save(int event)
{
	switch(event)
	{
	case ENTER:
		lcd_clear();
		lcd_cursor(0, 0, false);
		lcd_printf("saving...");
		lcd_flush();
		send_command(0, 0, "saverm", strlen("saverm"));
		break;
	case RECEIVE:
		menu_timer_start(0, 50, false);
		if(strncmp("OK", menu_socket_buf, 2)==0)
		{
			lcd_cursor(0, 0, false);
			lcd_printf("complete");
			lcd_flush();
		}
		break;
	case TIMEOUT0:
		menu_change(MENU_MAIN_LIST);
		break;
	}
}
