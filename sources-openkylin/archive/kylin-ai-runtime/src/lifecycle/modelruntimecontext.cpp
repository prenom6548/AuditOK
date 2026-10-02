/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modelruntimecontext.h"

#include "utils/logger.h"

namespace lifecycle {

ModelRuntimeContext::ModelRuntimeContext(const ModelDescriptor &descriptor, const ModelClass &modelClass,
                                         EngineCallStage loadTriggerStage, UnloadPolicy unloadPolicy)
    : descriptor_(descriptor),
      modelClass_(modelClass),
      unloadPolicy_(unloadPolicy),
      loadTriggerStage_(loadTriggerStage),
      loadController_(descriptor) {
    lastAccess_ = std::chrono::steady_clock::now();
}

void ModelRuntimeContext::onAcquire() {
    handleRefCount_.fetch_add(1, std::memory_order_relaxed);
    std::unique_lock<std::shared_mutex> lock(mutex_);
    lastAccess_ = std::chrono::steady_clock::now();
}

void ModelRuntimeContext::onRelease() {
    int current = handleRefCount_.fetch_sub(1, std::memory_order_relaxed) - 1;
    if (current < 0) {
        handleRefCount_ = 0;
    }
    if (current <= 0 && state_ == LifecycleState::kLoaded) {
        setState(LifecycleState::kIdle);
    }
}

void ModelRuntimeContext::markBusy() {
    activeTaskCount_.fetch_add(1, std::memory_order_relaxed);
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (state_ == LifecycleState::kLoaded || state_ == LifecycleState::kIdle) {
        state_ = LifecycleState::kActive;
        lastAccess_ = std::chrono::steady_clock::now();
    }
}

void ModelRuntimeContext::markIdle() {
    int current = activeTaskCount_.fetch_sub(1, std::memory_order_relaxed) - 1;
    if (current < 0) {
        activeTaskCount_ = 0;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (activeTaskCount_.load(std::memory_order_relaxed) == 0) {
        lastAccess_ = std::chrono::steady_clock::now();
        state_ = LifecycleState::kIdle;
    }
}

void ModelRuntimeContext::forceLoad() {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    // 如果正在加载中，不重复加载
    if (state_ == LifecycleState::kLoading) {
        LOGD("Model [{}] is already loading, skipping forceLoad.", descriptor_.fingerprint());
        lastAccess_ = std::chrono::steady_clock::now();
        return;
    }

    loadController_.load();
    if (state_ == LifecycleState::kUnloaded || state_ == LifecycleState::kIdle) {
        state_ = LifecycleState::kLoaded;
    }
    lastAccess_ = std::chrono::steady_clock::now();
}

bool ModelRuntimeContext::canUnload() const {
    return activeTaskCount_.load(std::memory_order_relaxed) == 0 && state_ != LifecycleState::kLoading &&
           state_ != LifecycleState::kUnloading;
}

bool ModelRuntimeContext::shouldUnload(std::chrono::steady_clock::time_point now,
                                       std::chrono::seconds idleTimeout) const {
    if (isProtected()) {
        LOGI("Model [{}] is protected, skip unload.", descriptor_.fingerprint());
        return false;
    }
    // 根据配置的卸载策略判断
    switch (unloadPolicy_) {
        case UnloadPolicy::kNeverUnload:
            // 永不自动卸载
            return false;

        case UnloadPolicy::kUnloadWhenIdle:
            // 只要没有活跃会话即可卸载（不检查handleRefCount）
            break;

        case UnloadPolicy::kUnloadWhenNoHandleAndIdle:
            // 必须没有handle持有才能卸载
            if (handleRefCount_.load(std::memory_order_relaxed) > 0) {
                return false;
            }
            break;
        case UnloadPolicy::kUnloadWhenIdleAndDisable:
            if (!isDisabled()) {
                return false;
            }
            break;
    }

    // 空闲超时 <= 0 表示不自动卸载
    if (idleTimeout.count() <= 0) {
        return false;
    }

    // 有活跃会话时不能卸载
    if (activeTaskCount_.load(std::memory_order_relaxed) > 0) {
        return false;
    }

    // 正在加载或卸载中不能再次卸载
    if (state_ == LifecycleState::kLoading || state_ == LifecycleState::kUnloading) {
        return false;
    }

    // 根据卸载策略检查状态
    switch (unloadPolicy_) {
        case UnloadPolicy::kNeverUnload:
            return false;

        case UnloadPolicy::kUnloadWhenIdle:
            // kLoaded或kIdle状态都可以卸载
            if (state_ != LifecycleState::kIdle && state_ != LifecycleState::kLoaded) {
                return false;
            }
            break;

        case UnloadPolicy::kUnloadWhenNoHandleAndIdle:
            // 只在kIdle状态卸载
            if (state_ != LifecycleState::kIdle) {
                return false;
            }
            break;

        case UnloadPolicy::kUnloadWhenIdleAndDisable:
            // 只在kIdle状态卸载
            if (state_ != LifecycleState::kIdle) {
                return false;
            }
            break;
    }

    // 检查是否达到空闲时间
    return (now - lastAccess_) >= idleTimeout;
}

bool ModelRuntimeContext::shouldUnload() const {
    if (isProtected()) {
        LOGI("Model [{}] is protected, skip unload.", descriptor_.fingerprint());
        return false;
    }
    // 根据配置的卸载策略判断
    switch (unloadPolicy_) {
        case UnloadPolicy::kNeverUnload:
            // 永不自动卸载
            return false;

        case UnloadPolicy::kUnloadWhenIdle:
            // 只要没有活跃会话即可卸载（不检查handleRefCount）
            break;

        case UnloadPolicy::kUnloadWhenNoHandleAndIdle:
            // 必须没有handle持有才能卸载
            if (handleRefCount_.load(std::memory_order_relaxed) > 0) {
                return false;
            }
            break;
        case UnloadPolicy::kUnloadWhenIdleAndDisable:
            if (!isDisabled()) {
                return false;
            }
            break;
    }

    // 有活跃会话时不能卸载
    if (activeTaskCount_.load(std::memory_order_relaxed) > 0) {
        return false;
    }

    // 正在加载或卸载中不能再次卸载
    if (state_ == LifecycleState::kLoading || state_ == LifecycleState::kUnloading) {
        return false;
    }

    // 根据卸载策略检查状态
    switch (unloadPolicy_) {
        case UnloadPolicy::kNeverUnload:
            return false;

        case UnloadPolicy::kUnloadWhenIdle:
            // kLoaded或kIdle状态都可以卸载
            if (state_ != LifecycleState::kIdle && state_ != LifecycleState::kLoaded) {
                return false;
            }
            break;

        case UnloadPolicy::kUnloadWhenNoHandleAndIdle:
            // 只在kIdle状态卸载
            if (state_ != LifecycleState::kIdle) {
                return false;
            }
            break;

        case UnloadPolicy::kUnloadWhenIdleAndDisable:
            // 只在kIdle状态卸载
            if (state_ != LifecycleState::kIdle) {
                return false;
            }
            break;
    }
    return true;
}

void ModelRuntimeContext::forceUnload() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!canUnload()) {
        return;
    }
    state_ = LifecycleState::kUnloading;
    loadController_.unload();
    state_ = LifecycleState::kUnloaded;
    asyncLoadComplete_ = false;
    asyncLoadSuccess_ = false;
    asyncLoadFuture_ = std::shared_future<void>();
    LOGD("Model [{}] forced to Unloaded state.", descriptor_.fingerprint());
}

bool ModelRuntimeContext::ensureLoaded(EngineCallStage stage) {
    if (loadTriggerStage_ != stage) {
        return false;
    }

    if (stage == EngineCallStage::kOnBusinessCall) {
        // 检查是否正在进行异步加载
        bool isLoading = false;
        bool loadComplete = false;
        bool hasValidFuture = false;
        {
            std::shared_lock<std::shared_mutex> readLock(mutex_);
            isLoading = (state_ == LifecycleState::kLoading);
            loadComplete = asyncLoadComplete_.load();
            hasValidFuture = asyncLoadFuture_.valid();
        }

        // 如果已经加载完成，就不需要等待了
        if (loadComplete) {
            LOGD("Model [{}] async load already complete, skip waiting.", descriptor_.fingerprint());
        } else if (isLoading && hasValidFuture) {
            // 正在加载且有有效的 future，才等待
            LOGI("Model [{}] is loading, waiting for completion...", descriptor_.fingerprint());
            bool loaded = waitForAsyncLoad(30000);  // 最多等待 30 秒
            if (!loaded) {
                LOGE("Model [{}] async load failed or timed out.", descriptor_.fingerprint());
                return false;
            }
            LOGI("Model [{}] loaded successfully (waited for async load).", descriptor_.fingerprint());
        }
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();

    // 如果正在加载中
    if (state_ == LifecycleState::kLoading) {
        // 直接返回 true，避免重复加载
        LOGD("Model [{}] is already loading, skipping duplicate load.", descriptor_.fingerprint());
        lastAccess_ = now;
        return true;
    }

    if (loadController_.isLoaded()) {
        if (state_ == LifecycleState::kUnloaded || state_ == LifecycleState::kIdle) {
            state_ = LifecycleState::kLoaded;
        }
        lastAccess_ = now;
        return true;
    }
    state_ = LifecycleState::kLoading;
    loadController_.load();
    state_ = LifecycleState::kLoaded;
    lastAccess_ = now;
    LOGI("Model [{}] loaded by trigger stage {}.", descriptor_.fingerprint(), static_cast<int>(stage));
    return true;
}

bool ModelRuntimeContext::startAsyncLoad(EngineCallStage stage) {
    if (loadTriggerStage_ != stage) {
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();

    if (loadController_.isLoaded()) {
        if (state_ == LifecycleState::kUnloaded || state_ == LifecycleState::kIdle) {
            state_ = LifecycleState::kLoaded;
        }
        lastAccess_ = now;
        asyncLoadComplete_ = true;
        asyncLoadSuccess_ = true;
        return true;
    }

    if (state_ == LifecycleState::kLoading) {
        LOGI("Model [{}] is already in loading state.", descriptor_.fingerprint());
        return true;  // 正在加载中，返回true表示已处理
    }

    state_ = LifecycleState::kLoading;
    asyncLoadComplete_ = false;
    asyncLoadSuccess_ = false;

    auto self = shared_from_this();
    asyncLoadFuture_ = std::async(std::launch::async, [self, stage, descriptor = descriptor_]() {
        LOGI("Model [{}] starting async load...", descriptor.fingerprint());
        bool success = false;
        try {
            self->loadController_.load();
            success = true;
            self->asyncLoadSuccess_ = true;
            LOGI("Model [{}] async load completed successfully.", descriptor.fingerprint());
        } catch (const std::exception &e) {
            LOGE("Model [{}] async load failed: {}", descriptor.fingerprint(), e.what());
            self->asyncLoadSuccess_ = false;
        }
        self->asyncLoadComplete_ = true;

        std::unique_lock<std::shared_mutex> lock(self->mutex_);
        if (success) {
            self->state_ = LifecycleState::kLoaded;
        } else {
            self->state_ = LifecycleState::kUnloaded;
        }
        self->lastAccess_ = std::chrono::steady_clock::now();
    });

    LOGI("Model [{}] async load started by trigger stage {}.", descriptor_.fingerprint(), static_cast<int>(stage));
    return true;
}

bool ModelRuntimeContext::isAsyncLoadComplete() const { return asyncLoadComplete_.load(); }

bool ModelRuntimeContext::waitForAsyncLoad(int timeoutMs) {
    if (asyncLoadComplete_.load()) {
        return asyncLoadSuccess_.load();
    }

    std::shared_lock<std::shared_mutex> readLock(mutex_);
    if (!asyncLoadFuture_.valid()) {
        return false;
    }

    if (asyncLoadComplete_.load()) {
        return asyncLoadSuccess_.load();
    }

    readLock.unlock();

    if (timeoutMs == 0) {
        asyncLoadFuture_.wait();
        return asyncLoadSuccess_;
    } else {
        auto status = asyncLoadFuture_.wait_for(std::chrono::milliseconds(timeoutMs));
        return status == std::future_status::ready && asyncLoadSuccess_;
    }
}

void ModelRuntimeContext::checkAndUnloadIfDisabled() {
    if (unloadPolicy_ != UnloadPolicy::kUnloadWhenIdleAndDisable || state_ != LifecycleState::kIdle) {
        return;
    }
    if (shouldUnload()) {
        LOGD("Model [{}] is disabled and idle, unloading now", descriptor_.fingerprint());
        forceUnload();
    }
}

void ModelRuntimeContext::setState(LifecycleState state) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    state_ = state;
}

}  // namespace lifecycle
