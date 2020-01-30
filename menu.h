
typedef int sub_menu_func_t(unsigned char btn);

typedef struct {
	char* title;
	sub_menu_func_t* func;
} menu_list_item_t;

typedef struct {
	// run-time
	int sub_menu;
	int index;
	int cursor;
	// list
	menu_list_item_t* list;
} menu_list_descriptor_t;

typedef struct {
	int x;
	int y;
	int max;
} menu_edit_position_t;

int menu_init(void);
int menu(void);

