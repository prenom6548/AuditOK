/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <optional>

#include "lifecycle/modelusagehandle.h"

namespace lifecycle {

struct LifecycleBorrowOptions {
    bool trackUsage{true};
    std::optional<EngineCallStage> callStage;
};

class LifecycleRequestScope {
public:
    explicit LifecycleRequestScope(ModelUsageHandle &handle, ModelDescriptor descriptor,
                                   const LifecycleBorrowOptions &options = {})
        : handle_(handle), descriptor_(std::move(descriptor)), trackingUsage_(options.trackUsage) {
        if (options.callStage.has_value()) {
            handle_.ensureLoaded(descriptor_, *options.callStage);
        }
        if (trackingUsage_) {
            handle_.markBusy(descriptor_);
        }
    }

    ~LifecycleRequestScope() {
        if (trackingUsage_) {
            handle_.markIdle(descriptor_);
        }
    }

    LifecycleRequestScope(const LifecycleRequestScope &) = delete;
    LifecycleRequestScope &operator=(const LifecycleRequestScope &) = delete;

private:
    ModelUsageHandle &handle_;
    ModelDescriptor descriptor_;
    bool trackingUsage_{false};
};

}  // namespace lifecycle
