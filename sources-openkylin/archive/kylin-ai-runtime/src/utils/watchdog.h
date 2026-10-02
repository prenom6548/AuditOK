/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILS_WATCHDOG_H
#define UTILS_WATCHDOG_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace util {

class Watchdog {
public:
    Watchdog() = default;
    ~Watchdog() { Finish(); }

    Watchdog(const Watchdog&) = delete;
    Watchdog& operator=(const Watchdog&) = delete;

    void Start(int64_t timeoutMs, std::function<void()> onTimeout) {
        if (thread_.joinable()) {
            Finish();
        }
        finished_.store(false);
        thread_ = std::thread([this, timeoutMs, onTimeout = std::move(onTimeout)]() mutable {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return finished_.load(); })) {
                lock.unlock();
                if (TryFinish()) {
                    onTimeout();
                }
            }
        });
    }

    bool TryFinish() { return !finished_.exchange(true); }

    bool IsFinished() const { return finished_.load(); }

    void Finish() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            finished_.store(true);
        }
        cv_.notify_all();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    std::atomic_bool finished_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
};

}  // namespace util

#endif  // UTILS_WATCHDOG_H
