#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"

//
// 
//
extern int menu_title(unsigned int event);
extern int menu_counter(unsigned int event);
extern int main_list(unsigned int event);

menu_func* menu_table[] = {
	menu_title,
	menu_counter,
	main_list,
	NULL
};

#define MENU_ID_MAX (sizeof(menu_table)/sizeof(menu_func*) - 1)

//
static int menu_core(unsigned int event);

// timer
typedef struct {
	bool enable;
	bool cyclic;
	unsigned int count;
	unsigned int count_max;
} timer_descriptor_t;
timer_descriptor_t timer_table[TIMER_ID_MAX];

//
// GPIO pin
//
#define KEY_EDGE_DETECT	0x02
#define KEY_ON			0x01
#define KEY_OFF			0x00
#define KEY_RELEASE		(KEY_EDGE_DETECT | KEY_OFF)
#define KEY_PRESS		(KEY_EDGE_DETECT | KEY_ON)

typedef struct {
	int pin;
	unsigned char crcnt;
	unsigned int lpcnt;
	unsigned char prev_detect;
} pin_descriptor_t;

pin_descriptor_t pin_desc[4];
int pin_init(void);
unsigned char pin_read(int index);

static unsigned int menu_id = 0xFFFF;
unsigned int posted_event = NO_EVENT;

//
// basic interfaces
//
int menu_init(void)
{
	// LCD initialize
	lcd_init();
	lcd_goto_home();

	// gpio pin initialize
	pin_init();
}


void menu_change(unsigned int next_id)
{
	int i;
	if((next_id < MENU_ID_MAX) && (menu_id != next_id))
	{
		menu_core(LEAVE);
		
		menu_id = next_id;
		for(i=0; i<TIMER_ID_MAX; i++)
			menu_timer_stop(i);
		posted_event = ENTER;
	}
}

//
// timer
//
void menu_timer_start(unsigned int timer_id, unsigned int count, bool cyclic)
{
	if (timer_id < TIMER_ID_MAX)
	{
		timer_table[timer_id].enable = true;
		timer_table[timer_id].count = 0;
		timer_table[timer_id].count_max = count;
		timer_table[timer_id].cyclic = cyclic;
	}
	
}

void menu_timer_stop(unsigned int timer_id)
{
	if (timer_id < TIMER_ID_MAX)
	{
		timer_table[timer_id].enable = false;
	}
}

//
// list menu
//
int menu_list_draw(list_info_t* list_info)
{
	int i;
	list_item_t* item;
	
	lcd_clear();
	
	// draw cursor
	if(list_info->index_cursor == 0)
	{
		lcd_goto_xy(0,0);
		lcd_putc(0x7e);
		lcd_goto_xy(0,1);
		lcd_putc(' ');
	}
	else
	{
		lcd_goto_xy(0,0);
		lcd_putc(' ');
		lcd_goto_xy(0,1);
		lcd_putc(0x7e);
	}
	
	item = list_info->items;
	for(i=0; i<list_info->index; i++)
	{
		if(item == NULL)
			return 0;
		item = (list_item_t*)item->next;
	}
	
	for(i=0; i<2; i++)
	{
		if((item == NULL) || (item->next == NULL))
		{
			break;
		}
		lcd_goto_xy(1,i);
		lcd_printf("%s", item->name);
		
		item = (list_item_t*)item->next;
	}
	lcd_flush();
	return 0;
}

int menu_list_exec(list_info_t* list_info, unsigned int event)
{
	int id = list_info->index + list_info->index_cursor;
	int i;
	list_item_t* item;
	
	item = list_info->items;
	for(i=0; i<list_info->index; i++)
	{
		if(item == NULL)
			return 0;
		item = (list_item_t*)item->next;
	}
	
	if((item != NULL) && (item->func != NULL))
	{
		return item->func(event);
	}
	return 0;
}

int menu_list_core(list_info_t* list_info, unsigned int event)
{
	switch(event)
	{
	case ENTER:
//		break;
	case DRAW:
		menu_list_draw(list_info);
		break;
	case TIMEOUT0:
	case TIMEOUT1:
		break;
	case KEY_L:
	case KEY_R:
		menu_list_exec(list_info, event);
		break;
	case KEY_U:
	case KEY_D:
	case KEY_LP_L:
	case KEY_LP_R:
	case KEY_LP_U:
	case KEY_LP_D:
		break;
	case LEAVE:
		break;
	}
}

//
// 
//
static int menu_core(unsigned int event)
{
	if(menu_id < MENU_ID_MAX)
	{
		return (menu_table[menu_id])(event);
	}
}

int menu(void)
{
	int i;
	unsigned int btn = 0;
	unsigned int event = NO_EVENT;
	
	if(posted_event != NO_EVENT)
	{
		if(menu_core(posted_event) < 0)
			return -1;
		posted_event = NO_EVENT;
	}
	
	for(i=0; i<4; i++)
	{
		// button priority L<R<U<D
		event = pin_read(i);
		
		if(event != NO_EVENT)
		{
			if(menu_core(event) < 0)
				return -1;
		}
	}
	
	for(i=0; i<TIMER_ID_MAX; i++)
	{
		// timer process
		if(timer_table[i].enable == false)
			continue;
		timer_table[i].count++;
		if(timer_table[i].count >= timer_table[i].count_max)
		{
			menu_core(TIMER_BASE+i);
			timer_table[i].count = 0;
			timer_table[i].enable = timer_table[i].cyclic;
		}
	}
	lcd_flush();
	
	return 0;
}

//
// GPIO pin function
//
int pin_init(void)
{
	int i;

	for(i=0; i<4; i++)
	{
		pin_desc[i].crcnt = 0;
		pin_desc[i].prev_detect = 0;
	}

	pin_desc[0].pin = KEY_L;
	pin_desc[1].pin = KEY_R;
	pin_desc[2].pin = KEY_U;
	pin_desc[3].pin = KEY_D;

	if(wiringPiSetupGpio() < 0)
	{
		printf("GPIO initialize fail\n");
		return -1;
	}
	pinMode(KEY_L, INPUT);
	pinMode(KEY_R, INPUT);
	pinMode(KEY_U, INPUT);
	pinMode(KEY_D, INPUT);

	pullUpDnControl(KEY_L, PUD_UP);
	pullUpDnControl(KEY_R, PUD_UP);
	pullUpDnControl(KEY_U, PUD_UP);
	pullUpDnControl(KEY_D, PUD_UP);

	return 0;
}

unsigned char pin_read(int index)
{
	unsigned char now_detect;
	unsigned char detect;

	if(digitalRead(pin_desc[index].pin) == 0)
	{
		now_detect = 1;
	}
	else
	{
		now_detect = 0;
	}
	if(now_detect == pin_desc[index].prev_detect)
	{
		// detect chatter, counter restart
		pin_desc[index].crcnt = 0;
		detect = pin_desc[index].prev_detect;
	}
	else
	{
		pin_desc[index].crcnt++;
		if(pin_desc[index].crcnt < MENU_KEY_REDUCE_CHATTER)
		{
			detect = pin_desc[index].prev_detect;
		}
		else
		{
			pin_desc[index].prev_detect = now_detect;
			detect = now_detect | KEY_EDGE_DETECT;	// edge flag on
		}
	}
	
	if((detect & KEY_EDGE_DETECT) == KEY_EDGE_DETECT)
	{
		if(detect == KEY_RELEASE)
		{
			// key release
			detect = pin_desc[index].pin;
		}
		else
		{
			// key press
			detect = KEY_NONE; 
		}
	}
	else if((detect & KEY_ON) == KEY_ON)
	{
		if(pin_desc[index].lpcnt <= MENU_KEY_LONG_PRESS)
		{
			pin_desc[index].lpcnt++;
		}
		if(pin_desc[index].lpcnt == MENU_KEY_LONG_PRESS)
		{
			detect = pin_desc[index].pin + 1;
		}

	}
	else
	{
		pin_desc[index].lpcnt = 0;
	}
	return detect;
}
