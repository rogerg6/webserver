#ifndef __UTIL_H
#define __UTIL_H

#include <unistd.h>

int readn(int fd, char *buf, int len);
int writen(int fd, char *buf, int len);
int setSockNonBlocking(int fd);

#endif