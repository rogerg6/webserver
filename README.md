# Introduction
用cpp写的linux下的webserver. 对网络, 多线程, linux环境编程的一个总结. 会持续迭代

# Version
### v0.1.0 
- epoll边缘触发 + EPOLLONESHOT + 非阻塞IO
- 采用线程池处理请求. 主线程负责accept连接, 把请求传递给线程池, 线程池单独起n个线程处理请求. 
- http请求采用有限状态机算法(FSM)解析http报文格式. 目前只支持GET方法.

- bug1: server开启10个线程处理请求, 5s并发量处理4w多请求, 但是不稳定, 有的时候只有2-3k
```
./webbench -t 5 -c 3000 -2 --get  http://127.0.0.1:8888/ 

Request:
GET / HTTP/1.1
User-Agent: WebBench 1.5
Host: 127.0.0.1
Connection: close


Runing info: 3000 clients, running 5 sec.

Speed=492168 pages/min, 1148392 bytes/sec.
Requests: 41014 susceed, 0 failed.
```
- bug2: webbench请求过后, curl单独请求会一直阻塞<br>
- 上述2个bug, 我把ulimit -n 65535, 设置进程允许打开的fd设置为65535, 就可以解决了. zbook单机可以达到10w/s的并发. 也不会出现bug2的情况.