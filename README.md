# Introduction
用cpp写的linux下的webserver. 对网络, 多线程, linux环境编程的一个总结. 会持续迭代

## v1
阻塞IO+单线程. 整个程序就一个线程, 循环accept + handle.

压测数据, 最高并发量:
```
Request:
GET / HTTP/1.1
User-Agent: WebBench 1.5
Host: 127.0.0.1
Connection: close


Runing info: 3000 clients, running 5 sec.

Speed=1224 pages/min, 2336 bytes/sec.
Requests: 102 susceed, 0 failed.
```

## v2
阻塞IO+多线程. 主线程循环accept, 对每一个connection起以thread进行处理.

压测数据, 最高并发量:
```
Request:
GET / HTTP/1.1
User-Agent: WebBench 1.5
Host: 127.0.0.1
Connection: close


Runing info: 3000 clients, running 5 sec.

Speed=4025004 pages/min, 5366624 bytes/sec.
Requests: 335417 susceed, 0 failed.
```

## v3
阻塞IO+线程池(10个线程). 主线程循环accept, 对每一个connection扔给线程池中处理.

压测数据, 最高并发量:
```
Request:
GET / HTTP/1.1
User-Agent: WebBench 1.5
Host: 127.0.0.1
Connection: close


Runing info: 3000 clients, running 5 sec.

Speed=9145908 pages/min, 12194512 bytes/sec.
Requests: 762159 susceed, 0 failed.
```



## webserver
### v0.1.0 
- epoll边缘触发 + EPOLLONESHOT + 非阻塞IO
- 采用线程池处理请求. 主线程负责accept连接, 把请求传递给线程池, 线程池单独起n个线程处理请求. 
- http请求采用有限状态机算法(FSM)解析http报文格式. 目前只支持GET方法. 且只支持request较短的请求
- echo服务

- bug1: server开启10个线程处理请求, 3000clients, 5s并发量处理9w多请求, 但是不稳定, 有的时候只有2-3k
```
./webbench -t 5 -c 3000 -2 --get  http://127.0.0.1:8888/ 

Request:
GET / HTTP/1.1
User-Agent: WebBench 1.5
Host: 127.0.0.1
Connection: close


Runing info: 3000 clients, running 5 sec.

Speed=1099428 pages/min, 1465904 bytes/sec.
Requests: 91619 susceed, 0 failed.
```
- bug2: webbench请求过后, 用netstat -natp | grep 127.0.0.1:8888查看会出现很多: `tcp        0      0 127.0.0.1:8888          127.0.0.1:47392         CLOSE_WAIT  3503351/./webserver`, 而且后续的curl单独请求会一直阻塞<br>
- 上述2个bug, 我把ulimit -n 65535, 设置进程允许打开的fd设置为65535, 就可以解决了. zbook单机可以达到10w/s的并发. 也不会出现bug2的情况.