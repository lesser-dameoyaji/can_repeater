#include <stdio.h>
#include <wiringPi.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"


extern int menu_title(int event);


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



int menu_init(void)
{
	// LCD initialize
	lcd_init();
	lcd_goto_home();

	// gpio pin initialize
	pin_init();
}

int menu(void)
{
	int i;
	unsigned char btn;
	static unsigned char mn = 0;

	for(i=0; i<4; i++)
	{
		if((pin_read(i) & 0x03) == 0x03)
		{
			// button priority L<R<U<D
			btn = pin_desc[i].pin;
		}
	}
	switch(mn)
	{
		default:
			mn += menu_title(btn);
			break;
//		case MENU_STATICS:
//			mn += menu_statics(btn);
//			break;
//		case MENU_LIST:
//			mn += main_list(btn);
//			break;
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
