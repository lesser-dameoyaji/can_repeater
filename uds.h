#ifndef __uds_h__
#define __uds_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

int uds_open_svr(char* pathname);
int uds_open_cli(char* pathname);
void uds_close(int fd);

#endif
