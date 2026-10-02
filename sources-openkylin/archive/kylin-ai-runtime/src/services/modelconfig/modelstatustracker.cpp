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

#include "modelstatustracker.h"

#include "utils/logger.h"

std::map<std::string, std::atomic_int> ModelStatusTracker::runningModelsCount;
ModelStatusTracker::ModelStatusTracker(const std::string &modelName) : modelName_(modelName) {
    runningModelsCount[modelName_]++;
}

ModelStatusTracker::~ModelStatusTracker() {
    if (runningModelsCount[modelName_] <= 0) {
        return;
    }

    runningModelsCount[modelName_]--;
}

ModelStatus ModelStatusTracker::modelStatus(const std::string &modelName) {
    LOGD("modelName {}", modelName);
    if (runningModelsCount.find(modelName) == runningModelsCount.end()) {
        return ModelStatus::Idle;
    }

    return runningModelsCount[modelName] > 0 ? ModelStatus::Running : ModelStatus::Idle;
}