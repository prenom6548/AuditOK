/*
 * Copyright 2025 KylinSoft Co., Ltd.
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
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#include "taskmanager/taskrunner.h"

namespace {

class ScopedTaskRunnerConfigOverride {
public:
    ScopedTaskRunnerConfigOverride() {
        auto dir =
            std::filesystem::temp_directory_path() / ("kylin-ai-runtime-testtaskrunner-" + std::to_string(getpid()));
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        configPath_ = dir / "task.json";
        task::setTaskRunnerConfigPathForTesting(configPath_);

        std::ofstream ofs(configPath_);
        ofs << R"({
  "maxConcurrentTasks": {
    "SubjectSegmentation": 9
  }
})";
        ofs.close();

        (void)task::TaskRunner::getInstance();
    }

    ~ScopedTaskRunnerConfigOverride() {
        task::clearTaskRunnerConfigPathForTesting();
        std::error_code ec;
        std::filesystem::remove(configPath_, ec);
        if (!configPath_.parent_path().empty()) {
            std::filesystem::remove(configPath_.parent_path(), ec);
        }
    }

private:
    std::filesystem::path configPath_;
};

ScopedTaskRunnerConfigOverride g_taskRunnerConfigOverride;

struct Gate {
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() { return open; });
    }

    void openAll() {
        std::lock_guard<std::mutex> lock(mutex);
        open = true;
        cv.notify_all();
    }

    std::mutex mutex;
    std::condition_variable cv;
    bool open{false};
};

void drainTaskRunner() { task::TaskRunner::getInstance().waitForTasks(); }

class ScopedPendingLimits {
public:
    ScopedPendingLimits(size_t maxTotal, size_t maxPerType) {
        task::TaskRunner::getInstance().setPendingLimitsForTesting(maxTotal, maxPerType);
    }

    ~ScopedPendingLimits() { task::TaskRunner::getInstance().setPendingLimitsForTesting(10000, 2000); }
};

std::string dumpState(const task::TaskRunner::DebugState& state) {
    std::ostringstream oss;
    oss << "pendingTotal=" << state.pendingTotal << " incoming=" << state.incomingSize << " ready=" << state.readySize
        << " canceled=" << state.canceledSize;
    oss << " perType={";
    for (size_t i = 0; i < state.pendingPerType.size(); i++) {
        if (i != 0) {
            oss << ",";
        }
        oss << i << ":pending=" << state.pendingPerType[i] << "/waiting=" << state.waitingSizes[i]
            << "/running=" << state.currentTasks[i] << "/max=" << state.maxConcurrentTasks[i];
    }
    oss << "}";
    return oss.str();
}

void assertWaitForTasks(std::chrono::milliseconds timeout) {
    if (!task::TaskRunner::getInstance().waitForTasksForTesting(timeout)) {
        const auto state = task::TaskRunner::getInstance().getDebugStateForTesting();
        FAIL() << "waitForTasks timed out: " << dumpState(state);
    }
}

}  // namespace

TEST(TaskRunnerTest, ConfigFileOverridesMaxConcurrency) {
    drainTaskRunner();
    const auto state = task::TaskRunner::getInstance().getDebugStateForTesting();
    EXPECT_EQ(state.maxConcurrentTasks[task::taskTypeIndex(task::TaskType::SubjectSegmentation)], 9);
}

TEST(TaskRunnerTest, WaitForTasksCompletes) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    std::atomic<int> executed{0};
    for (int i = 0; i < 50; i++) {
        auto result = task::TaskRunner::getInstance().addTask(
            {task::generateTaskId(), task::TaskType::TextRecognition,
             [&executed]() { executed.fetch_add(1, std::memory_order_relaxed); }});
        EXPECT_EQ(result, task::TaskRunner::EnqueueResult::Accepted);
    }
    task::TaskRunner::getInstance().waitForTasks();
    EXPECT_EQ(executed.load(std::memory_order_relaxed), 50);
}

TEST(TaskRunnerTest, MaxConcurrencyPerType) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextChat, 1);

    std::atomic<int> running{0};
    std::atomic<int> maxRunning{0};

    for (int i = 0; i < 20; i++) {
        auto result = task::TaskRunner::getInstance().addTask(
            {task::generateTaskId(), task::TaskType::TextChat, [&running, &maxRunning]() {
                 int r = running.fetch_add(1, std::memory_order_relaxed) + 1;
                 int observedMax = maxRunning.load(std::memory_order_relaxed);
                 while (r > observedMax &&
                        !maxRunning.compare_exchange_weak(observedMax, r, std::memory_order_relaxed)) {
                 }
                 std::this_thread::sleep_for(std::chrono::milliseconds(10));
                 running.fetch_sub(1, std::memory_order_relaxed);
             }});
        EXPECT_EQ(result, task::TaskRunner::EnqueueResult::Accepted);
    }

    task::TaskRunner::getInstance().waitForTasks();
    EXPECT_EQ(maxRunning.load(std::memory_order_relaxed), 1);
}

TEST(TaskRunnerTest, CancelWaitingTask) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextRecognition, 1);

    Gate gate;
    std::atomic<int> canceledTaskRan{0};

    const task::TaskId firstId = task::generateTaskId();
    EXPECT_EQ(
        task::TaskRunner::getInstance().addTask({firstId, task::TaskType::TextRecognition, [&gate]() { gate.wait(); }}),
        task::TaskRunner::EnqueueResult::Accepted);

    const task::TaskId canceledId = task::generateTaskId();
    EXPECT_EQ(task::TaskRunner::getInstance().addTask({canceledId, task::TaskType::TextRecognition,
                                                       [&canceledTaskRan]() { canceledTaskRan.fetch_add(1); }}),
              task::TaskRunner::EnqueueResult::Accepted);

    task::TaskRunner::getInstance().deleteTask(canceledId);

    gate.openAll();
    task::TaskRunner::getInstance().waitForTasks();

    EXPECT_EQ(canceledTaskRan.load(std::memory_order_relaxed), 0);
}

TEST(TaskRunnerTest, CancelRunningTaskDoesNotPreventCompletion) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextRecognition, 1);

    Gate gate;
    std::atomic<bool> started{false};
    std::atomic<bool> finished{false};

    const task::TaskId id = task::generateTaskId();
    EXPECT_EQ(task::TaskRunner::getInstance().addTask({id, task::TaskType::TextRecognition,
                                                       [&]() {
                                                           started.store(true, std::memory_order_relaxed);
                                                           gate.wait();
                                                           finished.store(true, std::memory_order_relaxed);
                                                       }}),
              task::TaskRunner::EnqueueResult::Accepted);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (!started.load(std::memory_order_relaxed) && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_TRUE(started.load(std::memory_order_relaxed));

    task::TaskRunner::getInstance().deleteTask(id);

    gate.openAll();
    task::TaskRunner::getInstance().waitForTasks();

    EXPECT_TRUE(finished.load(std::memory_order_relaxed));
}

TEST(TaskRunnerTest, BackpressurePerTypeLimit) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::OnceSpeechRecognition, 8);

    Gate gate;
    std::atomic<int> executed{0};

    for (int i = 0; i < 2100; i++) {
        auto result = task::TaskRunner::getInstance().addTask(
            {task::generateTaskId(), task::TaskType::OnceSpeechRecognition, [&gate, &executed]() {
                 gate.wait();
                 executed.fetch_add(1, std::memory_order_relaxed);
             }});
        if (i < 2000) {
            EXPECT_EQ(result, task::TaskRunner::EnqueueResult::Accepted);
        } else {
            EXPECT_EQ(result, task::TaskRunner::EnqueueResult::RejectedQueueFull);
        }
    }

    gate.openAll();
    task::TaskRunner::getInstance().waitForTasks();

    // TaskRunner has per-type pending limit (default 2000). With the gate blocking completion,
    // extra tasks should be rejected, so executed should not exceed the limit.
    EXPECT_LE(executed.load(std::memory_order_relaxed), 2000);
}

TEST(TaskRunnerTest, TotalPendingLimitRejects) {
    drainTaskRunner();
    ScopedPendingLimits limits(20, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextChat, 8);

    Gate gate;
    std::atomic<int> executed{0};
    for (int i = 0; i < 20; i++) {
        EXPECT_EQ(task::TaskRunner::getInstance().addTask({task::generateTaskId(), task::TaskType::TextChat,
                                                           [&]() {
                                                               gate.wait();
                                                               executed.fetch_add(1, std::memory_order_relaxed);
                                                           }}),
                  task::TaskRunner::EnqueueResult::Accepted);
    }

    EXPECT_EQ(task::TaskRunner::getInstance().addTask({task::generateTaskId(), task::TaskType::TextChat, []() {}}),
              task::TaskRunner::EnqueueResult::RejectedQueueFull);

    gate.openAll();
    task::TaskRunner::getInstance().waitForTasks();
    EXPECT_EQ(executed.load(std::memory_order_relaxed), 20);
}

TEST(TaskRunnerTest, RejectedQueueFullDoesNotAffectWaitAndSubsequentEnqueue) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 10);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextChat, 8);

    Gate gate;
    std::atomic<int> executed{0};
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(task::TaskRunner::getInstance().addTask({task::generateTaskId(), task::TaskType::TextChat,
                                                           [&]() {
                                                               gate.wait();
                                                               executed.fetch_add(1, std::memory_order_relaxed);
                                                           }}),
                  task::TaskRunner::EnqueueResult::Accepted);
    }

    EXPECT_EQ(task::TaskRunner::getInstance().addTask({task::generateTaskId(), task::TaskType::TextChat, []() {}}),
              task::TaskRunner::EnqueueResult::RejectedQueueFull);

    gate.openAll();
    assertWaitForTasks(std::chrono::milliseconds(2000));
    EXPECT_EQ(executed.load(std::memory_order_relaxed), 10);

    std::atomic<int> executed2{0};
    EXPECT_EQ(task::TaskRunner::getInstance().addTask({task::generateTaskId(), task::TaskType::TextChat,
                                                       [&]() { executed2.fetch_add(1, std::memory_order_relaxed); }}),
              task::TaskRunner::EnqueueResult::Accepted);
    assertWaitForTasks(std::chrono::milliseconds(2000));
    EXPECT_EQ(executed2.load(std::memory_order_relaxed), 1);
}

TEST(TaskRunnerTest, IncreaseConcurrencyUnblocksWaiting) {
    drainTaskRunner();
    ScopedPendingLimits limits(10000, 2000);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextChat, 1);

    Gate gate;
    std::atomic<int> running{0};
    std::atomic<int> maxRunning{0};

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(task::TaskRunner::getInstance().addTask(
                      {task::generateTaskId(), task::TaskType::TextChat,
                       [&]() {
                           int r = running.fetch_add(1, std::memory_order_relaxed) + 1;
                           int observedMax = maxRunning.load(std::memory_order_relaxed);
                           while (r > observedMax &&
                                  !maxRunning.compare_exchange_weak(observedMax, r, std::memory_order_relaxed)) {
                           }
                           gate.wait();
                           running.fetch_sub(1, std::memory_order_relaxed);
                       }}),
                  task::TaskRunner::EnqueueResult::Accepted);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_LE(maxRunning.load(std::memory_order_relaxed), 1);

    task::TaskRunner::getInstance().setMaxConcurrentTasks(task::TaskType::TextChat, 4);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (running.load(std::memory_order_relaxed) < 4 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_GE(running.load(std::memory_order_relaxed), 2);
    EXPECT_LE(running.load(std::memory_order_relaxed), 4);

    gate.openAll();
    task::TaskRunner::getInstance().waitForTasks();
    EXPECT_GE(maxRunning.load(std::memory_order_relaxed), 2);
    EXPECT_LE(maxRunning.load(std::memory_order_relaxed), 4);
}
