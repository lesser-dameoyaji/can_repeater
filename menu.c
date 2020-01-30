#include <stdio.h>
#include <wiringPi.h>

#include "menu.h"

#include "descriptors.h"
#include "lcd.h"
#include "config.h"

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

#define BTN_L 13
#define BTN_R 21
#define BTN_U 19
#define BTN_D 26

extern int configure(char** argv);
unsigned char update = 1;
#define MENU_MAIN_LIST_MAX	6

static int edit_frame_core(unsigned char btn, frame_descriptor_t* frame_desc);
static int menu_list_core(unsigned char btn, menu_list_descriptor_t* desc);

static int menu_filter(unsigned char btn);
static int menu_load(unsigned char btn);
static int menu_save(unsigned char btn);
static int menu_bridge_config(unsigned char btn);
static int menu_exit(unsigned char btn);

static int menu_send(unsigned char btn);
static int send_frame_menu1(unsigned char btn);
static int send_frame_menu2(unsigned char btn);
static int send_frame_menu3(unsigned char btn);
static int send_frame_menu4(unsigned char btn);

static int send_frame1(unsigned char btn);
static int edit_frame1(unsigned char btn);
static int send_frame2(unsigned char btn);
static int edit_frame2(unsigned char btn);
static int send_frame3(unsigned char btn);
static int edit_frame3(unsigned char btn);
static int send_frame4(unsigned char btn);
static int edit_frame4(unsigned char btn);
#define MENU_TITLE		0
#define MENU_STATICS	1
#define MENU_LIST		2

#define SUB_MENU_TOP		-1
#define SUB_SEND_FRAME	1
#define SUB_FILTER		2
#define SUB_BRIDGE_CFG	3
#define SUB_LOAD		4
#define SUB_SAVE		5
#define SUB_EXIT		6

#define EXIT_MENU		-1

//
// main list
//
menu_list_item_t main_item[] = {
	{"Send Frame", menu_send},
	{"Filter", menu_filter},
	{"BridgeConfig", menu_bridge_config},
	{"Load", menu_load},
	{"Save", menu_save},
	{"Exit", menu_exit},
	{"", NULL}
};
menu_list_descriptor_t main_desc = {
	SUB_MENU_TOP, 0, 0,
	main_item
};

//
// send frame
//
menu_list_item_t send_frame_item[] = {
	{"Frame1", send_frame_menu1},
	{"Frame2", send_frame_menu2},
	{"Frame3", send_frame_menu3},
	{"Frame4", send_frame_menu4},
	{"", NULL}
};
menu_list_descriptor_t send_frame_desc = {
	SUB_MENU_TOP, 0, 0,
	send_frame_item
};

menu_list_item_t send_frame_sub_item[] = {
	{"Send", send_frame1},
	{"Edit Frame", edit_frame1},
	{"", NULL}
};

menu_list_descriptor_t send_frame_sub_desc = {
	SUB_MENU_TOP, 0, 0,
	send_frame_sub_item
};

extern thread_descriptor_t thread_desc[4];
extern frame_descriptor_t frame_desc[4];

//
// menu functions
//
static int menu_load(unsigned char btn)
{
	static int count = 0;
	int num,i;
	char* item[8];
	
	if(count == 0)
	{
		lcd_clear();
		lcd_printf("loading");
		
		while((num = config_read(item)) > 0)
		{
			lcd_putc('.');
			printf("read:%d", num);
			for(i=0; i<num; i++)
				printf(":%d=%s", i, item[i]);
			printf("\n");
			configure(item);
		}
	}
	count++;
	
	if(btn == BTN_L)
	{
		count = 0;
		return EXIT_MENU;
	}
	if(count >= 20)
	{
		count = 0;
		return EXIT_MENU;
	}
	return 0;
}

static int menu_save(unsigned char btn)
{
	static int count = 0;
	int ret;
	
	if(count == 0)
	{
		lcd_clear();
		lcd_printf("saving");
		
		ret = config_save(true, "bridge1 = %s", (thread_desc[RX0].bridge_enable == true)? "ON":"OFF");
		if(ret >= 0)
			lcd_putc('.');
		else
			lcd_putc((ret==-1)?'x':'y');
		
		if(config_save(true, "bridge2 = %s", (thread_desc[RX1].bridge_enable == true)? "ON":"OFF") >= 0)
			lcd_putc('.');
		else
			lcd_putc('x');
		
		if(config_save(true, "frame1 = %d,%03X,%d,%02X%02X%02X%02X%02X%02X%02X%02X", 
				frame_desc[0].ch,
			 	frame_desc[0].frame.can_id,
				frame_desc[0].frame.can_dlc,
				frame_desc[0].frame.data[0], 
				frame_desc[0].frame.data[1], 
				frame_desc[0].frame.data[2], 
				frame_desc[0].frame.data[3], 
				frame_desc[0].frame.data[4], 
				frame_desc[0].frame.data[5], 
				frame_desc[0].frame.data[6], 
				frame_desc[0].frame.data[7]) >= 0)
			lcd_putc('.');
		else
			lcd_putc('x');
		
		if(config_save(true, "frame2 = %d,%03X,%d,%02X%02X%02X%02X%02X%02X%02X%02X", 
				frame_desc[1].ch,
			 	frame_desc[1].frame.can_id,
				frame_desc[1].frame.can_dlc,
				frame_desc[1].frame.data[0], 
				frame_desc[1].frame.data[1], 
				frame_desc[1].frame.data[2], 
				frame_desc[1].frame.data[3], 
				frame_desc[1].frame.data[4], 
				frame_desc[1].frame.data[5], 
				frame_desc[1].frame.data[6], 
				frame_desc[1].frame.data[7]) >= 0)
			lcd_putc('.');
		else
			lcd_putc('x');
		
		if(config_save(true, "frame3 = %d,%03X,%d,%02X%02X%02X%02X%02X%02X%02X%02X", 
				frame_desc[2].ch,
			 	frame_desc[2].frame.can_id,
				frame_desc[2].frame.can_dlc,
				frame_desc[2].frame.data[0], 
				frame_desc[2].frame.data[1], 
				frame_desc[2].frame.data[2], 
				frame_desc[2].frame.data[3], 
				frame_desc[2].frame.data[4], 
				frame_desc[2].frame.data[5], 
				frame_desc[2].frame.data[6], 
				frame_desc[2].frame.data[7]) >= 0)
			lcd_putc('.');
		else
			lcd_putc('x');
			
		if(config_save(false, "frame4 = %d,%03X,%d,%02X%02X%02X%02X%02X%02X%02X%02X", 
				frame_desc[3].ch,
			 	frame_desc[3].frame.can_id,
				frame_desc[3].frame.can_dlc,
				frame_desc[3].frame.data[0], 
				frame_desc[3].frame.data[1], 
				frame_desc[3].frame.data[2], 
				frame_desc[3].frame.data[3], 
				frame_desc[3].frame.data[4], 
				frame_desc[3].frame.data[5], 
				frame_desc[3].frame.data[6], 
				frame_desc[3].frame.data[7]) >= 0)
			lcd_putc('.');
		else
			lcd_putc('x');
	}
	count++;
	if(btn == BTN_L)
	{
		count = 0;
		return EXIT_MENU;
	}
	if(count >= 20)
	{
		count = 0;
		return EXIT_MENU;
	}
	return 0;
	
}

static int menu_title(unsigned char btn)
{
	static int count = 0;

	count++;
	if(count == 1)
	{
		lcd_cursor(0, 0, false);
		lcd_printf("can repeater");
		lcd_cursor(0, 1, false);
		lcd_printf(" ver0.1.0");
	}
	if((btn > 0) || (count == 50))
	{
		update++;
		return 1;
	}
	return 0;
}

menu_edit_position_t edit_frame_item[] = {
	// CH
	{2, 0, 1},
	// ID
	{7, 0, 0x0f},
	{8, 0, 0x0f},
	{9, 0, 0x0f},
	{15, 0, 0x08},
	// frame
	{0, 1, 0x0f},
	{1, 1, 0x0f},
	{2, 1, 0x0f},
	{3, 1, 0x0f},
	{4, 1, 0x0f},
	{5, 1, 0x0f},
	{6, 1, 0x0f},
	{7, 1, 0x0f},
	{8, 1, 0x0f},
	{9, 1, 0x0f},
	{10, 1, 0x0f},
	{11, 1, 0x0f},
	{12, 1, 0x0f},
	{13, 1, 0x0f},
	{14, 1, 0x0f},
	{15, 1, 0x0f},
	{-1, -1, 0x00}
};
static int menu_filter(unsigned char btn)
{
	return 0;
}

static int menu_statics(unsigned char btn)
{
	static int count[4]={0, 0, 0, 0};
	static char bridge[2] = {' ', ' '};
	
	// button
#if 0
	if(btn == BTN_U)
	{
		update = 1;
		lcd_clear();
		return MENU_LIST;
	}
#endif
	if(btn == BTN_D)
	{
		update = 1;
		lcd_clear();
		// next menu
		return 1;
	}

	// CH1 check
	if((count[TX0] != thread_desc[TX0].can_frame_count) ||
		(count[RX0] != thread_desc[RX0].can_frame_count))
	{
		count[TX0] = thread_desc[TX0].can_frame_count;
		count[RX0] = thread_desc[RX0].can_frame_count;
		update++;
	}
	if((bridge[0] == ' ') && (thread_desc[RX0].bridge_enable == true))
	{
		bridge[0] = '-';
	}
	if((bridge[0] == '-') && (thread_desc[RX0].bridge_enable == false))
	{
		bridge[0] = ' ';
	}
	// CH2 check
	if((count[TX1] != thread_desc[TX1].can_frame_count) ||
		(count[RX1] != thread_desc[RX1].can_frame_count))
	{
		count[TX1] = thread_desc[TX1].can_frame_count;
		count[RX1] = thread_desc[RX1].can_frame_count;
		update++;
	}
	if((bridge[1] == ' ') && (thread_desc[RX1].bridge_enable == true))
	{
		bridge[1] = '-';
	}
	if((bridge[1] == '-') && (thread_desc[RX1].bridge_enable == false))
	{
		bridge[1] = ' ';
	}

	if(update > 0)
	{
		update = 0;
		lcd_clear();
		lcd_printf("R0:%4d%cT1:%4d", 
				count[RX0],
				bridge[0],
			       	count[TX1]);
		lcd_cursor(0, 1, false);
		lcd_printf("T0:%4d%cR1:%4d", 
				count[TX0],
				bridge[1],
			       	count[RX1]);
	}
	return 0;

}

static int menu_bridge_config(unsigned char btn)
{
	static unsigned char cur = 0;
	int ch;
	// button
	if(btn == BTN_U)
	{
	       	if(cur == 1)
		{
			cur--;
			update++;
		}
	}
	if(btn == BTN_D)
	{
	       	if(cur == 0)
		{
			cur++;
			update++;
		}
	}
	if(btn == BTN_R)
	{
		ch = (cur==0)? RX0: RX1;

		if(thread_desc[ch].bridge_enable == false)
		{
			thread_desc[ch].bridge_enable = true;
		}
		else
		{
			thread_desc[ch].bridge_enable = false;
		}
		update++;
	}
	if(btn == BTN_L)
	{
		return -1;
	}

	if(update > 0)
	{
		update = 0;
		//
		lcd_clear();
		lcd_printf("%cCH1 Bridge %s", (cur==0)? '\x7e':' ', (thread_desc[RX0].bridge_enable==true)? "ON":"OFF");
		lcd_cursor(0, 1, false);
		lcd_printf("%cCH2 Bridge %s", (cur==1)? '\x7e':' ', (thread_desc[RX1].bridge_enable==true)? "ON":"OFF");
	}

}

static int menu_exit(unsigned char btn)
{
	extern int exit_process_request;
	static int count = 0;

	if(count == 0)
	{
		lcd_clear();
		lcd_printf("exit...");
		exit_process_request = 1;
	}
	if(count < 10)
		count++;
}

static int menu_send(unsigned char btn)
{
	return menu_list_core(btn, &send_frame_desc);
}

static int send_frame_menu1(unsigned char btn)
{
	send_frame_sub_item[0].func = send_frame1;
	send_frame_sub_item[1].func = edit_frame1;
	return menu_list_core(btn, &send_frame_sub_desc);
}
static int send_frame_menu2(unsigned char btn)
{}
static int send_frame_menu3(unsigned char btn)
{}
static int send_frame_menu4(unsigned char btn)
{}
static int send_frame1(unsigned char btn)
{
	int ch;
	struct can_frame frame = {
	 	frame_desc[0].frame.can_id,
		frame_desc[0].frame.can_dlc,
		0, 0, 0,			// dummy
		frame_desc[0].frame.data[0], 
		frame_desc[0].frame.data[1], 
		frame_desc[0].frame.data[2], 
		frame_desc[0].frame.data[3], 
		frame_desc[0].frame.data[4], 
		frame_desc[0].frame.data[5], 
		frame_desc[0].frame.data[6], 
		frame_desc[0].frame.data[7] };

	ch = (frame_desc[0].ch == 0)? TX0:TX1;
	
	send(thread_desc[ch].cli_fd, &frame, sizeof(struct can_frame), 0);
	return -1;
}
static int edit_frame1(unsigned char btn)
{
	edit_frame_core(btn, &frame_desc[0]);
}
static int send_frame2(unsigned char btn)
{}
static int edit_frame2(unsigned char btn)
{}
static int send_frame3(unsigned char btn)
{}
static int edit_frame3(unsigned char btn)
{}
static int send_frame4(unsigned char btn)
{}
static int edit_frame4(unsigned char btn)
{}

static int main_list(unsigned char btn)
{
	return menu_list_core(btn, &main_desc);
}
const char edit_frame_uline[] = "CHx ID:xxx";
const char edit_frame_dline[] = "0011223344556677";
static int edit_frame_core(unsigned char btn, frame_descriptor_t* frame_desc)
{
	static int index = 0;
	static int count = 0;
	bool nupdate = false;
	char n;
	// initialize
	if(count == 0)
	{
		lcd_cursor(-1, -1, true);
	}
	switch(btn)
	{
	case BTN_U:
		count = 1;

		n = lcd_getc();
		n -= (n <= '9')? '0':('A'-10);
		if(n < edit_frame_item[index].max)
			n++;
		else
			n = 0;


		nupdate = true;
		update++;
		lcd_cursor(edit_frame_item[index].x, edit_frame_item[index].y, true);
		break;
	case BTN_D:
		count = 1;

		n = lcd_getc();
		n -= (n <= '9')? '0':('A'-10);
		if(n > 0)
			n--;
		else
			n = (char)edit_frame_item[index].max;

		nupdate = true;
		update++;
		lcd_cursor(edit_frame_item[index].x, edit_frame_item[index].y, true);
		break;
	case BTN_R:
		if(edit_frame_item[index+1].x >= 0)
		{
			index++;
			lcd_cursor(edit_frame_item[index].x, edit_frame_item[index].y, true);
			update++;
		}
		break;
	case BTN_L:
		if(index > 0)
		{
			index--;
			lcd_cursor(edit_frame_item[index].x, edit_frame_item[index].y, true);
			update++;
		}
		else
		{
			count = 0;
			lcd_cursor(-1, -1, false);
			return EXIT_MENU;
		}
		break;
	}

	if(update > 0)
	{
		update = 0;
		if(nupdate == true)
		{
			// write back
			switch(index)
			{
			case 0:
				frame_desc->ch = n;
				break;
			case 1:
			 	frame_desc->frame.can_id &= 0x00ff;
			 	frame_desc->frame.can_id |= (n << 8);
				break;
			case 2:
			 	frame_desc->frame.can_id &= 0x0f0f;
			 	frame_desc->frame.can_id |= (n << 4);
				break;
			case 3:
			 	frame_desc->frame.can_id &= 0x0ff0;
			 	frame_desc->frame.can_id |= n;
				break;
			case 4:
				frame_desc->frame.can_dlc = n;
				break;
			default:
				if(((index - 5) & 0x01) == 0)
				{
					frame_desc->frame.data[(index-5)/2] &= 0x0f; 
					frame_desc->frame.data[(index-5)/2] |= (n<<4);
				}
				else
				{
					frame_desc->frame.data[(index-5)/2] &= 0xf0; 
					frame_desc->frame.data[(index-5)/2] |= n;
				}
				break;
			}
		}
		// read out
		lcd_cursor(0, 0, true);
		lcd_printf("CH%d ID=%03X DLC=%d", 
				frame_desc->ch,
			       	frame_desc->frame.can_id,
			       	frame_desc->frame.can_dlc);

		lcd_cursor(0, 1, true);
		lcd_printf("%02X%02X%02X%02X%02X%02X%02X%02X",
			frame_desc->frame.data[0],
			frame_desc->frame.data[1],
			frame_desc->frame.data[2],
			frame_desc->frame.data[3],
			frame_desc->frame.data[4],
			frame_desc->frame.data[5],
			frame_desc->frame.data[6],
			frame_desc->frame.data[7]);
		lcd_cursor(edit_frame_item[index].x, edit_frame_item[index].y, true);
	}
	count++;
	return 0;
}

static int menu_list_core(unsigned char btn, menu_list_descriptor_t* desc)
{
	int i;

	// call sub-menu func
	if(desc->sub_menu > SUB_MENU_TOP)
	{
		if(desc->list[desc->sub_menu].func(btn) < 0)
		{
			desc->sub_menu = SUB_MENU_TOP;
			update = 1;
		}
		return 0;
	}

	switch(btn)
	{
	case BTN_U:
		update++;
		if(desc->cursor == 0)
			desc->index--;
		else
			desc->cursor = 0;
		if(desc->index < 0)
		{
			desc->index = 0;
			desc->cursor = 0;
			return EXIT_MENU;
		}
		break;
	case BTN_D:
		update++;
		if(desc->cursor == 0)
			desc->cursor = 1;
		else
			desc->index++;
		if(desc->list[desc->index+1].func == NULL)
		{
			desc->cursor = 0;
		}
		break;
	case BTN_R:
		update++;
		desc->sub_menu = desc->index + desc->cursor;
		printf("sub_menu = %d\n", desc->sub_menu);
		return 0;
	case BTN_L:
		update++;
		desc->index = 0;
		desc->cursor = 0;
		return EXIT_MENU;
	}
	if(update > 0)
	{
		lcd_clear();
		update = 0;

		for(i = 0; (i < 2) && (desc->list[desc->index + i].func != NULL); i++)
		{
			lcd_cursor(0, i, false);
			lcd_printf("%c%s", (desc->cursor==i)?'\x7e':' ', desc->list[desc->index + i].title);
		}
	}
	return 0;
}

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
		case MENU_TITLE:
			mn += menu_title(btn);
			break;
		case MENU_STATICS:
			mn += menu_statics(btn);
			break;
		case MENU_LIST:
			mn += main_list(btn);
			break;
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

			

