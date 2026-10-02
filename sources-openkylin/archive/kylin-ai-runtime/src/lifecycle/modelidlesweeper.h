/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <atomic>
#include <thread>

#include "lifecycle/modellifecyclemanager.h"

namespace lifecycle {

class ModelIdleSweeper {
public:
    explicit ModelIdleSweeper(ModelLifecycleManager &manager);
    ~ModelIdleSweeper();

    void start();
    void stop();

private:
    void run();

private:
    ModelLifecycleManager &manager_;
    std::atomic_bool running_{false};
    std::thread worker_;
};

}  // namespace lifecycle
