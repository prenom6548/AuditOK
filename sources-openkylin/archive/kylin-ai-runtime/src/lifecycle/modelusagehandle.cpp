/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modelusagehandle.h"

#include "lifecycle/modellifecyclemanager.h"

namespace lifecycle {

ModelUsageHandle::ModelUsageHandle(ModelLifecycleManager *manager) : manager_(manager) {}

ModelUsageHandle::~ModelUsageHandle() { reset(); }

ModelUsageHandle::ModelUsageHandle(ModelUsageHandle &&other) noexcept { *this = std::move(other); }

ModelUsageHandle &ModelUsageHandle::operator=(ModelUsageHandle &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    manager_ = other.manager_;
    acquiredDescriptors_ = std::move(other.acquiredDescriptors_);
    other.manager_ = nullptr;
    other.acquiredDescriptors_.clear();
    return *this;
}

void ModelUsageHandle::ensureAcquired(const ModelDescriptor &descriptor) {
    if (!valid()) {
        return;
    }
    const auto key = descriptor.fingerprint();
    if (acquiredDescriptors_.find(key) != acquiredDescriptors_.end()) {
        return;
    }
    manager_->acquireHandle(descriptor);
    acquiredDescriptors_.emplace(key, descriptor);
}

void ModelUsageHandle::markBusy(const ModelDescriptor &descriptor) {
    if (!valid()) {
        return;
    }
    ensureAcquired(descriptor);
    manager_->markBusy(descriptor);
}

void ModelUsageHandle::markIdle(const ModelDescriptor &descriptor) {
    if (!valid()) {
        return;
    }
    ensureAcquired(descriptor);
    manager_->markIdle(descriptor);
}

void ModelUsageHandle::reset() {
    if (!valid()) {
        return;
    }
    for (const auto &item : acquiredDescriptors_) {
        manager_->releaseHandle(item.second);
    }
    acquiredDescriptors_.clear();
    manager_ = nullptr;
}

bool ModelUsageHandle::ensureLoaded(const ModelDescriptor &descriptor, EngineCallStage stage) {
    if (!valid()) {
        return false;
    }
    ensureAcquired(descriptor);
    return manager_->ensureLoaded(descriptor, stage);
}

bool ModelUsageHandle::startAsyncLoad(const ModelDescriptor &descriptor, EngineCallStage stage) {
    if (!valid()) {
        return false;
    }
    ensureAcquired(descriptor);
    return manager_->startAsyncLoad(descriptor, stage);
}

bool ModelUsageHandle::isAsyncLoadComplete(const ModelDescriptor &descriptor) const {
    if (!valid()) {
        return true;  // 无效的handle，认为加载完成（未加载）
    }
    return manager_->isAsyncLoadComplete(descriptor);
}

bool ModelUsageHandle::waitForAsyncLoad(const ModelDescriptor &descriptor, int timeoutMs) {
    if (!valid()) {
        return false;
    }
    return manager_->waitForAsyncLoad(descriptor, timeoutMs);
}

}  // namespace lifecycle
