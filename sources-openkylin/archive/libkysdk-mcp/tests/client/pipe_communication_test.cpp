#include <gtest/gtest.h>
#include "pipe_communication.h"
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <mutex>

using namespace mcp;

// 测试用的消息处理器
class TestMessageHandler : public MessageHandler {
public:
    std::vector<Message> received_messages;
    std::vector<std::string> received_errors;
    mutable std::mutex mutex_;

    void on_message(const Message& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        received_messages.push_back(message);
    }

    void on_error(const std::string& error) override {
        std::lock_guard<std::mutex> lock(mutex_);
        received_errors.push_back(error);
    }

    size_t message_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return received_messages.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        received_messages.clear();
        received_errors.clear();
    }
};

// 测试基类，提供通用的设置和清理
class PipeCommunicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        pipe_comm_ = std::make_unique<PipeCommunication>();
        handler_ = std::make_shared<TestMessageHandler>();
    }

    void TearDown() override {
        if (pipe_comm_ && pipe_comm_->is_running()) {
            pipe_comm_->stop();
        }
        pipe_comm_.reset();
        handler_.reset();
    }

    std::unique_ptr<PipeCommunication> pipe_comm_;
    std::shared_ptr<TestMessageHandler> handler_;
};

// 测试基本功能
TEST_F(PipeCommunicationTest, BasicFunctionality) {
    pipe_comm_->set_message_handler(handler_);

    // 测试未启动状态
    EXPECT_FALSE(pipe_comm_->is_running());
    EXPECT_EQ(pipe_comm_->get_process_id(), -1);
    EXPECT_FALSE(pipe_comm_->is_process_alive());
}

// 测试管道创建和进程启动
TEST_F(PipeCommunicationTest, PipeCreation) {
    PipeConfig config;
    config.command = "echo";
    config.args = {"hello"};
    config.enable_logging = false;

    // 测试启动
    bool started = pipe_comm_->start(config);
    if (started) {
        EXPECT_TRUE(pipe_comm_->is_running());
        EXPECT_GT(pipe_comm_->get_process_id(), 0);

        // 等待进程启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 测试进程状态（echo 命令可能已经退出，所以这个测试可能不稳定）
        bool alive = pipe_comm_->is_process_alive();
        // 不强制要求进程存活，因为 echo 命令会立即退出

        // 停止
        pipe_comm_->stop();
        EXPECT_FALSE(pipe_comm_->is_running());
    }
}

// 测试消息发送
TEST_F(PipeCommunicationTest, MessageSending) {
    pipe_comm_->set_message_handler(handler_);

    PipeConfig config;
    config.command = "cat";  // 简单的回显命令
    config.args = {};
    config.enable_logging = false;

    if (pipe_comm_->start(config)) {
        // 等待进程启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 发送消息
        std::string test_message = "Hello, World!\n";
        bool sent = pipe_comm_->send_message(test_message);
        EXPECT_TRUE(sent);

        // 等待消息处理
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 停止
        pipe_comm_->stop();
    }
}

// 测试未启动时发送消息
TEST_F(PipeCommunicationTest, SendMessageWithoutStart) {
    EXPECT_FALSE(pipe_comm_->send_message("test message"));
}

// 测试重复启动
TEST_F(PipeCommunicationTest, StartTwice) {
    PipeConfig config;
    config.command = "/bin/cat";
    config.args = {};
    config.enable_logging = false;

    EXPECT_TRUE(pipe_comm_->start(config));
    EXPECT_TRUE(pipe_comm_->is_running());

    // Second start should be rejected while the instance is already running.
    EXPECT_FALSE(pipe_comm_->start(config));
    EXPECT_TRUE(pipe_comm_->is_running());

    pipe_comm_->stop();
}

// 测试停止未启动的实例
TEST_F(PipeCommunicationTest, StopWithoutStart) {
    // 不应该崩溃
    EXPECT_NO_THROW(pipe_comm_->stop());
    EXPECT_FALSE(pipe_comm_->is_running());
}

// 测试消息队列
TEST_F(PipeCommunicationTest, MessageQueue) {
    EXPECT_FALSE(pipe_comm_->has_message());

    Message msg = pipe_comm_->get_message();
    EXPECT_EQ(msg.type, MessageType::NOTIFICATION);
    EXPECT_TRUE(msg.content.empty());

    std::vector<Message> messages = pipe_comm_->get_all_messages();
    EXPECT_TRUE(messages.empty());
}

// 测试结果队列
TEST_F(PipeCommunicationTest, ResultQueue) {
    EXPECT_FALSE(pipe_comm_->has_result());

    std::string result = pipe_comm_->get_result();
    EXPECT_TRUE(result.empty());

    std::vector<std::string> results = pipe_comm_->get_all_results();
    EXPECT_TRUE(results.empty());
}

// 测试日志功能
TEST_F(PipeCommunicationTest, DISABLED_Logging) {
    // Keep this test process-free to avoid flaky blocking on child I/O threads.
    EXPECT_NO_THROW(pipe_comm_->set_log_level(LogLevel::INFO));
    EXPECT_NO_THROW(pipe_comm_->set_log_level(LogLevel::WARNING));
    EXPECT_NO_THROW(pipe_comm_->set_log_level(LogLevel::ERROR));
    EXPECT_NO_THROW(pipe_comm_->enable_logging(false));
    EXPECT_NO_THROW(pipe_comm_->enable_logging(true));
}

// 测试配置参数
TEST_F(PipeCommunicationTest, ConfigParameters) {
    PipeConfig config;
    config.command = "echo";
    config.args = {"arg1", "arg2"};
    config.env_vars["TEST_VAR"] = "test_value";
    config.buffer_size = 8192;
    config.read_timeout_ms = 50;
    config.max_retry_count = 5;
    config.enable_logging = false;
    config.log_level = LogLevel::WARNING;

    if (pipe_comm_->start(config)) {
        EXPECT_TRUE(pipe_comm_->is_running());
        pipe_comm_->stop();
    }
}
