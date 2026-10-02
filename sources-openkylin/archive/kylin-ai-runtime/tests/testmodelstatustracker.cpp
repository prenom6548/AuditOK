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

#include <gtest/gtest.h>

#include <thread>

#include "modelconfig/modelstatustracker.h"

TEST(ModelStatusTrackerTest, SingleThreadLifecycle) {
    ModelStatusTracker tracker("百度");
    ASSERT_EQ(ModelStatusTracker::modelStatus("百度"), ModelStatus::Running);

    {
        { ModelStatusTracker tracker("百度"); }
        EXPECT_EQ(ModelStatusTracker::modelStatus("百度"), ModelStatus::Running);
    }
}

TEST(ModelStatusTrackerTest, MultiThreadNestedTracker) {
    ModelStatusTracker tracker("百度");
    std::thread t([]() {
        ModelStatusTracker tracker("百度");
        EXPECT_EQ(ModelStatusTracker::modelStatus("百度"), ModelStatus::Running);
    });
    t.join();
    EXPECT_EQ(ModelStatusTracker::modelStatus("百度"), ModelStatus::Running);
}
TEST(ModelStatusTrackerTest, StressMultiThread) {
    std::thread *threads[1000] = {};
    for (int i = 0; i < 1000; i++) {
        threads[i] = new std::thread([]() { ModelStatusTracker tracker("百度"); });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };

    ModelStatusTracker *trackers[1000] = {};
    for (int i = 0; i < 1000; i++) {
        trackers[i] = new ModelStatusTracker("百度");
    }

    for (int i = 0; i < 1000; i++) {
        delete trackers[i];
    }

    for (int i = 0; i < 1000; i++) {
        threads[i]->join();
        delete threads[i];
    }

    EXPECT_EQ(ModelStatusTracker::modelStatus("百度"), ModelStatus::Idle);
}
