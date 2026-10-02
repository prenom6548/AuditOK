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

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/Types.h"
#include "common/Promise.h"
#include "exec/QueryContext.h"
#include "plan/PlanNode.h"

namespace milvus {
namespace exec {

class Driver;
class Operator;
class Task;

struct DriverContext {
    int driverid_;
    int pipelineid_;
    uint32_t split_groupid_;
    uint32_t partitionid_;

    std::shared_ptr<Task> task_;
    Driver* driver_;

    explicit DriverContext(std::shared_ptr<Task> task,
                           int driverid,
                           int pipilineid,
                           uint32_t split_group_id,
                           uint32_t partition_id)
        : driverid_(driverid),
          pipelineid_(pipilineid),
          split_groupid_(split_group_id),
          partitionid_(partition_id),
          task_(task) {
    }

    std::shared_ptr<QueryConfig>
    GetQueryConfig();
};

struct DriverFactory {
    std::vector<std::shared_ptr<const plan::PlanNode>> plannodes_;
    // The (local) node that will consume results supplied by this pipeline.
    // Can be null. We use that to determine the max drivers.
    std::shared_ptr<const plan::PlanNode> consumer_node_;
    uint32_t max_drivers_;
    uint32_t num_drivers_;
    uint32_t num_total_drivers_;

    bool is_group_execution_;
    bool is_input_driver_;
    bool is_output_driver_;

    std::shared_ptr<Driver>
    CreateDriver(std::unique_ptr<DriverContext> ctx,
                 // TODO: support exchange function
                 // std::shared_ptr<ExchangeClient> exchange_client,
                 std::function<int(int pipilineid)> num_driver);

    // TODO: support ditribution compute
    bool
    SupportSingleThreadExecution() const {
        return true;
    }
};

class Driver : public std::enable_shared_from_this<Driver> {
 public:
    static void
    Enqueue(std::shared_ptr<Driver> instance);

    RowVectorPtr
    Next();

    DriverContext*
    get_driver_context() const {
        return ctx_.get();
    }

    const std::shared_ptr<Task>&
    get_task() const {
        return ctx_->task_;
    }

    void
    Init(std::unique_ptr<DriverContext> driver_ctx,
         std::unique_ptr<Operator> op);

    void
    CloseByTask() {
        Close();
    }

 private:
    Driver() = default;

    void
    EnqueueInternal() {
    }

    static void
    Run(std::shared_ptr<Driver> self);

    void
    RunInternal(std::shared_ptr<Driver>& self,
                RowVectorPtr& result);

    void
    Close();

    std::unique_ptr<DriverContext> ctx_;

    std::atomic_bool closed_{false};

    // 之前的设计中预留了并发执行的能力，但是实际上并没有使用
    // 现在改为了单线程执行，所以只需要一个operator
    // std::vector<std::unique_ptr<Operator>> operators_;
    std::unique_ptr<Operator> operator_;

    friend struct DriverFactory;
};

class LocalPlanner {
 public:
    static void
    Plan(const plan::PlanFragment& fragment,
         std::vector<std::unique_ptr<DriverFactory>>* driver_factories,
         const QueryConfig& config,
         uint32_t max_drivers);
};

}  // namespace exec
}  // namespace milvus