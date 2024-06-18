#include "request.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


char rqstr[] = "GET /hello HTTP/1.1\r\n"
               "Connection: keep-alive\r\n"
               "Keep-Alive: timeout=200\r\n"
               "Content-Type: application/x-www-form-urlencoded;charset=UTF-8\r\n";

int main(int argc, char **argv) {
    if (argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char *ip   = argv[1];
    int         port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client_address;
    socklen_t          client_addrlength = sizeof(client_address);
    int fd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
    if (fd < 0) {
        printf("errno is: %d\n", errno);
    } else {
        Request rq = Request(fd);
        rq.handle();
        close(fd);
    }

    close(listenfd);

    return 0;
}