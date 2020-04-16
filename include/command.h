/* 
 * 
 */
#define ARGUMENT_MAX 16			// used by parser


typedef void cmd_func(int id, int argc, void** argv);
void commander(int id, char* buf);

