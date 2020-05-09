#define VERSION_STRING "ver0.0.1"

// configurations

#define LOCAL_CH_MAX			2		// 自CAN CH数
#define GLOBAL_CH_MAX			2		// 全CAN CH数
#define CMD_SOCKET_BASE			8000	// 
#define ROUTING_TABLE_SIZE		32		// 
#define COUNT_MAX				9999	// CAN TX/RX counter 最大値
#define TIMER_ID_MAX			2		// timer数


#define MENU_CYCLIC				20		// menu呼び出し周期、keyサンプリング周期(ms)
#define MENU_KEY_REDUCE_CHATTER	2		// keyチャッター除去 = (keyサンプリング周期 * 左値)ms間レベル維持で確定
#define MENU_KEY_LONG_PRESS		100		// key長押し確定 = (keyサンプリング周期 * 左値)ms間レベル維持で確定
