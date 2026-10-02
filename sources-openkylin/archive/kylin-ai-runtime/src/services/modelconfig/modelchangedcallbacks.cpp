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

#include "modelchangedcallbacks.h"

void ModelChangedCallbacks::callAllCallbacks(AiCapability capability, DeployType deployType,
                                             ModelChangeType changeType) {
    std::lock_guard locker(mutex_);
    if (callbacksMap_.find(capability) == callbacksMap_.end()) {
        return;
    }
    auto &callbacks = callbacksMap_[capability][deployType];

    if (callbacks.empty()) {
        return;
    }

    for (auto &[uuid, callback] : callbacks) {
        if (callback) {
            callback(changeType);
        }
    }
}

void ModelChangedCallbacks::updateCallback(AiCapability capability, DeployType deployType, const std::string &uuid,
                                           const ModelChangedCallback &callback) {
    std::lock_guard locker(mutex_);
    callbacksMap_[capability][deployType][uuid] = callback;
}
