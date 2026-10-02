#include "performancetimer.h"

void PerformanceTimer::start(const std::string& stage) {
    std::lock_guard<std::mutex> lock(mutex_);
    startTimes_[stage] = std::chrono::high_resolution_clock::now();
}

void PerformanceTimer::end(const std::string& stage) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto it = startTimes_.find(stage);
    if (it != startTimes_.end()) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - it->second);
        stageDurations_[stage] = duration.count();
        startTimes_.erase(it);
    }
}

long long PerformanceTimer::getDuration(const std::string& stage) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = stageDurations_.find(stage);
    return it != stageDurations_.end() ? it->second : -1;
}

void PerformanceTimer::printReport() const {
    std::lock_guard<std::mutex> lock(mutex_);  // 注意：mutable mutex 或这里加 mutable
    PRINT_INFO("===== 性能报告 =====\n");
    for (const auto& pair : stageDurations_) {
        PRINT_INFO("%s: %ld ms\n", pair.first.c_str(), pair.second);
    }
    PRINT_INFO("^^^^^ 性能报告 ^^^^^\n");
}

void PerformanceTimer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    startTimes_.clear();
    stageDurations_.clear();
}
