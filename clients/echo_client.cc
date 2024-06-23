#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

const char *serv_ip   = "127.0.0.1";
uint16_t    serv_port = 8888;

int main(int ac, char *av[]) {
    int fd  = -1;
    int ret = -1;
    int nr, nw;

    fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);

    struct sockaddr_in serv_address;
    socklen_t          serv_addrlength = sizeof(serv_address);
    bzero(&serv_address, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_port   = htons(serv_port);
    inet_pton(AF_INET, serv_ip, &serv_address.sin_addr);


    ret = connect(fd, (struct sockaddr *)&serv_address, serv_addrlength);
    if (ret < 0) {
        perror("connect failed");
        return -1;
    }

    nw = write(fd, "hello", 5);
    if (nw < 0) {
        perror("write to socket failed");
        return -1;
    }

    char buf[1024];
    nr = read(fd, buf, sizeof(buf));
    if (nr < 0) {
        perror("read from socket failed");
        return -1;
    }
    close(fd);

    return 0;
}