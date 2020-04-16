#include <pthread.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// configurations

#define THREAD_ID_MAX		2		// thread数
#define CMD_SOCKET_BASE		8000	// 
#define ROUTING_TABLE_SIZE	32		//
#define CAN_ID_MAX			0x7FF	// Standard ID
#define CAN_ID_INVALID		0xFFF	// Invalid CAN ID

typedef struct {
	unsigned int id;
	unsigned int id_mask;
	
	int dst_id;
} route_t;

typedef struct {
	int id;
	pthread_t child_thread_handle;
	unsigned short cmd_port;					// server port number
	
	struct pollfd server_handles[2];			// server socket handle
	struct sockaddr_in server_addr;				// used by client
	int client_handle;
	int nfd;
	
	route_t routing_table[ROUTING_TABLE_SIZE];
	int routing_table_count;
	
	unsigned char exit_request;
} self_t;
#define IDX_SVH_SOCK	0
#define IDX_SVH_CAN		1


extern self_t selfs[THREAD_ID_MAX];


// self メモリアクセス用マクロ
//   使用する場合、必ずローカル変数 int idを用意し、thread IDを格納しておくこと
#define self	selfs[id]
#define self_svr_handle		self.server_handles[IDX_SVH_SOCK]
#define self_can_handle		self.server_handles[IDX_SVH_CAN]
#define self_cli_handle		self.client_handle

