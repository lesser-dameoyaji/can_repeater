#include <stdio.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"

int menu_title(int event)
{
	switch(event)
	{
	case ENTER:
		printf("enter title\n");
		lcd_cursor(0, 0, false);
		lcd_printf("can repeater2");
		lcd_cursor(0, 1, false);
		lcd_printf(" %s", VERSION_STRING);
		
		menu_timer_start(0, 100, false);
		break;
	case TIMEOUT0:
		printf("TIMEOUT0\n");
		menu_change(MENU_COUNTER);
		break;
	}
	return 0;
}
