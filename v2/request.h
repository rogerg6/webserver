#ifndef __REQUEST_H
#define __REQUEST_H

#include <cstring>
#include <map>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 4096

struct EchoService {
    EchoService() {}
    EchoService(int fd)
        : fd(fd) {}
    ~EchoService() {
        if (fd > 0) close(fd);
    }

    void handleRequest() {
        char buf[BUFLEN];
        int  rn = -1;
        while (1) {
            // 非阻塞读取
            rn = read(fd, buf, BUFLEN);
            if (rn < 0) {
                perror("Reading failed");
                break;
            } else if (rn == 0) {
                perror("Remote client has closed connection");
                break;
            }

            if (write(fd, buf, rn) < 0) {
                perror("write failed");
            }
            break;
        }
        delete this;
    }

    int fd;
};


#endif