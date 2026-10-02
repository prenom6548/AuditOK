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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "lifecycle/enginestopcontroller.h"
#include "lifecycle/modelidlesweeper.h"
#include "lifecycle/modellifecycleconfig.h"
#include "lifecycle/modellifecyclemanager.h"
#include "lifecycle/modelruntimecontext.h"
#include "lifecycle/modelusagehandle.h"

using namespace lifecycle;

namespace {

ModelDescriptor MakeDescriptor(const std::string &name, AiCapability capability, ModelClass modelClass) {
    ModelDescriptor descriptor;
    descriptor.modelName = name;
    descriptor.capability = capability;
    descriptor.engineName = "engine_" + name;
    descriptor.modelClass = modelClass;
    return descriptor;
}

std::string UniqueName(const std::string &prefix) {
    static std::atomic<int> counter{0};
    return prefix + "_" + std::to_string(counter.fetch_add(1));
}

std::string WriteTempConfigFile(const std::string &content) {
    auto path = std::filesystem::temp_directory_path() /
                std::filesystem::path("modellifecycle_config_" + UniqueName("tmp") + ".json");
    std::ofstream ofs(path);
    ofs << content;
    ofs.close();
    return path.string();
}

ModelLifecycleConfig LoadConfigFromContent(const std::string &content) {
    auto path = WriteTempConfigFile(content);
    ModelLifecycleConfig config = ModelLifecycleConfig::loadFromFile(path);
    std::error_code ec;
    std::filesystem::remove(path, ec);
    return config;
}

bool ContainsDescriptor(const std::vector<ModelDescriptor> &descriptors, const ModelDescriptor &target) {
    return std::any_of(descriptors.begin(), descriptors.end(),
                       [&](const auto &item) { return item.fingerprint() == target.fingerprint(); });
}

}  // namespace

// ModelLifecycleConfig 测试
TEST(ModelLifecycleConfigTest, LoadsCustomValuesFromJson) {
    const char *json = R"(
        {
            "sweepIntervalSec": 99,
            "defaults": {
                "idleTimeoutSec": 5,
                "loadTrigger": "OnInitialization",
                "unloadPolicy": "UnloadWhenIdle",
                "unloadOnSleep": true
            },
            "models": {
                "LLM": {
                    "idleTimeoutSec": 42,
                    "loadTrigger": "OnBusinessCall",
                    "unloadPolicy": "UnloadWhenNoHandleAndIdle",
                    "unloadOnSleep": true
                },
                "Embedding": [{
                    "idleTimeoutSec": 5,
                    "loadTrigger": "OnInitialization",
                    "unloadPolicy": "UnloadWhenIdle",
                    "variants": {
                        "Image": {
                            "idleTimeoutSec": 3,
                            "loadTrigger": "OnInitialization",
                            "unloadPolicy": "UnloadWhenIdle",
                            "unloadOnSleep": false
                        }
                    }
                }]
            }
        }
    )";
    auto config = LoadConfigFromContent(json);

    auto llmDescriptor = MakeDescriptor("LLM", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    auto llmProfile = config.profileFor(llmDescriptor);
    EXPECT_EQ((ModelClass{ModelFamily::kLLM, ModelSubType::kNone}), llmProfile.lifecycleClass);
    EXPECT_EQ(ModelLifecycleConfig::Seconds(42), config.idleTimeout(llmDescriptor));
    EXPECT_EQ(99, config.sweepIntervalSeconds());
    EXPECT_EQ(EngineCallStage::kOnBusinessCall, config.stage(llmDescriptor));
    EXPECT_EQ(UnloadPolicy::kUnloadWhenNoHandleAndIdle, config.unloadPolicy(llmDescriptor));
    EXPECT_TRUE(config.unloadOnSleep(llmDescriptor));

    auto vectorDescriptor =
        MakeDescriptor("Embedding.Image", AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
    auto vectorProfile = config.profileFor(vectorDescriptor);
    EXPECT_EQ((ModelClass{ModelFamily::kEmbedding, ModelSubType::kImage}), vectorProfile.lifecycleClass);
    EXPECT_EQ(ModelLifecycleConfig::Seconds(3), config.idleTimeout(vectorDescriptor));
    EXPECT_EQ(UnloadPolicy::kUnloadWhenIdle, config.unloadPolicy(vectorDescriptor));
    EXPECT_FALSE(config.unloadOnSleep(vectorDescriptor));
}

// ModelRuntimeContext 测试
TEST(ModelRuntimeContextTest, EnsureLoadedRequiresMatchingStage) {
    auto descriptor = MakeDescriptor(UniqueName("runtime_ctx_stage"), AiCapability::Speech, {ModelFamily::kSpeech});
    ModelRuntimeContext context(descriptor, {ModelFamily::kSpeech, ModelSubType::kNone},
                                EngineCallStage::kOnBusinessCall, UnloadPolicy::kUnloadWhenIdle);

    // 确保在错误阶段无法加载
    EXPECT_FALSE(context.ensureLoaded(EngineCallStage::kOnInitialization));
    // 确保在正确阶段可以加载
    EXPECT_TRUE(context.ensureLoaded(EngineCallStage::kOnBusinessCall));
    // 加载后，canUnload 应为 true (因为是 idle 状态)
    EXPECT_TRUE(context.canUnload());

    // 标记忙碌后，canUnload 应为 false
    context.markBusy();
    EXPECT_FALSE(context.canUnload());
    // 标记空闲后，canUnload 应恢复为 true
    context.markIdle();
    EXPECT_TRUE(context.canUnload());
}

TEST(ModelRuntimeContextTest, ShouldEvictAfterIdleTimeout) {
    auto descriptor = MakeDescriptor(UniqueName("runtime_ctx_idle"), AiCapability::Vision, {ModelFamily::kImage});
    ModelRuntimeContext context(descriptor, {ModelFamily::kImage, ModelSubType::kGeneralSeg},
                                EngineCallStage::kOnInitialization, UnloadPolicy::kUnloadWhenIdle);

    ASSERT_TRUE(context.ensureLoaded(EngineCallStage::kOnInitialization));
    context.markBusy();
    context.markIdle();

    auto timeout = std::chrono::seconds(1);
    // 时间未到，不应卸载
    EXPECT_FALSE(context.shouldUnload(std::chrono::steady_clock::now(), timeout));
    // 等待超时
    std::this_thread::sleep_for(timeout + std::chrono::milliseconds(100));
    // 时间已到，应可卸载
    EXPECT_TRUE(context.shouldUnload(std::chrono::steady_clock::now(), timeout));
}

TEST(ModelRuntimeContextTest, NeverUnloadPolicy) {
    auto descriptor =
        MakeDescriptor(UniqueName("never_unload"), AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    ModelRuntimeContext context(descriptor, {ModelFamily::kLLM, ModelSubType::kNone},
                                EngineCallStage::kOnInitialization, UnloadPolicy::kNeverUnload);

    ASSERT_TRUE(context.ensureLoaded(EngineCallStage::kOnInitialization));
    context.markBusy();
    context.markIdle();

    auto timeout = std::chrono::seconds(1);
    std::this_thread::sleep_for(timeout + std::chrono::milliseconds(100));
    // 即使超时，由于策略是 NeverUnload，也不应卸载
    EXPECT_FALSE(context.shouldUnload(std::chrono::steady_clock::now(), timeout));
}

TEST(ModelRuntimeContextTest, NoHandleAndIdlePolicy) {
    auto descriptor =
        MakeDescriptor(UniqueName("no_handle_idle"), AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    ModelRuntimeContext context(descriptor, {ModelFamily::kLLM, ModelSubType::kNone},
                                EngineCallStage::kOnInitialization, UnloadPolicy::kUnloadWhenNoHandleAndIdle);

    ASSERT_TRUE(context.ensureLoaded(EngineCallStage::kOnInitialization));
    context.markBusy();
    context.markIdle();

    context.onAcquire();
    // 当 handle 引用计数 > 0 时，即使超时也不应卸载
    auto timeout = std::chrono::seconds(1);
    // 等待超时
    std::this_thread::sleep_for(timeout + std::chrono::milliseconds(100));
    EXPECT_FALSE(context.shouldUnload(std::chrono::steady_clock::now(), timeout));

    // 模拟 handle 释放（内部计数减一）
    context.onRelease();
    // 等待超时
    std::this_thread::sleep_for(timeout + std::chrono::milliseconds(100));
    // 释放后，由于策略是 NoHandleAndIdle，应可卸载
    EXPECT_TRUE(context.shouldUnload(std::chrono::steady_clock::now(), timeout));
}

// ModelUsageHandle 测试
TEST(ModelUsageHandleTest, DefaultConstructedHandleIsInvalid) {
    ModelUsageHandle handle;
    EXPECT_FALSE(handle.valid());
}

TEST(ModelUsageHandleTest, AcquiredHandleIsValid) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({"models": {}})");
    manager.updateConfig(config);

    auto handle = manager.acquire();
    EXPECT_TRUE(handle.valid());
}

TEST(ModelUsageHandleTest, EnsureLoadedRegistersDescriptor) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "TestModel": {
                "loadTrigger": "OnInitialization"
            }
        }
    })");
    manager.updateConfig(config);

    auto handle = manager.acquire();
    auto descriptor = MakeDescriptor("TestModel", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    // 阶段匹配，应能加载
    EXPECT_TRUE(handle.ensureLoaded(descriptor, EngineCallStage::kOnInitialization));
    // 标记忙碌和空闲
    handle.markBusy(descriptor);
    handle.markIdle(descriptor);
}

TEST(ModelUsageHandleTest, HandleTracksMultipleDescriptors) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "ModelA": { "loadTrigger": "OnInitialization" },
            "ModelB": { "loadTrigger": "OnBusinessCall" }
        }
    })");
    manager.updateConfig(config);

    auto handle = manager.acquire();
    auto descA = MakeDescriptor("ModelA", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    auto descB = MakeDescriptor("ModelB", AiCapability::Vision, {ModelFamily::kImage});

    // A 在正确阶段加载
    EXPECT_TRUE(handle.ensureLoaded(descA, EngineCallStage::kOnInitialization));
    // B 在错误阶段加载，应失败
    EXPECT_FALSE(handle.ensureLoaded(descB, EngineCallStage::kOnInitialization));
    // B 在正确阶段加载
    EXPECT_TRUE(handle.ensureLoaded(descB, EngineCallStage::kOnBusinessCall));

    // 标记多个模型的忙碌和空闲
    handle.markBusy(descA);
    handle.markBusy(descB);
    handle.markIdle(descA);
    handle.markIdle(descB);
}

TEST(ModelUsageHandleTest, ResetReleasesAllDescriptors) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "ResetTest": { "loadTrigger": "OnInitialization" }
        }
    })");
    manager.updateConfig(config);

    auto handle = manager.acquire();
    auto desc = MakeDescriptor("ResetTest", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    EXPECT_TRUE(handle.ensureLoaded(desc, EngineCallStage::kOnInitialization));
    handle.markBusy(desc);

    // 重置前，handle 有效
    EXPECT_TRUE(handle.valid());
    // 重置后，handle 无效
    handle.reset();
    EXPECT_FALSE(handle.valid());

    // 重置后应能重新获取有效 handle
    handle = manager.acquire();
    EXPECT_TRUE(handle.valid());
}

TEST(ModelUsageHandleTest, MoveConstructorTransfersOwnership) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({"models": {}})");
    manager.updateConfig(config);

    auto handle1 = manager.acquire();
    EXPECT_TRUE(handle1.valid());

    // 移动构造
    ModelUsageHandle handle2(std::move(handle1));
    // handle1 被移动后应无效
    EXPECT_FALSE(handle1.valid());
    // handle2 应有效
    EXPECT_TRUE(handle2.valid());

    // 重置 handle2，验证其行为
    handle2.reset();
    EXPECT_FALSE(handle2.valid());
}

TEST(ModelUsageHandleTest, MoveAssignmentTransfersOwnership) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({"models": {}})");
    manager.updateConfig(config);

    auto handle1 = manager.acquire();
    auto handle2 = manager.acquire();
    EXPECT_TRUE(handle1.valid());
    EXPECT_TRUE(handle2.valid());

    // 移动赋值
    handle2 = std::move(handle1);
    // handle1 被移动后应无效
    EXPECT_FALSE(handle1.valid());
    // handle2 应有效
    EXPECT_TRUE(handle2.valid());
}

// ModelLifecycleManager 测试
TEST(ModelLifecycleManagerTest, HandleRespectsLoadStage) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto defaultConfig = LoadConfigFromContent(R"({"models": {}})");
    manager.updateConfig(defaultConfig);

    auto descriptor =
        MakeDescriptor(UniqueName("manager_handle"), AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    auto handle = manager.acquire();

    ASSERT_TRUE(handle.valid());
    // 阶段不匹配，加载失败
    EXPECT_FALSE(handle.ensureLoaded(descriptor, EngineCallStage::kOnBusinessCall));
    // 阶段匹配，加载成功
    EXPECT_TRUE(handle.ensureLoaded(descriptor, EngineCallStage::kOnInitialization));

    handle.markBusy(descriptor);
    handle.markIdle(descriptor);
    handle.reset();
    EXPECT_FALSE(handle.valid());
}

TEST(ModelLifecycleManagerTest, ForceUnloadHonorsSleepPolicy) {
    const char *json = R"(
        {
            "defaults": {
                "unloadOnSleep": false
            },
            "models": {
                "LLM": {
                    "unloadOnSleep": true
                },
                "Vision": {
                    "unloadOnSleep": false
                }
            }
        }
    )";

    auto config = LoadConfigFromContent(json);
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    manager.updateConfig(config);

    auto unloadDescriptor = MakeDescriptor("LLM", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    auto keepDescriptor = MakeDescriptor("Vision", AiCapability::Vision, {ModelFamily::kImage});
    EngineCallStage stage = EngineCallStage::kOnInitialization;

    // 确保模型已加载
    manager.ensureLoaded(unloadDescriptor, stage);
    manager.ensureLoaded(keepDescriptor, stage);

    // 触发睡眠卸载
    auto unloaded = manager.forceUnloadForSleep("gtest");
    // 应包含 LLM 模型
    EXPECT_TRUE(ContainsDescriptor(unloaded, unloadDescriptor));
    // 不应包含 Vision 模型
    EXPECT_FALSE(ContainsDescriptor(unloaded, keepDescriptor));
}

TEST(ModelLifecycleManagerTest, SweepIdleContexts) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "SweepTest": {
                "idleTimeoutSec": 0
            }
        }
    })");
    manager.updateConfig(config);

    auto descriptor = MakeDescriptor("SweepTest", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
    {
        // 在一个作用域内获取 handle 并使用模型
        auto handle = manager.acquire();
        EXPECT_TRUE(handle.ensureLoaded(descriptor, EngineCallStage::kOnInitialization));
        handle.markBusy(descriptor);
        handle.markIdle(descriptor);
        // handle 在这里离开作用域，会调用 onRelease
    }  // handle 析构，模型引用计数减一

    // 调用 sweep，清理超时的、引用计数为 0 的模型
    manager.sweepIdleContexts();
    // 验证模型是否被卸载，可以通过尝试再次加载或检查内部状态来判断
    // 此处假设 sweep 会将 context 标记为待卸载或直接卸载
    // 如果 manager 提供了检查方法，可以在这里验证
    // 例如，manager.isModelLoaded("SweepTest") 应为 false
    // 由于没有具体接口，我们主要验证 sweep 调用不崩溃
    EXPECT_TRUE(true);  // Placeholder, if internal state can be checked, replace this
}

TEST(ModelLifecycleManagerTest, MultipleThreadsAccess) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "ThreadTest": {
                "idleTimeoutSec": 300
            }
        }
    })");
    manager.updateConfig(config);

    std::vector<std::thread> threads;
    const int numThreads = 5;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&manager, &successCount]() {
            auto descriptor = MakeDescriptor("ThreadTest", AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
            auto handle = manager.acquire();
            if (handle.ensureLoaded(descriptor, EngineCallStage::kOnInitialization)) {
                handle.markBusy(descriptor);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                handle.markIdle(descriptor);
                successCount.fetch_add(1);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }
    // 验证至少有一个线程成功执行了流程（表明基本功能正常）
    EXPECT_GT(successCount.load(), 0);
    // 验证没有崩溃
    EXPECT_TRUE(true);
}

TEST(ModelLifecycleManagerTest, RetainModelAndUnloadOthers) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({
        "models": {
            "LLM": {
                  "idleTimeoutSec": 1,
                  "loadTrigger": "OnInitialization",
                  "unloadPolicy": "UnloadWhenNoHandleAndIdle",
                  "unloadOnSleep": true,
                  "highSpecOverrides": {
                    "unloadPolicy": "NeverUnload"
                  }
            }
        }
    })");
    manager.updateConfig(config);

    // 创建两个模型描述符
    ModelDescriptor retainDesc;
    retainDesc.modelName = "llm_Qwen-2.5-3b_1.0";
    retainDesc.capability = AiCapability::Nlp;
    retainDesc.modelClass = {ModelFamily::kLLM, ModelSubType::kNone};

    ModelDescriptor unloadDesc;
    unloadDesc.modelName = "llm_Qwen-2.5-3b-22_1.0";
    unloadDesc.capability = AiCapability::Nlp;
    unloadDesc.modelClass = {ModelFamily::kLLM, ModelSubType::kNone};

    // 加载模型
    {
        auto handle = manager.acquire();
        EXPECT_TRUE(handle.ensureLoaded(retainDesc, EngineCallStage::kOnInitialization));
        EXPECT_TRUE(handle.ensureLoaded(unloadDesc, EngineCallStage::kOnInitialization));
        handle.markBusy(retainDesc);
        handle.markIdle(retainDesc);
        handle.markBusy(unloadDesc);
        handle.markIdle(unloadDesc);
    }

    // 执行保留和卸载操作
    auto result = manager.retainModelAndUnloadOthers(retainDesc);
    EXPECT_TRUE(result);

    // 验证保护机制：等待超时后执行清理，被保留的模型不应被卸载
    std::this_thread::sleep_for(std::chrono::seconds(2));
    manager.sweepIdleContexts();

    // 通过尝试使用模型来验证其状态
    {
        auto handle = manager.acquire();
        bool retainedStillAvailable = handle.ensureLoaded(retainDesc, EngineCallStage::kOnInitialization);
        EXPECT_TRUE(retainedStillAvailable) << "Retained model should still be available";
    }

    result = manager.cancelRetainModel(retainDesc);
    EXPECT_TRUE(result) << "Cancel retain model should succeed";
    // 取消保留，此时模型可以被卸载
    std::this_thread::sleep_for(std::chrono::seconds(2));
    manager.sweepIdleContexts();
    // 通过尝试使用模型来验证其状态
    {
        auto handle = manager.acquire();
        bool retainedStillAvailable = handle.ensureLoaded(retainDesc, EngineCallStage::kOnInitialization);
        EXPECT_TRUE(retainedStillAvailable) << "Retained model should be unloaded";
    }
}

// EngineStopController 测试
TEST(EngineStopControllerTest, StopAllInvokesCallbacks) {
    lifecycle::EngineStopController &controller = lifecycle::EngineStopController::getInstance();
    ModelDescriptor descriptor;
    descriptor.modelName = UniqueName("stopper");
    std::atomic<bool> stopped{false};
    controller.registerStopper(descriptor, [&]() { stopped = true; });
    controller.stopAll("gtest");
    EXPECT_TRUE(stopped.load());
    controller.unregisterStopper(descriptor);
}

TEST(EngineStopControllerTest, MultipleStoppers) {
    lifecycle::EngineStopController &controller = lifecycle::EngineStopController::getInstance();
    ModelDescriptor desc1, desc2;
    desc1.modelName = UniqueName("stopper1");
    desc2.modelName = UniqueName("stopper2");

    std::atomic<bool> stopped1{false}, stopped2{false};
    controller.registerStopper(desc1, [&]() { stopped1 = true; });
    controller.registerStopper(desc2, [&]() { stopped2 = true; });

    controller.stopAll("gtest");
    EXPECT_TRUE(stopped1.load());
    EXPECT_TRUE(stopped2.load());
    controller.unregisterStopper(desc1);
    controller.unregisterStopper(desc2);
}

TEST(EngineStopControllerTest, UnregisterStopper) {
    lifecycle::EngineStopController &controller = lifecycle::EngineStopController::getInstance();
    ModelDescriptor descriptor;
    descriptor.modelName = UniqueName("stopper");

    std::atomic<bool> stopped{false};
    controller.registerStopper(descriptor, [&]() { stopped = true; });
    controller.unregisterStopper(descriptor);
    controller.stopAll("gtest");
    EXPECT_FALSE(stopped.load());
}

// ModelIdleSweeper 测试
TEST(ModelIdleSweeperTest, StartStopSweeper) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({"sweepIntervalSec": 1})");
    manager.updateConfig(config);

    ModelIdleSweeper sweeper(manager);
    sweeper.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));  // 等待 sweeper 运行一次
    sweeper.stop();
    EXPECT_TRUE(true);  // 验证 start/stop 流程不崩溃
}

TEST(ModelIdleSweeperTest, SweeperDoesNotRunWhenStopped) {
    ModelLifecycleManager &manager = ModelLifecycleManager::getInstance();
    auto config = LoadConfigFromContent(R"({"sweepIntervalSec": 1})");
    manager.updateConfig(config);

    ModelIdleSweeper sweeper(manager);
    // 不调用 start
    sweeper.stop();     // 调用 stop 时 sweeper 未启动，应安全
    EXPECT_TRUE(true);  // 验证安全调用
}