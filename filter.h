#include <stdlib.h>

int framefilter_init(framefilter_que_t* que);
int framefilter_add(framefilter_que_t* que, struct can_frame* frame);
bool is_framefilter_block(framefilter_que_t* que, struct can_frame* frame);
