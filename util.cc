#include "util.h"
#include <fcntl.h>

int readn(int fd, char *buf, int len) {}

int writen(int fd, char *buf, int len) {}

int setSockNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) return -1;

    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1) return -1;
    return 0;
}