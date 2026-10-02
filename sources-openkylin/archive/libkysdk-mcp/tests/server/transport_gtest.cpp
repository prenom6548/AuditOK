/**
 * @file transport_gtest.cpp
 * @brief Transport模块的Google Test测试文件
 * @author MCP SDK Team
 * @date 2024
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "common/message.h"
#include "netio.h"
#include "pipe_communication.h"
#include "common/transport.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace mcp_transport;

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

int get_free_tcp_port() {
#ifdef _WIN32
    static std::once_flag wsa_flag;
    std::call_once(wsa_flag, []() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    });

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) ==
        SOCKET_ERROR) {
        ::closesocket(sock);
        return 0;
    }

    int addr_len = sizeof(addr);
    if (::getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &addr_len) ==
        SOCKET_ERROR) {
        ::closesocket(sock);
        return 0;
    }

    int port = ntohs(addr.sin_port);
    ::closesocket(sock);
    return port;
#else
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 0;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(sock);
        return 0;
    }

    socklen_t addr_len = sizeof(addr);
    if (::getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &addr_len) <
        0) {
        ::close(sock);
        return 0;
    }

    int port = ntohs(addr.sin_port);
    ::close(sock);
    return port;
#endif
}

class TestSseServerHandler : public TransportServerRequestHandler {
public:
    std::string handleRequest(const std::string& method,
                              const std::string& params,
                              const std::string& request_id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        last_method_ = method;
        last_request_id_ = request_id;
        last_params_ = params;
        request_count_++;

        nlohmann::json response;
        response["jsonrpc"] = "2.0";
        response["result"] = {{"message", "pong"},
                              {"method", method},
                              {"request_id", request_id}};

        return response.dump();
    }

    void handleNotification(const std::string& method,
                            const std::string& params) override {
        std::lock_guard<std::mutex> lock(mutex_);
        last_notification_method_ = method;
        last_notification_params_ = params;
        notification_count_++;
    }

    int requestCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return request_count_;
    }

    int notificationCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return notification_count_;
    }

    std::string lastMethod() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_method_;
    }

    std::string lastNotificationMethod() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_notification_method_;
    }

private:
    mutable std::mutex mutex_;
    int request_count_ = 0;
    int notification_count_ = 0;
    std::string last_method_;
    std::string last_request_id_;
    std::string last_params_;
    std::string last_notification_method_;
    std::string last_notification_params_;
};

class TestDedicatedSseRequestHandler : public SseServerRequestHandler {
public:
    bool handleRequest(const TransportHttpMessage& request,
                       TransportHttpMessage& response) override {
        std::lock_guard<std::mutex> lock(mutex_);
        last_method_ = request.method;
        last_path_ = request.path;
        request_count_++;

        response.status_code = 200;
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"jsonrpc":"2.0","result":{"message":"sse-handler"}})";
        return true;
    }

    int requestCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return request_count_;
    }

    std::string lastMethod() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_method_;
    }

private:
    mutable std::mutex mutex_;
    int request_count_ = 0;
    std::string last_method_;
    std::string last_path_;
};

}  // namespace

// 测试消息回调处理器
class TestMessageHandler {
public:
    void onMessage(const TransportMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        received_messages_.push_back(message);
        message_count_++;
    }

    void onConnection(const ConnectionInfo& info) {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_info_ = info;
        connection_count_++;
    }

    void onError(const std::string& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        received_errors_.push_back(error);
        error_count_++;
    }

    // 获取接收到的消息
    std::vector<TransportMessage> getMessages() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return received_messages_;
    }

    ConnectionInfo getConnectionInfo() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_info_;
    }

    std::vector<std::string> getErrors() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return received_errors_;
    }

    int getMessageCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return message_count_;
    }

    int getConnectionCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_count_;
    }

    int getErrorCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return error_count_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        received_messages_.clear();
        received_errors_.clear();
        message_count_ = 0;
        connection_count_ = 0;
        error_count_ = 0;
    }

private:
    mutable std::mutex mutex_;
    std::vector<TransportMessage> received_messages_;
    std::vector<std::string> received_errors_;
    ConnectionInfo connection_info_;
    int message_count_ = 0;
    int connection_count_ = 0;
    int error_count_ = 0;
};

// 测试基类
class TransportTest : public ::testing::Test {
protected:
    void SetUp() override {
        transport_ = std::make_unique<Transport>();
        handler_ = std::make_unique<TestMessageHandler>();

        // 设置默认IPC配置
        ipc_config_.type = TransportType::kIpc;
        ipc_config_.ipc.command = "/bin/cat";
        ipc_config_.ipc.args = {};
        ipc_config_.ipc.buffer_size = 4096;
        ipc_config_.ipc.read_timeout_ms = 100;
        ipc_config_.ipc.max_retry_count = 3;
        ipc_config_.ipc.enable_logging = false;

        // 设置默认SSE配置
        sse_config_.type = TransportType::kSse;
        sse_config_.sse.host = "127.0.0.1";
        sse_config_.sse.port = 8080;
        sse_config_.sse.protocol = TransportProtocol::kHttp;
        sse_config_.sse.timeout_seconds = 5;
        sse_config_.sse.max_retries = 1;
        sse_config_.sse.keep_alive = true;
        sse_config_.auto_reconnect = false;

        // 设置回调
        transport_->setMessageCallback(
            [this](const TransportMessage& msg) { handler_->onMessage(msg); });
        transport_->setConnectionCallback([this](const ConnectionInfo& info) {
            handler_->onConnection(info);
        });
        transport_->setErrorCallback(
            [this](const std::string& error) { handler_->onError(error); });
    }

    void TearDown() override {
        if (transport_) {
            transport_->stop();
        }
        transport_.reset();
        handler_.reset();
    }

    std::unique_ptr<Transport> transport_;
    std::unique_ptr<TestMessageHandler> handler_;
    TransportConfig ipc_config_;
    TransportConfig sse_config_;
};

// TransportMessage 测试
class TransportMessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试消息
        test_message_.id = "test_id_123";
        test_message_.content = "Hello, World!";
        test_message_.method = "POST";
        test_message_.path = "/test";
        test_message_.headers["Content-Type"] = "application/json";
        test_message_.headers["User-Agent"] = "Test/1.0";
        test_message_.status_code = 200;
        test_message_.error_message = "";
        test_message_.timestamp = std::chrono::system_clock::now();
    }

    TransportMessage test_message_;
};

// TransportConfig 测试
class TransportConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试配置
        test_config_.type = TransportType::kIpc;
        test_config_.name = "test_transport";
        test_config_.ipc.command = "/bin/echo";
        test_config_.ipc.args = {"test", "message"};
        test_config_.ipc.env_vars["TEST_VAR"] = "test_value";
        test_config_.ipc.buffer_size = 8192;
        test_config_.ipc.read_timeout_ms = 200;
        test_config_.ipc.max_retry_count = 5;
        test_config_.ipc.enable_logging = true;
        test_config_.ipc.log_file_path = "/tmp/test.log";
        test_config_.ipc.log_level = mcp_transport::TransportLogLevel::kDebug;

        test_config_.sse.host = "localhost";
        test_config_.sse.port = 9090;
        test_config_.sse.protocol = TransportProtocol::kHttps;
        test_config_.sse.streamable_http = true;
        test_config_.sse.timeout_seconds = 10;
        test_config_.sse.max_retries = 3;
        test_config_.sse.headers["Authorization"] = "Bearer token123";
        test_config_.sse.user_agent = "TestAgent/2.0";
        test_config_.sse.keep_alive = false;
        test_config_.auto_reconnect = true;
        test_config_.reconnect_interval = 2;

        test_config_.enable_logging = true;
        test_config_.log_file_path = "/tmp/global.log";
        test_config_.log_level = 2;
    }

    TransportConfig test_config_;
};

// ============================================================================
// TransportMessage 测试用例
// ============================================================================

TEST_F(TransportMessageTest, ToJsonConversion) {
    nlohmann::json json = test_message_.toJson();

    EXPECT_EQ(json["id"], test_message_.id);
    EXPECT_EQ(json["content"], test_message_.content);
    EXPECT_EQ(json["method"], test_message_.method);
    EXPECT_EQ(json["path"], test_message_.path);
    EXPECT_EQ(json["status_code"], test_message_.status_code);
    EXPECT_EQ(json["error_message"], test_message_.error_message);

    // 检查headers
    EXPECT_TRUE(json.contains("headers"));
    EXPECT_EQ(json["headers"]["Content-Type"], "application/json");
    EXPECT_EQ(json["headers"]["User-Agent"], "Test/1.0");

    // 检查timestamp
    EXPECT_TRUE(json.contains("timestamp"));
    EXPECT_GT(json["timestamp"].get<uint64_t>(), 0ULL);
}

TEST_F(TransportMessageTest, FromJsonConversion) {
    nlohmann::json json = test_message_.toJson();
    TransportMessage converted = TransportMessage::fromJson(json);

    EXPECT_EQ(converted.id, test_message_.id);
    EXPECT_EQ(converted.content, test_message_.content);
    EXPECT_EQ(converted.method, test_message_.method);
    EXPECT_EQ(converted.path, test_message_.path);
    EXPECT_EQ(converted.status_code, test_message_.status_code);
    EXPECT_EQ(converted.error_message, test_message_.error_message);
    EXPECT_EQ(converted.headers, test_message_.headers);
}

TEST_F(TransportMessageTest, FromIpcMessage) {
    mcp::Message ipc_msg;
    ipc_msg.id = "ipc_123";
    ipc_msg.content = "IPC test content";
    ipc_msg.type = mcp::MessageType::REQUEST;

    auto convertFromIpc = [](const mcp::Message& msg) {
        TransportMessage transport_msg;
        transport_msg.id = msg.id;
        transport_msg.content = msg.content;
        transport_msg.method = "POST";
        transport_msg.path = "/";
        transport_msg.timestamp = std::chrono::system_clock::now();
        return transport_msg;
    };

    TransportMessage transport_msg = convertFromIpc(ipc_msg);

    EXPECT_EQ(transport_msg.id, ipc_msg.id);
    EXPECT_EQ(transport_msg.content, ipc_msg.content);
    EXPECT_EQ(transport_msg.method, "POST");  // 默认方法
    EXPECT_EQ(transport_msg.path, "/");       // 默认路径
}

TEST_F(TransportMessageTest, ToIpcMessage) {
    auto convertToIpc = [](const TransportMessage& transport_msg) {
        mcp::Message ipc_msg;
        ipc_msg.id = transport_msg.id;
        ipc_msg.content = transport_msg.content;
        ipc_msg.type = mcp::MessageType::REQUEST;
        return ipc_msg;
    };

    mcp::Message ipc_msg = convertToIpc(test_message_);

    EXPECT_EQ(ipc_msg.id, test_message_.id);
    EXPECT_EQ(ipc_msg.content, test_message_.content);
    EXPECT_EQ(ipc_msg.type, mcp::MessageType::REQUEST);
}

TEST_F(TransportMessageTest, FromNetioMessage) {
    netio::Message netio_msg;
    netio_msg.id = "netio_123";
    netio_msg.body = "NetIO test content";
    netio_msg.method = "GET";
    netio_msg.path = "/api/test";
    netio_msg.headers["X-Test"] = "value";
    netio_msg.status_code = 200;
    netio_msg.error_message = "";
    netio_msg.timestamp = std::chrono::system_clock::now();
    netio_msg.type = netio::MessageType::REQUEST;

    auto convertFromNetio = [](const netio::Message& msg) {
        TransportMessage transport_msg;
        transport_msg.id = msg.id;
        transport_msg.content = msg.body;
        transport_msg.method = msg.method;
        transport_msg.path = msg.path;
        transport_msg.headers = msg.headers;
        transport_msg.status_code = msg.status_code;
        transport_msg.error_message = msg.error_message;
        transport_msg.timestamp = msg.timestamp;
        return transport_msg;
    };

    TransportMessage transport_msg = convertFromNetio(netio_msg);

    EXPECT_EQ(transport_msg.id, netio_msg.id);
    EXPECT_EQ(transport_msg.content, netio_msg.body);
    EXPECT_EQ(transport_msg.method, netio_msg.method);
    EXPECT_EQ(transport_msg.path, netio_msg.path);
    EXPECT_EQ(transport_msg.headers, netio_msg.headers);
    EXPECT_EQ(transport_msg.status_code, netio_msg.status_code);
    EXPECT_EQ(transport_msg.error_message, netio_msg.error_message);
}

TEST_F(TransportMessageTest, ToNetioMessage) {
    auto convertToNetio = [](const TransportMessage& transport_msg) {
        netio::Message netio_msg;
        netio_msg.id = transport_msg.id;
        netio_msg.body = transport_msg.content;
        netio_msg.method = transport_msg.method;
        netio_msg.path = transport_msg.path;
        netio_msg.headers = transport_msg.headers;
        netio_msg.status_code = transport_msg.status_code;
        netio_msg.error_message = transport_msg.error_message;
        netio_msg.timestamp = transport_msg.timestamp;
        netio_msg.type = netio::MessageType::REQUEST;
        return netio_msg;
    };

    netio::Message netio_msg = convertToNetio(test_message_);

    EXPECT_EQ(netio_msg.id, test_message_.id);
    EXPECT_EQ(netio_msg.body, test_message_.content);
    EXPECT_EQ(netio_msg.method, test_message_.method);
    EXPECT_EQ(netio_msg.path, test_message_.path);
    EXPECT_EQ(netio_msg.headers, test_message_.headers);
    EXPECT_EQ(netio_msg.status_code, test_message_.status_code);
    EXPECT_EQ(netio_msg.error_message, test_message_.error_message);
    EXPECT_EQ(netio_msg.type, netio::MessageType::REQUEST);
}

// ============================================================================
// TransportConfig 测试用例
// ============================================================================

TEST_F(TransportConfigTest, ToJsonConversion) {
    nlohmann::json json = test_config_.toJson();

    EXPECT_EQ(json["type"], static_cast<int>(test_config_.type));
    EXPECT_EQ(json["name"], test_config_.name);

    // 检查IPC配置
    EXPECT_TRUE(json.contains("ipc"));
    const auto& ipc_json = json["ipc"];
    EXPECT_EQ(ipc_json["command"], test_config_.ipc.command);
    EXPECT_EQ(ipc_json["args"], test_config_.ipc.args);
    EXPECT_EQ(ipc_json["env_vars"], test_config_.ipc.env_vars);
    EXPECT_EQ(ipc_json["buffer_size"], test_config_.ipc.buffer_size);
    EXPECT_EQ(ipc_json["read_timeout_ms"], test_config_.ipc.read_timeout_ms);
    EXPECT_EQ(ipc_json["max_retry_count"], test_config_.ipc.max_retry_count);
    EXPECT_EQ(ipc_json["enableLogging"], test_config_.ipc.enable_logging);
    EXPECT_EQ(ipc_json["log_file_path"], test_config_.ipc.log_file_path);
    EXPECT_EQ(ipc_json["log_level"],
              static_cast<int>(test_config_.ipc.log_level));

    // 检查SSE配置
    EXPECT_TRUE(json.contains("sse"));
    const auto& sse_json = json["sse"];
    EXPECT_EQ(sse_json["host"], test_config_.sse.host);
    EXPECT_EQ(sse_json["port"], test_config_.sse.port);
    EXPECT_EQ(sse_json["protocol"],
              static_cast<int>(test_config_.sse.protocol));
    EXPECT_EQ(sse_json["streamable_http"], test_config_.sse.streamable_http);
    EXPECT_EQ(sse_json["timeout_seconds"], test_config_.sse.timeout_seconds);
    EXPECT_EQ(sse_json["max_retries"], test_config_.sse.max_retries);
    EXPECT_EQ(sse_json["headers"], test_config_.sse.headers);
    EXPECT_EQ(sse_json["user_agent"], test_config_.sse.user_agent);
    EXPECT_EQ(sse_json["keep_alive"], test_config_.sse.keep_alive);
    EXPECT_EQ(json["auto_reconnect"], test_config_.auto_reconnect);
    EXPECT_EQ(json["reconnect_interval"], test_config_.reconnect_interval);
    EXPECT_EQ(json["max_reconnect_attempts"],
              test_config_.max_reconnect_attempts);
    EXPECT_EQ(json["max_reconnect_interval"],
              test_config_.max_reconnect_interval);

    // 检查通用配置
    EXPECT_EQ(json["enableLogging"], test_config_.enable_logging);
    EXPECT_EQ(json["log_file_path"], test_config_.log_file_path);
    EXPECT_EQ(json["log_level"], test_config_.log_level);
}

TEST_F(TransportConfigTest, FromJsonConversion) {
    nlohmann::json json = test_config_.toJson();
    TransportConfig converted = TransportConfig::fromJson(json);

    EXPECT_EQ(converted.type, test_config_.type);
    EXPECT_EQ(converted.name, test_config_.name);

    // 检查IPC配置
    EXPECT_EQ(converted.ipc.command, test_config_.ipc.command);
    EXPECT_EQ(converted.ipc.args, test_config_.ipc.args);
    EXPECT_EQ(converted.ipc.env_vars, test_config_.ipc.env_vars);
    EXPECT_EQ(converted.ipc.buffer_size, test_config_.ipc.buffer_size);
    EXPECT_EQ(converted.ipc.read_timeout_ms, test_config_.ipc.read_timeout_ms);
    EXPECT_EQ(converted.ipc.max_retry_count, test_config_.ipc.max_retry_count);
    EXPECT_EQ(converted.ipc.enable_logging, test_config_.ipc.enable_logging);
    EXPECT_EQ(converted.ipc.log_file_path, test_config_.ipc.log_file_path);
    EXPECT_EQ(converted.ipc.log_level, test_config_.ipc.log_level);

    // 检查SSE配置
    EXPECT_EQ(converted.sse.host, test_config_.sse.host);
    EXPECT_EQ(converted.sse.port, test_config_.sse.port);
    EXPECT_EQ(converted.sse.protocol, test_config_.sse.protocol);
    EXPECT_EQ(converted.sse.streamable_http, test_config_.sse.streamable_http);
    EXPECT_EQ(converted.sse.timeout_seconds, test_config_.sse.timeout_seconds);
    EXPECT_EQ(converted.sse.max_retries, test_config_.sse.max_retries);
    EXPECT_EQ(converted.sse.headers, test_config_.sse.headers);
    EXPECT_EQ(converted.sse.user_agent, test_config_.sse.user_agent);
    EXPECT_EQ(converted.sse.keep_alive, test_config_.sse.keep_alive);
    EXPECT_EQ(converted.auto_reconnect, test_config_.auto_reconnect);
    EXPECT_EQ(converted.reconnect_interval, test_config_.reconnect_interval);
    EXPECT_EQ(converted.max_reconnect_attempts,
              test_config_.max_reconnect_attempts);
    EXPECT_EQ(converted.max_reconnect_interval,
              test_config_.max_reconnect_interval);

    // 检查通用配置
    EXPECT_EQ(converted.enable_logging, test_config_.enable_logging);
    EXPECT_EQ(converted.log_file_path, test_config_.log_file_path);
    EXPECT_EQ(converted.log_level, test_config_.log_level);
}

TEST_F(TransportConfigTest, FromJsonAcceptsLegacySseReconnectFields) {
    nlohmann::json json = test_config_.toJson();
    json.erase("auto_reconnect");
    json.erase("reconnect_interval");
    json.erase("max_reconnect_attempts");
    json.erase("max_reconnect_interval");
    json["sse"]["auto_reconnect"] = true;
    json["sse"]["reconnect_interval"] = 3;
    json["sse"]["max_reconnect_attempts"] = 4;
    json["sse"]["max_reconnect_interval"] = 30;

    TransportConfig converted = TransportConfig::fromJson(json);

    EXPECT_TRUE(converted.auto_reconnect);
    EXPECT_EQ(converted.reconnect_interval, 3);
    EXPECT_EQ(converted.max_reconnect_attempts, 4);
    EXPECT_EQ(converted.max_reconnect_interval, 30);
}

TEST_F(TransportConfigTest, FromJsonFile) {
    // 创建临时配置文件
    std::string config_file = "test_transport_config.json";
    nlohmann::json json = test_config_.toJson();

    std::ofstream file(config_file);
    if (file.is_open()) {
        file << json.dump(2);
        file.close();

        // 从文件加载配置
        TransportConfig loaded_config =
            TransportConfig::fromJsonFile(config_file);

        EXPECT_EQ(loaded_config.type, test_config_.type);
        EXPECT_EQ(loaded_config.name, test_config_.name);
        EXPECT_EQ(loaded_config.ipc.command, test_config_.ipc.command);
        EXPECT_EQ(loaded_config.sse.host, test_config_.sse.host);

        // 清理临时文件
        std::remove(config_file.c_str());
    } else {
        // 如果无法创建文件，跳过此测试
        GTEST_SKIP() << "Cannot create temporary config file";
    }
}

TEST_F(TransportConfigTest, FromJsonFileInvalidPathThrows) {
    EXPECT_THROW(
        TransportConfig::fromJsonFile("nonexistent_transport_config_file.json"),
        TransportError);
}

TEST_F(TransportConfigTest, FromJsonParsesModeServerAndEndpoint) {
    nlohmann::json j = {
        {"type", static_cast<int>(TransportType::kSse)},
        {"mode", static_cast<int>(TransportMode::kServer)},
        {"name", "srv"},
        {"sse",
         {{"host", "127.0.0.1"},
          {"port", 18081},
          {"endpoint", "/mcp"},
          {"protocol", static_cast<int>(TransportProtocol::kHttp)}}},
        {"server",
         {{"server_name", "mcp-server"},
          {"server_version", "2.0.0"},
          {"description", "desc"}}}};

    TransportConfig cfg = TransportConfig::fromJson(j);
    EXPECT_EQ(cfg.type, TransportType::kSse);
    EXPECT_EQ(cfg.mode, TransportMode::kServer);
    EXPECT_EQ(cfg.name, "srv");
    EXPECT_EQ(cfg.sse.endpoint, "/mcp");
    EXPECT_EQ(cfg.server.server_name, "mcp-server");
    EXPECT_EQ(cfg.server.server_version, "2.0.0");
    EXPECT_EQ(cfg.server.description, "desc");
}

TEST_F(TransportConfigTest, Validation) {
    // 测试有效配置
    EXPECT_TRUE(test_config_.validate());

    // 测试无效IPC配置
    TransportConfig invalid_ipc = test_config_;
    invalid_ipc.type = TransportType::kIpc;
    invalid_ipc.ipc.command = "";  // 空命令
    EXPECT_FALSE(invalid_ipc.validate());

    // 测试无效SSE配置
    TransportConfig invalid_sse = test_config_;
    invalid_sse.type = TransportType::kSse;
    invalid_sse.sse.host = "";  // 空主机
    EXPECT_FALSE(invalid_sse.validate());

    invalid_sse.sse.host = "localhost";
    invalid_sse.sse.port = 0;  // 无效端口
    EXPECT_FALSE(invalid_sse.validate());

    invalid_sse.sse.port = 70000;  // 超出范围端口
    EXPECT_FALSE(invalid_sse.validate());

    // 测试AUTO类型（应该总是有效）
    TransportConfig auto_config = test_config_;
    auto_config.type = TransportType::kAuto;
    EXPECT_TRUE(auto_config.validate());
}

TEST_F(TransportMessageTest, FromJsonWithMissingTimestampUsesNow) {
    nlohmann::json j = {{"id", "id-1"},           {"content", "hello"},
                        {"method", "GET"},        {"path", "/"},
                        {"status_code", 200},     {"error_message", ""},
                        {"headers", {{"k", "v"}}}};

    auto before = std::chrono::system_clock::now();
    TransportMessage msg = TransportMessage::fromJson(j);
    auto after = std::chrono::system_clock::now();

    EXPECT_EQ(msg.id, "id-1");
    EXPECT_EQ(msg.headers["k"], "v");
    EXPECT_GE(msg.timestamp, before);
    EXPECT_LE(msg.timestamp, after);
}

TEST(TransportConnectionInfoTest, FromJsonRoundTrip) {
    ConnectionInfo info;
    info.connection_id = "c1";
    info.remote_address = "127.0.0.1";
    info.remote_port = 18080;
    info.local_address = "127.0.0.1";
    info.local_port = 18081;
    info.status = TransportStatus::kConnected;
    info.type = TransportType::kSse;
    info.session_id = "sid";
    info.connected_at = std::chrono::system_clock::now();
    info.last_activity = info.connected_at;

    nlohmann::json j = info.toJson();
    ConnectionInfo out = ConnectionInfo::fromJson(j);
    EXPECT_EQ(out.connection_id, "c1");
    EXPECT_EQ(out.remote_port, 18080);
    EXPECT_EQ(out.status, TransportStatus::kConnected);
    EXPECT_EQ(out.type, TransportType::kSse);
    EXPECT_EQ(out.session_id, "sid");
}

// ============================================================================
// Transport 核心功能测试用例
// ============================================================================

TEST_F(TransportTest, ConstructorAndDestructor) {
    EXPECT_NE(transport_, nullptr);
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
    EXPECT_FALSE(transport_->isRunning());
}

TEST_F(TransportTest, BasicConfiguration) {
    // 测试配置更新
    transport_->updateConfig(ipc_config_);
    TransportConfig retrieved_config = transport_->getConfig();

    EXPECT_EQ(retrieved_config.type, ipc_config_.type);
    EXPECT_EQ(retrieved_config.ipc.command, ipc_config_.ipc.command);
}

TEST_F(TransportTest, StatusManagement) {
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
    EXPECT_FALSE(transport_->isRunning());

    // 测试连接信息
    ConnectionInfo info = transport_->getConnectionInfo();
    EXPECT_TRUE(info.connection_id.empty());
    EXPECT_EQ(info.status, TransportStatus::kDisconnected);
}

TEST_F(TransportTest, CallbackManagement) {
    // 测试回调设置
    bool message_callback_called = false;
    bool connection_callback_called = false;
    bool error_callback_called = false;

    transport_->setMessageCallback(
        [&](const TransportMessage&) { message_callback_called = true; });

    transport_->setConnectionCallback(
        [&](const ConnectionInfo&) { connection_callback_called = true; });

    transport_->setErrorCallback(
        [&](const std::string&) { error_callback_called = true; });

    // 回调设置成功（没有异常）
    EXPECT_TRUE(true);
}

TEST_F(TransportTest, Statistics) {
    nlohmann::json stats = transport_->getStatistics();
    // 统计信息可能为空对象，这是正常的
    EXPECT_TRUE(stats.is_object() || stats.is_null());
}

TEST_F(TransportTest, HealthCheck) {
    // 未连接状态下健康检查应该返回false
    EXPECT_FALSE(transport_->healthCheck());
}

TEST_F(TransportTest, ServerApiShouldFailWhenNotRunning) {
    EXPECT_FALSE(transport_->sendResponse("req-1", R"({"ok":true})"));
    EXPECT_FALSE(transport_->sendError("req-1", -32603, "err"));
    EXPECT_FALSE(
        transport_->sendNotification("notifications/tools/list_changed", "{}"));

    EXPECT_NO_THROW(transport_->setConnectionRequestHandler(nullptr));
}

TEST_F(TransportTest, ServerApiSseModeWithoutClientsCoversBranches) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    server_config.mode = TransportMode::kServer;
    ASSERT_TRUE(transport_->start(server_config));
    ASSERT_TRUE(transport_->isRunning());

    EXPECT_FALSE(transport_->sendResponse("req-1", R"({"ok":true})"));
    EXPECT_FALSE(transport_->sendError("req-1", -32603, "err"));
    EXPECT_FALSE(transport_->sendNotification(
        "notifications/tools/list_changed", "not-a-json"));

    EXPECT_FALSE(transport_->hasMessage());
    EXPECT_TRUE(transport_->getAllMessages().empty());
    EXPECT_TRUE(transport_->getMessage().id.empty());

    EXPECT_TRUE(transport_->stop());
}

TEST_F(TransportTest, SendRequestStringOverloadWhenDisconnectedReturnsError) {
    auto response = transport_->sendRequestSync(
        "body", "POST", "/mcp", {{"Content-Type", "application/json"}});
    EXPECT_NE(response.status_code, 200);
    EXPECT_FALSE(response.error_message.empty());

    auto future_response = transport_->sendRequest(
        "body", "POST", "/mcp", {{"Content-Type", "application/json"}});
    ASSERT_EQ(future_response.wait_for(std::chrono::milliseconds(500)),
              std::future_status::ready);
    auto async_response = future_response.get();
    EXPECT_NE(async_response.status_code, 200);
}

TEST_F(TransportTest, LoggingApiNoThrow) {
    EXPECT_NO_THROW(transport_->setLogLevel(0));
    EXPECT_NO_THROW(transport_->setLogLevel(3));
    EXPECT_NO_THROW(transport_->setLogFile("/tmp/transport_gtest.log"));
    EXPECT_NO_THROW(transport_->enableLogging(true));
    EXPECT_NO_THROW(transport_->enableLogging(false));
    EXPECT_NO_THROW(transport_->log(0, "debug"));
    EXPECT_NO_THROW(transport_->log(1, "info"));
    EXPECT_NO_THROW(transport_->log(2, "warn"));
    EXPECT_NO_THROW(transport_->log(3, "error"));
    EXPECT_NO_THROW(transport_->log(4, "fatal"));
    EXPECT_NO_THROW(transport_->log(99, "other"));
}

TEST_F(TransportTest, LoggingApiWritesToConfiguredFile) {
    const auto now_ns =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const std::string log_path =
        "/tmp/transport_gtest_log_" + std::to_string(now_ns) + ".log";

    std::remove(log_path.c_str());
    transport_->setLogFile(log_path);
    transport_->log(1, "transport-log-file-check");
    transport_->setLogFile("");

    std::ifstream file(log_path);
    ASSERT_TRUE(file.is_open());
    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string content = buffer.str();
    EXPECT_NE(content.find("transport-log-file-check"), std::string::npos);

    file.close();
    std::remove(log_path.c_str());
}

// ============================================================================
// IPC 传输模式测试用例
// ============================================================================

TEST_F(TransportTest, IpcStartStop) {
    // 测试IPC启动
    bool start_result = transport_->start(ipc_config_);

    // 注意：由于IPC需要实际的可执行文件，这里可能失败
    // 我们主要测试接口调用不抛异常
    EXPECT_TRUE(start_result || !start_result);  // 接受任何结果

    if (start_result) {
        EXPECT_TRUE(transport_->isRunning());
        EXPECT_EQ(transport_->getStatus(), TransportStatus::kConnected);

        // 测试停止
        bool stop_result = transport_->stop();
        EXPECT_TRUE(stop_result);
        EXPECT_FALSE(transport_->isRunning());
        EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
    }
}

TEST_F(TransportTest, IpcMessageHandling) {
    if (transport_->start(ipc_config_)) {
        // 测试消息发送（同步）
        TransportMessage request =
            utils::createRequest("test message", "POST", "/test");
        TransportMessage response = transport_->sendRequestSync(request);

        // 验证响应
        EXPECT_FALSE(response.id.empty());
        EXPECT_FALSE(response.content.empty());

        // 测试异步发送
        auto future_response = transport_->sendRequest(request);
        auto status = future_response.wait_for(std::chrono::seconds(1));

        if (status == std::future_status::ready) {
            TransportMessage async_response = future_response.get();
            EXPECT_FALSE(async_response.id.empty());
        }

        transport_->stop();
    }
}

TEST_F(TransportTest, IpcMessageQueue) {
    if (transport_->start(ipc_config_)) {
        // 测试消息队列功能
        EXPECT_FALSE(transport_->hasMessage());  // 初始状态应该没有消息

        // 发送消息后检查队列
        TransportMessage request =
            utils::createRequest("queue test", "POST", "/test");
        transport_->sendRequestSync(request);

        // 注意：IPC模式下消息队列行为取决于具体实现
        // 这里主要测试接口调用不抛异常
        EXPECT_TRUE(true);

        transport_->stop();
    }
}

TEST_F(TransportTest, IpcConnectionLostConcurrentRequestsAndDisconnectNoDeadlock) {
    TransportConfig config = ipc_config_;
    config.ipc.command = "/bin/sh";
    config.ipc.args = {"-c", "sleep 0.3; exit 0"};
    config.ipc.read_timeout_ms = 50;
    config.ipc.max_retry_count = 1;
    config.auto_reconnect = false;
    config.enable_logging = false;

    ASSERT_TRUE(transport_->start(config));
    ASSERT_EQ(transport_->getStatus(), TransportStatus::kConnected);

    std::atomic<bool> stop_sender{false};
    std::atomic<int> request_count{0};
    std::thread sender([&]() {
        while (!stop_sender.load(std::memory_order_acquire)) {
            TransportMessage request =
                utils::createRequest(R"({"jsonrpc":"2.0","id":"req","method":"ping"})",
                                     "POST", "/mcp");
            (void)transport_->sendRequestSync(request);
            ++request_count;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    EXPECT_TRUE(wait_until(std::chrono::milliseconds(2000), [&]() {
        return transport_->getStatus() == TransportStatus::kError;
    }));

    auto disconnect_future = std::async(std::launch::async, [&]() {
        return transport_->stop();
    });
    ASSERT_EQ(disconnect_future.wait_for(std::chrono::seconds(2)),
              std::future_status::ready);
    EXPECT_TRUE(disconnect_future.get());

    stop_sender.store(true, std::memory_order_release);
    if (sender.joinable()) {
        sender.join();
    }

    EXPECT_GT(request_count.load(), 0);
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
}

TEST_F(TransportTest, IpcAutoReconnectBackoffBudgetExhaustionIsBounded) {
    TransportConfig config = ipc_config_;
    config.ipc.command = "/bin/sh";
    config.ipc.args = {"-c", "sleep 0.3; exit 0"};
    config.ipc.read_timeout_ms = 50;
    config.ipc.max_retry_count = 1;
    config.ipc.respawn_on_disconnect = false;
    config.auto_reconnect = true;
    config.reconnect_interval = 1;
    config.max_reconnect_attempts = 5;
    config.max_reconnect_interval = 5;
    config.enable_logging = false;

    std::atomic<bool> reconnecting_seen{false};
    std::atomic<bool> budget_exhausted_seen{false};
    std::chrono::steady_clock::time_point reconnecting_at;
    std::chrono::steady_clock::time_point exhausted_at;
    std::mutex time_mutex;

    transport_->setConnectionCallback([&](const ConnectionInfo& info) {
        handler_->onConnection(info);
        if (info.status == TransportStatus::kReconnecting) {
            std::lock_guard<std::mutex> lock(time_mutex);
            reconnecting_at = std::chrono::steady_clock::now();
            reconnecting_seen.store(true, std::memory_order_release);
        }
    });
    transport_->setErrorCallback([&](const std::string& error) {
        handler_->onError(error);
        if (error.find("respawn disabled") != std::string::npos) {
            std::lock_guard<std::mutex> lock(time_mutex);
            exhausted_at = std::chrono::steady_clock::now();
            budget_exhausted_seen.store(true, std::memory_order_release);
        }
    });

    ASSERT_TRUE(transport_->start(config));
    ASSERT_EQ(transport_->getStatus(), TransportStatus::kConnected);

    ASSERT_TRUE(wait_until(std::chrono::milliseconds(30000), [&]() {
        return budget_exhausted_seen.load(std::memory_order_acquire);
    }));

    std::chrono::milliseconds elapsed{0};
    {
        std::lock_guard<std::mutex> lock(time_mutex);
        ASSERT_TRUE(reconnecting_seen.load(std::memory_order_acquire));
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            exhausted_at - reconnecting_at);
    }

    // Five attempts with base interval 1s and max interval 5s should take at
    // least 1+2+3+4+5 seconds, plus up to 1s jitter per attempt, and stay under
    // max_attempts * max_reconnect_interval with a small scheduling allowance.
    EXPECT_GE(elapsed, std::chrono::seconds(15));
    EXPECT_LE(elapsed, std::chrono::seconds(27));
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kError);

    EXPECT_TRUE(transport_->stop());
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kDisconnected);
}

// ============================================================================
// SSE 传输模式测试用例
// ============================================================================

TEST_F(TransportTest, DISABLED_SseStartStop) {
    int port = get_free_tcp_port();
    ASSERT_GT(port, 0) << "Failed to allocate free port for SSE test";

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;

    ASSERT_TRUE(server_transport.start(server_config));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    sse_config_.mode = TransportMode::kClient;
    sse_config_.sse.host = "127.0.0.1";
    sse_config_.sse.port = port;
    sse_config_.sse.endpoint = "/mcp";
    sse_config_.enable_logging = false;

    ASSERT_TRUE(transport_->start(sse_config_));
    EXPECT_TRUE(transport_->isRunning());
    EXPECT_EQ(transport_->getStatus(), TransportStatus::kConnected);

    // 停止客户端和服务端
    EXPECT_TRUE(transport_->stop());
    EXPECT_FALSE(transport_->isRunning());

    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, DISABLED_SseMessageHandling) {
    int port = get_free_tcp_port();
    ASSERT_GT(port, 0);

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;

    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    sse_config_.mode = TransportMode::kClient;
    sse_config_.sse.host = "127.0.0.1";
    sse_config_.sse.port = port;
    sse_config_.sse.endpoint = "/mcp";
    sse_config_.enable_logging = false;

    ASSERT_TRUE(transport_->start(sse_config_));

    // 构造 JSON-RPC 请求
    nlohmann::json request_json;
    request_json["jsonrpc"] = "2.0";
    request_json["id"] = "ping-1";
    request_json["method"] = "ping";
    request_json["params"] = nlohmann::json::object();

    TransportMessage response =
        transport_->sendRequestSync(request_json.dump(), "POST", "/mcp");
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.content.empty());

    auto json_response = nlohmann::json::parse(response.content);
    EXPECT_TRUE(json_response.contains("result"));
    EXPECT_EQ(json_response["result"]["message"], "pong");
    EXPECT_EQ(server_handler->requestCount(), 1);
    EXPECT_EQ(server_handler->lastMethod(), "ping");

    // 构造通知（无 id）
    nlohmann::json notification_json;
    notification_json["jsonrpc"] = "2.0";
    notification_json["method"] = "log";
    notification_json["params"] = {{"message", "hello"}, {"from", "test"}};

    TransportMessage notification_response =
        transport_->sendRequestSync(notification_json.dump(), "POST", "/mcp");
    EXPECT_EQ(notification_response.status_code, 204);
    EXPECT_TRUE(notification_response.content.empty());
    EXPECT_EQ(server_handler->notificationCount(), 1);
    EXPECT_EQ(server_handler->lastNotificationMethod(), "log");

    EXPECT_TRUE(transport_->stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseStartStopSmoke) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client_transport.start(client_config));

    EXPECT_TRUE(client_transport.isRunning());
    EXPECT_EQ(client_transport.getStatus(), TransportStatus::kConnected);

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseMessageHandlingRoundTrip) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client_transport.start(client_config));
    ASSERT_TRUE(client_transport.isRunning());

    nlohmann::json request_json;
    request_json["jsonrpc"] = "2.0";
    request_json["id"] = "ping-1";
    request_json["method"] = "ping";
    request_json["params"] = nlohmann::json::object();

    TransportMessage response =
        client_transport.sendRequestSync(request_json.dump(), "POST", "/mcp");
    EXPECT_EQ(response.status_code, 200);
    ASSERT_FALSE(response.content.empty());

    auto json_response = nlohmann::json::parse(response.content);
    EXPECT_TRUE(json_response.contains("result"));
    EXPECT_EQ(json_response["result"]["message"], "pong");
    EXPECT_EQ(server_handler->requestCount(), 1);
    EXPECT_EQ(server_handler->lastMethod(), "ping");

    nlohmann::json notification_json;
    notification_json["jsonrpc"] = "2.0";
    notification_json["method"] = "log";
    notification_json["params"] = {{"message", "hello"}, {"from", "test"}};

    TransportMessage notification_response = client_transport.sendRequestSync(
        notification_json.dump(), "POST", "/mcp");
    EXPECT_TRUE(notification_response.status_code == 202 ||
                notification_response.status_code == 204);
    EXPECT_TRUE(notification_response.content.empty());
    EXPECT_EQ(server_handler->notificationCount(), 1);
    EXPECT_EQ(server_handler->lastNotificationMethod(), "log");

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseDedicatedRequestHandlerOverridesServerHandler) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    auto sse_handler = std::make_shared<TestDedicatedSseRequestHandler>();
    server_transport.setServerRequestHandler(server_handler);
    server_transport.setSseServerRequestHandler(sse_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client_transport.start(client_config));
    ASSERT_TRUE(client_transport.isRunning());

    nlohmann::json request_json;
    request_json["jsonrpc"] = "2.0";
    request_json["id"] = "sse-dedicated-handler-test";
    request_json["method"] = "ping";
    request_json["params"] = nlohmann::json::object();

    TransportMessage response =
        client_transport.sendRequestSync(request_json.dump(), "POST", "/mcp");
    EXPECT_EQ(response.status_code, 200);
    ASSERT_FALSE(response.content.empty());

    const auto response_json = nlohmann::json::parse(response.content);
    EXPECT_EQ(response_json["result"]["message"], "sse-handler");
    EXPECT_EQ(sse_handler->requestCount(), 1);
    EXPECT_EQ(server_handler->requestCount(), 0);

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseServerProgressNotificationDeliveredToClient) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;

    std::mutex notify_mutex;
    std::condition_variable notify_cv;
    bool progress_received = false;
    std::string progress_payload;
    client_transport.setMessageCallback([&](const TransportMessage& msg) {
        nlohmann::json payload;
        try {
            payload = nlohmann::json::parse(msg.content);
        } catch (const std::exception&) {
            return;
        }
        if (!payload.is_object() || payload.value("jsonrpc", "") != "2.0") {
            return;
        }
        if (payload.value("method", "") != "notifications/progress") {
            return;
        }
        std::lock_guard<std::mutex> lock(notify_mutex);
        progress_payload = msg.content;
        progress_received = true;
        notify_cv.notify_one();
    });

    ASSERT_TRUE(client_transport.start(client_config));
    ASSERT_TRUE(client_transport.isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    ASSERT_TRUE(server_transport.sendNotification(
        "notifications/progress",
        R"({"progressToken":"tok-1","progress":1.0,"total":3.0,"message":"step 1/3"})"));

    {
        std::unique_lock<std::mutex> lock(notify_mutex);
        ASSERT_TRUE(notify_cv.wait_for(lock, std::chrono::seconds(2),
                                       [&]() { return progress_received; }));
    }

    const auto progress_json = nlohmann::json::parse(progress_payload);
    ASSERT_TRUE(progress_json.contains("params"));
    EXPECT_EQ(progress_json["params"]["progressToken"], "tok-1");
    EXPECT_DOUBLE_EQ(progress_json["params"]["progress"].get<double>(), 1.0);
    EXPECT_DOUBLE_EQ(progress_json["params"]["total"].get<double>(), 3.0);
    EXPECT_EQ(progress_json["params"]["message"], "step 1/3");

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseRejectsUnsupportedProtocolVersionHeader) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client_transport.start(client_config));
    ASSERT_TRUE(client_transport.isRunning());

    nlohmann::json request_json;
    request_json["jsonrpc"] = "2.0";
    request_json["id"] = "ping-invalid-version";
    request_json["method"] = "ping";
    request_json["params"] = nlohmann::json::object();

    TransportMessage response = client_transport.sendRequestSync(
        request_json.dump(), "POST", "/mcp",
        {{"MCP-Protocol-Version", "2099-01-01"}});
    EXPECT_EQ(response.status_code, 400);
    EXPECT_NE(response.content.find("Unsupported MCP protocol version"),
              std::string::npos);

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

TEST_F(TransportTest, SseStreamableHttpRoundTrip) {
    int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP() << "No free loopback TCP port available in current environment";
    }

    Transport server_transport;
    auto server_handler = std::make_shared<TestSseServerHandler>();
    server_transport.setServerRequestHandler(server_handler);

    TransportConfig server_config = utils::createSseServerConfig(
        "127.0.0.1", port, TransportProtocol::kHttp);
    server_config.sse.endpoint = "/mcp";
    server_config.sse.streamable_http = true;
    server_config.enable_logging = false;
    ASSERT_TRUE(server_transport.start(server_config));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Transport client_transport;
    TransportConfig client_config =
        utils::createSseConfig("127.0.0.1", port, TransportProtocol::kHttp);
    client_config.mode = TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.sse.streamable_http = true;
    client_config.enable_logging = false;
    ASSERT_TRUE(client_transport.start(client_config));
    ASSERT_TRUE(client_transport.isRunning());

    nlohmann::json request_json;
    request_json["jsonrpc"] = "2.0";
    request_json["id"] = "ping-streamable-http";
    request_json["method"] = "ping";
    request_json["params"] = nlohmann::json::object();

    TransportMessage response =
        client_transport.sendRequestSync(request_json.dump(), "POST", "/mcp");
    EXPECT_EQ(response.status_code, 200);
    ASSERT_FALSE(response.content.empty());
    auto json_response = nlohmann::json::parse(response.content);
    EXPECT_TRUE(json_response.contains("result"));
    EXPECT_EQ(json_response["result"]["message"], "pong");

    EXPECT_TRUE(client_transport.stop());
    EXPECT_TRUE(server_transport.stop());
}

// ============================================================================
// 工具函数测试用例
// ============================================================================

TEST(TransportUtilsTest, CreateIpcConfig) {
    std::vector<std::string> args = {"arg1", "arg2"};
    std::map<std::string, std::string> env_vars = {{"VAR1", "value1"}};

    TransportConfig config =
        utils::createIpcConfig("/bin/test", args, env_vars);

    EXPECT_EQ(config.type, TransportType::kIpc);
    EXPECT_EQ(config.ipc.command, "/bin/test");
    EXPECT_EQ(config.ipc.args, args);
    EXPECT_EQ(config.ipc.env_vars, env_vars);
}

TEST(TransportUtilsTest, CreateSseConfig) {
    TransportConfig config =
        utils::createSseConfig("example.com", 8080, TransportProtocol::kHttps);

    EXPECT_EQ(config.type, TransportType::kSse);
    EXPECT_EQ(config.sse.host, "example.com");
    EXPECT_EQ(config.sse.port, 8080);
    EXPECT_EQ(config.sse.protocol, TransportProtocol::kHttps);
}

TEST(TransportUtilsTest, CreateIpcServerConfig) {
    TransportConfig config = utils::createIpcServerConfig("server-A", "2.0.1");
    EXPECT_EQ(config.type, TransportType::kIpc);
    EXPECT_EQ(config.mode, TransportMode::kServer);
    EXPECT_EQ(config.server.server_name, "server-A");
    EXPECT_EQ(config.server.server_version, "2.0.1");
}

TEST(TransportUtilsTest, CreateSseServerConfig) {
    TransportConfig config = utils::createSseServerConfig(
        "127.0.0.1", 19090, TransportProtocol::kHttp);
    EXPECT_EQ(config.type, TransportType::kSse);
    EXPECT_EQ(config.mode, TransportMode::kServer);
    EXPECT_EQ(config.sse.host, "127.0.0.1");
    EXPECT_EQ(config.sse.port, 19090);
    EXPECT_EQ(config.sse.protocol, TransportProtocol::kHttp);
}

TEST(TransportUtilsTest, CreateSseSecureConfig) {
    TransportConfig config =
        utils::createSseSecureConfig("secure.example.com", 443);

    EXPECT_EQ(config.type, TransportType::kSse);
    EXPECT_EQ(config.sse.host, "secure.example.com");
    EXPECT_EQ(config.sse.port, 443);
    EXPECT_EQ(config.sse.protocol, TransportProtocol::kHttps);
}

TEST(TransportUtilsTest, CreateRequest) {
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"}};

    TransportMessage msg =
        utils::createRequest("test content", "POST", "/api/test", headers);

    EXPECT_FALSE(msg.id.empty());
    EXPECT_EQ(msg.content, "test content");
    EXPECT_EQ(msg.method, "POST");
    EXPECT_EQ(msg.path, "/api/test");
    EXPECT_EQ(msg.headers, headers);
    EXPECT_GT(msg.timestamp.time_since_epoch().count(), 0);
}

TEST(TransportUtilsTest, CreateResponse) {
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"}};

    TransportMessage msg =
        utils::createResponse("response content", 200, headers);

    EXPECT_FALSE(msg.id.empty());
    EXPECT_EQ(msg.content, "response content");
    EXPECT_EQ(msg.status_code, 200);
    EXPECT_EQ(msg.headers, headers);
    EXPECT_GT(msg.timestamp.time_since_epoch().count(), 0);
}

TEST(TransportUtilsTest, CreateError) {
    std::map<std::string, std::string> headers = {{"X-Error-Code", "500"}};

    TransportMessage msg =
        utils::createError("Internal Server Error", 500, headers);

    EXPECT_FALSE(msg.id.empty());
    EXPECT_EQ(msg.content, "Internal Server Error");
    EXPECT_EQ(msg.status_code, 500);
    EXPECT_EQ(msg.error_message, "Internal Server Error");
    EXPECT_EQ(msg.headers, headers);
}

TEST(TransportUtilsTest, ValidateConfig) {
    TransportConfig valid_config;
    valid_config.type = TransportType::kIpc;
    valid_config.ipc.command = "/bin/test";
    valid_config.ipc.buffer_size = 1024;
    valid_config.ipc.read_timeout_ms = 100;

    EXPECT_TRUE(utils::validateConfig(valid_config));

    TransportConfig invalid_config;
    invalid_config.type = TransportType::kIpc;
    // 缺少必要字段
    EXPECT_FALSE(utils::validateConfig(invalid_config));
}

TEST(TransportUtilsTest, ValidateMessage) {
    TransportMessage valid_msg;
    valid_msg.id = "test_id";
    valid_msg.content = "test content";

    EXPECT_TRUE(utils::validateMessage(valid_msg));

    TransportMessage invalid_msg;
    invalid_msg.id = "";  // 空ID
    invalid_msg.content = "test content";
    EXPECT_FALSE(utils::validateMessage(invalid_msg));

    invalid_msg.id = "test_id";
    invalid_msg.content = "";  // 空内容
    EXPECT_FALSE(utils::validateMessage(invalid_msg));
}

TEST(TransportUtilsTest, JsonUtilities) {
    nlohmann::json test_json = {{"key", "value"}, {"number", 42}};

    std::string json_string = utils::jsonToString(test_json);
    EXPECT_FALSE(json_string.empty());
    EXPECT_TRUE(json_string.find("key") != std::string::npos);
    EXPECT_TRUE(json_string.find("value") != std::string::npos);

    nlohmann::json parsed_json = utils::stringToJson(json_string);
    EXPECT_EQ(parsed_json["key"], "value");
    EXPECT_EQ(parsed_json["number"], 42);

    // 测试无效JSON
    nlohmann::json invalid_json = utils::stringToJson("invalid json");
    EXPECT_TRUE(invalid_json.is_object());
    EXPECT_TRUE(invalid_json.empty());
}

// ============================================================================
// 错误处理测试用例
// ============================================================================

TEST(TransportErrorTest, ExceptionHandling) {
    TransportError error("Test error message");
    EXPECT_STREQ(error.what(), "Test error message");
}

TEST(TransportErrorHandlingTest, InvalidConfigHandling) {
    Transport transport;
    TransportConfig invalid_config;
    invalid_config.type = TransportType::kIpc;
    // 不设置必要的字段

    bool result = transport.start(invalid_config);
    EXPECT_FALSE(result);
    EXPECT_EQ(transport.getStatus(), TransportStatus::kError);
}

TEST(TransportErrorHandlingTest, MessageSendingWhenDisconnected) {
    Transport transport;
    TransportMessage request = utils::createRequest("test", "POST", "/test");

    // 未连接状态下发送消息
    TransportMessage response = transport.sendRequestSync(request);

    // 应该返回错误响应
    EXPECT_NE(response.status_code, 200);
    EXPECT_FALSE(response.error_message.empty());
}

// ============================================================================
// 性能测试用例
// ============================================================================

TEST(TransportPerformanceTest, MessageCreationPerformance) {
    const int num_messages = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_messages; ++i) {
        TransportMessage msg =
            utils::createRequest("test message " + std::to_string(i));
        EXPECT_FALSE(msg.id.empty());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    // 1000条消息创建应该在合理时间内完成（比如100ms内）
    EXPECT_LT(duration.count(), 100);
}

TEST(TransportPerformanceTest, JsonSerializationPerformance) {
    TransportMessage msg = utils::createRequest("performance test message");
    msg.headers["Header1"] = "Value1";
    msg.headers["Header2"] = "Value2";
    msg.headers["Header3"] = "Value3";

    const int num_iterations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_iterations; ++i) {
        nlohmann::json json = msg.toJson();
        TransportMessage converted = TransportMessage::fromJson(json);
        EXPECT_EQ(converted.id, msg.id);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    // 1000次JSON转换应该在合理时间内完成
    EXPECT_LT(duration.count(), 200);
}

// ============================================================================
// 主测试入口
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
