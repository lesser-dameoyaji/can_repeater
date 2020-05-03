#include <pthread.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define VERSION_STRING "ver0.0.1"

// configurations

#define THREAD_ID_MAX		2		// thread数
#define CMD_SOCKET_BASE		8000	// 
#define ROUTING_TABLE_SIZE	32		//
#define COUNT_MAX			9999	//
#define TIMER_ID_MAX		2		// timer数

// types
#define CAN_ID_MAX			0x7FF	// Standard ID
#define CAN_ID_INVALID		0xFFF	// Invalid CAN ID

typedef int bool;
#define true				1
#define false				0

typedef struct {
	unsigned int id;
	unsigned int id_mask;
	
	int dst_id;
} route_t;

typedef struct {
	int id;
	pthread_t child_thread_handle;
	unsigned short cmd_port;					// server port number
	
	struct pollfd server_handles[2];			// server socket handle. [0]command(UDP), [1]can rx
	struct sockaddr_in server_addr;				// used by client
	int client_handle;
	int nfd;
	
	int tx_count;
	int rx_count;
	
	route_t routing_table[ROUTING_TABLE_SIZE];
	int routing_table_count;
	
	unsigned char status;
} self_t;
#define IDX_SVH_SOCK	0
#define IDX_SVH_CAN		1

#define STATUS_STOP			-1
#define STATUS_EXIT			0
#define STATUS_RUN			1
#define STATUS_RUN_IFUP		2
#define STATUS_RUN_IFOPN	3

extern self_t selfs[THREAD_ID_MAX];


// self メモリアクセス用マクロ
//   使用する場合、必ずローカル変数 int idを用意し、thread IDを格納しておくこと
#define self	selfs[id]
#define self_svr_handle		self.server_handles[IDX_SVH_SOCK]
#define self_can_handle		self.server_handles[IDX_SVH_CAN]
#define self_cli_handle		self.client_handle

