/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modelidlesweeper.h"

#include <chrono>

#include "utils/logger.h"

namespace lifecycle {

ModelIdleSweeper::ModelIdleSweeper(ModelLifecycleManager &manager) : manager_(manager) {}

ModelIdleSweeper::~ModelIdleSweeper() { stop(); }

void ModelIdleSweeper::start() {
    if (running_) {
        return;
    }
    running_ = true;
    worker_ = std::thread(&ModelIdleSweeper::run, this);
}

void ModelIdleSweeper::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void ModelIdleSweeper::run() {
    while (running_) {
        const int interval = manager_.config().sweepIntervalSeconds();
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        manager_.sweepIdleContexts();
    }
}

}  // namespace lifecycle
