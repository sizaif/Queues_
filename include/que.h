/*** 
 * @          佛曰:  
 * @                  写字楼里写字间，写字间里程序员；  
 * @                  程序人员写程序，又拿程序换酒钱。  
 * @                  酒醒只在网上坐，酒醉还来网下眠；  
 * @                  酒醉酒醒日复日，网上网下年复年。  
 * @                  但愿老死电脑间，不愿鞠躬老板前；  
 * @                  奔驰宝马贵者趣，公交自行程序员。  
 * @                  别人笑我忒疯癫，我笑自己命太贱；  
 * @                  不见满街漂亮妹，哪个归得程序员？
 * @
 * @Author: sizaif
 * @Date: 2023-08-30 21:28:59
 * @LastEditTime: 2023-08-30 21:31:26
 * @Description: 
 * @
 */

/*** 
 * @
 * @   ┏┓　　　┏┓
 * @ ┏┛┻━━━┛┻┓
 * @ ┃　　　　　　　┃
 * @ ┃　　　━　　　┃
 * @ ┃　＞　　　＜　┃
 * @ ┃　　　　　　　┃
 * @ ┃...　⌒　...　┃
 * @ ┃　　　　　　　┃
 * @ ┗━┓　　　┏━┛
 * @     ┃　　　┃　
 * @     ┃　　　┃
 * @     ┃　　　┃
 * @     ┃　　　┃  神兽保佑
 * @     ┃　　　┃  代码无bug　　
 * @     ┃　　　┃
 * @     ┃　　　┗━━━┓
 * @     ┃　　　　　　　┣┓
 * @     ┃　　　　　　　┏┛
 * @     ┗┓┓┏━┳┓┏┛
 * @       ┃┫┫　┃┫┫
 * @       ┗┻┛　┗┻┛
 * @
 * @Author: sizaif
 * @Date: 2023-08-30 21:27:58
 * @LastEditTime: 2023-08-30 21:28:59
 * @Description: 
 * @
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__


#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <random>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>

// C++17 
#include <memory_resource> 


typedef unsigned long long u64;


// memory pool
// C++17 mempory pool for all
std::pmr::monotonic_buffer_resource     memory_pool(1310720);
std::pmr::polymorphic_allocator<u64>    vec_allocator(&memory_pool);
std::pmr::polymorphic_allocator<Data>   afque_allocator(&memory_pool);
// 数据块
struct Data
{
    Data() : end_(false),io_msg(false),inst_type(-1),extern_type(-1)
    {

        write_context.reserve(400);
        vu1_size = vu2_size = vu3_size = 0;

    };
    
    std::string write_context;

    u64 vu1[200];
    u64 vu2[200];
    u64 vu3[200];

    size_t vu1_size;
    size_t vu2_size;
    size_t vu3_size;

    int sock_count;
    bool end_;       
    bool io_msg;     
    int inst_type;   
    int extern_type; 
    int seq_;        
    
    void clear()
    {
        write_context.clear();
        memset(vu1, 0, sizeof(vu1));
        memset(vu2, 0, sizeof(vu2));
        memset(vu3, 0, sizeof(vu3));
        vu1_size = vu2_size = vu3_size = 0;
        sock_count = 0;
        end_ = false;
        io_msg = false;
        inst_type = -1;
        extern_type = -1;
        seq_ = 0;
    }
};

// 线程安全的队列模板类
template <typename T>
class SafeQueue
{
public:
    SafeQueue() : exit_(false) {}
    // 入队列
    void push(const T &value) ;
    // 出队列
    bool try_pop(T &value) ;
    void wait_and_pop(T &value);
    bool empty() const ;
    void notify_all();
    void stop();
    int  size();
private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
    bool exit_;
};

#define QUEUE_INT unsigned long
#define ARRAY_LOCK_FREE_Q_DEFAULT_SIZE 2097152 // 2^21
//环形无锁队列模板类，实现快读快写
template <typename ELEM_T, QUEUE_INT Q_SIZE = ARRAY_LOCK_FREE_Q_DEFAULT_SIZE>
class ArrayLockFreeQueue
{
public:

    ArrayLockFreeQueue();
    virtual ~ArrayLockFreeQueue();

    QUEUE_INT size();
    // 入队列
    bool enqueue(const ELEM_T &a_data);

    bool dequeue(ELEM_T &a_data);
    // 出队列
    bool try_dequeue(ELEM_T &a_data);

    void stop();

private:

    ELEM_T m_thequeue[Q_SIZE];
    bool                exit_;
    volatile QUEUE_INT m_count;

    volatile QUEUE_INT m_writeIndex;

    volatile QUEUE_INT m_readIndex;

    volatile QUEUE_INT m_maximumReadIndex;

    inline QUEUE_INT countToIndex(QUEUE_INT a_count);
};

//环形无锁队列模板类，实现快读快写,采用智能指针管理,使用内存池
template <typename ELEM_T, QUEUE_INT Q_SIZE = ARRAY_LOCK_FREE_Q_DEFAULT_SIZE>
class ArrayLockFreeQueuePtr{

public:
    ArrayLockFreeQueuePtr();
    virtual ~ArrayLockFreeQueuePtr();

    QUEUE_INT size();
    // 入队列
    bool enqueue_sharedptr(const std::shared_ptr<ELEM_T>& a_data);    // 共享指针
    bool dequeue_sharedptr(std::shared_ptr<ELEM_T>& a_data);          // 共享指针
    bool try_dequeue_sharedptr(std::shared_ptr<ELEM_T>& a_data);      // 共享指针

    bool enqueue_ptr(ELEM_T* a_data);      // 裸指针
    bool dequeue_ptr(ELEM_T*& a_data);     // 裸指针
    bool try_dequeue_ptr(ELEM_T*& a_data); // 裸指针


    void stop();
private:
    // new 连续对齐的内存
    std::aligned_storage_t<sizeof(std::shared_ptr<ELEM_T>), alignof(std::shared_ptr<ELEM_T>)> m_thequeue[Q_SIZE];

    ELEM_T* m_thequeue_v2[Q_SIZE];

    // C++ 17 memory pool for free lock queue
    std::pmr::monotonic_buffer_resource m_pool;
    // std::pmr::polymorphic_allocator<std::shared_ptr<ELEM_T>> m_alloc{&m_pool};  // 使用自己的内存池
    std::pmr::polymorphic_allocator<std::shared_ptr<ELEM_T>> m_alloc{&memory_pool};  // 使用全局的 memory_pool


    bool exit_;
    volatile QUEUE_INT m_count;
    volatile QUEUE_INT m_writeIndex;
    volatile QUEUE_INT m_readIndex;
    volatile QUEUE_INT m_maximumReadIndex;

    inline QUEUE_INT countToIndex(QUEUE_INT a_count);
};


#endif