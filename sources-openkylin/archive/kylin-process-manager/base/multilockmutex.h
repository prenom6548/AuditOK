#ifndef MULTILOCKMUTEX_H
#define MULTILOCKMUTEX_H

#include <vector>
#include <mutex>
#include <utility>
#include <thread>
#include <condition_variable>
#include <queue>
#include <array>

// 公平锁
class FairMutex{
public:
    FairMutex() = default;
    ~FairMutex() = default;

    void lock() {
        std::unique_lock<std::mutex> guard(m_internalMtx);
        m_waitingThreads.push(std::this_thread::get_id());
        m_cv.wait(guard, [this]() {
            return !m_waitingThreads.empty() && 
                m_waitingThreads.front() == std::this_thread::get_id();
        });
        m_locked = true;
        m_waitingThreads.pop();
    }

    void unlock() {
        std::unique_lock<std::mutex> guard(m_internalMtx);
        if (m_locked){
            m_locked = false;
            if (!m_waitingThreads.empty()) {
                m_cv.notify_one();
            }
        }
    }

    bool try_lock() {
        std::unique_lock<std::mutex> guard(m_internalMtx);
        if (!m_locked && m_waitingThreads.empty()) {
            m_locked = true;
            return true;
        }
        return false;
    }

private:
    std::mutex m_internalMtx;
    std::condition_variable m_cv;
    std::queue<std::thread::id> m_waitingThreads;
    bool m_locked {false};
};

template <size_t... Is>
struct IndexSequence {};

template <size_t N, size_t... Is>
struct MakeIndexSequence : MakeIndexSequence<N - 1, N - 1, Is...> {};

template <size_t... Is>
struct MakeIndexSequence<0, Is...> {
    using Type = IndexSequence<Is...>;
};

// 连续索引包
template <size_t N>
using MakeIndexSequenceT = typename MakeIndexSequence<N>::Type;

template <typename Mutex, size_t N>
class MultiLockMutex {
public:
    constexpr MultiLockMutex(std::array<Mutex, N>& locks)
        : m_locks(locks) {}

    MultiLockMutex(const MultiLockMutex&) = delete;
    MultiLockMutex& operator=(const MultiLockMutex&) = delete;

    void lock() {
        LockHelper<decltype(MakeIndexSequenceT<N>{})>::DoLock(m_locks);
    }

    void unlock() {
        for (size_t i = 0; i < N; ++i) {
            m_locks[i].unlock();
        }
    }

private:
    std::array<Mutex, N> &m_locks;

    template <typename Seq>
    struct LockHelper;

    template <size_t... Is>
    struct LockHelper<IndexSequence<Is...>> {
        static void DoLock(std::array<Mutex, N>& locks) {
            std::lock(locks[Is]...);
        }
    };

};

#endif // !MULTILOCKMUTEX_H