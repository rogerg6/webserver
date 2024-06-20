#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "epoll.h"
#include "request.h"
#include "threadPool.h"
#include "util.h"

const char *ip   = "127.0.0.1";
uint16_t    port = 8888;

void handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags   = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) return;
}

int main(int ac, char *av[]) {
    handle_for_sigpipe();

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
    ret = listen(listenfd, 1024);
    assert(ret != -1);

    // init thread pool, 10 work threads
    ThreadPool th_pool(10);
    th_pool.Start();

    // init epoll
    Epoller      epoller;
    EchoService *echo_req = new EchoService(listenfd);
    uint32_t     events   = EPOLLIN | EPOLLET;
    if (setSockNonBlocking(listenfd) < 0) {
        perror("setnonblocking error.");
        return -2;
    }
    epoller.AddFd(listenfd, echo_req, events);

    while (true) {
        epoller.HandleEvents(listenfd, th_pool);
    }

    return 0;
}