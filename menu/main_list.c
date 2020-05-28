#include <stdio.h>

#include "common.h"
#include "menu.h"

extern int menu_save(int event);
extern int menu_load_enter(int event);
extern int menu_save_enter(int event);
extern int menu_exit(int event);

list_item_t main_list_items[] = {
	{&main_list_items[1], "send", menu_exit},
	{&main_list_items[2], "filter", menu_exit},
	{&main_list_items[3], "load", menu_load_enter},
	{&main_list_items[4], "save", menu_save_enter},
	{&main_list_items[5], "exit", menu_exit},
	{NULL, NULL, NULL}
};

static list_info_t main_list_info = {
	0, 0,
	(sizeof(main_list_items) / sizeof(list_item_t) - 1),
	&main_list_items[0]
};

int main_list(int event)
{
	return menu_list_core(&main_list_info, event);
}

