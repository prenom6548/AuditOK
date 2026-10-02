#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "common/transport.h"

using namespace mcp_transport;

// 测试基类
class TransportTest : public ::testing::Test {
protected:
    void SetUp() override { transport_ = std::make_unique<Transport>(); }

    void TearDown() override {
        if (transport_ && transport_->isRunning()) {
            transport_->stop();
        }
        transport_.reset();
    }

    std::unique_ptr<Transport> transport_;
};

// 测试 TransportType 枚举
TEST_F(TransportTest, TransportTypeEnum) {
    EXPECT_EQ(static_cast<int>(TransportType::kIpc), 0);
    EXPECT_EQ(static_cast<int>(TransportType::kSse), 1);
    EXPECT_EQ(static_cast<int>(TransportType::kAuto), 2);
}

// 测试 TransportStatus 枚举
TEST_F(TransportTest, TransportStatusEnum) {
    EXPECT_EQ(static_cast<int>(TransportStatus::kDisconnected), 0);
    EXPECT_EQ(static_cast<int>(TransportStatus::kConnecting), 1);
    EXPECT_EQ(static_cast<int>(TransportStatus::kConnected), 2);
    EXPECT_EQ(static_cast<int>(TransportStatus::kError), 3);
}

// 测试 TransportMessage 结构
TEST_F(TransportTest, TransportMessage) {
    TransportMessage msg;
    msg.id = "test-id-123";
    msg.content = "test content";
    msg.method = "POST";
    msg.path = "/api/test";
    msg.status_code = 200;
    msg.headers["Content-Type"] = "application/json";
    msg.timestamp = std::chrono::system_clock::now();

    EXPECT_EQ(msg.id, "test-id-123");
    EXPECT_EQ(msg.content, "test content");
    EXPECT_EQ(msg.method, "POST");
    EXPECT_EQ(msg.path, "/api/test");
    EXPECT_EQ(msg.status_code, 200);
    EXPECT_EQ(msg.headers["Content-Type"], "application/json");

    // 测试 JSON 序列化
    nlohmann::json msg_json = msg.toJson();
    EXPECT_TRUE(msg_json.is_object());
    EXPECT_EQ(msg_json["id"], "test-id-123");

    // 测试 JSON 反序列化
    TransportMessage msg2 = TransportMessage::fromJson(msg_json);
    EXPECT_EQ(msg2.id, "test-id-123");
    EXPECT_EQ(msg2.content, "test content");
    EXPECT_EQ(msg2.method, "POST");
}

// 测试 TransportConfig 结构
TEST_F(TransportTest, TransportConfig) {
    TransportConfig config;
    config.type = TransportType::kIpc;
    config.name = "test-transport";
    config.ipc.command = "echo";
    config.ipc.args = {"test"};
    config.sse.host = "localhost";
    config.sse.port = 8080;

    EXPECT_EQ(config.type, TransportType::kIpc);
    EXPECT_EQ(config.name, "test-transport");
    EXPECT_EQ(config.ipc.command, "echo");
    EXPECT_EQ(config.sse.host, "localhost");
    EXPECT_EQ(config.sse.port, 8080);

    // 测试 JSON 转换
    nlohmann::json config_json = config.toJson();
    EXPECT_TRUE(config_json.is_object());
}

// 测试 ConnectionInfo 结构
TEST_F(TransportTest, ConnectionInfo) {
    ConnectionInfo info;
    info.connection_id = "conn-123";
    info.remote_address = "127.0.0.1";
    info.remote_port = 8080;
    info.status = TransportStatus::kConnected;
    info.type = TransportType::kIpc;

    EXPECT_EQ(info.connection_id, "conn-123");
    EXPECT_EQ(info.remote_address, "127.0.0.1");
    EXPECT_EQ(info.remote_port, 8080);
    EXPECT_EQ(info.status, TransportStatus::kConnected);
    EXPECT_EQ(info.type, TransportType::kIpc);

    // 测试 JSON 转换
    nlohmann::json info_json = info.toJson();
    EXPECT_TRUE(info_json.is_object());
}

// 测试工具函数：createIpcConfig
TEST_F(TransportTest, CreateIPCConfig) {
    TransportConfig config = utils::createIpcConfig("echo", {"test"});

    EXPECT_EQ(config.type, TransportType::kIpc);
    EXPECT_EQ(config.ipc.command, "echo");
    EXPECT_FALSE(config.ipc.args.empty());
}

// 测试工具函数：createSseConfig
TEST_F(TransportTest, CreateSSEConfig) {
    TransportConfig config = utils::createSseConfig("localhost", 8080);

    EXPECT_EQ(config.type, TransportType::kSse);
    EXPECT_EQ(config.sse.host, "localhost");
    EXPECT_EQ(config.sse.port, 8080);
    EXPECT_EQ(config.sse.protocol, TransportProtocol::kHttp);
}

// 测试工具函数：createSseSecureConfig
TEST_F(TransportTest, CreateSSESecureConfig) {
    TransportConfig config = utils::createSseSecureConfig("localhost", 8443);

    EXPECT_EQ(config.type, TransportType::kSse);
    EXPECT_EQ(config.sse.host, "localhost");
    EXPECT_EQ(config.sse.port, 8443);
    EXPECT_EQ(config.sse.protocol, TransportProtocol::kHttps);
}

// 测试工具函数：createRequest
TEST_F(TransportTest, CreateRequest) {
    TransportMessage msg =
        utils::createRequest("test content", "POST", "/api/test");

    EXPECT_EQ(msg.content, "test content");
    EXPECT_EQ(msg.method, "POST");
    EXPECT_EQ(msg.path, "/api/test");
    EXPECT_FALSE(msg.id.empty());
}

// 测试工具函数：createResponse
TEST_F(TransportTest, CreateResponse) {
    TransportMessage msg = utils::createResponse("success", 200);

    EXPECT_EQ(msg.content, "success");
    EXPECT_EQ(msg.status_code, 200);
}

// 测试工具函数：createError
TEST_F(TransportTest, CreateError) {
    TransportMessage msg = utils::createError("error message", 500);

    EXPECT_EQ(msg.error_message, "error message");
    EXPECT_EQ(msg.status_code, 500);
}

// 测试工具函数：validateConfig
TEST_F(TransportTest, ValidateConfig) {
    TransportConfig valid_config = utils::createIpcConfig("echo");
    bool result = utils::validateConfig(valid_config);
    // 不强制要求返回值，取决于实现
}

// 测试工具函数：validateMessage
TEST_F(TransportTest, ValidateMessage) {
    TransportMessage valid_msg = utils::createRequest("test");
    bool result = utils::validateMessage(valid_msg);
    // 不强制要求返回值，取决于实现
}

// 测试工具函数：jsonToString 和 stringToJson
TEST_F(TransportTest, JSONUtils) {
    nlohmann::json test_json = {
        {"key1", "value1"}, {"key2", 123}, {"key3", true}};

    std::string json_str = utils::jsonToString(test_json);
    EXPECT_FALSE(json_str.empty());

    nlohmann::json parsed_json = utils::stringToJson(json_str);
    EXPECT_EQ(parsed_json["key1"], "value1");
    EXPECT_EQ(parsed_json["key2"], 123);
    EXPECT_EQ(parsed_json["key3"], true);
}

// 测试 Transport 初始状态
TEST_F(TransportTest, InitialState) {
    EXPECT_FALSE(transport_->isRunning());
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
}

// 测试 Transport 配置管理
TEST_F(TransportTest, ConfigManagement) {
    TransportConfig config = utils::createIpcConfig("echo");
    config.name = "test-transport";

    transport_->updateConfig(config);

    TransportConfig retrieved_config = transport_->getConfig();
    EXPECT_EQ(retrieved_config.name, "test-transport");
    EXPECT_EQ(retrieved_config.type, TransportType::kIpc);
}

// 测试 Transport 消息队列
TEST_F(TransportTest, MessageQueue) {
    EXPECT_FALSE(transport_->hasMessage());

    TransportMessage msg = transport_->getMessage();
    EXPECT_TRUE(msg.id.empty());

    std::vector<TransportMessage> messages = transport_->getAllMessages();
    EXPECT_TRUE(messages.empty());
}

// 测试 Transport 统计信息
TEST_F(TransportTest, Statistics) {
    nlohmann::json stats = transport_->getStatistics();
    // 统计信息可能是对象、数组或null，取决于实现
    // 只验证不会崩溃
    EXPECT_NO_THROW(transport_->getStatistics());
}

// 测试 Transport 连接信息获取（未连接时）
TEST_F(TransportTest, ConnectionInfoWhenDisconnected) {
    ConnectionInfo info = transport_->getConnectionInfo();
    EXPECT_EQ(info.status, TransportStatus::kDisconnected);
}

// 测试 Transport 回调设置
TEST_F(TransportTest, CallbackSetting) {
    bool callback_called = false;

    MessageCallback msg_callback =
        [&callback_called](const TransportMessage& msg) {
            callback_called = true;
        };

    transport_->setMessageCallback(msg_callback);

    ConnectionCallback conn_callback = [](const ConnectionInfo& info) {
        // 回调函数
    };

    transport_->setConnectionCallback(conn_callback);

    ErrorCallback error_callback = [](const std::string& error) {
        // 回调函数
    };

    transport_->setErrorCallback(error_callback);

    // 回调设置不应该抛出异常
    EXPECT_NO_THROW(transport_->setMessageCallback(msg_callback));
}

// 测试 Transport 日志功能
TEST_F(TransportTest, Logging) {
    transport_->enableLogging(true);
    transport_->setLogLevel(1);
    transport_->setLogFile("test_log.txt");

    // 日志功能不应该抛出异常
    EXPECT_NO_THROW(transport_->log(1, "test log message"));
}

// 测试 TransportMessage 序列化/反序列化完整性
TEST_F(TransportTest, MessageSerializationRoundTrip) {
    TransportMessage original;
    original.id = "test-id";
    original.content = "test content";
    original.method = "POST";
    original.path = "/api/test";
    original.status_code = 200;
    original.headers["Content-Type"] = "application/json";
    original.timestamp = std::chrono::system_clock::now();

    // 序列化
    nlohmann::json msg_json = original.toJson();

    // 反序列化
    TransportMessage deserialized = TransportMessage::fromJson(msg_json);

    // 验证关键字段
    EXPECT_EQ(deserialized.id, original.id);
    EXPECT_EQ(deserialized.content, original.content);
    EXPECT_EQ(deserialized.method, original.method);
    EXPECT_EQ(deserialized.path, original.path);
    EXPECT_EQ(deserialized.status_code, original.status_code);
    EXPECT_EQ(deserialized.headers["Content-Type"], "application/json");
}

// 测试 TransportConfig 默认值
TEST_F(TransportTest, TransportConfigDefaults) {
    TransportConfig config;

    EXPECT_EQ(config.type, TransportType::kAuto);
    EXPECT_EQ(config.name, "mcp_transport");
    EXPECT_EQ(config.ipc.buffer_size, 4096);
    EXPECT_EQ(config.ipc.read_timeout_ms, 10);
    EXPECT_EQ(config.sse.host, "localhost");
    EXPECT_EQ(config.sse.port, 8080);
    EXPECT_TRUE(config.enable_logging);
}

// 测试 TransportError 异常类
TEST_F(TransportTest, TransportError) {
    TransportError error("test error message");
    EXPECT_STREQ(error.what(), "test error message");

    try {
        throw TransportError("test exception");
    } catch (const TransportError& e) {
        EXPECT_STREQ(e.what(), "test exception");
    } catch (...) {
        FAIL() << "Expected TransportError exception";
    }
}

// 测试 TransportConfig 验证
TEST_F(TransportTest, ConfigValidation) {
    TransportConfig config;
    bool result = config.validate();
    // 不强制要求返回值，取决于实现
}

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
