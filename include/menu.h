

// events
#define NO_EVENT	0
#define ENTER		1
#define TIMER_BASE	10
#define TIMEOUT0	(TIMER_BASE+0)
#define TIMEOUT1	(TIMER_BASE+1)

#define KEY_NONE	NO_EVENT
#define KEY_L		13
#define KEY_R		21
#define KEY_U		19
#define KEY_D		26

typedef unsigned int menu_func(unsigned int event);


int menu_init(void);
int menu(void);
void menu_change(unsigned int next_id);
void menu_timer_start(unsigned int timer_id, unsigned int count, bool cyclic);
void menu_timer_stop(unsigned int timer_id);
