/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "lifecycle/modellifecycletypes.h"

namespace lifecycle {

// Thread-safe registry for stopping in-flight engine operations.
class EngineStopController {
public:
    struct StopperEntry {
        mutable std::mutex entry_mutex_;
        std::function<void()> stopFunction;
        std::atomic<int> refCount{1};

        explicit StopperEntry(std::function<void()> fn) : stopFunction(std::move(fn)) {}
    };
    static EngineStopController &getInstance();

    EngineStopController(const EngineStopController &) = delete;
    EngineStopController &operator=(const EngineStopController &) = delete;

    void registerStopper(const ModelDescriptor &descriptor, std::function<void()> stopFn, bool incrementRefCount = true);
    void unregisterStopper(const ModelDescriptor &descriptor);
    void stopAll(const std::string &reason);

    void stopForModel(const ModelDescriptor &descriptor, const std::string &reason);

private:
    EngineStopController() = default;

    std::string keyFromDescriptor(const ModelDescriptor &descriptor) const;

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<StopperEntry>> stoppers_;
};

}  // namespace lifecycle
