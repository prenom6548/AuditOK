/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "lifecycle/modellifecycleconfig.h"
#include "lifecycle/modelruntimecontext.h"
#include "lifecycle/modelusagehandle.h"

namespace lifecycle {

class ModelLifecycleManager {
public:
    static ModelLifecycleManager &getInstance();

    ModelLifecycleManager(const ModelLifecycleManager &) = delete;
    ModelLifecycleManager &operator=(const ModelLifecycleManager &) = delete;

    void loadDefaultConfig();
    void updateConfig(const ModelLifecycleConfig &config);

    ModelUsageHandle acquire();
    void acquireHandle(const ModelDescriptor &descriptor);
    void markBusy(const ModelDescriptor &descriptor);
    void markIdle(const ModelDescriptor &descriptor);
    void releaseHandle(const ModelDescriptor &descriptor);
    bool ensureLoaded(const ModelDescriptor &descriptor, EngineCallStage stage);

    bool startAsyncLoad(const ModelDescriptor &descriptor, EngineCallStage stage);
    bool isAsyncLoadComplete(const ModelDescriptor &descriptor) const;
    bool waitForAsyncLoad(const ModelDescriptor &descriptor, int timeoutMs = 0);

    void sweepIdleContexts();
    std::vector<ModelDescriptor> forceUnloadForSleep(const std::string &reason);
    void forceLoadModels(const std::vector<ModelDescriptor> &descriptors);

    void loadModelsOnBoot();

    ModelLifecycleConfig config() const;

    bool retainModelAndUnloadOthers(const ModelDescriptor &descriptor);
    bool cancelRetainModel(const ModelDescriptor &descriptor);

    void registerModelConfigCallback();
    void handleModelConfigChanged(ModelChangeType type) const;
    void forceLoadModelsParallel(const std::vector<ModelDescriptor> &descriptors);

private:
    ModelLifecycleManager() = default;

    std::shared_ptr<ModelRuntimeContext> findOrCreateContext(const ModelDescriptor &descriptor);
    std::shared_ptr<ModelRuntimeContext> findContextByModelName(const std::string &modelName);
    void protectModel(const ModelDescriptor &descriptor);
    void unprotectModel(const ModelDescriptor &descriptor);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<ModelRuntimeContext>> contexts_;
    ModelLifecycleConfig config_{ModelLifecycleConfig::loadFromFile(ModelLifecycleConfig::defaultConfigPath())};
    bool isCallbackRegistered_{false};
};

}  // namespace lifecycle
