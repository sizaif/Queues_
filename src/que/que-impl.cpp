/*** 
 * @                               |~~~~~~~|
 * @                               |       |
 * @                               |       |
 * @                               |       |
 * @                               |       |
 * @                               |       |
 * @    |~.\\\_\~~~~~~~~~~~~~~xx~~~         ~~~~~~~~~~~~~~~~~~~~~/_//;~|
 * @    |  \  o \_         ,XXXXX),                         _..-~ o /  |
 * @    |    ~~\  ~-.     XXXXX`)))),                 _.--~~   .-~~~   |
 * @     ~~~~~~~`\   ~\~~~XXX' _/ ';))     |~~~~~~..-~     _.-~ ~~~~~~~
 * @              `\   ~~--`_\~\, ;;;\)__.---.~~~      _.-~
 * @                ~-.       `:;;/;; \          _..-~~
 * @                   ~-._      `''        /-~-~
 * @                       `\              /  /
 * @                         |         ,   | |
 * @                          |  '        /  |
 * @                           \/;          |
 * @                            ;;          |
 * @                            `;   .       |
 * @                            |~~~-----.....|
 * @                           | \             \
 * @                          | /\~~--...__    |
 * @                          (|  `\       __-\|
 * @                          ||    \_   /~    |
 * @                          |)     \~-'      |
 * @                           |      | \      '
 * @                           |      |  \    :
 * @                            \     |  |    |
 * @                             |    )  (    )
 * @                              \  /;  /\  |
 * @                              |    |/   |
 * @                              |    |   |
 * @                               \  .'  ||
 * @                               |  |  | |
 * @                               (  | |  |
 * @                               |   \ \ |
 * @                               || o `.)|
 * @                               |`\\) |
 * @                               |       |
 * @                               |       |
 * @
 * @Author: sizaif
 * @Date: 2023-08-30 21:33:23
 * @LastEditTime: 2023-08-30 21:34:32
 * @Description: 
 * @
 */




#include "../include/que.h" 
#include "../include/atom_opt.h"     // 原子操作


/***
 *  start Class SafeQueue  implement
 * ***********************************************************************************
 */

template <typename T>
void SafeQueue<T>::push(const T &value)
{
    // 实现push函数的代码
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(value);
    lock.unlock();
    m_cond.notify_one();
}

template <typename T>
bool SafeQueue<T>::try_pop(T &value)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty())
    {
        return false;
    }
    value = move(m_queue.front());
    m_queue.pop();
    return true;
}

template <typename T>
void SafeQueue<T>::wait_and_pop(T &value)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty() && !exit_)
    {
        m_cond.wait(lock);
    }
    if (exit_)
        return;
    // m_cond.wait(lock, [this](){ return ( !m_queue.empty() && !exit_); });
    if (!m_queue.empty())
    {
        value = std::move(m_queue.front());
        m_queue.pop();
    }
}

template <typename T>
bool SafeQueue<T>::empty() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template <typename T>
void SafeQueue<T>::notify_all()
{
    m_cond.notify_all();
}
template <typename T>
int SafeQueue<T>::size()
{
    return m_queue.size();
}
template <typename T>
void SafeQueue<T>::stop()
{
    exit_ = true;
}

/***
 *  end Class SafeQueue  implement
 * ***********************************************************************************
 */


/***
 *  start Class ArrayLockFreeQueue  implement
 * ***********************************************************************************
 */
template <typename ELEM_T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueue<ELEM_T, Q_SIZE>::ArrayLockFreeQueue() : m_writeIndex(0), m_readIndex(0), m_maximumReadIndex(0)
{
    m_count = 0;
    exit_ = false;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueue<ELEM_T, Q_SIZE>::~ArrayLockFreeQueue() {}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
inline QUEUE_INT ArrayLockFreeQueue<ELEM_T, Q_SIZE>::countToIndex(QUEUE_INT a_count)
{
    return (a_count % Q_SIZE);
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
QUEUE_INT ArrayLockFreeQueue<ELEM_T, Q_SIZE>::size()
{
    QUEUE_INT currentWriteIndex = m_writeIndex;
    QUEUE_INT currentReadIndex = m_readIndex;

    if (currentWriteIndex >= currentReadIndex)
        return currentWriteIndex - currentReadIndex;
    else
        return Q_SIZE + currentWriteIndex - currentReadIndex;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueue<ELEM_T, Q_SIZE>::enqueue(const ELEM_T &a_data)
{
    QUEUE_INT currentWriteIndex;
    QUEUE_INT currentReadIndex;
    do
    {
        currentWriteIndex = m_writeIndex;
        currentReadIndex = m_readIndex;
        if (countToIndex(currentWriteIndex + 1) == countToIndex(currentReadIndex))
        {
            return false;
        }
    } while (!CAS(&m_writeIndex, currentWriteIndex, (currentWriteIndex + 1)));

    m_thequeue[countToIndex(currentWriteIndex)] = a_data;

    while (!CAS(&m_maximumReadIndex, currentWriteIndex, (currentWriteIndex + 1)))
    {
        sched_yield();
    }
    AtomicAdd(&m_count, 1);
    return true;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueue<ELEM_T, Q_SIZE>::try_dequeue(ELEM_T &a_data)
{
    return dequeue(a_data);
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueue<ELEM_T, Q_SIZE>::dequeue(ELEM_T &a_data)
{
    QUEUE_INT currentMaximumReadIndex;
    QUEUE_INT currentReadIndex;

    do
    {
        currentReadIndex = m_readIndex;
        currentMaximumReadIndex = m_maximumReadIndex;

        if (countToIndex(currentReadIndex) ==
            countToIndex(currentMaximumReadIndex))
        {
            return false;
        }

        a_data = m_thequeue[countToIndex(currentReadIndex)];

        if (CAS(&m_readIndex, currentReadIndex, (currentReadIndex + 1)))
        {
            AtomicSub(&m_count, 1);
            return true;
        }
    } while (true);

    assert(0);

    return false;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
void ArrayLockFreeQueue<ELEM_T, Q_SIZE>::stop()
{
    exit_ = true;
}
/***
 *  end Class ArrayLockFreeQueue  implement
 * ***********************************************************************************
 */

/***
 *  start Class ArrayLockFreeQueuePtr  implement
 * ***********************************************************************************
 */
template <typename ELEM_T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::ArrayLockFreeQueuePtr() : m_pool(Q_SIZE), m_alloc(&m_pool), exit_(false), m_count(0),
                                                                 m_writeIndex(0), m_readIndex(0), m_maximumReadIndex(0)
{

    // use C++17 memory pool
    {
        for (QUEUE_INT i = 0; i < Q_SIZE; ++i)
        {
            // 使用C++17 内存池分配内存
            auto ptr = m_alloc.allocate(1);
            // 使用 std::shared_ptr<ELEM_T> 的定制 deleter
            new (std::addressof(m_thequeue[i])) std::shared_ptr<ELEM_T>(reinterpret_cast<ELEM_T *>(ptr), [this](ELEM_T *p)
                                                                        {
                // 调用内存池的 deallocate 函数
                m_alloc.deallocate(reinterpret_cast<std::shared_ptr<ELEM_T>*>(p), 1); });
        }
    }
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::~ArrayLockFreeQueuePtr()
{

}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
inline QUEUE_INT ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::countToIndex(QUEUE_INT a_count)
{
    return (a_count % Q_SIZE);
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
QUEUE_INT ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::size()
{
    QUEUE_INT currentWriteIndex = m_writeIndex;
    QUEUE_INT currentReadIndex = m_readIndex;

    if (currentWriteIndex >= currentReadIndex)
        return currentWriteIndex - currentReadIndex;
    else
        return Q_SIZE + currentWriteIndex - currentReadIndex;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::enqueue_sharedptr(const std::shared_ptr<ELEM_T> &a_data)
{

    QUEUE_INT currentWriteIndex;
    QUEUE_INT currentReadIndex;
    do
    {
        currentWriteIndex = m_writeIndex;
        currentReadIndex = m_readIndex;
        if (countToIndex(currentWriteIndex + 1) == countToIndex(currentReadIndex))
        {
            return false;
        }
    } while (!CAS(&m_writeIndex, currentWriteIndex, (currentWriteIndex + 1)));

    // 使用C++17 内存池分配内存并构造元素
    {
        auto ptr = m_alloc.allocate(1);
        // 使用 std::shared_ptr<ELEM_T> 的定制 deleter
        new (ptr) std::shared_ptr<ELEM_T>(reinterpret_cast<ELEM_T *>(ptr), [this](ELEM_T *p)
                                          {
            // 调用内存池的 deallocate 函数
            m_alloc.deallocate(reinterpret_cast<std::shared_ptr<ELEM_T>*>(p), 1); });
        // 使用指针 p 构造 std::shared_ptr
        *reinterpret_cast<std::shared_ptr<ELEM_T> *>(ptr) = a_data;
        // 将指针 p 存储在环形队列中
        new (std::addressof(m_thequeue[countToIndex(currentWriteIndex)])) std::shared_ptr<ELEM_T>(
            std::move(*reinterpret_cast<std::shared_ptr<ELEM_T> *>(ptr)));
    }

    // 未使用内存池
    {
        // *m_thequeue[countToIndex(currentWriteIndex)] = std::move(*new_data);
    }

    while (!CAS(&m_maximumReadIndex, currentWriteIndex, (currentWriteIndex + 1)))
    {
        sched_yield();
    }
    AtomicAdd(&m_count, 1);
    return true;
}

template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::try_dequeue_sharedptr(std::shared_ptr<ELEM_T> &a_data)
{
    return dequeue_sharedptr(a_data);
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::dequeue_sharedptr(std::shared_ptr<ELEM_T> &a_data)
{
    QUEUE_INT currentMaximumReadIndex;
    QUEUE_INT currentReadIndex;

    do
    {
        currentReadIndex = m_readIndex;
        currentMaximumReadIndex = m_maximumReadIndex;

        if (countToIndex(currentReadIndex) ==
            countToIndex(currentMaximumReadIndex))
        {
            return false;
        }
        // use C++17 memory pool
        {
            a_data = std::move(*reinterpret_cast<std::shared_ptr<ELEM_T> *>(&m_thequeue[countToIndex(currentReadIndex)]));
        }

        // not use memory pool
        {
            // a_data = std::make_unique<ELEM_T>(std::move(*m_thequeue[countToIndex(currentReadIndex)]));
        }
        if (CAS(&m_readIndex, currentReadIndex, (currentReadIndex + 1)))
        {
            AtomicSub(&m_count, 1);
            return true;
        }
    } while (true);

    assert(0);

    return false;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
void ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::stop()
{
    exit_ = true;
}

template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::enqueue_ptr(ELEM_T *a_data)
{

    QUEUE_INT currentWriteIndex;
    QUEUE_INT currentReadIndex;
    do
    {
        currentWriteIndex = m_writeIndex;
        currentReadIndex = m_readIndex;
        if (countToIndex(currentWriteIndex + 1) == countToIndex(currentReadIndex))
        {
            return false;
        }
    } while (!CAS(&m_writeIndex, currentWriteIndex, (currentWriteIndex + 1)));

    // 使用C++17 内存池分配内存并构造元素
    {
        // 分配一块内存
        ELEM_T *ptr = afque_allocator.allocate(1);
        // 这块内存上构建 ELEM_T对象
        // new (ptr) ELEM_T(*a_data);
        new (ptr) ELEM_T(std::move(*a_data));

        // 环形队列存储的是指向ELEM_T类型的指针。
        // 构建一个指针 指向这个ELEM_T对象 并将其存对队列中
        new (std::addressof(m_thequeue_v2[countToIndex(currentWriteIndex)])) ELEM_T *(ptr);
    }
    while (!CAS(&m_maximumReadIndex, currentWriteIndex, (currentWriteIndex + 1)))
    {
        sched_yield();
    }
    AtomicAdd(&m_count, 1);
    return true;
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::try_dequeue_ptr(ELEM_T *&a_data)
{
    return dequeue_ptr(a_data);
}
template <typename ELEM_T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueuePtr<ELEM_T, Q_SIZE>::dequeue_ptr(ELEM_T *&a_data)
{
    QUEUE_INT currentMaximumReadIndex;
    QUEUE_INT currentReadIndex;

    do
    {
        currentReadIndex = m_readIndex;
        currentMaximumReadIndex = m_maximumReadIndex;

        if (countToIndex(currentReadIndex) ==
            countToIndex(currentMaximumReadIndex))
        {
            return false;
        }
        // use C++17 memory pool
        {
            // 获取存储的元素指针
            ELEM_T *ptr = *reinterpret_cast<ELEM_T **>(&m_thequeue_v2[countToIndex(currentReadIndex)]);
            if (ptr == nullptr)
            {
                return false;
            }
            a_data = ptr;
        }

        if (CAS(&m_readIndex, currentReadIndex, (currentReadIndex + 1)))
        {
            AtomicSub(&m_count, 1);
            return true;
        }
    } while (true);

    assert(0);

    return false;
}

/***
 *  end Class ArrayLockFreeQueuePtr  implement
 * ***********************************************************************************
 */

