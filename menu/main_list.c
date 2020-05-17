#include <stdio.h>

#include "common.h"
#include "menu.h"

extern int menu_exit(unsigned int event);

list_item_t main_list_items[] = {
	{&main_list_items[1], "exit", menu_exit},
	{NULL, NULL, NULL}
};

static list_info_t main_list_info = {
	0, 0,
	&main_list_items[0]
};

int main_list(unsigned int event)
{
	return menu_list_core(&main_list_info, event);
}

