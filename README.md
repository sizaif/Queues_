<!--
 *                                |~~~~~~~|
 *                                |       |
 *                                |       |
 *                                |       |
 *                                |       |
 *                                |       |
 *     |~.\\\_\~~~~~~~~~~~~~~xx~~~         ~~~~~~~~~~~~~~~~~~~~~/_//;~|
 *     |  \  o \_         ,XXXXX),                         _..-~ o /  |
 *     |    ~~\  ~-.     XXXXX`)))),                 _.--~~   .-~~~   |
 *      ~~~~~~~`\   ~\~~~XXX' _/ ';))     |~~~~~~..-~     _.-~ ~~~~~~~
 *               `\   ~~--`_\~\, ;;;\)__.---.~~~      _.-~
 *                 ~-.       `:;;/;; \          _..-~~
 *                    ~-._      `''        /-~-~
 *                        `\              /  /
 *                          |         ,   | |
 *                           |  '        /  |
 *                            \/;          |
 *                             ;;          |
 *                             `;   .       |
 *                             |~~~-----.....|
 *                            | \             \
 *                           | /\~~--...__    |
 *                           (|  `\       __-\|
 *                           ||    \_   /~    |
 *                           |)     \~-'      |
 *                            |      | \      '
 *                            |      |  \    :
 *                             \     |  |    |
 *                              |    )  (    )
 *                               \  /;  /\  |
 *                               |    |/   |
 *                               |    |   |
 *                                \  .'  ||
 *                                |  |  | |
 *                                (  | |  |
 *                                |   \ \ |
 *                                || o `.)|
 *                                |`\\) |
 *                                |       |
 *                                |       |
 * 
 * @Author: sizaif
 * @Date: 2023-08-31 23:24:14
 * @LastEditTime: 2023-08-31 23:30:11
 * @Description: 
 * 
 -->

# Queues_
线程间/进程间消息队列通信


# 2023-08-30 21:03:35

## 线程间通信，单读单写
  1. 采用无锁环形队列
     1. 数据使用结构体指针-A
     2. 数据使用结构体智能指针-B
     3. 使用C++17内存池优化
     4. 使用类封装
  2. 采用锁保证线程读写安全

## 进程间通信，单读单写
  1. 采用基于共享内存的环形队列
     1. 用mmap共享内存方式通信
     2. 用shmget共享内存方式通信

 
     

    
  

## 编译参数：

`-std=c++17`

`-lrt`

`-lpthread`



