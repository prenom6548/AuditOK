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
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#include "kylin-ai/ai-engine/textgenerationengine.h"
#include "kylin-ai/common/error.h"

#define private public
#include "taskmanager/taskrunner.h"
#undef private

#include "services/genai/nlp/chatbot.h"
#include "services/modelconfig/modelconfigmanager.h"
#include "utils/util.h"

namespace {

class ScopedChatConfigOverride {
public:
    explicit ScopedChatConfigOverride(int64_t timeoutSec) { writeConfig(makeTimeoutConfig(timeoutSec)); }

    ScopedChatConfigOverride(const std::string& content, bool writeFile) { writeConfig(content, writeFile); }

    static std::string makeTimeoutConfig(int64_t timeoutSec) {
        std::ostringstream oss;
        oss << "{\n"
            << "  \"taskTimeoutSec\": {\n"
            << "    \"TextChat\": " << timeoutSec << "\n"
            << "  }\n"
            << "}\n";
        return oss.str();
    }

    static std::string makeTimeoutAndQueueConfig(int64_t timeoutSec, int queueMaxSize) {
        std::ostringstream oss;
        oss << "{\n"
            << "  \"taskTimeoutSec\": {\n"
            << "    \"TextChat\": " << timeoutSec << "\n"
            << "  },\n"
            << "  \"requestQueueMaxSize\": {\n"
            << "    \"TextChat\": " << queueMaxSize << "\n"
            << "  }\n"
            << "}\n";
        return oss.str();
    }

    ~ScopedChatConfigOverride() { restoreConfig(); }

private:
    void initPath() {
        auto dir =
            std::filesystem::temp_directory_path() / ("kylin-ai-runtime-testchatbot-" + std::to_string(getpid()));
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        configPath_ = dir / "task.json";
    }

    void backupConfig() { task::setTaskRunnerConfigPathForTesting(configPath_); }

    void writeConfig(const std::string& content, bool writeFile = true) {
        initPath();
        backupConfig();
        if (writeFile) {
            std::ofstream ofs(configPath_);
            ofs << content;
            ofs.close();
        } else {
            std::error_code ec;
            std::filesystem::remove(configPath_, ec);
        }
    }

    void restoreConfig() {
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

class ScopedPendingLimits {
public:
    ScopedPendingLimits(size_t maxTotal, size_t maxPerType) {
        task::TaskRunner::getInstance().setPendingLimitsForTesting(maxTotal, maxPerType);
    }
    ~ScopedPendingLimits() { task::TaskRunner::getInstance().setPendingLimitsForTesting(10000, 2000); }
};

struct CallbackState {
    std::mutex mutex;
    std::condition_variable cv;
    int finished{0};
    int results{0};
    int lastError{0};
    int rejected{0};
};

class FakeTextGenerationEngine : public ai_engine::gen_ai::nlp::AbstractTextGenerationEngine {
public:
    FakeTextGenerationEngine(bool blockOnChat, int sleepMs, bool emitCallback, bool initSuccess, bool chatSuccess,
                             bool generateSuccess, int errorCode, const std::string& errorMessage,
                             bool emitDelayedCallback, int callbackDelayMs)
        : blockOnChat_(blockOnChat),
          sleepMs_(sleepMs),
          emitCallback_(emitCallback),
          initSuccess_(initSuccess),
          chatSuccess_(chatSuccess),
          generateSuccess_(generateSuccess),
          errorCode_(errorCode),
          errorMessage_(errorMessage),
          emitDelayedCallback_(emitDelayedCallback),
          callbackDelayMs_(callbackDelayMs) {}

    static std::shared_ptr<FakeTextGenerationEngine> createBlocking() {
        return std::make_shared<FakeTextGenerationEngine>(true, 0, false, true, true, true, -1, "", false, 0);
    }

    static std::shared_ptr<FakeTextGenerationEngine> createSuccess(int sleepMs, bool emitCallback) {
        return std::make_shared<FakeTextGenerationEngine>(false, sleepMs, emitCallback, true, true, true, -1, "", false,
                                                          0);
    }

    static std::shared_ptr<FakeTextGenerationEngine> createInitFailure(int errorCode) {
        return std::make_shared<FakeTextGenerationEngine>(false, 0, false, false, true, true, errorCode, "init failed",
                                                          false, 0);
    }

    static std::shared_ptr<FakeTextGenerationEngine> createChatFailure(int errorCode) {
        return std::make_shared<FakeTextGenerationEngine>(false, 0, false, true, false, true, errorCode, "chat failed",
                                                          false, 0);
    }

    static std::shared_ptr<FakeTextGenerationEngine> createGenerateFailure(int errorCode) {
        return std::make_shared<FakeTextGenerationEngine>(false, 0, false, true, true, false, errorCode,
                                                          "generate failed", false, 0);
    }

    static std::shared_ptr<FakeTextGenerationEngine> createDelayedCallback(int timeoutMs) {
        return std::make_shared<FakeTextGenerationEngine>(true, 0, true, true, true, true, -1, "", true, timeoutMs);
    }

    std::string engineName() const override { return "fake-engine"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return false; }
    std::string modelInfo() const override { return "{}"; }
    std::string currentModel() const override { return "fake-model"; }
    std::string currentModelName() const override { return "fake-model"; }

    void setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) override {
        callback_ = callback;
    }

    ai_engine::result::AiEngineBaseResult init() override {
        if (!initSuccess_) {
            return {false, ai_engine::EngineError(errorCode_, errorMessage_)};
        }
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult destroy() override { return {true, ai_engine::EngineError()}; }

    ai_engine::result::AiEngineBaseResult generateContent(const std::string& input) override {
        return runRequest(input, generateSuccess_);
    }

    ai_engine::result::AiEngineBaseResult chat(const std::string& message) override {
        return runRequest(message, chatSuccess_);
    }

    void stopGenerateContent() override { stopChat(); }

    void stopChat() override {
        ai_engine::gen_ai::nlp::TextGenerationResult result{};
        result.success = true;
        result.modelName = "fake-model";
        result.text = R"({"result":"stopped","is_end":true})";
        if (callback_) {
            callback_(result);
        }
        stopRequested_.store(true);
        cv_.notify_all();
    }

    int maxRunning() const { return maxRunning_.load(std::memory_order_relaxed); }
    int chatCalls() const { return chatCalls_.load(std::memory_order_relaxed); }

private:
    ai_engine::result::AiEngineBaseResult runRequest(const std::string&, bool success) {
        chatCalls_.fetch_add(1, std::memory_order_relaxed);
        int running = currentRunning_.fetch_add(1, std::memory_order_relaxed) + 1;
        int observedMax = maxRunning_.load(std::memory_order_relaxed);
        while (running > observedMax &&
               !maxRunning_.compare_exchange_weak(observedMax, running, std::memory_order_relaxed)) {
        }

        if (emitCallback_ && callback_) {
            auto emit = [this]() {
                ai_engine::gen_ai::nlp::TextGenerationResult result{};
                result.success = true;
                result.modelName = "fake-model";
                result.text = R"({"result":"ok","is_end":true})";
                callback_(result);
            };
            if (emitDelayedCallback_) {
                std::thread delayed([emit, delay = callbackDelayMs_]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    emit();
                });
                delayed.detach();
            } else {
                emit();
            }
        }

        if (blockOnChat_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stopRequested_.load(); });
        } else if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }

        currentRunning_.fetch_sub(1, std::memory_order_relaxed);
        if (!success) {
            return {false, ai_engine::EngineError(errorCode_, errorMessage_)};
        }
        return {true, ai_engine::EngineError()};
    }

    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback_{};
    bool blockOnChat_{false};
    int sleepMs_{0};
    bool emitCallback_{false};
    bool initSuccess_{true};
    bool chatSuccess_{true};
    bool generateSuccess_{true};
    int errorCode_{-1};
    std::string errorMessage_{};
    bool emitDelayedCallback_{false};
    int callbackDelayMs_{0};
    std::atomic_bool stopRequested_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<int> currentRunning_{0};
    std::atomic<int> maxRunning_{0};
    std::atomic<int> chatCalls_{0};
};

Json::Value makeChatRequest(const std::string& question) {
    Json::Value body;
    body["question"] = question;
    body["top_k"] = 1;
    body["stream"] = false;
    body["web_search"] = false;
    body["enable_thinking"] = false;
    return body;
}

bool waitFor(std::condition_variable& cv, std::mutex& mutex, const std::function<bool()>& predicate,
             std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex);
    return cv.wait_for(lock, timeout, predicate);
}

}  // namespace

TEST(ChatbotTest, TimeoutTriggersError) {
    ScopedChatConfigOverride config(1);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    bool finished = false;
    int resultCalls = 0;
    int errorCode = 0;

    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(mutex);
            resultCalls += 1;
            errorCode = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(mutex);
            finished = true;
            cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return finished; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(resultCalls, 1);
    EXPECT_EQ(errorCode, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);

    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, ProcessesRequestsSequentially) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(10, true);

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    int finished = 0;
    int resultCalls = 0;

    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        finished += 1;
        if (finished >= 5) {
            cv.notify_all();
        }
    };

    for (int i = 0; i < 5; i++) {
        chatbot->chat(
            util::jsonToString(makeChatRequest("hi")),
            [&](const Chatbot::ChatResult&) {
                std::lock_guard<std::mutex> lock(mutex);
                resultCalls += 1;
            },
            onFinished);
    }

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return finished >= 5; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(resultCalls, 5);
    EXPECT_EQ(engine->chatCalls(), 5);
    EXPECT_EQ(engine->maxRunning(), 1);

    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, TimeoutConfigMissingUsesDefault) {
    ScopedChatConfigOverride config("", false);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(5, true);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, TimeoutConfigInvalidFallsBack) {
    ScopedChatConfigOverride config("not-json", true);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(5, true);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, TimeoutConfigZeroFallsBack) {
    ScopedChatConfigOverride config(0);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(5, true);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, QueueFullRejectsAllRequests) {
    ScopedChatConfigOverride config(5);
    ScopedPendingLimits limits(0, 0);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(0, false);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.error_.getCode();
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished += 1;
        if (state.finished >= 3) {
            state.cv.notify_all();
        }
    };

    for (int i = 0; i < 3; i++) {
        chatbot->chat(util::jsonToString(makeChatRequest("hi")), onResult, onFinished);
    }

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 3; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 3);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(ChatbotTest, QueueMaxSizeConfigRejectsWhenFull) {
    ScopedChatConfigOverride config(ScopedChatConfigOverride::makeTimeoutAndQueueConfig(5, 1), true);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.error_.getCode();
        if (state.lastError == AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS) {
            state.rejected += 1;
            state.cv.notify_all();
        }
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished += 1;
        if (state.finished >= 3) {
            state.cv.notify_all();
        }
    };

    for (int i = 0; i < 3; i++) {
        chatbot->chat(util::jsonToString(makeChatRequest("hi")), onResult, onFinished);
    }

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.rejected >= 1; }, std::chrono::milliseconds(1000)));
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return engine->chatCalls() >= 1; }, std::chrono::milliseconds(1000)));

    chatbot->stopChat();

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.rejected, 1);
}

TEST(ChatbotTest, QueueFullReturnsTooManyRequestsError) {
    ScopedChatConfigOverride config(ScopedChatConfigOverride::makeTimeoutAndQueueConfig(5, 1), true);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    int tooManyRequests = 0;
    int finished = 0;

    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(mutex);
        if (result.error_.getCode() == AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS) {
            tooManyRequests += 1;
            cv.notify_all();
        }
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        finished += 1;
        cv.notify_all();
    };

    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-1", onResult, onFinished);
    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-2", onResult, onFinished);
    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-3", onResult, onFinished);

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return tooManyRequests >= 1; }, std::chrono::milliseconds(1000)));
    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return engine->chatCalls() >= 1; }, std::chrono::milliseconds(1000)));

    chatbot->stopChat();
    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return finished >= 2; }, std::chrono::milliseconds(2000)));
}

TEST(ChatbotTest, InitFailureReturnsError) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    int errorCode = (int)ai_engine::NlpEngineErrorCode::NomoreMemory;
    auto engine = FakeTextGenerationEngine::createInitFailure(errorCode);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, errorCode);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, ChatFailureReturnsError) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    int errorCode = (int)ai_engine::NlpEngineErrorCode::NomoreMemory;
    auto engine = FakeTextGenerationEngine::createChatFailure(errorCode);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, errorCode);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, GenerateFailureReturnsError) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    int errorCode = (int)ai_engine::NlpEngineErrorCode::NomoreMemory;
    auto engine = FakeTextGenerationEngine::createGenerateFailure(errorCode);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    Json::Value body;
    body["prompt"] = "hi";
    chatbot->generateContent(
        util::jsonToString(body),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, errorCode);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, NetworkErrorReturnsErrorWhenNotDefaultConfig) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    int errorCode = (int)ai_engine::NlpEngineErrorCode::NetworkDisconnected;
    auto engine = FakeTextGenerationEngine::createChatFailure(errorCode);

    GenNlpModelConfig requestConfig{};
    requestConfig.name = "test-model";
    requestConfig.type = DeployType::Custom;
    chatbot->setEngineForTesting(engine, requestConfig);

    CallbackState state;
    Json::Value request = makeChatRequest("hi");
    request["model_name"] = requestConfig.name;
    request["deploy_type"] = "Custom";
    chatbot->chat(
        util::jsonToString(request),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, errorCode);
}

TEST(ChatbotTest, CallbackNullDoesNotCrash) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createSuccess(5, true);

    chatbot->setEngineForTesting(engine);
    chatbot->chat(util::jsonToString(makeChatRequest("hi")), nullptr, nullptr);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, StopChatRequestsEngineStop) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult&) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    chatbot->stopChat();

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, StopChatWithRequestIdStopsCurrentRequest) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    bool gotResult = false;
    int finished = 0;
    Chatbot::ChatResult stopResult{};

    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")), "req-1",
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(mutex);
            if (result.finishReason_ == "stop") {
                stopResult = result;
                gotResult = true;
            }
        },
        [&]() {
            std::lock_guard<std::mutex> lock(mutex);
            finished += 1;
            cv.notify_all();
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    chatbot->stopChat("req-1");

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return gotResult && finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(stopResult.requestId_, "req-1");
    EXPECT_EQ(stopResult.error_.getCode(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(stopResult.isEnd_);
    EXPECT_EQ(stopResult.finishReason_, "stop");
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, StopChatWithRequestIdStopsPendingRequest) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    int finished = 0;
    int stoppedReq1 = 0;
    int stoppedReq2 = 0;
    Chatbot::ChatResult pendingResult{};

    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(mutex);
        if (result.finishReason_ != "stop") {
            return;
        }
        if (result.requestId_ == "req-1") {
            stoppedReq1 += 1;
        } else if (result.requestId_ == "req-2") {
            pendingResult = result;
            stoppedReq2 += 1;
        }
        cv.notify_all();
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        finished += 1;
        cv.notify_all();
    };

    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-1", onResult, onFinished);
    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-2", onResult, onFinished);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    chatbot->stopChat("req-2");

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return stoppedReq2 == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(stoppedReq1, 0);
    EXPECT_EQ(finished, 1);
    EXPECT_EQ(pendingResult.requestId_, "req-2");
    EXPECT_EQ(pendingResult.error_.getCode(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(pendingResult.isEnd_);
    EXPECT_EQ(pendingResult.finishReason_, "stop");

    chatbot->stopChat("req-1");
    ASSERT_TRUE(waitFor(
        cv, mutex,
        [&]() {
            std::cout << "fnished " << finished << std::endl;
            return finished >= 2 && stoppedReq1 == 1;
        },
        std::chrono::milliseconds(1000)));
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, StopChatStopsSpecifiedRequestWhenMultipleQueued) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    int finished = 0;
    int stoppedReq1 = 0;
    int stoppedReq3 = 0;
    Chatbot::ChatResult stoppedResult{};

    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(mutex);
        if (result.finishReason_ != "stop") {
            return;
        }
        if (result.requestId_ == "req-1") {
            stoppedReq1 += 1;
        } else if (result.requestId_ == "req-3") {
            stoppedReq3 += 1;
            stoppedResult = result;
        }
        cv.notify_all();
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        finished += 1;
        cv.notify_all();
    };

    for (int i = 1; i <= 5; i++) {
        chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-" + std::to_string(i), onResult, onFinished);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    chatbot->stopChat("req-3");

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return stoppedReq3 == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(stoppedReq1, 0);
    EXPECT_EQ(finished, 1);
    EXPECT_EQ(stoppedResult.requestId_, "req-3");
    EXPECT_EQ(stoppedResult.error_.getCode(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(stoppedResult.isEnd_);
    EXPECT_EQ(stoppedResult.finishReason_, "stop");

    chatbot->stopChat("req-1");
    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return stoppedReq1 == 1 && finished >= 2; }, std::chrono::milliseconds(1000)));
    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return finished >= 5; }, std::chrono::milliseconds(1000)));
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, StopChatWithUnknownRequestIdDoesNotStopCurrentRequest) {
    ScopedChatConfigOverride config(5);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createBlocking();

    chatbot->setEngineForTesting(engine);

    std::mutex mutex;
    std::condition_variable cv;
    int finished = 0;
    int results = 0;
    Chatbot::ChatResult lastResult{};

    auto onResult = [&](const Chatbot::ChatResult& result) {
        std::lock_guard<std::mutex> lock(mutex);
        results += 1;
        lastResult = result;
        cv.notify_all();
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        finished += 1;
        cv.notify_all();
    };

    chatbot->chat(util::jsonToString(makeChatRequest("hi")), "req-1", onResult, onFinished);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    chatbot->stopChat("req-2");

    EXPECT_FALSE(waitFor(
        cv, mutex, [&]() { return finished > 0 || results > 0; }, std::chrono::milliseconds(200)));

    chatbot->stopChat("req-1");

    ASSERT_TRUE(waitFor(
        cv, mutex, [&]() { return finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(results, 1);
    EXPECT_EQ(lastResult.requestId_, "req-1");
    EXPECT_EQ(lastResult.error_.getCode(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(lastResult.isEnd_);
    EXPECT_EQ(lastResult.finishReason_, "stop");
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, TimeoutSuppressesLateCallbacks) {
    ScopedChatConfigOverride config(1);

    auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    auto& modelConfig = ModelConfigManager::getInstance();
    EngineSelector selector(pluginManager, modelConfig);
    auto chatbot = std::make_shared<Chatbot>(selector);
    auto engine = FakeTextGenerationEngine::createDelayedCallback(1500);

    chatbot->setEngineForTesting(engine);

    CallbackState state;
    chatbot->chat(
        util::jsonToString(makeChatRequest("hi")),
        [&](const Chatbot::ChatResult& result) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.results += 1;
            state.lastError = result.error_.getCode();
        },
        [&]() {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.finished += 1;
            state.cv.notify_all();
        });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(ChatbotTest, TaskRunnerStoppingRejectedInSubprocess) {
    EXPECT_EXIT(
        {
            ScopedChatConfigOverride config(5);
            auto& pluginManager = ai_engine::AiEnginePluginManager::getInstance();
            auto& modelConfig = ModelConfigManager::getInstance();
            EngineSelector selector(pluginManager, modelConfig);
            auto chatbot = std::make_shared<Chatbot>(selector);
            auto engine = FakeTextGenerationEngine::createSuccess(0, false);

            chatbot->setEngineForTesting(engine);

            task::TaskRunner::getInstance().stop.store(true);

            CallbackState state;
            chatbot->chat(
                util::jsonToString(makeChatRequest("hi")),
                [&](const Chatbot::ChatResult& result) {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    state.results += 1;
                    state.lastError = result.error_.getCode();
                },
                [&]() {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    state.finished += 1;
                    state.cv.notify_all();
                });

            if (!waitFor(
                    state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(500))) {
                _Exit(1);
            }
            if (state.lastError != AiSdkCommonErrorCode::AISDK_SERVICE_ERROR) {
                _Exit(1);
            }
            _Exit(0);
        },
        ::testing::ExitedWithCode(0), "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    const int result = RUN_ALL_TESTS();
    std::fflush(stdout);
    std::fflush(stderr);
    _Exit(result == 0 ? 0 : 1);
}
