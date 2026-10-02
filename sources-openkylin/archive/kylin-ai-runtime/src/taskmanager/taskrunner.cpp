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

#include "taskrunner.h"

#include <jsoncpp/json/json.h>
#include <sys/types.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <mutex>
#include <optional>
#include <thread>

#include "utils/logger.h"

namespace task {

namespace {

#ifdef TASKRUNNER_TESTING
std::optional<std::filesystem::path> g_taskRunnerConfigOverride;
#endif

std::filesystem::path taskRunnerDefaultConfigPath() {
    return std::filesystem::path(CONFIGS_PATH) / "runtime" / "task.json";
}

std::filesystem::path taskRunnerConfigPath() {
#ifdef TASKRUNNER_TESTING
    if (g_taskRunnerConfigOverride.has_value()) {
        return *g_taskRunnerConfigOverride;
    }
#endif
    return taskRunnerDefaultConfigPath();
}

std::optional<Json::Value> readJsonValue(const std::filesystem::path &jsonFile) {
    std::ifstream ifs(jsonFile);
    if (!ifs.is_open()) {
        LOGI("TaskRunner config not found: {}, use defaults", jsonFile.string());
        return std::nullopt;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::Value root;
    std::string errs;
    if (!Json::parseFromStream(builder, ifs, &root, &errs)) {
        LOGW("Failed to parse TaskRunner config: {}, error: {}", jsonFile.string(), errs);
        return std::nullopt;
    }
    return root;
}

bool parsePositiveInt(const Json::Value &value, int &out) {
    if (!value.isInt() && !value.isUInt()) {
        return false;
    }
    out = value.asInt();
    return out > 0;
}

bool parsePositiveInt64(const Json::Value &value, int64_t &out) {
    if (!(value.isInt64() || value.isUInt64() || value.isInt() || value.isUInt())) {
        return false;
    }
    out = value.asInt64();
    return out > 0;
}

bool tryParseTaskType(const std::string &name, TaskType &out) {
    for (size_t i = 0; i < kTaskTypeNames.size(); i++) {
        if (name == kTaskTypeNames[i]) {
            out = static_cast<TaskType>(i);
            return true;
        }
    }
    return false;
}

template <size_t N>
void applyMaxConcurrentTasksObject(const Json::Value &maxConcurrent, std::array<int, N> &maxConcurrentTasks,
                                   int clampMax) {
    for (const auto &name : maxConcurrent.getMemberNames()) {
        TaskType taskType{};
        if (!tryParseTaskType(name, taskType)) {
            LOGW("Unknown task type in TaskRunner config, key: {}", name);
            continue;
        }

        int maxTasks = 0;
        if (!parsePositiveInt(maxConcurrent[name], maxTasks)) {
            LOGW("Invalid maxConcurrentTasks value, task type: {}, value: {}", name,
                 maxConcurrent[name].toStyledString());
            continue;
        }
        if (maxTasks > clampMax) {
            LOGW("Clamp maxConcurrentTasks for type {} from {} to {}", name, maxTasks, clampMax);
            maxTasks = clampMax;
        }
        maxConcurrentTasks[taskTypeIndex(taskType)] = maxTasks;
    }
}

template <size_t N>
void applyMaxConcurrentTasksArray(const Json::Value &maxConcurrent, std::array<int, N> &maxConcurrentTasks,
                                  int clampMax) {
    const size_t size = std::min(static_cast<size_t>(maxConcurrent.size()), N);
    for (size_t i = 0; i < size; i++) {
        int maxTasks = 0;
        if (!parsePositiveInt(maxConcurrent[static_cast<int>(i)], maxTasks)) {
            LOGW("Invalid maxConcurrentTasks array item, index: {}, value: {}", i,
                 maxConcurrent[static_cast<int>(i)].toStyledString());
            continue;
        }
        if (maxTasks > clampMax) {
            LOGW("Clamp maxConcurrentTasks for index {} from {} to {}", i, maxTasks, clampMax);
            maxTasks = clampMax;
        }
        maxConcurrentTasks[i] = maxTasks;
    }
}

template <size_t N>
void applyMaxConcurrentTasksFromConfig(const Json::Value &root, std::array<int, N> &maxConcurrentTasks, int clampMax) {
    const Json::Value maxConcurrent = root["maxConcurrentTasks"];
    if (maxConcurrent.isObject()) {
        applyMaxConcurrentTasksObject(maxConcurrent, maxConcurrentTasks, clampMax);
        return;
    }
    if (maxConcurrent.isArray()) {
        applyMaxConcurrentTasksArray(maxConcurrent, maxConcurrentTasks, clampMax);
        return;
    }
    if (!maxConcurrent.isNull()) {
        LOGW("Invalid TaskRunner config, maxConcurrentTasks must be object or array");
    }
}

}  // namespace

TaskId generateTaskId() {
    static std::atomic<TaskId> taskId{1};
    TaskId id = taskId.fetch_add(1, std::memory_order_relaxed);
    if (id == INVALID_TASK_ID) {
        id = taskId.fetch_add(1, std::memory_order_relaxed);
    }
    return id;
}

std::optional<int64_t> loadTaskTimeoutMs(TaskType taskType) {
    const auto rootOpt = readJsonValue(taskRunnerConfigPath());
    if (!rootOpt.has_value()) {
        return std::nullopt;
    }

    const Json::Value timeouts = (*rootOpt)["taskTimeoutSec"];
    const std::string taskName(kTaskTypeNames[taskTypeIndex(taskType)]);
    constexpr int64_t kMsPerSec = 1000;
    if (timeouts.isObject()) {
        if (!timeouts.isMember(taskName)) {
            return std::nullopt;
        }
        int64_t timeoutMs = 0;
        if (!parsePositiveInt64(timeouts[taskName], timeoutMs)) {
            LOGW("Invalid taskTimeoutSec value, task type: {}, value: {}", taskName,
                 timeouts[taskName].toStyledString());
            return std::nullopt;
        }
        return timeoutMs * kMsPerSec;
    }
    if (timeouts.isArray()) {
        const auto index = static_cast<Json::ArrayIndex>(taskTypeIndex(taskType));
        if (index >= timeouts.size()) {
            return std::nullopt;
        }
        int64_t timeoutMs = 0;
        if (!parsePositiveInt64(timeouts[index], timeoutMs)) {
            LOGW("Invalid taskTimeoutSec array value, task type: {}, value: {}", taskName,
                 timeouts[index].toStyledString());
            return std::nullopt;
        }
        return timeoutMs * kMsPerSec;
    }
    if (!timeouts.isNull()) {
        LOGW("Invalid TaskRunner config, taskTimeoutSec must be object or array");
    }
    return std::nullopt;
}

std::optional<int> loadTaskQueueMaxSize(TaskType taskType) {
    const auto rootOpt = readJsonValue(taskRunnerConfigPath());
    if (!rootOpt.has_value()) {
        return std::nullopt;
    }

    const Json::Value queueLimits = (*rootOpt)["requestQueueMaxSize"];
    const std::string taskName(kTaskTypeNames[taskTypeIndex(taskType)]);
    if (queueLimits.isObject()) {
        if (!queueLimits.isMember(taskName)) {
            return std::nullopt;
        }
        int maxSize = 0;
        if (!parsePositiveInt(queueLimits[taskName], maxSize)) {
            LOGW("Invalid requestQueueMaxSize value, task type: {}, value: {}", taskName,
                 queueLimits[taskName].toStyledString());
            return std::nullopt;
        }
        return maxSize;
    }
    if (queueLimits.isArray()) {
        const auto index = static_cast<Json::ArrayIndex>(taskTypeIndex(taskType));
        if (index >= queueLimits.size()) {
            return std::nullopt;
        }
        int maxSize = 0;
        if (!parsePositiveInt(queueLimits[index], maxSize)) {
            LOGW("Invalid requestQueueMaxSize array value, task type: {}, value: {}", taskName,
                 queueLimits[index].toStyledString());
            return std::nullopt;
        }
        return maxSize;
    }
    if (!queueLimits.isNull()) {
        LOGW("Invalid TaskRunner config, requestQueueMaxSize must be object or array");
    }
    return std::nullopt;
}

Task::Task(TaskId taskId, TaskType taskType, std::function<void()> task)
    : taskId(taskId), taskType(taskType), task(task) {}

void Task::operator()() const { task(); }

TaskRunner &TaskRunner::getInstance() {
    static TaskRunner taskRunner;
    return taskRunner;
}

void TaskRunner::initSchedulerStateLocked() {
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        maxConcurrentTasks_[i] = 1;
        currentTasks_[i] = 0;
        blockedCounters_[i] = 0;
        pendingPerType_[i] = 0;
        maxPendingPerType_[i] = kDefaultMaxPendingPerType;
    }
}

void TaskRunner::applyDefaultMaxConcurrency() {
    maxConcurrentTasks_[toIndex(TaskType::TextChat)] = 8;
    maxConcurrentTasks_[toIndex(TaskType::OnceTextToSpeech)] = 2;
    maxConcurrentTasks_[toIndex(TaskType::ContinuousTextToSpeech)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::OnceSpeechRecognition)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::ContinuousSpeechRecognition)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::TextToImage)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::TextRecognition)] = 8;
    maxConcurrentTasks_[toIndex(TaskType::SubjectSegmentation)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::ImageEmbedding)] = 4;
    maxConcurrentTasks_[toIndex(TaskType::TextEmbedding)] = 4;
}

unsigned int TaskRunner::computeWorkerCount() const {
    unsigned int workerCount = std::thread::hardware_concurrency();
    if (workerCount == 0) {
        workerCount = 4;
    }
    const unsigned int maxWorkers = (workerCount <= kMaxWorkerThreadsSmall) ? kMaxWorkerThreadsSmall : workerCount;
    return std::min(workerCount, maxWorkers);
}

void TaskRunner::startThreads(unsigned int workerCount) {
    workerThreads_.reserve(workerCount);
    for (unsigned int i = 0; i < workerCount; i++) {
        workerThreads_.emplace_back(&TaskRunner::workerLoop, this);
    }
    dispatchThread = std::thread(&TaskRunner::run, this);
}

void TaskRunner::loadAndApplyConfigMaxConcurrency() {
    const auto configPath = taskRunnerConfigPath();
    const auto rootOpt = readJsonValue(configPath);
    if (!rootOpt.has_value()) {
        return;
    }

    const int clampMax = static_cast<int>(kDefaultMaxPendingPerType);
    applyMaxConcurrentTasksFromConfig(*rootOpt, maxConcurrentTasks_, clampMax);
}

TaskRunner::TaskRunner() {
    initSchedulerStateLocked();
    applyDefaultMaxConcurrency();
    loadAndApplyConfigMaxConcurrency();
    startThreads(computeWorkerCount());
}

TaskRunner::~TaskRunner() {
    {
        std::lock_guard<std::mutex> locker(mutex_);
        stop = true;
    }
    cv_.notify_all();

    if (dispatchThread.joinable()) {
        dispatchThread.join();
    }

    for (auto &t : workerThreads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

TaskRunner::EnqueueResult TaskRunner::addTask(Task task) {
    const size_t index = toIndex(task.taskType);
    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (stop.load(std::memory_order_relaxed)) {
            LOGE("TaskRunner is stopping, reject task: {}", task.taskId);
            return EnqueueResult::RejectedStopping;
        }
        if (pendingTotal_ >= maxPendingTotal_ || pendingPerType_[index] >= maxPendingPerType_[index]) {
            LOGE("Task queue is full, reject task: {}, type: {}", task.taskId, int(task.taskType));
            return EnqueueResult::RejectedQueueFull;
        }
        pendingTotal_ += 1;
        pendingPerType_[index] += 1;
        incomingQueue_.emplace_back(std::move(task));
    }
    cv_.notify_all();
    return EnqueueResult::Accepted;
}

void TaskRunner::deleteTask(TaskId taskId) {
    std::lock_guard<std::mutex> locker(mutex_);
    canceledTasks_.insert(taskId);
    cv_.notify_all();
}

void TaskRunner::waitForTasks() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return pendingTotal_ == 0; });
}

void TaskRunner::setMaxConcurrentTasks(TaskType taskType, int maxTasks) {
    if (maxTasks <= 0) {
        LOGE("Invalid maxTasks: {}, task type: {}", maxTasks, int(taskType));
        return;
    }
    std::lock_guard<std::mutex> locker(mutex_);
    maxConcurrentTasks_[toIndex(taskType)] = maxTasks;
    cv_.notify_all();
}

#ifdef TASKRUNNER_TESTING
void TaskRunner::setPendingLimitsForTesting(size_t maxPendingTotal, size_t maxPendingPerType) {
    std::lock_guard<std::mutex> locker(mutex_);
    maxPendingTotal_ = maxPendingTotal;
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        maxPendingPerType_[i] = maxPendingPerType;
    }
    cv_.notify_all();
}

bool TaskRunner::waitForTasksForTesting(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, timeout, [this]() { return pendingTotal_ == 0; });
}

TaskRunner::DebugState TaskRunner::getDebugStateForTesting() {
    std::lock_guard<std::mutex> locker(mutex_);
    DebugState state{};
    state.pendingTotal = pendingTotal_;
    state.incomingSize = incomingQueue_.size();
    state.readySize = readyQueue_.size();
    state.canceledSize = canceledTasks_.size();
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        state.pendingPerType[i] = pendingPerType_[i];
        state.currentTasks[i] = currentTasks_[i];
        state.maxConcurrentTasks[i] = maxConcurrentTasks_[i];
        state.waitingSizes[i] = waitingQueues_[i].size();
    }
    return state;
}
#endif

void TaskRunner::run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        cv_.wait(lock, [this]() {
            if (stop.load(std::memory_order_relaxed)) {
                return true;
            }
            if (!incomingQueue_.empty()) {
                return true;
            }
            for (size_t i = 0; i < kTaskTypeCount; i++) {
                if (!waitingQueues_[i].empty() && currentTasks_[i] < maxConcurrentTasks_[i]) {
                    return true;
                }
            }
            return false;
        });

        if (stop.load(std::memory_order_relaxed) && pendingTotal_ == 0) {
            break;
        }

        scheduleWaitingTasksLocked();
        scheduleIncomingTasksLocked();
        logBlockedTasksLocked();

        if (!readyQueue_.empty()) {
            cv_.notify_all();
        }
    }
}

void TaskRunner::workerLoop() {
    while (true) {
        Task task;
        bool notifyDispatcher = false;
        bool canceled = false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stop.load(std::memory_order_relaxed) || !readyQueue_.empty(); });

            if (stop.load(std::memory_order_relaxed) && readyQueue_.empty()) {
                return;
            }

            task = std::move(readyQueue_.front());
            readyQueue_.pop_front();

            if (isCanceledTaskLocked(task.taskId)) {
                dropCanceledTaskLocked(task.taskType, task.taskId, true);
                notifyDispatcher = true;
                canceled = true;
            }
        }
        if (canceled) {
            if (notifyDispatcher) {
                cv_.notify_all();
            }
            continue;
        }

        try {
            task();
        } catch (const std::exception &e) {
            LOGE("Task threw exception, taskId: {}, taskType: {}, error: {}", task.taskId, int(task.taskType),
                 e.what());
        } catch (...) {
            LOGE("Task threw unknown exception, taskId: {}, taskType: {}", task.taskId, int(task.taskType));
        }

        {
            std::lock_guard<std::mutex> locker(mutex_);
            markTaskFinishedLocked(task.taskType);
            notifyDispatcher = true;
        }
        if (notifyDispatcher) {
            cv_.notify_all();
        }
    }
}

bool TaskRunner::isCanceledTaskLocked(TaskId taskId) const {
    return canceledTasks_.find(taskId) != canceledTasks_.end();
}

void TaskRunner::removeCanceledTaskLocked(TaskId taskId) { canceledTasks_.erase(taskId); }

void TaskRunner::markTaskFinishedLocked(TaskType taskType) {
    const size_t index = toIndex(taskType);
    currentTasks_[index] = std::max(0, currentTasks_[index] - 1);
    if (pendingTotal_ > 0) {
        pendingTotal_ -= 1;
    }
    if (pendingPerType_[index] > 0) {
        pendingPerType_[index] -= 1;
    }
}

void TaskRunner::dropCanceledTaskLocked(TaskType taskType, TaskId taskId, bool countedAsRunning) {
    removeCanceledTaskLocked(taskId);
    const size_t index = toIndex(taskType);
    if (countedAsRunning) {
        currentTasks_[index] = std::max(0, currentTasks_[index] - 1);
    }
    if (pendingTotal_ > 0) {
        pendingTotal_ -= 1;
    }
    if (pendingPerType_[index] > 0) {
        pendingPerType_[index] -= 1;
    }
}

bool TaskRunner::canScheduleLocked(TaskType taskType) const {
    const size_t index = toIndex(taskType);
    return currentTasks_[index] < maxConcurrentTasks_[index];
}

bool TaskRunner::hasWorkLocked() const {
    if (!incomingQueue_.empty()) {
        return true;
    }
    if (!readyQueue_.empty()) {
        return true;
    }
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        if (!waitingQueues_[i].empty() && currentTasks_[i] < maxConcurrentTasks_[i]) {
            return true;
        }
    }
    return false;
}

void TaskRunner::scheduleWaitingTasksLocked() {
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        const TaskType taskType = static_cast<TaskType>(i);
        while (currentTasks_[i] < maxConcurrentTasks_[i] && !waitingQueues_[i].empty()) {
            Task task = std::move(waitingQueues_[i].front());
            waitingQueues_[i].pop_front();

            if (isCanceledTaskLocked(task.taskId)) {
                dropCanceledTaskLocked(taskType, task.taskId, false);
                continue;
            }

            currentTasks_[i] += 1;
            readyQueue_.emplace_back(std::move(task));
        }
    }
}

void TaskRunner::scheduleIncomingTasksLocked() {
    while (!incomingQueue_.empty()) {
        Task task = std::move(incomingQueue_.front());
        incomingQueue_.pop_front();

        const size_t index = toIndex(task.taskType);

        if (isCanceledTaskLocked(task.taskId)) {
            dropCanceledTaskLocked(task.taskType, task.taskId, false);
            continue;
        }

        if (canScheduleLocked(task.taskType)) {
            currentTasks_[index] += 1;
            readyQueue_.emplace_back(std::move(task));
            continue;
        }

        waitingQueues_[index].emplace_back(std::move(task));
    }
}

void TaskRunner::logBlockedTasksLocked() {
    // 当某个类型的任务达到该类型任务容量限制时，且waiting队列持续非空，认为该类型任务整体阻塞
    for (size_t i = 0; i < kTaskTypeCount; i++) {
        if (waitingQueues_[i].empty()) {
            blockedCounters_[i] = 0;
            continue;
        }

        if (currentTasks_[i] < maxConcurrentTasks_[i]) {
            blockedCounters_[i] = 0;
            continue;
        }

        blockedCounters_[i] += 1;
        if (blockedCounters_[i] == 10000) {
            LOGE("This type of task is blocked in its entirety, task type: {}", int(static_cast<TaskType>(i)));
            blockedCounters_[i] = 0;
        }
    }
}

#ifdef TASKRUNNER_TESTING
void setTaskRunnerConfigPathForTesting(const std::filesystem::path &path) { g_taskRunnerConfigOverride = path; }

void clearTaskRunnerConfigPathForTesting() { g_taskRunnerConfigOverride.reset(); }
#endif
}  // namespace task
