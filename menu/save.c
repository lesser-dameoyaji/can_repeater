#include <stdio.h>

#include "common.h"
#include "command.h"
#include "menu.h"
#include "lcd.h"

static int global_index;
static int global_ch;

static int menu_save_core(void)
{
	char buf[32];
	
	for(; global_ch<GLOBAL_CH_MAX; global_ch++, global_index=0)
	{
		for(; global_index<get_command_table_size(); global_index++)
		{
			if(is_need_save_command(global_index) == false)
			{
				continue;
			}
			sprintf(buf, "%srm stat all", get_command_name(global_index));
			send_command(0, global_ch, buf, strlen(buf));
			printf("save %d:%s\n", global_ch, buf);
			
			return 1;		// wait response
		}
	}
	return 0;				// complete
}



int menu_save_enter(int event)
{
	menu_change(MENU_SAVE);
}

int menu_save(int event)
{
	char buf[32];
	int ret = -1;
	
	switch(event)
	{
	case ENTER:
		lcd_clear();
		lcd_cursor(0, 0, false);
		lcd_printf("saving...");
		lcd_flush();
		
		global_index = 0;
		global_ch = 0;
		ret = 0;
		// none break
	case RECEIVE:
		if(ret < 0)
		{
			sprintf(buf, "%s", get_command_name(global_index));
			ret = strncmp(buf, menu_socket_buf, strlen(buf));
			global_index++;
		}
		if(ret == 0)
		{
			ret = menu_save_core();
			if(ret > 0)
			{
				// wait response
			}
			else if(ret == 0)
			{
				// complete
				lcd_cursor(0, 0, false);
				lcd_printf("complete");
				lcd_flush();
				menu_timer_start(0, 50, false);
			}
			else
			{
				// error
				lcd_cursor(0, 0, false);
				lcd_printf("error");
				lcd_flush();
				menu_timer_start(0, 50, false);
			}
		}
		else
		{
			// error
			lcd_cursor(0, 0, false);
			lcd_printf("error");
			lcd_flush();
			menu_timer_start(0, 50, false);
		}
		break;
	case TIMEOUT0:
		menu_change(MENU_MAIN_LIST);
		break;
	}
}
