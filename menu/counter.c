#include <stdio.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"

int menu_counter(int event)
{
	int id, update=0;
	static int tx_count[LOCAL_CH_MAX], rx_count[LOCAL_CH_MAX];
	
	switch(event)
	{
	case ENTER:
		for(id=0; id < LOCAL_CH_MAX; id++)
		{
			lcd_cursor(0, id, false);
			lcd_printf("%d TX%4d RX%4d", id, self.tx_count, self.rx_count);
			tx_count[id] = self.tx_count;
			rx_count[id] = self.rx_count;
		}
		
		lcd_flush();
		menu_timer_start(0, 5, true);
		break;
	case TIMEOUT0:
		for(id=0; id < LOCAL_CH_MAX; id++)
		{
			// XV‚µ‚Ä‚È‚©‚Á‚½‚ç‰½‚à‚µ‚È‚¢
			if((tx_count[id] == self.tx_count) && (rx_count[id] == self.rx_count))
				continue;
			lcd_cursor(0, id, false);
			lcd_printf("%d TX%4d RX%4d", id, self.tx_count, self.rx_count);
			tx_count[id] = self.tx_count;
			rx_count[id] = self.rx_count;
			update++;
		}
		if(update > 0)
			lcd_flush();
		break;
	case KEY_D:
		menu_change(MENU_MAIN_LIST);
		break;
	case KEY_U:
	case KEY_R:
	case KEY_L:
		printf("KEY%d\n", event);
		break;
	case KEY_LP_U:
	case KEY_LP_D:
	case KEY_LP_R:
	case KEY_LP_L:
		printf("KEY%d LONG\n", event);
		break;
	
	}
	return 0;
}
