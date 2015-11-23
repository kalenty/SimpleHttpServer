# SimpleHttpServer

* 计算机网络课程项目
* 使用socket编写一个简单的http服务器
* 时间：第6周-第11周

| 实际完成日期        |预计完成日期| 进度  |
| ------------- |-------------| -------------|
| 10.12      | -| 创建github仓库|
| 11.23    | -  | 单线程单reactor模式server<sup>[1]</sup>|
|-|11.30|测试|
|-|遥遥无期|开发多线程版本<sup>[2]</sup>|
|-|遥遥遥无期|解耦，实现反射<sup>[3]</sup>|

- [1] 基于io多路复用（epoll）以及非阻塞io的http 1.0服务器
- [2] 预计使用线程池实现（一个主要线程作dispatcher，其余作为worker）
- [3] 此处设想为类似nodejs实现DI，且使用动态链接实现不关闭进程的热升级（待推敲）
 
# 参考资料

- http://www.cs.wustl.edu/~schmidt/PDF/reactor-siemens.pdf
- CSAPP chapter 11
- man epoll
- 《Linux多线程服务端编程：使用muduo C++网络库》（参考网络库原理部分）
