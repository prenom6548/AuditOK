/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SERVICES_MODEL_CONFIG_MODEL_CHANGED_CALLBACKS_H
#define SERVICES_MODEL_CONFIG_MODEL_CHANGED_CALLBACKS_H

#include <functional>
#include <map>
#include <mutex>
#include <string>

#include "modelconfigenums.h"

class ModelChangedCallbacks {
public:
    using ModelChangedCallback = std::function<void(ModelChangeType changeType)>;

public:
    ModelChangedCallbacks() = default;

    ~ModelChangedCallbacks() = default;

    void callAllCallbacks(AiCapability capability, DeployType deployType, ModelChangeType changeType);

    void updateCallback(AiCapability capability, DeployType deployType, const std::string &uuid,
                        const ModelChangedCallback &callback);

private:
    std::mutex mutex_;
    std::map<AiCapability, std::map<DeployType, std::map<std::string, ModelChangedCallback> > > callbacksMap_;
};

#endif  // SERVICES_MODEL_CONFIG_MODEL_CHANGED_CALLBACKS_H
