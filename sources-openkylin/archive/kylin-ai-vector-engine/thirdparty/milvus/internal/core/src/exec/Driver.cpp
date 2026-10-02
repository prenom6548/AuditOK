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

#include "Driver.h"

#include <cassert>
#include <memory>

#include "exec/operator/FilterBits.h"
#include "exec/operator/Operator.h"
#include "exec/Task.h"

#include "common/EasyAssert.h"
#include "log/Log.h"

namespace milvus {
namespace exec {

std::shared_ptr<QueryConfig>
DriverContext::GetQueryConfig() {
    return task_->query_context()->query_config();
}

std::shared_ptr<Driver>
DriverFactory::CreateDriver(std::unique_ptr<DriverContext> ctx,
                            std::function<int(int pipelineid)> num_drivers) {
    auto driver = std::shared_ptr<Driver>(new Driver());
    ctx->driver_ = driver.get();

    for (size_t i = 0; i < plannodes_.size(); ++i) {
        auto id = plannodes_.size();
        auto plannode = plannodes_[i];
        auto filternode =
            std::dynamic_pointer_cast<const plan::FilterBitsNode>(plannode);
        AssertInfo(filternode != nullptr,
                   "Current plannode is not a FilterBitsNode");

        auto op = std::make_unique<FilterBits>(id, ctx.get(), filternode);
        // TODO: add more operators

        driver->Init(std::move(ctx), std::move(op));
    }

    return driver;
}

void
Driver::Enqueue(std::shared_ptr<Driver> driver) {
    if (driver->closed_) {
        return;
    }

    Driver::Run(driver);
}

void
Driver::Run(std::shared_ptr<Driver> self) {
    RowVectorPtr result;
    self->RunInternal(self, result);

    AssertInfo(result == nullptr,
               "The last operator (sink) must not produce any results.");
}

void
Driver::Init(std::unique_ptr<DriverContext> ctx, std::unique_ptr<Operator> op) {
    assert(ctx != nullptr);
    ctx_ = std::move(ctx);
    AssertInfo(op != nullptr, "operator in driver must not empty");

    operator_ = std::move(op);
}

void
Driver::Close() {
    if (closed_) {
        return;
    }

    operator_->Close();

    closed_ = true;

    Task::RemoveDriver(ctx_->task_, this);
}

RowVectorPtr
Driver::Next() {
    auto self = shared_from_this();

    RowVectorPtr result;
    RunInternal(self, result);

    return result;
}

#define CALL_OPERATOR(call_func, operator, method_name)                        \
    try {                                                                      \
        call_func;                                                             \
    } catch (SegcoreError & e) {                                               \
        auto err_msg = fmt::format(                                            \
            "Operator::{} failed for [Operator:{}, plan node id: "             \
            "{}] : {}",                                                        \
            method_name,                                                       \
            operator->get_operator_type(),                                     \
            operator->get_plannode_id(),                                       \
            e.what());                                                         \
        LOG_ERROR(err_msg);                                                    \
        throw ExecOperatorException(err_msg);                                  \
    } catch (std::exception & e) {                                             \
        throw ExecOperatorException(                                           \
            fmt::format("Operator::{} failed for [Operator:{}, plan node id: " \
                        "{}] : {}",                                            \
                        method_name,                                           \
                        operator->get_operator_type(),                         \
                        operator->get_plannode_id(),                           \
                        e.what()));                                            \
    }

void
Driver::RunInternal(std::shared_ptr<Driver>& self, RowVectorPtr& result) {
    try {
        auto op = operator_.get();

        // 执行operator，获取输出结果
        CALL_OPERATOR(result = op->GetOutput(), op, "GetOutput");
        if (result) {
            AssertInfo(result->size() > 0,
                       fmt::format("GetOutput must return nullptr or "
                                   "a non-empty vector: {}",
                                   op->get_operator_type()));
            return;
        }

        // 检查是否完成
        if (op->IsFinished()) {
            Close();
            return;
        }
    } catch (std::exception& e) {
        get_task()->SetError(std::current_exception());
        return;
    }
}

static bool
MustStartNewPipeline(std::shared_ptr<const plan::PlanNode> plannode,
                     int source_id) {
    //TODO: support LocalMerge and other shuffle
    return source_id != 0;
}

uint32_t
MaxDrivers(const DriverFactory* factory, const QueryConfig& config) {
    return 1;
}

static void
SplitPlan(const std::shared_ptr<const plan::PlanNode>& plannode,
          std::vector<std::shared_ptr<const plan::PlanNode>>* current_plannodes,
          const std::shared_ptr<const plan::PlanNode>& consumer_node,
          std::vector<std::unique_ptr<DriverFactory>>* driver_factories) {
    if (!current_plannodes) {
        driver_factories->push_back(std::make_unique<DriverFactory>());
        current_plannodes = &driver_factories->back()->plannodes_;
        driver_factories->back()->consumer_node_ = consumer_node;
    }

    auto sources = plannode->sources();
    if (sources.empty()) {
        driver_factories->back()->is_input_driver_ = true;
    } else {
        for (int i = 0; i < sources.size(); ++i) {
            SplitPlan(
                sources[i],
                MustStartNewPipeline(plannode, i) ? nullptr : current_plannodes,
                plannode,
                driver_factories);
        }
    }
    current_plannodes->push_back(plannode);
}

void
LocalPlanner::Plan(
    const plan::PlanFragment& fragment,
    std::vector<std::unique_ptr<DriverFactory>>* driver_factories,
    const QueryConfig& config,
    uint32_t max_drivers) {
    SplitPlan(fragment.plan_node_, nullptr, nullptr, driver_factories);

    (*driver_factories)[0]->is_output_driver_ = true;

    for (auto& factory : *driver_factories) {
        factory->max_drivers_ = MaxDrivers(factory.get(), config);
        factory->num_drivers_ = std::min(factory->max_drivers_, max_drivers);

        if (factory->is_group_execution_) {
            factory->num_total_drivers_ =
                factory->num_drivers_ * fragment.num_splitgroups_;
        } else {
            factory->num_total_drivers_ = factory->num_drivers_;
        }
    }
}

}  // namespace exec
}  // namespace milvus
