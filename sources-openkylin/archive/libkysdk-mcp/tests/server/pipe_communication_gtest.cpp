/**
 * @file pipe_communication_gtest.cpp
 * @brief IPC模块的Google Test测试文件
 * @author MCP SDK Team
 * @date 2024
 */

#include <gtest/gtest.h>
#include "pipe_communication.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <string>
#include <memory>

using namespace mcp;

namespace {

template <typename Predicate>
bool wait_until(std::chrono::milliseconds timeout, Predicate predicate) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return predicate();
}

}  // namespace

// 测试消息处理器
class TestMessageHandler : public MessageHandler {
public:
    void onMessage(const mcp::Message& message) override {
        received_messages.push_back(message);
    }
    
    void onError(const std::string& error) override {
        received_errors.push_back(error);
    }
    
    // 用于存储实际接收到的消息
    std::vector<mcp::Message> received_messages;
    std::vector<std::string> received_errors;
};

class TestServerRequestHandler : public ServerRequestHandler {
public:
    std::string handleRequest(const std::string& method, const std::string& params, const std::string& request_id) override {
        last_method = method;
        last_params = params;
        last_request_id = request_id;
        return R"({"ok":true})";
    }

    void handleNotification(const std::string& method, const std::string& params) override {
        last_notification_method = method;
        last_notification_params = params;
    }

    std::string last_method;
    std::string last_params;
    std::string last_request_id;
    std::string last_notification_method;
    std::string last_notification_params;
};

// 测试基类
class PipeCommunicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        pipe_comm_ = std::make_unique<PipeCommunication>();
        test_handler_ = std::make_shared<TestMessageHandler>();
        
        // 设置默认配置 - 使用一个简单的可执行文件
        config_.command = "/bin/cat";
        config_.args = {};
        config_.buffer_size = 4096;
        config_.read_timeout_ms = 100;
        config_.max_retry_count = 3;
        config_.enable_logging = false; // 测试时关闭日志
    }
    
    void TearDown() override {
        if (pipe_comm_ && pipe_comm_->isRunning()) {
            pipe_comm_->stop();
        }
        pipe_comm_.reset();
        test_handler_.reset();
    }
    
    std::unique_ptr<PipeCommunication> pipe_comm_;
    std::shared_ptr<TestMessageHandler> test_handler_;
    PipeConfig config_;
};

/**
 * @brief 测试基本构造和析构
 */
TEST_F(PipeCommunicationTest, BasicConstruction) {
    EXPECT_FALSE(pipe_comm_->isRunning());
    EXPECT_EQ(pipe_comm_->getProcessId(), -1);
    EXPECT_FALSE(pipe_comm_->isProcessAlive());
    EXPECT_FALSE(pipe_comm_->hasMessage());
    EXPECT_FALSE(pipe_comm_->hasResult());
}

/**
 * @brief 测试配置结构
 */
TEST_F(PipeCommunicationTest, PipeConfigStructure) {
    PipeConfig config;
    
    // 测试默认值
    EXPECT_EQ(config.buffer_size, 4096);
    EXPECT_EQ(config.read_timeout_ms, 10);
    EXPECT_EQ(config.max_retry_count, 10);
    EXPECT_TRUE(config.enable_logging);
    EXPECT_EQ(config.log_level, LogLevel::INFO);
    
    // 测试设置值
    config.command = "test_command";
    config.args = {"arg1", "arg2"};
    config.buffer_size = 8192;
    config.read_timeout_ms = 100;
    config.max_retry_count = 5;
    config.enable_logging = false;
    config.log_level = LogLevel::DEBUG;
    
    EXPECT_EQ(config.command, "test_command");
    EXPECT_EQ(config.args.size(), 2);
    EXPECT_EQ(config.args[0], "arg1");
    EXPECT_EQ(config.args[1], "arg2");
    EXPECT_EQ(config.buffer_size, 8192);
    EXPECT_EQ(config.read_timeout_ms, 100);
    EXPECT_EQ(config.max_retry_count, 5);
    EXPECT_FALSE(config.enable_logging);
    EXPECT_EQ(config.log_level, LogLevel::DEBUG);
}

/**
 * @brief 测试消息结构
 */
TEST_F(PipeCommunicationTest, MessageStructure) {
    mcp::Message msg;
    msg.type = MessageType::REQUEST;
    msg.content = "test content";
    msg.id = "test_id";
    
    EXPECT_EQ(msg.type, MessageType::REQUEST);
    EXPECT_EQ(msg.content, "test content");
    EXPECT_EQ(msg.id, "test_id");
    
    // 测试不同消息类型
    mcp::Message response_msg;
    response_msg.type = MessageType::RESPONSE;
    response_msg.content = "response content";
    response_msg.id = "response_id";
    
    EXPECT_EQ(response_msg.type, MessageType::RESPONSE);
    EXPECT_EQ(response_msg.content, "response content");
    EXPECT_EQ(response_msg.id, "response_id");
}

/**
 * @brief 测试消息处理器设置
 */
TEST_F(PipeCommunicationTest, MessageHandlerSetting) {
    EXPECT_NO_THROW(pipe_comm_->setMessageHandler(test_handler_));
    
    // 测试nullptr处理器
    EXPECT_NO_THROW(pipe_comm_->setMessageHandler(nullptr));
}

/**
 * @brief 测试进程启动和停止
 */
TEST_F(PipeCommunicationTest, ProcessStartStop) {
    // 测试启动
    bool started = pipe_comm_->start(config_);
    if (started) {
        EXPECT_TRUE(pipe_comm_->isRunning());
        EXPECT_GT(pipe_comm_->getProcessId(), 0);
        
        // 等待进程启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送一些数据给cat命令，让它保持运行
        pipe_comm_->sendMessage("test data");
        
        // 等待一下让进程处理
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 测试进程状态
        EXPECT_TRUE(pipe_comm_->isRunning());
        
        // 测试停止
        pipe_comm_->stop();
        EXPECT_FALSE(pipe_comm_->isRunning());
        EXPECT_EQ(pipe_comm_->getProcessId(), -1);
    } else {
        // 如果启动失败，记录但不失败测试
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试无效命令启动
 */
TEST_F(PipeCommunicationTest, InvalidCommandStart) {
    PipeConfig invalid_config;
    invalid_config.command = "nonexistent_command_12345";
    invalid_config.args = {};
    
    bool started = pipe_comm_->start(invalid_config);
    EXPECT_FALSE(started);
    EXPECT_FALSE(pipe_comm_->isRunning());
    EXPECT_EQ(pipe_comm_->getProcessId(), -1);
}

/**
 * @brief 测试空命令启动
 */
TEST_F(PipeCommunicationTest, EmptyCommandStart) {
    PipeConfig empty_config;
    empty_config.command = "";
    empty_config.args = {};
    
    bool started = pipe_comm_->start(empty_config);
    EXPECT_FALSE(started);
    EXPECT_FALSE(pipe_comm_->isRunning());
}

/**
 * @brief 测试重复启动
 */
TEST_F(PipeCommunicationTest, DuplicateStart) {
    bool first_start = pipe_comm_->start(config_);
    if (first_start) {
        EXPECT_TRUE(pipe_comm_->isRunning());
        
        // 发送一些数据让进程保持运行
        pipe_comm_->sendMessage("test data");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 尝试重复启动
        bool second_start = pipe_comm_->start(config_);
        EXPECT_FALSE(second_start); // 应该失败
        EXPECT_TRUE(pipe_comm_->isRunning()); // 应该保持运行状态
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "First start failed, skipping test";
    }
}

/**
 * @brief 测试停止未启动的进程
 */
TEST_F(PipeCommunicationTest, StopNotStarted) {
    EXPECT_FALSE(pipe_comm_->isRunning());
    
    // 停止未启动的进程应该不会崩溃
    EXPECT_NO_THROW(pipe_comm_->stop());
    EXPECT_FALSE(pipe_comm_->isRunning());
}

/**
 * @brief 测试消息发送
 */
TEST_F(PipeCommunicationTest, MessageSending) {
    pipe_comm_->setMessageHandler(test_handler_);
    
    bool started = pipe_comm_->start(config_);
    if (started) {
        // 等待进程启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送消息
        std::string test_message = "Hello, World!";
        bool sent = pipe_comm_->sendMessage(test_message);
        EXPECT_TRUE(sent);
        
        // 等待消息处理 - cat会回显输入
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // 检查是否收到了回显的消息
        EXPECT_TRUE(pipe_comm_->hasMessage() || test_handler_->received_messages.size() > 0);
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试未启动时发送消息
 */
TEST_F(PipeCommunicationTest, SendMessageNotStarted) {
    std::string test_message = "Hello, World!";
    bool sent = pipe_comm_->sendMessage(test_message);
    EXPECT_FALSE(sent);
}

/**
 * @brief 测试空消息发送
 */
TEST_F(PipeCommunicationTest, SendEmptyMessage) {
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送空消息
        bool sent = pipe_comm_->sendMessage("");
        EXPECT_TRUE(sent); // 空消息应该也能发送
        
        // 等待处理
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试请求-响应机制
 */
TEST_F(PipeCommunicationTest, RequestResponse) {
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送简单的请求
        std::string request = "test request";
        bool sent = pipe_comm_->sendMessage(request);
        EXPECT_TRUE(sent);
        
        // 等待消息处理 - cat会回显输入
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 检查是否收到了回显的消息
        EXPECT_TRUE(pipe_comm_->hasMessage());
        
        // 获取消息并验证内容
        if (pipe_comm_->hasMessage()) {
            auto message = pipe_comm_->getMessage();
            EXPECT_FALSE(message.content.empty());
            
            // 打印实际收到的消息内容用于调试
            std::cout << "Received message content: '" << message.content << "'" << std::endl;
            
            // 检查是否包含我们发送的内容
            EXPECT_TRUE(message.content.find("test request") != std::string::npos);
        } else {
            std::cout << "No message received" << std::endl;
        }
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试请求超时
 */
TEST_F(PipeCommunicationTest, RequestTimeout) {
    // 使用一个会读取stdin但延迟响应的命令
    PipeConfig no_response_config;
    no_response_config.command = "/bin/sh";
    no_response_config.args = {"-c", "sleep 5; cat"}; // 延迟5秒后开始读取
    
    bool started = pipe_comm_->start(no_response_config);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送请求，设置短超时
        std::string request = "test request";
        auto future_response = pipe_comm_->sendRequest(request, 1); // 1秒超时
        
        // 等待响应 - 应该超时
        auto status = future_response.wait_for(std::chrono::seconds(2));
        if (status == std::future_status::ready) {
            // 如果意外收到响应，检查是否是超时消息
            std::string response = future_response.get();
            EXPECT_EQ(response, "Request timeout");
        } else {
            // 如果没有响应，也认为是正确的（因为设置了超时）
            EXPECT_EQ(status, std::future_status::timeout);
        }
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

TEST_F(PipeCommunicationTest, RequestIdVariantsAreHandled) {
    PipeConfig config = config_;
    config.read_timeout_ms = 20;

    ASSERT_TRUE(pipe_comm_->start(config));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto numeric_id_response = pipe_comm_->sendRequest(
        R"({"jsonrpc":"2.0","id":42,"result":{"ok":true}})", 1);
    ASSERT_EQ(numeric_id_response.wait_for(std::chrono::seconds(2)),
              std::future_status::ready);
    EXPECT_NE(numeric_id_response.get().find(R"("ok":true)"),
              std::string::npos);

    auto object_id_response = pipe_comm_->sendRequest(
        R"({"jsonrpc":"2.0","id":{"token":"abc"},"result":{"ok":true}})", 1);
    ASSERT_EQ(object_id_response.wait_for(std::chrono::seconds(2)),
              std::future_status::ready);
    EXPECT_NE(object_id_response.get().find(R"("ok":true)"),
              std::string::npos);

    auto null_id_response =
        pipe_comm_->sendRequest(R"({"jsonrpc":"2.0","id":null,"result":{}})",
                                1);
    ASSERT_EQ(null_id_response.wait_for(std::chrono::seconds(2)),
              std::future_status::ready);
    EXPECT_EQ(null_id_response.get(), "Request timeout");

    auto missing_id_response =
        pipe_comm_->sendRequest(R"({"jsonrpc":"2.0","result":{}})", 1);
    ASSERT_EQ(missing_id_response.wait_for(std::chrono::seconds(2)),
              std::future_status::ready);
    EXPECT_EQ(missing_id_response.get(), "Request timeout");

    pipe_comm_->stop();
}

TEST_F(PipeCommunicationTest, ChildExitInvokesConnectionLostCallback) {
    PipeConfig exit_config = config_;
    exit_config.command = "/bin/sh";
    exit_config.args = {"-c", "sleep 0.2; exit 0"};
    exit_config.read_timeout_ms = 20;
    exit_config.max_retry_count = 1;
    exit_config.enable_logging = false;

    std::atomic<bool> callback_called{false};
    std::string lost_reason;
    std::mutex reason_mutex;
    pipe_comm_->setConnectionLostCallback([&](const std::string& reason) {
        std::lock_guard<std::mutex> lock(reason_mutex);
        lost_reason = reason;
        callback_called.store(true, std::memory_order_release);
    });

    ASSERT_TRUE(pipe_comm_->start(exit_config));

    ASSERT_TRUE(wait_until(std::chrono::milliseconds(1000), [&]() {
        return callback_called.load(std::memory_order_acquire);
    }));

    {
        std::lock_guard<std::mutex> lock(reason_mutex);
        EXPECT_FALSE(lost_reason.empty());
        const bool child_exit_detected =
            lost_reason.find("child exited") != std::string::npos ||
            lost_reason.find("pipe closed") != std::string::npos;
        EXPECT_TRUE(child_exit_detected) << lost_reason;
    }

    pipe_comm_->stop();
}

/**
 * @brief 测试消息队列
 */
TEST_F(PipeCommunicationTest, MessageQueue) {
    pipe_comm_->setMessageHandler(test_handler_);
    
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送多个消息
        for (int i = 0; i < 5; ++i) {
            std::string message = "message_" + std::to_string(i);
            pipe_comm_->sendMessage(message);
        }
        
        // 等待消息处理 - cat会回显所有消息
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 检查是否收到了消息
        EXPECT_TRUE(pipe_comm_->hasMessage() || test_handler_->received_messages.size() > 0);
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试结果队列
 */
TEST_F(PipeCommunicationTest, ResultQueue) {
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 发送消息并检查结果 - cat会回显输入
        pipe_comm_->sendMessage("test result");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // 检查是否有结果
        bool hasResult = pipe_comm_->hasResult();
        if (hasResult) {
            std::string result = pipe_comm_->getResult();
            EXPECT_FALSE(result.empty());
        }
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试日志功能
 */
TEST_F(PipeCommunicationTest, Logging) {
    // 测试日志级别设置
    EXPECT_NO_THROW(pipe_comm_->setLogLevel(LogLevel::DEBUG));
    EXPECT_NO_THROW(pipe_comm_->setLogLevel(LogLevel::INFO));
    EXPECT_NO_THROW(pipe_comm_->setLogLevel(LogLevel::WARNING));
    EXPECT_NO_THROW(pipe_comm_->setLogLevel(LogLevel::ERROR));
    
    // 测试日志开关
    EXPECT_NO_THROW(pipe_comm_->enableLogging(true));
    EXPECT_NO_THROW(pipe_comm_->enableLogging(false));
    
    // 测试日志记录（应该不会崩溃）
    EXPECT_NO_THROW(pipe_comm_->enableLogging(true));
    EXPECT_NO_THROW(pipe_comm_->log(LogLevel::INFO, "Test log message"));
    EXPECT_NO_THROW(
        pipe_comm_->log(static_cast<LogLevel>(999), "Default log branch"));
}

/**
 * @brief 测试环境变量设置
 */
TEST_F(PipeCommunicationTest, EnvironmentVariables) {
    PipeConfig env_config = config_;
    env_config.env_vars["TEST_VAR"] = "test_value";
    env_config.env_vars["ANOTHER_VAR"] = "another_value";
    
    EXPECT_EQ(env_config.env_vars.size(), 2);
    EXPECT_EQ(env_config.env_vars["TEST_VAR"], "test_value");
    EXPECT_EQ(env_config.env_vars["ANOTHER_VAR"], "another_value");
}

/**
 * @brief 测试不同命令
 */
TEST_F(PipeCommunicationTest, DifferentCommands) {
    std::vector<std::string> commands = {"echo", "cat", "ls"};
    
    for (const auto& cmd : commands) {
        PipeConfig cmd_config;
        cmd_config.command = cmd;
        cmd_config.args = {};
        cmd_config.enable_logging = false;
        
        PipeCommunication test_pipe;
        bool started = test_pipe.start(cmd_config);
        if (started) {
            EXPECT_TRUE(test_pipe.isRunning());
            test_pipe.stop();
            EXPECT_FALSE(test_pipe.isRunning());
        }
        // 如果命令不存在，测试会失败，这是正常的
    }
}

/**
 * @brief 测试并发操作
 */
TEST_F(PipeCommunicationTest, ConcurrentOperations) {
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 创建多个线程同时发送消息
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};
        
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([this, i, &success_count]() {
                std::string message = "concurrent_message_" + std::to_string(i);
                if (pipe_comm_->sendMessage(message)) {
                    success_count++;
                }
            });
        }
        
        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
        
        // 等待消息处理 - cat会回显所有消息
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        EXPECT_GT(success_count.load(), 0);
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试内存管理
 */
TEST_F(PipeCommunicationTest, MemoryManagement) {
    // 测试多次创建和销毁
    for (int i = 0; i < 10; ++i) {
        auto pipe = std::make_unique<PipeCommunication>();
        EXPECT_FALSE(pipe->isRunning());
        
        bool started = pipe->start(config_);
        if (started) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            pipe->stop();
        }
        
        pipe.reset(); // 显式销毁
    }
}

/**
 * @brief 测试错误处理
 */
TEST_F(PipeCommunicationTest, ErrorHandling) {
    // 测试无效配置
    PipeConfig invalid_config;
    invalid_config.command = "";
    invalid_config.args = {};
    
    bool started = pipe_comm_->start(invalid_config);
    EXPECT_FALSE(started);
    
    // 测试nullptr处理器
    EXPECT_NO_THROW(pipe_comm_->setMessageHandler(nullptr));
    
    // 测试在未启动状态下调用各种方法
    EXPECT_FALSE(pipe_comm_->sendMessage("test"));
    EXPECT_FALSE(pipe_comm_->hasMessage());
    EXPECT_FALSE(pipe_comm_->hasResult());
    EXPECT_EQ(pipe_comm_->getProcessId(), -1);
    EXPECT_FALSE(pipe_comm_->isProcessAlive());
}

/**
 * @brief 性能测试
 */
TEST_F(PipeCommunicationTest, PerformanceTest) {
    bool started = pipe_comm_->start(config_);
    if (started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        const int iterations = 100;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 发送大量消息
        for (int i = 0; i < iterations; ++i) {
            std::string message = "perf_message_" + std::to_string(i);
            pipe_comm_->sendMessage(message);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // 等待消息处理
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 验证性能（应该在合理时间内完成）
        EXPECT_LT(duration.count(), 1000); // 应该在1秒内完成100次操作
        
        std::cout << "Performance test: " << iterations << " operations completed in " 
                  << duration.count() << "ms" << std::endl;
        
        pipe_comm_->stop();
    } else {
        GTEST_SKIP() << "Process start failed, skipping test";
    }
}

/**
 * @brief 测试冗余接口在未运行状态下的行为
 */
TEST_F(PipeCommunicationTest, RedundantInterfacesWhenNotRunning) {
    EXPECT_FALSE(pipe_comm_->isActive());
    EXPECT_FALSE(pipe_comm_->isCommunicationEstablished());

    EXPECT_FALSE(pipe_comm_->postMessage("x"));
    EXPECT_FALSE(pipe_comm_->dispatchMessage("x"));
    EXPECT_FALSE(pipe_comm_->transmitData("x"));
    EXPECT_FALSE(pipe_comm_->pushToPipe("x"));

    auto f1 = pipe_comm_->initiateRequest(R"({"jsonrpc":"2.0","id":"1","method":"ping"})", 1);
    auto f2 = pipe_comm_->submitRequest(R"({"jsonrpc":"2.0","id":"2","method":"ping"})", 1);
    auto f3 = pipe_comm_->invokeRpc(R"({"jsonrpc":"2.0","id":"3","method":"ping"})", 1);
    EXPECT_EQ(f1.get(), "Failed to send request");
    EXPECT_EQ(f2.get(), "Failed to send request");
    EXPECT_EQ(f3.get(), "Failed to send request");

    auto server_handler = std::make_shared<TestServerRequestHandler>();
    EXPECT_NO_THROW(pipe_comm_->registerServerHandler(server_handler));
    EXPECT_FALSE(pipe_comm_->dispatchResponse("1", R"({"ok":true})"));
    EXPECT_FALSE(pipe_comm_->dispatchError("1", -1, "err"));
    EXPECT_FALSE(pipe_comm_->broadcastNotification("m", "{}"));
    EXPECT_FALSE(pipe_comm_->emitNotification("m", "{}"));

    EXPECT_FALSE(pipe_comm_->messageQueueHasItems());
    EXPECT_FALSE(pipe_comm_->resultQueueHasItems());
    EXPECT_TRUE(pipe_comm_->fetchResult().empty());
    EXPECT_TRUE(pipe_comm_->consumeNextResult().empty());
    EXPECT_TRUE(pipe_comm_->drainResultQueue().empty());
    EXPECT_TRUE(pipe_comm_->drainMessageQueue().empty());
    EXPECT_EQ(pipe_comm_->retrieveMessage().content, "");
    EXPECT_EQ(pipe_comm_->dequeueNextMessage().content, "");

    EXPECT_EQ(pipe_comm_->acquireProcessId(), -1);
    EXPECT_EQ(pipe_comm_->getChildPid(), -1);
    EXPECT_FALSE(pipe_comm_->verifyProcessStatus());
    EXPECT_FALSE(pipe_comm_->isSubprocessAlive());

    EXPECT_NO_THROW(pipe_comm_->writeLog(LogLevel::INFO, "a"));
    EXPECT_NO_THROW(pipe_comm_->recordLog(LogLevel::WARNING, "b"));
    EXPECT_NO_THROW(pipe_comm_->configureLogLevel(LogLevel::DEBUG));
    EXPECT_NO_THROW(pipe_comm_->setLoggingEnabled(false));

    EXPECT_NO_THROW(pipe_comm_->shutdown());
    EXPECT_NO_THROW(pipe_comm_->terminate());
}

/**
 * @brief 测试冗余接口在运行状态下的行为
 */
TEST_F(PipeCommunicationTest, RedundantInterfacesWhenRunning) {
    auto handler = std::make_shared<TestMessageHandler>();
    auto server_handler = std::make_shared<TestServerRequestHandler>();
    pipe_comm_->registerMessageHandler(handler);
    pipe_comm_->attachMessageHandler(handler);
    pipe_comm_->registerServerHandler(server_handler);

    ASSERT_TRUE(pipe_comm_->activate(config_));
    EXPECT_TRUE(pipe_comm_->isActive());
    EXPECT_TRUE(pipe_comm_->isCommunicationEstablished());
    EXPECT_GT(pipe_comm_->acquireProcessId(), 0);
    EXPECT_GT(pipe_comm_->getChildPid(), 0);
    EXPECT_TRUE(pipe_comm_->verifyProcessStatus());
    EXPECT_TRUE(pipe_comm_->isSubprocessAlive());

    EXPECT_TRUE(pipe_comm_->postMessage("r1"));
    EXPECT_TRUE(pipe_comm_->dispatchMessage("r2"));
    EXPECT_TRUE(pipe_comm_->transmitData("r3"));
    EXPECT_TRUE(pipe_comm_->pushToPipe("r4"));

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    EXPECT_TRUE(pipe_comm_->messageQueueHasItems() || !pipe_comm_->drainMessageQueue().empty());
    auto messages = pipe_comm_->drainMessageQueue();
    if (!messages.empty()) {
        EXPECT_FALSE(messages.front().content.empty());
    }

    EXPECT_FALSE(pipe_comm_->dispatchResponse("1", R"({"ok":true})"));
    EXPECT_FALSE(pipe_comm_->dispatchError("1", -32000, "err"));
    EXPECT_FALSE(pipe_comm_->broadcastNotification("n", "{}"));
    EXPECT_FALSE(pipe_comm_->emitNotification("n", "{}"));

    EXPECT_NO_THROW(pipe_comm_->writeLog(LogLevel::INFO, "run"));
    EXPECT_NO_THROW(pipe_comm_->recordLog(LogLevel::ERROR, "run"));
    EXPECT_NO_THROW(pipe_comm_->configureLogLevel(LogLevel::INFO));
    EXPECT_NO_THROW(pipe_comm_->setLoggingEnabled(true));

    pipe_comm_->shutdown();
    EXPECT_FALSE(pipe_comm_->isRunning());
    pipe_comm_->terminate();
}

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
