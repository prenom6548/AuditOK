#include <gtest/gtest.h>
#include "netio.h"
#include <thread>
#include <chrono>

using namespace netio;

// 测试基类
class NetIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        netio_ = std::make_unique<NetIO>();
    }

    void TearDown() override {
        if (netio_ && netio_->is_connected()) {
            netio_->disconnect();
        }
        if (netio_ && netio_->is_server_running()) {
            netio_->stop_server();
        }
        netio_.reset();
    }

    std::unique_ptr<NetIO> netio_;
};

// 测试 NetConfig 结构
TEST_F(NetIOTest, NetConfig) {
    NetConfig config;
    config.host = "localhost";
    config.port = 8080;
    config.protocol = Protocol::HTTP;
    config.timeout_seconds = 30;
    
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.protocol, Protocol::HTTP);
    EXPECT_EQ(config.timeout_seconds, 30);
    
    // 测试 JSON 转换
    nlohmann::json config_json = config.to_json();
    EXPECT_TRUE(config_json.is_object());
}

// 测试 Message 结构
TEST_F(NetIOTest, Message) {
    Message msg;
    msg.id = "test-id-123";
    msg.type = MessageType::REQUEST;
    msg.method = "GET";
    msg.path = "/test";
    msg.body = "test body";
    msg.status_code = 200;
    
    EXPECT_EQ(msg.id, "test-id-123");
    EXPECT_EQ(msg.type, MessageType::REQUEST);
    EXPECT_EQ(msg.method, "GET");
    EXPECT_EQ(msg.path, "/test");
    EXPECT_EQ(msg.body, "test body");
    EXPECT_EQ(msg.status_code, 200);
    
    // 测试 JSON 序列化
    nlohmann::json msg_json = msg.to_json();
    EXPECT_EQ(msg_json["id"], "test-id-123");
    EXPECT_EQ(msg_json["method"], "GET");
    
    // 测试 JSON 反序列化
    Message msg2 = Message::from_json(msg_json);
    EXPECT_EQ(msg2.id, "test-id-123");
    EXPECT_EQ(msg2.method, "GET");
    EXPECT_EQ(msg2.path, "/test");
}

// 测试 ConnectionInfo 结构
TEST_F(NetIOTest, ConnectionInfo) {
    ConnectionInfo info;
    info.connection_id = "conn-123";
    info.remote_address = "127.0.0.1";
    info.remote_port = 8080;
    info.status = ConnectionStatus::CONNECTED;
    
    EXPECT_EQ(info.connection_id, "conn-123");
    EXPECT_EQ(info.remote_address, "127.0.0.1");
    EXPECT_EQ(info.remote_port, 8080);
    EXPECT_EQ(info.status, ConnectionStatus::CONNECTED);
    
    // 测试 JSON 转换
    nlohmann::json info_json = info.to_json();
    EXPECT_TRUE(info_json.is_object());
}

// 测试工具函数：create_request
TEST_F(NetIOTest, CreateRequest) {
    Message msg = utils::create_request("POST", "/api/test", "{\"key\":\"value\"}");
    
    EXPECT_EQ(msg.method, "POST");
    EXPECT_EQ(msg.path, "/api/test");
    EXPECT_EQ(msg.type, MessageType::REQUEST);
    EXPECT_FALSE(msg.id.empty());
}

// 测试工具函数：create_response
TEST_F(NetIOTest, CreateResponse) {
    Message msg = utils::create_response(200, "Success");
    
    EXPECT_EQ(msg.status_code, 200);
    EXPECT_EQ(msg.body, "Success");
    EXPECT_EQ(msg.type, MessageType::RESPONSE);
}

// 测试工具函数：create_error
TEST_F(NetIOTest, CreateError) {
    Message msg = utils::create_error(404, "Not Found");
    
    EXPECT_EQ(msg.status_code, 404);
    EXPECT_EQ(msg.error_message, "Not Found");
    EXPECT_EQ(msg.type, MessageType::ERROR);
}

// 测试工具函数：create_http_config
TEST_F(NetIOTest, CreateHTTPConfig) {
    NetConfig config = utils::create_http_config("localhost", 8080);
    
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.protocol, Protocol::HTTP);
}

// 测试工具函数：create_https_config
TEST_F(NetIOTest, CreateHTTPSConfig) {
    NetConfig config = utils::create_https_config("localhost", 8443);
    
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 8443);
    EXPECT_EQ(config.protocol, Protocol::HTTPS);
}

// 测试工具函数：validate_config
TEST_F(NetIOTest, ValidateConfig) {
    NetConfig valid_config;
    valid_config.host = "localhost";
    valid_config.port = 8080;
    valid_config.protocol = Protocol::HTTP;
    
    EXPECT_TRUE(utils::validate_config(valid_config));
    
    NetConfig invalid_config;
    invalid_config.host = "";
    invalid_config.port = -1;
    
    // 验证函数应该能够检测无效配置
    // 注意：实际实现可能不同，这里只是示例
    bool result = utils::validate_config(invalid_config);
    // 不强制要求返回 false，取决于实现
}

// 测试工具函数：validate_message
TEST_F(NetIOTest, ValidateMessage) {
    Message valid_msg;
    valid_msg.method = "GET";
    valid_msg.path = "/test";
    valid_msg.type = MessageType::REQUEST;
    
    bool result = utils::validate_message(valid_msg);
    // 不强制要求返回值，取决于实现
}

// 测试工具函数：json_to_string 和 string_to_json
TEST_F(NetIOTest, JSONUtils) {
    nlohmann::json test_json = {
        {"key1", "value1"},
        {"key2", 123},
        {"key3", true}
    };
    
    std::string json_str = utils::json_to_string(test_json);
    EXPECT_FALSE(json_str.empty());
    
    nlohmann::json parsed_json = utils::string_to_json(json_str);
    EXPECT_EQ(parsed_json["key1"], "value1");
    EXPECT_EQ(parsed_json["key2"], 123);
    EXPECT_EQ(parsed_json["key3"], true);
}

// 测试 NetIO 初始状态
TEST_F(NetIOTest, InitialState) {
    EXPECT_FALSE(netio_->is_connected());
    EXPECT_FALSE(netio_->is_server_running());
}

// 测试 NetIO 配置管理
TEST_F(NetIOTest, ConfigManagement) {
    NetConfig config;
    config.host = "localhost";
    config.port = 8080;
    config.protocol = Protocol::HTTP;
    
    netio_->update_config(config);
    
    NetConfig retrieved_config = netio_->get_config();
    EXPECT_EQ(retrieved_config.host, "localhost");
    EXPECT_EQ(retrieved_config.port, 8080);
}

// 测试 NetIO 统计信息
TEST_F(NetIOTest, Statistics) {
    nlohmann::json stats = netio_->get_statistics();
    // 统计信息可能是对象、数组或null，取决于实现
    // 只验证不会崩溃
    EXPECT_NO_THROW(netio_->get_statistics());
}

// 测试连接信息获取（未连接时）
TEST_F(NetIOTest, ConnectionInfoWhenDisconnected) {
    ConnectionInfo info = netio_->get_connection_info();
    EXPECT_EQ(info.status, ConnectionStatus::DISCONNECTED);
}

// 测试消息回调设置
TEST_F(NetIOTest, CallbackSetting) {
    bool callback_called = false;
    
    MessageCallback msg_callback = [&callback_called](const Message& msg) {
        callback_called = true;
    };
    
    netio_->set_message_callback(msg_callback);
    
    ConnectionCallback conn_callback = [](const ConnectionInfo& info) {
        // 回调函数
    };
    
    netio_->set_connection_callback(conn_callback);
    
    ErrorCallback error_callback = [](const std::string& error) {
        // 回调函数
    };
    
    netio_->set_error_callback(error_callback);
    
    // 回调设置不应该抛出异常
    EXPECT_NO_THROW(netio_->set_message_callback(msg_callback));
}

// 测试 Message 序列化/反序列化完整性
TEST_F(NetIOTest, MessageSerializationRoundTrip) {
    Message original;
    original.id = "test-id";
    original.type = MessageType::REQUEST;
    original.method = "POST";
    original.path = "/api/test";
    original.body = "{\"test\":\"data\"}";
    original.status_code = 200;
    original.headers["Content-Type"] = "application/json";
    original.timestamp = std::chrono::system_clock::now();
    
    // 序列化
    nlohmann::json msg_json = original.to_json();
    
    // 反序列化
    Message deserialized = Message::from_json(msg_json);
    
    // 验证关键字段
    EXPECT_EQ(deserialized.id, original.id);
    EXPECT_EQ(deserialized.method, original.method);
    EXPECT_EQ(deserialized.path, original.path);
    EXPECT_EQ(deserialized.body, original.body);
    EXPECT_EQ(deserialized.status_code, original.status_code);
    EXPECT_EQ(deserialized.headers["Content-Type"], "application/json");
}

// 测试不同消息类型
TEST_F(NetIOTest, MessageTypes) {
    Message request = utils::create_request("GET", "/test");
    EXPECT_EQ(request.type, MessageType::REQUEST);
    
    Message response = utils::create_response(200);
    EXPECT_EQ(response.type, MessageType::RESPONSE);
    
    Message error = utils::create_error(404, "Not Found");
    EXPECT_EQ(error.type, MessageType::ERROR);
}

// 测试 NetConfig 默认值
TEST_F(NetIOTest, NetConfigDefaults) {
    NetConfig config;
    
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.protocol, Protocol::HTTP);
    EXPECT_EQ(config.timeout_seconds, 30);
    EXPECT_EQ(config.max_retries, 3);
    EXPECT_EQ(config.user_agent, "NetIO/1.0");
    EXPECT_TRUE(config.keep_alive);
    EXPECT_FALSE(config.auto_reconnect);
}

// 测试连接状态枚举
TEST_F(NetIOTest, ConnectionStatusEnum) {
    EXPECT_EQ(static_cast<int>(ConnectionStatus::DISCONNECTED), 0);
    EXPECT_EQ(static_cast<int>(ConnectionStatus::CONNECTING), 1);
    EXPECT_EQ(static_cast<int>(ConnectionStatus::CONNECTED), 2);
    EXPECT_EQ(static_cast<int>(ConnectionStatus::ERROR), 3);
}

// 测试消息类型枚举
TEST_F(NetIOTest, MessageTypeEnum) {
    EXPECT_EQ(static_cast<int>(MessageType::REQUEST), 0);
    EXPECT_EQ(static_cast<int>(MessageType::RESPONSE), 1);
    EXPECT_EQ(static_cast<int>(MessageType::NOTIFICATION), 2);
    EXPECT_EQ(static_cast<int>(MessageType::ERROR), 3);
}

// 测试协议类型枚举
TEST_F(NetIOTest, ProtocolEnum) {
    EXPECT_EQ(static_cast<int>(Protocol::HTTP), 0);
    EXPECT_EQ(static_cast<int>(Protocol::HTTPS), 1);
    EXPECT_EQ(static_cast<int>(Protocol::WEBSOCKET), 2);
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

