/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/enginestopcontroller.h"

#include <chrono>
#include <future>
#include <utility>

#include "utils/logger.h"

namespace lifecycle {

EngineStopController &EngineStopController::getInstance() {
    static EngineStopController instance;
    return instance;
}

void EngineStopController::registerStopper(const ModelDescriptor &descriptor, std::function<void()> stopFn, bool incrementRefCount) {
    if (!stopFn) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = keyFromDescriptor(descriptor);
    auto &entry = stoppers_[key];

    if (entry) {
        if (incrementRefCount) {
            int newRefCount = entry->refCount.fetch_add(1, std::memory_order_relaxed) + 1;
            LOGI("EngineStopController registering...[{}], refCount now: {}", key, newRefCount);
        } else {
            LOGI("EngineStopController updating function...[{}], refCount remains: {}", key, entry->refCount.load());
        }
        {
            std::lock_guard<std::mutex> entry_lock(entry->entry_mutex_);
            entry->stopFunction = std::move(stopFn);
        }
    } else {
        LOGI("EngineStopController registering...[{}], refCount now: 1", key);
        entry = std::make_shared<StopperEntry>(std::move(stopFn));
    }
}

void EngineStopController::unregisterStopper(const ModelDescriptor &descriptor) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = keyFromDescriptor(descriptor);
    auto it = stoppers_.find(key);
    if (it != stoppers_.end()) {
        int newRefCount = it->second->refCount.fetch_sub(1, std::memory_order_relaxed) - 1;
        LOGI("EngineStopController unregistering...[{}], refCount now: {}", key, newRefCount);
        if (newRefCount <= 0) {
            stoppers_.erase(it);
        }
    }
}

void EngineStopController::stopAll(const std::string &reason) {
    std::unordered_map<std::string, std::shared_ptr<StopperEntry>> stoppers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stoppers = stoppers_;
    }
    for (const auto &item : stoppers) {
        const auto &key = item.first;
        std::function<void()> fn;
        {
            std::lock_guard<std::mutex> entry_lock(item.second->entry_mutex_);
            fn = item.second->stopFunction;
        }
        if (!fn) {
            LOGW("EngineStopController skip [{}]: stopFunction is empty, reason: {}", key, reason);
            continue;
        }
        try {
            LOGI("EngineStopController stopping...[{}], reason: {}", key, reason);
            auto future = std::async(std::launch::async, fn);
            // Bound stop to avoid blocking sleep; 5s should be sufficient for graceful stop.
            auto status = future.wait_for(std::chrono::seconds(5));
            if (status != std::future_status::ready) {
                LOGW("EngineStopController stop for [{}] timed out, reason: {}", key, reason);
            }
        } catch (const std::exception &ex) {
            LOGW("EngineStopController stop for [{}] threw: {}, reason: {}", key, ex.what(), reason);
        } catch (...) {
            LOGW("EngineStopController stop for [{}] threw unknown error, reason: {}", key, reason);
        }
    }
}

void EngineStopController::stopForModel(const ModelDescriptor &descriptor, const std::string &reason) {
    std::string key = keyFromDescriptor(descriptor);
    std::function<void()> fn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = stoppers_.find(key);
        if (iter == stoppers_.end()) {
            LOGW("EngineStopController stop for [{}] not found, reason: {}", key, reason);
            return;  // 没有找到对应的停止函数，直接返回
        }
        {
            std::lock_guard<std::mutex> entry_lock(iter->second->entry_mutex_);
            fn = iter->second->stopFunction;
        }
    }
    if (!fn) {
        LOGW("EngineStopController stop for [{}]: stopFunction is empty, reason: {}", key, reason);
        return;
    }
    try {
        auto future = std::async(std::launch::async, fn);
        // Bound stop to avoid blocking sleep; 5s should be sufficient for graceful stop.
        auto status = future.wait_for(std::chrono::seconds(5));
        if (status != std::future_status::ready) {
            LOGW("EngineStopController stop for [{}] timed out, reason: {}", key, reason);
        }
    } catch (const std::exception &ex) {
        LOGW("EngineStopController stop for [{}] threw: {}, reason: {}", key, ex.what(), reason);
    } catch (...) {
        LOGW("EngineStopController stop for [{}] threw unknown error, reason: {}", key, reason);
    }
}

std::string EngineStopController::keyFromDescriptor(const ModelDescriptor &descriptor) const {
    if (!descriptor.modelName.empty()) {
        return descriptor.fingerprint();
    }
    if (!descriptor.engineName.empty()) {
        return descriptor.engineName;
    }
    return "unknown";
}

}  // namespace lifecycle
