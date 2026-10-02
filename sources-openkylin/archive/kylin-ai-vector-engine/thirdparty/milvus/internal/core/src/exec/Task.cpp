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

#include "Task.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "common/EasyAssert.h"
#include "log/Log.h"

namespace milvus {
namespace exec {

// Special group id to reflect the ungrouped execution.
constexpr uint32_t kUngroupedGroupId{std::numeric_limits<uint32_t>::max()};

std::string
MakeUuid() {
    return boost::lexical_cast<std::string>(boost::uuids::random_generator()());
}

std::shared_ptr<Task>
Task::Create(const std::string& task_id,
             plan::PlanFragment plan_fragment,
             int destination,
             std::shared_ptr<QueryContext> query_context,
             std::function<void(std::exception_ptr)> on_error) {
    return std::shared_ptr<Task>(new Task(task_id,
                                          std::move(plan_fragment),
                                          destination,
                                          std::move(query_context),
                                          std::move(on_error)));
}

void
Task::SetError(const std::exception_ptr& exception) {
    {
        std::lock_guard<std::mutex> l(mutex_);
        if (!IsRunningLocked()) {
            return;
        }

        if (exception_ != nullptr) {
            return;
        }
        exception_ = exception;
    }

    Terminate(TaskState::kFailed);

    if (on_error_) {
        on_error_(exception_);
    }
}

void
Task::SetError(const std::string& message) {
    try {
        throw std::runtime_error(message);
    } catch (const std::runtime_error& e) {
        SetError(std::current_exception());
    }
}

void
Task::Terminate(TaskState state) {
    if (driver_) {
        driver_->CloseByTask();
    }
}

RowVectorPtr
Task::Next(ContinueFuture* future) {
    // NOTE: Task::Next is single-threaded execution
    AssertInfo(plan_fragment_.execution_strategy_ ==
                   plan::ExecutionStrategy::kUngrouped,
               "Single-threaded execution supports only ungrouped execution");

    AssertInfo(state_ == TaskState::kRunning,
               "Task has already finished processing.");

    if (!driver_) {
        std::vector<std::unique_ptr<DriverFactory>> driver_factories;
        // 创建单个 Driver
        LocalPlanner::Plan(plan_fragment_,
                           &driver_factories,
                           *query_context_->query_config(),
                           1);

        AssertInfo(
            driver_factories.size() == 1,
            "Single-threaded execution should have exactly one driver factory");

        driver_factory_ = std::move(driver_factories[0]);
        AssertInfo(driver_factory_->SupportSingleThreadExecution(),
                   "Driver factory must support single-threaded execution");

        auto self = shared_from_this();
        driver_ = driver_factory_->CreateDriver(
            std::make_unique<DriverContext>(self, 0, 0, kUngroupedGroupId, 0),
            [self](size_t i) { return 1; }  // 只有一个 Driver
        );
    }

    // 直接处理单个 Driver
    auto result = driver_->Next();

    if (result) {
        return result;
    }

    if (error()) {
        std::rethrow_exception(error());
    }

    return nullptr;
}

}  // namespace exec
}  // namespace milvus
