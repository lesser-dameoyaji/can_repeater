/* 
 * 
 */
typedef void cmd_func(int id, int argc, void** argv);

void commander(int id, char* buf);
bool is_need_response(int id, char* cmd_str);
int get_command_table_size(void);
char* get_command_name(int func_id);
cmd_func* get_command_func(int func_id);
bool is_need_save_command(int func_id);

int send_command(int id, int dst_id, char* data, int len);
int send_response(int id, char* data, int len);

void configure(void);
