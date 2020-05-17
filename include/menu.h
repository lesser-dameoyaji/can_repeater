// menu id
#define MENU_TITLE		0
#define MENU_COUNTER	1
#define MENU_MAIN_LIST	2

// events
#define NO_EVENT	0
#define ENTER		1
#define DRAW		2
#define LEAVE		9
#define TIMER_BASE	10
#define TIMEOUT0	(TIMER_BASE+0)
#define TIMEOUT1	(TIMER_BASE+1)

#define KEY_NONE	NO_EVENT
#define KEY_L		13
#define KEY_R		21
#define KEY_U		19
#define KEY_D		26
#define KEY_LP_L	(13+1)
#define KEY_LP_R	(21+1)
#define KEY_LP_U	(19+1)
#define KEY_LP_D	(26+1)

typedef int menu_func(unsigned int event);

typedef struct {
	void* next;
	char* name;
	menu_func* func;
} list_item_t;

typedef struct {
	int index_cursor;			// 画面内表示位置
	int index;					// リスト表示位置(画面最上行に表示するリストアイテム番号)
	list_item_t* items;
} list_info_t;

int menu_init(void);
int menu(void);
void menu_change(unsigned int next_id);
void menu_timer_start(unsigned int timer_id, unsigned int count, bool cyclic);
void menu_timer_stop(unsigned int timer_id);

int menu_list_core(list_info_t* list_info, unsigned int event);
