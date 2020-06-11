#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <wiringPi.h>

#include "common.h"
#include "menu.h"
#include "lcd.h"
#include "rpt.h"

//
// 
//
extern int menu_title(int event);
extern int menu_counter(int event);
extern int main_list(int event);
extern int menu_load(int event);
extern int menu_save(int event);

menu_func* menu_table[] = {
	menu_title,
	menu_counter,
	main_list,
	menu_load,
	menu_save,
	NULL
};

#define MENU_ID_MAX (sizeof(menu_table)/sizeof(menu_func*) - 1)

//
#define EVENT_QUE_DEPTH		16
typedef struct {
	int in;
	int out;
	int count;
} event_queue_t;
static event_queue_t event_que;
static int event_que_buf[EVENT_QUE_DEPTH];

static int event_que_init(void);
static int event_que_in(int val);
static int event_que_out(int* val);
static int event_que_count(void);
static int menu_core(int event);

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
char menu_socket_buf[256];

static int event_que_init(void)
{
	event_que.in = 0;
	event_que.out = 0;
	event_que.count = 0;
}
static int event_que_in(int val)
{
	if(event_que.count >= EVENT_QUE_DEPTH)
		return -1;
	event_que_buf[event_que.in] = val;
	event_que.in++;
	event_que.count++;
	
	if(event_que.in >= EVENT_QUE_DEPTH)
		event_que.in = 0;
}
static int event_que_out(int* val)
{
	if(event_que.count == 0)
		return -1;
		
	*val = event_que_buf[event_que.out];
	event_que.out++;
	event_que.count--;
	
	if(event_que.out >= EVENT_QUE_DEPTH)
		event_que.out = 0;
}
static int event_que_count(void)
{
	return event_que.count;
}
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
		event_que_in(LEAVE);
		
		menu_id = next_id;
		for(i=0; i<TIMER_ID_MAX; i++)
			menu_timer_stop(i);
		event_que_in(ENTER);
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
int menu_list_add(list_info_t* list, int id, char* name, menu_func* func)
{
	// malloc
	void* mem;
	
	mem = malloc(sizeof(list_item_t));
}

int menu_list_delete(list_info_t* list, int id)
{
	list_item_t *item, *prev, *next;
	
	if(id >= list->items_num)
	{
		return -1;		// index out of range
	}
	
	prev = NULL;
	item = list->items;
	next = NULL;
	
	// rewind
	for(;id > 0; id--)
	{
		if(item == NULL)
		{
			return -1;		// no id
		}
		prev = item;
		item = item->next;
	}
	if(item == NULL)
		return -1;
	next = item->next;
	
	// unlinkage
	if(prev == NULL)
	{
		list->items = next;
	}
	else
	{
		prev->next = next;
	}
	
	// free allocated memory
	if(item->alloced == true)
	{
		// free
		free(item);
	}
}

int menu_list_num(list_info_t* list)
{
	return list->items_num;
}

int menu_list_draw(list_info_t* list_info)
{
	int i;
	list_item_t* item;
	
	lcd_clear();
	
	// draw cursor
	if(list_info->index_cursor == 0)
	{
		lcd_cursor(0,0, false);
		lcd_putc(0x7e);
		lcd_cursor(0,1, false);
		lcd_putc(' ');
	}
	else
	{
		lcd_cursor(0,0, false);
		lcd_putc(' ');
		lcd_cursor(0,1, false);
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
		lcd_cursor(1,i, false);
		lcd_printf("%s", item->name);
		
		item = (list_item_t*)item->next;
	}
	lcd_flush();
	return 0;
}

int menu_list_exec(list_info_t* list_info, int event)
{
	int idx = list_info->index + list_info->index_cursor;
	int i;
	list_item_t* item;
	
	item = list_info->items;
	for(i=0; i<idx; i++)
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

int menu_list_core(list_info_t* list_info, int event)
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
		menu_list_exec(list_info, ENTER);
		break;
	case KEY_U:
		if(list_info->index_cursor == 0)
		{
			// カーソルは最上行
			if(list_info->index == 0)
			{
				// 更にリスト上端 → 前画面へ
				menu_change(MENU_COUNTER);
			}
			else
			{
				// リストを下スクロール
				list_info->index--;
				event_que_in(DRAW);
			}
		}
		else
		{
			list_info->index_cursor--;
			event_que_in(DRAW);
		}
		break;
	case KEY_D:
		if(list_info->index_cursor == 1)
		{
			// カーソルは下行
			// リストを上スクロール
			list_info->index++;
			if(list_info->index == (list_info->items_num-1))
			{
				// リスト下端 に到達 → カーソルも上行へ
				list_info->index_cursor = 0;
			}
			event_que_in(DRAW);
		}
		else
		{
			// カーソルは上行
			if(list_info->index == (list_info->items_num-1))
			{
				// 下に表示するitemなし → 無視
				printf("ignored\n");
			}
			else
			{
				// カーソル下移動
				list_info->index_cursor++;
				event_que_in(DRAW);
			}
		}
		break;
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
static int menu_core(int event)
{
	if(menu_id < MENU_ID_MAX)
	{
		return (menu_table[menu_id])(event);
	}
}

int menu(void)
{
	int i, loop=true, from_len, l, ret;
	unsigned int btn = 0;
	int event = NO_EVENT;
	struct pollfd server_handle;
	struct sockaddr_in from_addr;
	
	server_handle.fd = open_server_socket(CMD_SOCKET_BASE-1);
	server_handle.events = POLLIN;
	if (server_handle.fd < 0)
	{
		printf("create socket error\n");
		return -1;
	}
	
	while(loop == true)
	{
		ret = poll(&server_handle, 1, MENU_CYCLIC);
		if(ret > 0)
		{
			// 
			from_len = sizeof(from_addr);
			l = recvfrom(server_handle.fd, menu_socket_buf, 256, 0, (struct sockaddr *)&from_addr, &from_len);
			menu_socket_buf[l] = 0;
			event_que_in(RECEIVE);
		}
		else
		{
			for(i=0; i<4; i++)
			{
				// button priority L<R<U<D
				event = pin_read(i);
				
				if(event != NO_EVENT)
				{
					event_que_in(event);
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
					event_que_in(TIMER_BASE+i);
					timer_table[i].count = 0;
					timer_table[i].enable = timer_table[i].cyclic;
				}
			}
		}
		
		while(event_que_count() > 0)
		{
			if(event_que_out(&event) < 0)
				break;
			if(menu_core(event) < 0)
			{
				loop = false;
				break;
			}
		}
		lcd_flush();
	}
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
