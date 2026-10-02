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

#ifndef TASKRUNNER_H
#define TASKRUNNER_H

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <set>
#include <string_view>
#include <thread>
#include <vector>

namespace task {

using TaskId = uint64_t;

constexpr TaskId INVALID_TASK_ID = 0;  // 0是无效任务ID

TaskId generateTaskId();

// Single source of truth for TaskType:
// - Add a new task type only here.
// - TaskType enum values and kTaskTypeCount will update automatically.
#define TASKRUNNER_TASKTYPE_LIST(X) \
    X(TextChat)                     \
    X(OnceTextToSpeech)             \
    X(ContinuousTextToSpeech)       \
    X(OnceSpeechRecognition)        \
    X(ContinuousSpeechRecognition)  \
    X(TextToImage)                  \
    X(TextRecognition)              \
    X(SubjectSegmentation)          \
    X(Segmentation)                 \
    X(ImageEmbedding)               \
    X(TextEmbedding)

#define TASKRUNNER_TASKTYPE_ENUM_ITEM(name) name,

enum class TaskType { TASKRUNNER_TASKTYPE_LIST(TASKRUNNER_TASKTYPE_ENUM_ITEM) };

#undef TASKRUNNER_TASKTYPE_ENUM_ITEM

#define TASKRUNNER_TASKTYPE_COUNT_ONE(name) +1

constexpr size_t kTaskTypeCountValue = 0 TASKRUNNER_TASKTYPE_LIST(TASKRUNNER_TASKTYPE_COUNT_ONE);

#undef TASKRUNNER_TASKTYPE_COUNT_ONE

#define TASKRUNNER_TASKTYPE_STRING_ITEM(name) std::string_view(#name),
inline constexpr std::array<std::string_view, kTaskTypeCountValue> kTaskTypeNames = {
    TASKRUNNER_TASKTYPE_LIST(TASKRUNNER_TASKTYPE_STRING_ITEM)};
#undef TASKRUNNER_TASKTYPE_STRING_ITEM

constexpr size_t taskTypeIndex(TaskType taskType) { return static_cast<size_t>(taskType); }
#undef TASKRUNNER_TASKTYPE_LIST

std::optional<int64_t> loadTaskTimeoutMs(TaskType taskType);
std::optional<int> loadTaskQueueMaxSize(TaskType taskType);

class Task {
public:
    Task() = default;
    Task(TaskId taskId, TaskType taskType, std::function<void()> task);
    void operator()() const;

public:
    std::function<void()> task;
    TaskId taskId;
    TaskType taskType;
};

class TaskRunner {
public:
    enum class EnqueueResult {
        Accepted,
        RejectedStopping,
        RejectedQueueFull,
    };

    static TaskRunner& getInstance();
    ~TaskRunner();

    EnqueueResult addTask(Task task);
    void deleteTask(TaskId taskId);
    void waitForTasks();
    void setMaxConcurrentTasks(TaskType taskType, int maxTasks);

#ifdef TASKRUNNER_TESTING
    struct DebugState {
        size_t pendingTotal{0};
        size_t incomingSize{0};
        size_t readySize{0};
        size_t canceledSize{0};
        std::array<size_t, kTaskTypeCountValue> pendingPerType{};
        std::array<int, kTaskTypeCountValue> currentTasks{};
        std::array<int, kTaskTypeCountValue> maxConcurrentTasks{};
        std::array<size_t, kTaskTypeCountValue> waitingSizes{};
    };

    void setPendingLimitsForTesting(size_t maxPendingTotal, size_t maxPendingPerType);
    bool waitForTasksForTesting(std::chrono::milliseconds timeout);
    DebugState getDebugStateForTesting();
#endif

private:
    TaskRunner();
    void run();

    static constexpr size_t kTaskTypeCount = kTaskTypeCountValue;
    static constexpr size_t toIndex(TaskType taskType) { return static_cast<size_t>(taskType); }

    void initSchedulerStateLocked();
    void applyDefaultMaxConcurrency();
    void loadAndApplyConfigMaxConcurrency();
    unsigned int computeWorkerCount() const;
    void startThreads(unsigned int workerCount);

    bool isCanceledTaskLocked(TaskId taskId) const;
    void removeCanceledTaskLocked(TaskId taskId);
    void markTaskFinishedLocked(TaskType taskType);
    void dropCanceledTaskLocked(TaskType taskType, TaskId taskId, bool countedAsRunning);
    bool canScheduleLocked(TaskType taskType) const;
    bool hasWorkLocked() const;
    void scheduleWaitingTasksLocked();
    void scheduleIncomingTasksLocked();
    void logBlockedTasksLocked();
    void workerLoop();

private:
    static constexpr size_t kDefaultMaxPendingTotal = 10000;
    static constexpr size_t kDefaultMaxPendingPerType = 2000;
    static constexpr unsigned int kMaxWorkerThreadsSmall = 16;

    std::atomic<bool> stop{false};
    std::mutex mutex_;
    std::condition_variable cv_;

    std::thread dispatchThread;
    std::vector<std::thread> workerThreads_;

    std::deque<Task> incomingQueue_;
    std::array<std::deque<Task>, kTaskTypeCount> waitingQueues_{};
    std::deque<Task> readyQueue_;

    std::set<TaskId> canceledTasks_;

    std::array<int, kTaskTypeCount> maxConcurrentTasks_{};
    std::array<int, kTaskTypeCount> currentTasks_{};

    // 每个类型的waiting队列连续非空计数器，用于检测整体阻塞并打印日志
    std::array<int, kTaskTypeCount> blockedCounters_{};

    size_t pendingTotal_{0};
    std::array<size_t, kTaskTypeCount> pendingPerType_{};
    size_t maxPendingTotal_{kDefaultMaxPendingTotal};
    std::array<size_t, kTaskTypeCount> maxPendingPerType_{};
};

#ifdef TASKRUNNER_TESTING
void setTaskRunnerConfigPathForTesting(const std::filesystem::path& path);
void clearTaskRunnerConfigPathForTesting();
#endif

}  // namespace task

#endif  // TASKRUNNER_H
