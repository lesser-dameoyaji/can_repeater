#include <stdio.h>
#include <wiringPi.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"


extern unsigned int menu_title(unsigned int event);
extern unsigned int menu_counter(unsigned int event);

menu_func* menu_table[] = {
	menu_title,
	menu_counter,
	NULL
};

#define MENU_ID_MAX (sizeof(menu_table)/sizeof(menu_func*) - 1)


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
typedef struct {
	int pin;
	unsigned char crcnt;
	unsigned char prev_detect;
} pin_descriptor_t;

pin_descriptor_t pin_desc[4];
int pin_init(void);
unsigned char pin_read(int index);

static unsigned int menu_id = 0xFFFF;
unsigned int posted_event = NO_EVENT;

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
		menu_id = next_id;
		for(i=0; i<TIMER_ID_MAX; i++)
			menu_timer_stop(i);
		posted_event = ENTER;
	}
}

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

void menu_core(unsigned int event)
{
	if(menu_id < MENU_ID_MAX)
	{
		(menu_table[menu_id])(event);
	}
}

int menu(void)
{
	int i;
	unsigned int btn = 0;
	unsigned int event = NO_EVENT;
	
	if(posted_event != NO_EVENT)
	{
		menu_core(posted_event);
		posted_event = NO_EVENT;
	}
	
	for(i=0; i<4; i++)
	{
		if((pin_read(i) & 0x03) == 0x03)
		{
			// button priority L<R<U<D
			event = pin_desc[i].pin;
			
			if(event != NO_EVENT)
			{
				menu_core(event);
			}
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

	pin_desc[0].pin = BTN_L;
	pin_desc[1].pin = BTN_R;
	pin_desc[2].pin = BTN_U;
	pin_desc[3].pin = BTN_D;

	if(wiringPiSetupGpio() < 0)
	{
		printf("GPIO initialize fail\n");
		return -1;
	}
	pinMode(BTN_L, INPUT);
	pinMode(BTN_R, INPUT);
	pinMode(BTN_U, INPUT);
	pinMode(BTN_D, INPUT);

	pullUpDnControl(BTN_L, PUD_UP);
	pullUpDnControl(BTN_R, PUD_UP);
	pullUpDnControl(BTN_U, PUD_UP);
	pullUpDnControl(BTN_D, PUD_UP);

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
		pin_desc[index].crcnt = 0;
		detect = pin_desc[index].prev_detect;
	}
	else
	{
		pin_desc[index].crcnt++;
		if(pin_desc[index].crcnt < 2)
		{
			detect = pin_desc[index].prev_detect;
		}
		else
		{
			pin_desc[index].prev_detect = now_detect;
			detect = now_detect | 0x02;	// edge flag on
		}
	}
	return detect;
}
