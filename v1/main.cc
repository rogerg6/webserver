#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"

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
    int listenfd = -1, connfd = -1;
    int ret = -1;

    handle_for_sigpipe();

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 1024);
    assert(ret != -1);

    struct sockaddr_in client_address;
    socklen_t          client_addrlength = sizeof(client_address);
    while (true) {
        memset(&client_address, 0, sizeof(struct sockaddr_in));
        connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0) {
            perror("accept failed");
            continue;
        }

        EchoService *es = new EchoService(connfd);
        es->handleRequest();
    }

    return 0;
}