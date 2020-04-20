#include <stdio.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"

int menu_title(int event)
{
	static int count = 0;

	count++;
	if(count == 1)
	{
		lcd_cursor(0, 0, false);
		lcd_printf("can repeater");
		lcd_cursor(0, 1, false);
		lcd_printf(" %s", VERSION_STRING);
	}
	if((event > 0) || (count == 50))
	{
//		update++;
		return 1;
	}
	return 0;
}
