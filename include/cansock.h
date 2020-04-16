#ifndef __can_h__
#define __can_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int csock_open(char* ifname);
int csock_send(int can_fd, canid_t canid, unsigned char* data, int data_len);
int csock_recv(int cs_fd, struct can_frame* frame);
void csock_close(int fd);

#endif

