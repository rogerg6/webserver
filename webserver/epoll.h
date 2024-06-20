#ifndef __EPOLL_H
#define __EPOLL_H

#include "request.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "threadPool.h"
// #include "threadpool.h"
#include "util.h"

const int MAX_EVENT_NUMBER = 10000;
const int LISTENQ          = 1024;

void handHttpFunc(void *args) {
    Request *rq = (Request *)args;
    rq->handle();
}

void handleEchoFunc(void *args) {
    EchoService *rq = (EchoService *)args;
    rq->handleRequest();
}

class Epoller {
public:
    Epoller() {
        epollfd_ = epoll_create(LISTENQ + 1);
        if (epollfd_ == -1) perror("epoll create failed.");
        events_ = new epoll_event[MAX_EVENT_NUMBER];
    }

    ~Epoller() {
        if (events_) delete[] events_;
    }

    int AddFd(int fd, void *request, uint32_t events) {
        struct epoll_event ev;
        ev.events   = events;
        ev.data.ptr = request;

        int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        if (ret < 0) {
            perror("epoll add error.");
            return -1;
        }
        return 0;
    }

    int DelFd(int fd, void *request, uint32_t events) {
        struct epoll_event ev;
        ev.events   = events;
        ev.data.ptr = request;

        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
        if (ret < 0) {
            perror("epoll del error.");
            return -1;
        }
        return 0;
    }

    int ModFd(int fd, void *request, uint32_t events) {
        struct epoll_event ev;
        ev.events   = events;
        ev.data.ptr = request;

        int ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        if (ret < 0) {
            perror("epoll mod error.");
            return -1;
        }
        return 0;
    }

    void HandleEvents(int listenfd, ThreadPool &th_pool) {
        int num = epoll_wait(epollfd_, events_, MAX_EVENT_NUMBER + 1, -1);
        if (num == 0)
            return;
        else if (num < 0 && errno != EINTR) {
            perror("epoll wait failed.");
            return;
        }

        for (int i = 0; i < num; i++) {
            EchoService *rq = (EchoService *)events_[i].data.ptr;
            int          fd = rq->fd;

            if (fd == listenfd) {
                // handle connection
                // printf("Got a new connection.\n");
                struct sockaddr_in client_address;
                memset(&client_address, 0, sizeof(struct sockaddr_in));
                socklen_t client_addrlength = sizeof(client_address);
                int       connfd            = -1;

                // 处理connection必须用while一下子把所有的连接都接入
                while ((connfd = accept(
                            listenfd, (struct sockaddr *)&client_address, &client_addrlength)) >
                       0) {
                    if (setSockNonBlocking(connfd) < 0) {
                        perror("setnonblocking error.");
                        close(connfd);
                        continue;
                    }
                    EchoService *newreq = new EchoService(connfd);
                    uint32_t     events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    AddFd(connfd, newreq, events);
                }
            } else {
                // handle request
                // printf("Got a new request.\n");
                // 排除错误事件
                if ((events_[i].events & EPOLLERR) || (events_[i].events & EPOLLHUP) ||
                    (!(events_[i].events & EPOLLIN))) {
                    printf("Error event\n");
                    delete rq;
                    continue;
                }

                task_t task;
                task.func = handleEchoFunc;
                task.args = rq;
                th_pool.AddTask(task);
            }
        }
    }

private:
    epoll_event *events_  = NULL;
    int          epollfd_ = -1;
};


#endif