#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "epoll.h"
#include "request.h"
#include "threadPool.h"
#include "util.h"

int main(int ac, char *av[]) {
    if (ac < 3) {
        printf("Usage: %s ip port.\n", basename(av[0]));
        return -1;
    }

    char    *ip   = av[1];
    uint16_t port = atoi(av[2]);

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

    // init thread pool, 10 work threads
    ThreadPool th_pool(10);
    th_pool.Start();

    // init epoll
    Epoller  epoller;
    Request *req = new Request();
    req->SetFd(listenfd);
    uint32_t events = EPOLLIN | EPOLLET;
    setSockNonBlocking(listenfd);
    epoller.AddFd(listenfd, req, events);

    while (true) {
        epoller.HandleEvents(listenfd, th_pool);
    }

    return 0;
}