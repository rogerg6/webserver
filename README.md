# Introduction
用cpp写的linux下的webserver. 对网络, 多线程, linux环境编程的一个总结. 会持续迭代

## 目录
├── clients<br>
├── go-servers go版本的并发服务器(和cpp的比较)<br>
│   ├── echo-server 基于go net标准库写的<br>
│   ├── echo-server-gnet 基于第三方库gnet写的<br>
│   └── echo-server-netpoll 基于第三方库netpoll写的<br>
├── v1<br>
├── v2<br>
├── v3<br>
└── v4<br>

## v1
阻塞IO+单线程. 整个程序就一个线程, 循环阻塞accept + handle.

## v2
阻塞IO+多线程. 主线程循环阻塞accept, 对每一个connection起以thread进行处理.

## v3
阻塞IO+线程池(10个线程). 主线程循环阻塞accept, 对每一个connection扔给线程池中处理.


## v4
- epoll边缘触发 + 非阻塞IO
- 采用线程池处理请求. 主线程负责epoll事件分发, 把accept进来的连接传递给线程池, 线程池单独起n个线程处理请求. 
- http请求采用有限状态机算法(FSM)解析http报文格式. 目前只支持GET方法. 且只支持request较短的请求
- echo服务

- bug1 
  - desc<br> 
    webbench请求过后, 用netstat -natp | grep 127.0.0.1:8888查看会出现很多: `tcp        0      0 127.0.0.1:8888          127.0.0.1:47392         CLOSE_WAIT  3503351/./webserver`, 而且后续的curl单独请求会一直阻塞, 总的并发success很低, 只有2-3k.
  - 排查结果<br>通过抓包查看close_wait的端口, 发现client主动关闭发送FIN, server回复ACK, 但是server一直没有回复FIN, 说明没有调用close(); 同时lsof查看server打开的fd并没有对应的close_wait端口, 所以怀疑是所有的这些CLOSE_WAIT是在tcp连接就绪队列中, 但是还未被accept的连接, 此时client关闭后, 由establish状态迁移到close_wait状态. 但是**奇怪**的是epoll并没有检测到这些连接. <br>后来参考陈硕的书中说的, 出现这个现象的原因在于accept时到达最大的fd数, 此时listenfd上的还有连接没有accept进来, 然后accept循环退出, 导致以后的连接epoll ET都不会触发(**ET触发一定要一次性把所有的数据都读完**). 而同时连接就绪队列已满, 新的请求页得不到响应, 导致后续没有accept
<br>
  - 解决方案:
    1.  我把ulimit -n 65535, 设置进程允许打开的fd设置为65535, 就可以解决了. zbook单机可以达到10w/s的并发. 也不会出现bug2的情况. 但是这个解决方法治标不治本. 
    2. 把listenfd epoll改成LT. 这个现象就没有了, 因为LT只要有连接就读
    3. 限制最大连接数. 即超过这个阈值的连接数都拒绝, listenfd epoll还是ET, 但是结果就是压测的时候当客户端数量大于最大连接数时, 成功很少, 失败很多, 客户端数量<=最大连接数时, 并发成功率较高. 所以如果要支持多客户端并发, 目前的方案还是调整进程最大fd数量



## 性能比较
测试方法:<br>
`./webbench -t 5 -c 1000 -2 --get  http://127.0.0.1:8888/`   
1000个客户端连续请求5s, 统计成功失败个数

echo server: 最大打开文件描述符数量=1024, listen队列1024
| 版本                           | Total Succss req w | Total Failed w | Success w req/s | Failed w req/s |
| ------------------------------ | ------------------ | -------------- | --------------- | -------------- |
| go-servers/echo-server         | 37.1               | 0              | 7.42            | 0              |
| go-servers/echo-server-gnet    | 43.2               | 0              | 8.64            | 0              |
| go-servers/echo-server-netpoll | 20.0               | 0              | 4.0             | 0              |
| v1                             | 0.04               | 0              | 0.008           | 0              |
| v2                             | 21.6               | 0              | 4.32            | 0              |
| v3                             | 90.4               | 0              | 18.5            | 0              |
| v4  ET & 最大连接数1000限制    | 57.5               | 0.14           | 11.5            | 0.028          |
| v4  LT & 无最大连接数限制      | 54.2               | 0              | 10.84           | 0              |


注. 
1. go-servers/echo-server的测试有o-test的测试有CLOSE_WAIT, 是accpet后的, 估计是后续IO没有关闭
2. go的3个echo-servers的max-fds-num和listen队列大小都不知道



