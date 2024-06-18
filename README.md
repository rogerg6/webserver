# Introduction
用cpp写的linux下的webserver. 对网络, 多线程, linux环境编程的一个总结. 会持续迭代

# Version
### v0.1.0 
- epoll边缘触发 + EPOLLONESHOT + 非阻塞IO
- 采用线程池处理请求. 主线程负责accept连接, 把请求传递给线程池, 线程池单独起n个线程处理请求. 
- http请求采用有限状态机算法(FSM)解析http报文格式. 目前只支持GET方法.