// Licensed to the LF AI & Data foundation under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/Types.h"
#include "exec/Driver.h"
#include "exec/QueryContext.h"
#include "plan/PlanNode.h"

namespace milvus {
namespace exec {

enum class TaskState { kRunning, kFinished, kCanceled, kAborted, kFailed };

std::string
MakeUuid();
class Task : public std::enable_shared_from_this<Task> {
 public:
    static std::shared_ptr<Task>
    Create(const std::string& task_id,
           plan::PlanFragment plan_fragment,
           int destination,
           std::shared_ptr<QueryContext> query_context,
           std::function<void(std::exception_ptr)> on_error = nullptr);

    Task(const std::string& task_id,
         plan::PlanFragment plan_fragment,
         int destination,
         std::shared_ptr<QueryContext> query_ctx,
         std::function<void(std::exception_ptr)> on_error)
        : uuid_{MakeUuid()},
          taskid_(task_id),
          plan_fragment_(std::move(plan_fragment)),
          destination_(destination),
          query_context_(std::move(query_ctx)),
          on_error_(on_error) {
    }

    ~Task() {
    }

    const std::string&
    uuid() const {
        return uuid_;
    }

    const std::string&
    taskid() const {
        return taskid_;
    }

    const int
    destination() const {
        return destination_;
    }

    const std::shared_ptr<QueryContext>&
    query_context() const {
        return query_context_;
    }

    static void
    Start(std::shared_ptr<Task> self,
          uint32_t max_drivers,
          uint32_t concurrent_split_groups = 1);

    static void
    RemoveDriver(std::shared_ptr<Task> self, Driver* instance) {
        std::lock_guard<std::mutex> lock(self->mutex_);
        if (self->driver_.get() != instance) {
            return;
        }
        self->driver_ = nullptr;
        self->DriverClosedLocked();
    }

    RowVectorPtr
    Next(ContinueFuture* future = nullptr);

    void
    SetError(const std::exception_ptr& exception);

    void
    SetError(const std::string& message);

    bool
    IsRunning() const {
        std::lock_guard<std::mutex> l(mutex_);
        return (state_ == TaskState::kRunning);
    }

    bool
    IsFinished() const {
        std::lock_guard<std::mutex> l(mutex_);
        return (state_ == TaskState::kFinished);
    }

    bool
    IsRunningLocked() const {
        return (state_ == TaskState::kRunning);
    }

    bool
    IsFinishedLocked() const {
        return (state_ == TaskState::kFinished);
    }

    void
    Terminate(TaskState state);

    std::exception_ptr
    error() const {
        std::lock_guard<std::mutex> l(mutex_);
        return exception_;
    }

    void
    DriverClosedLocked() {
        return;
    }

    void
    RequestCancel() {
        Terminate(TaskState::kCanceled);
    }

 private:
    std::string uuid_;

    std::string taskid_;

    plan::PlanFragment plan_fragment_;

    int destination_;

    std::shared_ptr<QueryContext> query_context_;

    std::exception_ptr exception_ = nullptr;

    std::function<void(std::exception_ptr)> on_error_;

    std::unique_ptr<DriverFactory> driver_factory_;

    // 当前实际上并不会出现多Driver的情况，因此只保留一个Driver以简化代码
    std::shared_ptr<Driver> driver_;

    mutable std::mutex mutex_;

    TaskState state_ = TaskState::kRunning;
};

}  // namespace exec
}  // namespace milvus