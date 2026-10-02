#pragma once

#include <kyai-business-framework/log.h>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>

class PerformanceTimer {
public:
    static PerformanceTimer& getInstance() {
        static PerformanceTimer instance;
        return instance;
    }

    PerformanceTimer(const PerformanceTimer&) = delete;
    PerformanceTimer& operator=(const PerformanceTimer&) = delete;

    PerformanceTimer(PerformanceTimer&&) = delete;
    PerformanceTimer& operator=(PerformanceTimer&&) = delete;

    void start(const std::string& stage);
    void end(const std::string& stage);
    long long getDuration(const std::string& stage) const;
    void printReport() const;
    void clear();

private:
    PerformanceTimer() = default;

    ~PerformanceTimer() = default;

private:
    mutable std::mutex mutex_;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> startTimes_;
    std::map<std::string, long long> stageDurations_;  // 毫秒
};