#include <stdio.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"

unsigned int menu_title(unsigned int event)
{
	if(event == ENTER)
	{
		printf("enter title\n");
		lcd_cursor(0, 0, false);
		lcd_printf("can repeater2");
		lcd_cursor(0, 1, false);
		lcd_printf(" %s", VERSION_STRING);
		
		menu_timer_start(0, 50, false);
	}
	if(event == TIMEOUT0)
	{
		printf("TIMEOUT0\n");
	}
	return 0;
}
