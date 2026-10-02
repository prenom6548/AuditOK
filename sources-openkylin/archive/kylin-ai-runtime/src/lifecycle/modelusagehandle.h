/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "lifecycle/modellifecycletypes.h"

namespace lifecycle {

class ModelLifecycleManager;
class ModelRuntimeContext;

class ModelUsageHandle {
public:
    ModelUsageHandle() = default;
    ~ModelUsageHandle();

    ModelUsageHandle(const ModelUsageHandle &) = delete;
    ModelUsageHandle &operator=(const ModelUsageHandle &) = delete;

    ModelUsageHandle(ModelUsageHandle &&other) noexcept;
    ModelUsageHandle &operator=(ModelUsageHandle &&other) noexcept;

    void markBusy(const ModelDescriptor &descriptor);
    void markIdle(const ModelDescriptor &descriptor);
    void reset();
    bool ensureLoaded(const ModelDescriptor &descriptor, EngineCallStage stage);

    bool startAsyncLoad(const ModelDescriptor &descriptor, EngineCallStage stage);
    bool isAsyncLoadComplete(const ModelDescriptor &descriptor) const;
    bool waitForAsyncLoad(const ModelDescriptor &descriptor, int timeoutMs = 0);

    bool valid() const { return manager_ != nullptr; }

private:
    friend class ModelLifecycleManager;
    explicit ModelUsageHandle(ModelLifecycleManager *manager);
    void ensureAcquired(const ModelDescriptor &descriptor);

private:
    ModelLifecycleManager *manager_{nullptr};
    std::unordered_map<std::string, ModelDescriptor> acquiredDescriptors_;
};

}  // namespace lifecycle
