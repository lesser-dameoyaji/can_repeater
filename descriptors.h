#include <pthread.h>
#include <poll.h>
#include "cansock.h"

#include "types.h"

#define TX0	0
#define RX0	1
#define TX1	2
#define RX1	3

typedef int bool;
#define true	1
#define false	0

typedef struct {
	void* next;
	struct can_frame frame;
} framefilter_descriptor_t;

typedef struct {
	pthread_mutex_t mutex;
	framefilter_descriptor_t* top;
	int count;
} framefilter_que_t;

typedef void* thread_func(void* arg);
typedef int handler_func(void* arg);

typedef struct {
	int ch;
	struct can_frame frame;
} frame_descriptor_t;

typedef struct {
	// runtime
	pthread_t thread_handle;
	// socket framework
	int svr_fd;
	int acptd_fd;
	// can socket
	int can_fd;
	// 
	int cli_fd;
	// 
	int* brg_fd;
	struct pollfd fds[4];
	int fds_num;

	// statistics
	int can_frame_count;

	// control
	bool bridge_enable;
	framefilter_que_t* framefilter_que;		// one time filter
	
	// configurations
	int id;
	int dir;
	handler_func* handlers[4];
	char can_name[16];
	char uds_path[128];
//	char uds_path_bridge[128];
	int bridge_thread;
	bool bridge_enable_default;
} thread_descriptor_t;

