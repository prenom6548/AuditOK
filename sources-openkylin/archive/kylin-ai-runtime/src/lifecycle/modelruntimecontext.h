/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "lifecycle/modellifecycletypes.h"
#include "lifecycle/modelresourcecontroller.h"

namespace lifecycle {

class ModelRuntimeContext : public std::enable_shared_from_this<ModelRuntimeContext> {
public:
    ModelRuntimeContext(const ModelDescriptor &descriptor, const ModelClass &modelClass,
                        EngineCallStage loadTriggerStage, UnloadPolicy unloadPolicy);

    void onAcquire();
    void onRelease();
    void markBusy();
    void markIdle();
    void forceLoad();
    bool ensureLoaded(EngineCallStage stage);

    bool startAsyncLoad(EngineCallStage stage);
    bool isAsyncLoadComplete() const;
    bool waitForAsyncLoad(int timeoutMs = 0);

    bool canUnload() const;
    bool shouldUnload(std::chrono::steady_clock::time_point now, std::chrono::seconds idleTimeout) const;
    bool shouldUnload() const;

    void forceUnload();

    ModelClass modelClass() const { return modelClass_; }
    const ModelDescriptor &descriptor() const { return descriptor_; }

    void setProtected(bool status) { isProtected_ = status; }
    bool isProtected() const { return isProtected_; }

    void setDisabled(bool disabled) { isDisabled_ = disabled; }
    bool isDisabled() const { return isDisabled_; }
    void checkAndUnloadIfDisabled();

private:
    void setState(LifecycleState state);

private:
    ModelDescriptor descriptor_;
    ModelClass modelClass_;
    UnloadPolicy unloadPolicy_{UnloadPolicy::kUnloadWhenIdle};
    EngineCallStage loadTriggerStage_{EngineCallStage::kOnInitialization};
    mutable std::shared_mutex mutex_;
    LifecycleState state_{LifecycleState::kUnloaded};
    std::atomic_int activeTaskCount_{0};
    std::atomic_int handleRefCount_{0};
    std::chrono::steady_clock::time_point lastAccess_;
    ModelResourceController loadController_;
    std::atomic_bool isProtected_{false};
    std::atomic_bool isDisabled_{false};
    std::shared_future<void> asyncLoadFuture_;
    std::atomic_bool asyncLoadComplete_{false};
    std::atomic_bool asyncLoadSuccess_{false};
};

}  // namespace lifecycle
