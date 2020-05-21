/* 
 * 
 */
typedef void cmd_func(int id, int argc, void** argv);

void commander(int id, char* buf);
bool is_need_response(int id, char* cmd_str);

int send_command(int id, int dst_id, char* data, int len);
int send_response(int id, char* data, int len);
