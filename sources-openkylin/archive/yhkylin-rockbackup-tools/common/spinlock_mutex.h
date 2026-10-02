/**
 * @author 赵民勇
 * @note 基于c++11搞的原子自旋锁, 可用于std::shared_lock<spinlock_mutex>、std::lock_guard<spinlock_mutex>等
*/

#pragma once

#ifndef __SPINLOCK_MUTEX_H__
#define __SPINLOCK_MUTEX_H__

#include <atomic>
#include <mutex>

class spinlock_mutex
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT; //注意：不能用初始化列表初始化，拷贝构造和复制构造函数已经删除

public:
    spinlock_mutex() {}

    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

    bool try_lock()
    {
        return !flag.test_and_set(std::memory_order_acquire);
    }
};

#endif //!__SPINLOCK_MUTEX_H__
