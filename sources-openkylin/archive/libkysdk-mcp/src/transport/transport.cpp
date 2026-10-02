#include "common/transport.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "../ipc/pipe_communication.h"
#include "../server/server_dispatch.h"
#include "../session/session_manager.h"
#include "../log/logger.h"
#include "../net/netio.h"

namespace mcp_transport {

namespace {

bool fillJsonRpcResponseBody(const std::string& payload,
                             const nlohmann::json& request_id,
                             std::string& body) {
    if (mcp::internal::isJsonRpcPayload(payload)) {
        nlohmann::json envelope = nlohmann::json::parse(payload);
        envelope["id"] = request_id;
        body = envelope.dump();
        return true;
    }

    nlohmann::json response_json;
    response_json["jsonrpc"] = "2.0";
    response_json["id"] = request_id;
    if (!payload.empty()) {
        try {
            response_json["result"] = nlohmann::json::parse(payload);
        } catch (const std::exception&) {
            response_json["result"] = payload;
        }
    } else {
        response_json["result"] = nlohmann::json::object();
    }

    body = response_json.dump();
    return true;
}

mcp::Message toIpcMessage(const TransportMessage& message) {
    mcp::Message ipc_msg;
    ipc_msg.id = message.id;
    ipc_msg.content = message.content;
    ipc_msg.type = mcp::MessageType::REQUEST;
    return ipc_msg;
}

TransportMessage fromIpcMessage(const mcp::Message& msg) {
    TransportMessage transport_msg;
    transport_msg.id = msg.id;
    transport_msg.content = msg.content;
    transport_msg.method = "POST";
    transport_msg.path = "/";
    transport_msg.timestamp = std::chrono::system_clock::now();
    return transport_msg;
}

netio::Message toNetioMessage(const TransportMessage& message) {
    netio::Message netio_msg;
    netio_msg.id = message.id;
    netio_msg.body = message.content;
    netio_msg.method = message.method;
    netio_msg.path = message.path;
    netio_msg.headers = message.headers;
    netio_msg.status_code = message.status_code;
    netio_msg.error_message = message.error_message;
    netio_msg.timestamp = message.timestamp;
    netio_msg.type = netio::MessageType::REQUEST;
    return netio_msg;
}

TransportMessage fromNetioMessage(const netio::Message& msg) {
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
}

mcp_transport::TransportHttpMessage toTransportHttpMessage(
    const netio::Message& msg) {
    mcp_transport::TransportHttpMessage out;
    out.id = msg.id;
    out.method = msg.method;
    out.path = msg.path;
    out.body = msg.body;
    out.headers = msg.headers;
    out.status_code = msg.status_code;
    out.error_message = msg.error_message;
    return out;
}

netio::Message toNetioHttpMessage(
    const mcp_transport::TransportHttpMessage& msg) {
    netio::Message out;
    out.id = msg.id;
    out.method = msg.method;
    out.path = msg.path;
    out.body = msg.body;
    out.headers = msg.headers;
    out.status_code = msg.status_code;
    out.error_message = msg.error_message;
    out.type = netio::MessageType::RESPONSE;
    out.timestamp = std::chrono::system_clock::now();
    return out;
}

netio::Protocol toNetioProtocol(mcp_transport::TransportProtocol protocol) {
    switch (protocol) {
        case mcp_transport::TransportProtocol::kHttp:
            return netio::Protocol::HTTP;
        case mcp_transport::TransportProtocol::kHttps:
            return netio::Protocol::HTTPS;
        case mcp_transport::TransportProtocol::kWebSocket:
            return netio::Protocol::WEBSOCKET;
        default:
            return netio::Protocol::HTTP;
    }
}

void copyConnectionInfoFromNetio(const netio::ConnectionInfo& src,
                                 mcp_transport::ConnectionInfo& dst) {
    dst.connection_id = src.connection_id;
    dst.remote_address = src.remote_address;
    dst.remote_port = src.remote_port;
    dst.local_address = src.local_address;
    dst.local_port = src.local_port;
    dst.connected_at = src.connected_at;
    dst.last_activity = src.last_activity;
    dst.session_id = src.session_id;
}

mcp::LogLevel toPipeLogLevel(TransportLogLevel level) {
    switch (level) {
        case TransportLogLevel::kDebug:
            return mcp::LogLevel::DEBUG;
        case TransportLogLevel::kInfo:
            return mcp::LogLevel::INFO;
        case TransportLogLevel::kWarning:
            return mcp::LogLevel::WARNING;
        case TransportLogLevel::kError:
            return mcp::LogLevel::ERROR;
        default:
            return mcp::LogLevel::INFO;
    }
}

TransportLogLevel toTransportLogLevel(int value) {
    switch (value) {
        case 0:
            return TransportLogLevel::kDebug;
        case 1:
            return TransportLogLevel::kInfo;
        case 2:
            return TransportLogLevel::kWarning;
        case 3:
            return TransportLogLevel::kError;
        default:
            return TransportLogLevel::kInfo;
    }
}

TransportProtocol toTransportProtocol(int value) {
    switch (value) {
        case 0:
            return TransportProtocol::kHttp;
        case 1:
            return TransportProtocol::kHttps;
        case 2:
            return TransportProtocol::kWebSocket;
        default:
            return TransportProtocol::kHttp;
    }
}

}  // namespace

// TransportMessage implementation
nlohmann::json TransportMessage::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["content"] = content;
    j["method"] = method;
    j["path"] = path;
    j["headers"] = headers;
    j["status_code"] = status_code;
    j["error_message"] = error_message;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                         timestamp.time_since_epoch())
                         .count();
    return j;
}

TransportMessage TransportMessage::fromJson(const nlohmann::json& j) {
    TransportMessage msg;
    msg.id = j.value("id", "");
    msg.content = j.value("content", "");
    msg.method = j.value("method", "POST");
    msg.path = j.value("path", "/");
    msg.status_code = j.value("status_code", 200);
    msg.error_message = j.value("error_message", "");

    if (j.contains("headers") && j["headers"].is_object()) {
        for (auto& [key, value] : j["headers"].items()) {
            msg.headers[key] = value.get<std::string>();
        }
    }

    if (j.contains("timestamp")) {
        auto timestamp_ms = j["timestamp"].get<uint64_t>();
        msg.timestamp = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(timestamp_ms));
    } else {
        msg.timestamp = std::chrono::system_clock::now();
    }

    return msg;
}

// ConnectionInfo implementation
nlohmann::json ConnectionInfo::toJson() const {
    nlohmann::json j;
    j["connection_id"] = connection_id;
    j["remote_address"] = remote_address;
    j["remote_port"] = remote_port;
    j["local_address"] = local_address;
    j["local_port"] = local_port;
    j["status"] = static_cast<int>(status);
    j["type"] = static_cast<int>(type);
    j["connected_at"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                            connected_at.time_since_epoch())
                            .count();
    j["last_activity"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                             last_activity.time_since_epoch())
                             .count();
    j["session_id"] = session_id;
    return j;
}

ConnectionInfo ConnectionInfo::fromJson(const nlohmann::json& j) {
    ConnectionInfo info;

    info.connection_id = j.value("connection_id", "");
    info.remote_address = j.value("remote_address", "");
    info.remote_port = j.value("remote_port", 0);
    info.local_address = j.value("local_address", "");
    info.local_port = j.value("local_port", 0);
    info.session_id = j.value("session_id", "");

    if (j.contains("status")) {
        info.status = static_cast<TransportStatus>(j["status"].get<int>());
    }
    if (j.contains("type")) {
        info.type = static_cast<TransportType>(j["type"].get<int>());
    }
    if (j.contains("connected_at")) {
        const auto connected_at_ms = j["connected_at"].get<uint64_t>();
        info.connected_at = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(connected_at_ms));
    }
    if (j.contains("last_activity")) {
        const auto last_activity_ms = j["last_activity"].get<uint64_t>();
        info.last_activity = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(last_activity_ms));
    }

    return info;
}

// TransportConfig implementation
TransportConfig TransportConfig::fromJsonFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw TransportError("Cannot open config file: " + file_path);
        }

        nlohmann::json j;
        file >> j;
        return fromJson(j);
    } catch (const std::exception& e) {
        throw TransportError("Failed to load config from file: " +
                             std::string(e.what()));
    }
}

TransportConfig TransportConfig::fromJson(const nlohmann::json& j) {
    TransportConfig config;

    // Basic config
    if (j.contains("type")) {
        config.type = static_cast<TransportType>(j["type"].get<int>());
    }
    if (j.contains("mode")) {
        config.mode = static_cast<TransportMode>(j["mode"].get<int>());
    }
    if (j.contains("name")) {
        config.name = j["name"].get<std::string>();
    }
    if (j.contains("auto_reconnect")) {
        config.auto_reconnect = j["auto_reconnect"].get<bool>();
    }
    if (j.contains("reconnect_interval")) {
        config.reconnect_interval = j["reconnect_interval"].get<int>();
    }
    if (j.contains("max_reconnect_attempts")) {
        config.max_reconnect_attempts = j["max_reconnect_attempts"].get<int>();
    }
    if (j.contains("max_reconnect_interval")) {
        config.max_reconnect_interval =
            j["max_reconnect_interval"].get<int>();
    }

    // IPC config
    if (j.contains("ipc")) {
        const auto& ipc_config = j["ipc"];
        if (ipc_config.contains("command")) {
            config.ipc.command = ipc_config["command"].get<std::string>();
        }
        if (ipc_config.contains("args") && ipc_config["args"].is_array()) {
            for (const auto& arg : ipc_config["args"]) {
                config.ipc.args.push_back(arg.get<std::string>());
            }
        }
        if (ipc_config.contains("env_vars") &&
            ipc_config["env_vars"].is_object()) {
            for (auto& [key, value] : ipc_config["env_vars"].items()) {
                config.ipc.env_vars[key] = value.get<std::string>();
            }
        }
        if (ipc_config.contains("buffer_size")) {
            config.ipc.buffer_size = ipc_config["buffer_size"].get<int>();
        }
        if (ipc_config.contains("read_timeout_ms")) {
            config.ipc.read_timeout_ms =
                ipc_config["read_timeout_ms"].get<int>();
        }
        if (ipc_config.contains("max_retry_count")) {
            config.ipc.max_retry_count =
                ipc_config["max_retry_count"].get<int>();
        }
        if (ipc_config.contains("respawn_on_disconnect")) {
            config.ipc.respawn_on_disconnect =
                ipc_config["respawn_on_disconnect"].get<bool>();
        }
        if (ipc_config.contains("enableLogging")) {
            config.ipc.enable_logging = ipc_config["enableLogging"].get<bool>();
        }
        if (ipc_config.contains("log_file_path")) {
            config.ipc.log_file_path =
                ipc_config["log_file_path"].get<std::string>();
        }
        if (ipc_config.contains("log_level")) {
            config.ipc.log_level =
                toTransportLogLevel(ipc_config["log_level"].get<int>());
        }
    }

    // Server config
    if (j.contains("server")) {
        const auto& server_config = j["server"];
        if (server_config.contains("server_name")) {
            config.server.server_name =
                server_config["server_name"].get<std::string>();
        }
        if (server_config.contains("server_version")) {
            config.server.server_version =
                server_config["server_version"].get<std::string>();
        }
        if (server_config.contains("description")) {
            config.server.description =
                server_config["description"].get<std::string>();
        }
    }

    // SSE config
    if (j.contains("sse")) {
        const auto& sse_config = j["sse"];
        if (sse_config.contains("host")) {
            config.sse.host = sse_config["host"].get<std::string>();
        }
        if (sse_config.contains("port")) {
            config.sse.port = sse_config["port"].get<int>();
        }
        if (sse_config.contains("protocol")) {
            config.sse.protocol =
                toTransportProtocol(sse_config["protocol"].get<int>());
        }
        if (sse_config.contains("streamable_http")) {
            config.sse.streamable_http =
                sse_config["streamable_http"].get<bool>();
        }
        if (sse_config.contains("timeout_seconds")) {
            config.sse.timeout_seconds =
                sse_config["timeout_seconds"].get<int>();
        }
        if (sse_config.contains("max_retries")) {
            config.sse.max_retries = sse_config["max_retries"].get<int>();
        }
        if (sse_config.contains("headers") &&
            sse_config["headers"].is_object()) {
            for (auto& [key, value] : sse_config["headers"].items()) {
                config.sse.headers[key] = value.get<std::string>();
            }
        }
        if (sse_config.contains("user_agent")) {
            config.sse.user_agent = sse_config["user_agent"].get<std::string>();
        }
        if (sse_config.contains("keep_alive")) {
            config.sse.keep_alive = sse_config["keep_alive"].get<bool>();
        }
        if (!j.contains("auto_reconnect") &&
            sse_config.contains("auto_reconnect")) {
            config.auto_reconnect =
                sse_config["auto_reconnect"].get<bool>();
        }
        if (!j.contains("reconnect_interval") &&
            sse_config.contains("reconnect_interval")) {
            config.reconnect_interval =
                sse_config["reconnect_interval"].get<int>();
        }
        if (!j.contains("max_reconnect_attempts") &&
            sse_config.contains("max_reconnect_attempts")) {
            config.max_reconnect_attempts =
                sse_config["max_reconnect_attempts"].get<int>();
        }
        if (!j.contains("max_reconnect_interval") &&
            sse_config.contains("max_reconnect_interval")) {
            config.max_reconnect_interval =
                sse_config["max_reconnect_interval"].get<int>();
        }
        if (sse_config.contains("keepalive_interval")) {
            config.sse.keepalive_interval =
                sse_config["keepalive_interval"].get<int>();
        }
        if (sse_config.contains("endpoint")) {
            config.sse.endpoint = sse_config["endpoint"].get<std::string>();
        }
    }

    // Common config
    if (j.contains("enableLogging")) {
        config.enable_logging = j["enableLogging"].get<bool>();
    }
    if (j.contains("log_file_path")) {
        config.log_file_path = j["log_file_path"].get<std::string>();
    }
    if (j.contains("log_level")) {
        config.log_level = j["log_level"].get<int>();
    }

    return config;
}

nlohmann::json TransportConfig::toJson() const {
    nlohmann::json j;
    j["type"] = static_cast<int>(type);
    j["mode"] = static_cast<int>(mode);
    j["name"] = name;
    j["auto_reconnect"] = auto_reconnect;
    j["reconnect_interval"] = reconnect_interval;
    j["max_reconnect_attempts"] = max_reconnect_attempts;
    j["max_reconnect_interval"] = max_reconnect_interval;

    // IPC config
    nlohmann::json ipc_config;
    ipc_config["command"] = ipc.command;
    ipc_config["args"] = ipc.args;
    ipc_config["env_vars"] = ipc.env_vars;
    ipc_config["buffer_size"] = ipc.buffer_size;
    ipc_config["read_timeout_ms"] = ipc.read_timeout_ms;
    ipc_config["max_retry_count"] = ipc.max_retry_count;
    ipc_config["respawn_on_disconnect"] = ipc.respawn_on_disconnect;
    ipc_config["enableLogging"] = ipc.enable_logging;
    ipc_config["log_file_path"] = ipc.log_file_path;
    ipc_config["log_level"] = static_cast<int>(ipc.log_level);
    j["ipc"] = ipc_config;

    // Server config
    nlohmann::json server_config;
    server_config["server_name"] = server.server_name;
    server_config["server_version"] = server.server_version;
    server_config["description"] = server.description;
    j["server"] = server_config;

    // SSE config
    nlohmann::json sse_config;
    sse_config["host"] = sse.host;
    sse_config["port"] = sse.port;
    sse_config["protocol"] = static_cast<int>(sse.protocol);
    sse_config["streamable_http"] = sse.streamable_http;
    sse_config["timeout_seconds"] = sse.timeout_seconds;
    sse_config["max_retries"] = sse.max_retries;
    sse_config["headers"] = sse.headers;
    sse_config["user_agent"] = sse.user_agent;
    sse_config["keep_alive"] = sse.keep_alive;
    sse_config["keepalive_interval"] = sse.keepalive_interval;
    sse_config["endpoint"] = sse.endpoint;
    j["sse"] = sse_config;

    // Common config
    j["enableLogging"] = enable_logging;
    j["log_file_path"] = log_file_path;
    j["log_level"] = log_level;

    return j;
}

bool TransportConfig::validate() const {
    if (type == TransportType::kIpc) {
        return !ipc.command.empty() && ipc.buffer_size > 0 &&
               ipc.read_timeout_ms > 0;
    } else if (type == TransportType::kSse) {
        if (sse.host.empty() || sse.port <= 0 || sse.port > 65535 ||
            sse.timeout_seconds <= 0 || sse.endpoint.empty()) {
            return false;
        }
        // Current SDK does not provide TLS certificate/key configuration for
        // server-side listen. Reject unsupported server HTTPS config early.
        if (mode == TransportMode::kServer &&
            sse.protocol == TransportProtocol::kHttps) {
            return false;
        }
        // Protocol must be one of the defined values
        int proto = static_cast<int>(sse.protocol);
        if (proto < static_cast<int>(TransportProtocol::kHttp) ||
            proto > static_cast<int>(TransportProtocol::kWebSocket)) {
            return false;
        }
        return true;
    } else if (type == TransportType::kAuto) {
        return true;  // Auto selection; config validation happens at runtime
    }
    return false;
}

// Transport internal implementation
class Transport::Impl {
public:
    // TransportMessageHandler for IPC message handling
    class TransportMessageHandler : public mcp::MessageHandler {
    public:
        explicit TransportMessageHandler(Impl* impl) : impl_(impl) {}

        void onMessage(const mcp::Message& message) override {
            if (impl_ && impl_->message_callback_) {
                TransportMessage transport_msg = fromIpcMessage(message);
                impl_->message_callback_(transport_msg);
            }
        }

        void onError(const std::string& error) override {
            if (impl_ && impl_->error_callback_) {
                impl_->error_callback_(error);
            }
        }

    private:
        Impl* impl_;
    };

    // Adapter for IPC TransportServerRequestHandler
    class TransportServerRequestHandlerAdapter
        : public mcp::ServerRequestHandler {
    public:
        explicit TransportServerRequestHandlerAdapter(Impl* impl)
            : impl_(impl) {}

        std::string handleRequest(const std::string& method,
                                  const std::string& params,
                                  const std::string& request_id) override {
            if (impl_ && impl_->server_request_handler_) {
                return impl_->server_request_handler_->handleRequest(
                    method, params, request_id);
            }
            return "{\"error\": \"No server request handler available\"}";
        }

        void handleNotification(const std::string& method,
                                const std::string& params) override {
            if (impl_ && impl_->server_request_handler_) {
                impl_->server_request_handler_->handleNotification(method,
                                                                   params);
            }
        }

    private:
        Impl* impl_;
    };

    // Adapter for SSE TransportServerRequestHandler
    class SseServerRequestHandlerAdapter : public netio::ServerRequestHandler {
    public:
        explicit SseServerRequestHandlerAdapter(Impl* impl) : impl_(impl) {}
        static constexpr const char* kDefaultProtocolVersion = "2025-03-26";
        static constexpr const char* kMcpProtocolVersionHeader =
            "MCP-Protocol-Version";
        static constexpr const char* kMcpProtocolVersionHeaderCamel =
            "Mcp-Protocol-Version";
        static constexpr const char* kMcpProtocolVersionHeaderLower =
            "mcp-protocol-version";
        static constexpr const char* kMcpSessionIdHeader = "Mcp-Session-Id";
        static constexpr const char* kMcpSessionIdHeaderLower =
            "mcp-session-id";
        static constexpr const char* kXSessionIdHeader = "X-Session-Id";

        // Expose interface for upper layer
        bool handleRequest(const netio::Message& request,
                           netio::Message& response) override {
            response.type = netio::MessageType::RESPONSE;
            response.id = request.id;
            response.timestamp = std::chrono::system_clock::now();
            response.headers["Content-Type"] = "application/json";

            auto logger = mcp_logger::getModuleLogger("transport");

            if (!impl_) {
                logger->error("SSE server request handler not configured");
                response.status_code = 500;
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32601,"message":"Server handler not available"}})";
                return true;
            }

            std::shared_ptr<TransportServerRequestHandler> request_handler;
            std::shared_ptr<SseServerRequestHandler> sse_request_handler;
            {
                std::lock_guard<std::mutex> lock(impl_->mutex_);
                request_handler = impl_->server_request_handler_;
                sse_request_handler = impl_->sse_server_request_handler_;
            }

            if (!request_handler && !sse_request_handler) {
                logger->error("SSE server request handler not configured");
                response.status_code = 500;
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32601,"message":"Server handler not available"}})";
                return true;
            }

            nlohmann::json request_json;
            try {
                request_json = nlohmann::json::parse(request.body);
                const std::size_t preview_len =
                    std::min<std::size_t>(request.body.size(), 512);
                logger->info("SSE server received request body (preview): " +
                             request.body.substr(0, preview_len) +
                             (request.body.size() > preview_len
                                  ? "...(truncated)"
                                  : ""));
            } catch (const std::exception& e) {
                logger->error("Invalid JSON in SSE server request: " +
                              std::string(e.what()));
                response.status_code = 400;
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32700,"message":"Parse error"}})";
                return true;
            }

            // Check whether this is a server-request response (has
            // result/error, no method)
            bool is_jsonrpc_response = !request_json.contains("method") &&
                                       (request_json.contains("result") ||
                                        request_json.contains("error")) &&
                                       request_json.contains("id");
            if (is_jsonrpc_response) {
                std::string resp_id;
                if (request_json["id"].is_string()) {
                    resp_id = request_json["id"].get<std::string>();
                } else {
                    resp_id = request_json["id"].dump();
                }
                if (impl_->fulfillPendingRequest(resp_id, request.body)) {
                    logger->info("Fulfilled pending SSE request: " + resp_id);
                    response.status_code = 202;
                    response.body.clear();
                    response.headers.erase("Content-Type");
                    return false;
                }
                // No matching pending request; handle as normal request
            }

            std::string method = request_json.value("method", "");
            logger->info("SSE server request method parsed: " + method);
            if (method.empty()) {
                logger->error("SSE server request missing method field");
                response.status_code = 400;
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Invalid request: missing method"}})";
                return true;
            }

            std::string protocol_version;
            auto protocol_header_it =
                request.headers.find(kMcpProtocolVersionHeader);
            if (protocol_header_it != request.headers.end()) {
                protocol_version = protocol_header_it->second;
            }
            if (protocol_version.empty() &&
                method == mcp::methods::INITIALIZE &&
                request_json.contains("params") &&
                request_json["params"].is_object()) {
                protocol_version =
                    request_json["params"].value("protocolVersion", "");
            }
            if (protocol_version.empty()) {
                // Compatibility fallback for clients that do not send the
                // header yet.
                protocol_version = kDefaultProtocolVersion;
            }
            const bool protocol_supported =
                mcp::utils::isValidProtocolVersion(protocol_version);
            if (!protocol_supported) {
                if (method == mcp::methods::INITIALIZE) {
                    logger->warning(
                        "Initialize request uses unsupported protocol version: " +
                        protocol_version +
                        ", forwarding to server handler for negotiation");
                    protocol_version = mcp::PROTOCOL_VERSION;
                } else {
                    logger->error("Unsupported MCP protocol version: " +
                                  protocol_version);
                    response.status_code = 400;
                    response.body =
                        R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Unsupported MCP protocol version"}})";
                    return true;
                }
            }
            if (!protocol_version.empty()) {
                response.headers[kMcpProtocolVersionHeader] = protocol_version;
                response.headers[kMcpProtocolVersionHeaderCamel] =
                    protocol_version;
                response.headers[kMcpProtocolVersionHeaderLower] =
                    protocol_version;
            }

            std::string session_id;
            auto session_header_it = request.headers.find(kMcpSessionIdHeader);
            if (session_header_it != request.headers.end()) {
                session_id = session_header_it->second;
            }
            if (session_id.empty()) {
                session_header_it = request.headers.find("mcp-session-id");
                if (session_header_it != request.headers.end()) {
                    session_id = session_header_it->second;
                }
            }
            if (session_id.empty()) {
                session_header_it = request.headers.find(kXSessionIdHeader);
                if (session_header_it != request.headers.end()) {
                    session_id = session_header_it->second;
                }
            }
            if (session_id.empty()) {
                session_header_it = request.headers.find("x-session-id");
                if (session_header_it != request.headers.end()) {
                    session_id = session_header_it->second;
                }
            }
            if (method == mcp::methods::INITIALIZE) {
                const std::string hdr_mcp =
                    request.headers.count(kMcpSessionIdHeader)
                        ? request.headers.at(kMcpSessionIdHeader)
                        : "";
                const std::string hdr_mcp_lower =
                    request.headers.count("mcp-session-id")
                        ? request.headers.at("mcp-session-id")
                        : "";
                const std::string hdr_x =
                    request.headers.count(kXSessionIdHeader)
                        ? request.headers.at(kXSessionIdHeader)
                        : "";
                const std::string hdr_x_lower =
                    request.headers.count("x-session-id")
                        ? request.headers.at("x-session-id")
                        : "";
                logger->info(
                    "Initialize request session headers: Mcp-Session-Id=" +
                    hdr_mcp + ", mcp-session-id=" + hdr_mcp_lower +
                    ", X-Session-Id=" + hdr_x +
                    ", x-session-id=" + hdr_x_lower);
            }
            if (session_id.empty() && impl_) {
                std::lock_guard<std::mutex> lock(impl_->mutex_);
                session_id = impl_->connection_info_.session_id;
            }
            if (session_id.empty() && method == mcp::methods::INITIALIZE &&
                impl_) {
                try {
                    std::string existing_session_id;
                    {
                        std::lock_guard<std::mutex> lock(impl_->mutex_);
                        existing_session_id =
                            impl_->connection_info_.session_id;
                    }
                    if (!existing_session_id.empty()) {
                        session_id = existing_session_id;
                    }
                    if (session_id.empty()) {
                        session_id = impl_->createSessionForSseRequest(
                            toTransportHttpMessage(request));
                    }
                    {
                        std::lock_guard<std::mutex> lock(impl_->mutex_);
                        impl_->connection_info_.session_id = session_id;
                        impl_->connection_info_.status =
                            TransportStatus::kConnected;
                        impl_->connection_info_.type = TransportType::kSse;
                        impl_->connection_info_.last_activity =
                            response.timestamp;
                    }
                } catch (const std::exception& e) {
                    logger->error(
                        "Failed to create session for initialize request: " +
                        std::string(e.what()));
                }
            }
            if (!session_id.empty()) {
                response.headers[kMcpSessionIdHeader] = session_id;
                response.headers[kMcpSessionIdHeaderLower] = session_id;
                response.headers[kXSessionIdHeader] = session_id;
            }

            std::string params = "{}";
            if (request_json.contains("params")) {
                params = request_json["params"].dump();
            }

            bool has_id =
                request_json.contains("id") && !request_json["id"].is_null();
            std::string request_id;
            if (has_id) {
                if (request_json["id"].is_string()) {
                    request_id = request_json["id"].get<std::string>();
                } else if (request_json["id"].is_number_integer() ||
                           request_json["id"].is_number_unsigned()) {
                    request_id =
                        std::to_string(request_json["id"].get<long long>());
                } else {
                    request_id = request_json["id"].dump();
                }
            }

            /*
            std::string session_id;
            try {
                session_id = impl_->createSessionForSseRequest(request,
            request_json, request_id, has_id); if (!session_id.empty()) { if
            (has_id) { request_json["id"] = session_id; request_id = session_id;
                    }
                    response.headers[kSessionIdHeader] = session_id;
                    logger->info("Assigned session id " + session_id + " to SSE
            request: " + method);
                }
            } catch (const std::exception& e) {
                logger->error("Failed to create session for SSE request: " +
            std::string(e.what()));
            }*/

            if (!has_id) {
                logger->info("Handling SSE server notification: " + method);
                try {
                    if (request_handler) {
                        request_handler->handleNotification(method, params);
                    }
                } catch (const std::exception& e) {
                    logger->error(
                        "SSE server notification handler threw exception: " +
                        std::string(e.what()));
                }
                response.body.clear();
                response.headers.erase("Content-Type");
                response.status_code = 202;
                return true;
            }

            logger->info("Handling SSE server request: " + method +
                         " (ID: " + request_id + ")");

            try {
                if (sse_request_handler) {
                    TransportHttpMessage transport_request =
                        toTransportHttpMessage(request);
                    TransportHttpMessage transport_response =
                        toTransportHttpMessage(response);
                    if (!sse_request_handler->handleRequest(transport_request,
                                                            transport_response)) {
                        response.status_code = 500;
                        response.body =
                            R"({"jsonrpc":"2.0","error":{"code":-32603,"message":"SSE server request handler returned false"}})";
                        return true;
                    }
                    response = toNetioHttpMessage(transport_response);
                    response.id = request.id;
                    if (response.body.empty()) {
                        response.body = "{}";
                    }
                } else {
                    std::string result_payload =
                        request_handler->handleRequest(method, params, request_id);
                    response.status_code = 200;
                    fillJsonRpcResponseBody(result_payload, request_json["id"],
                                            response.body);
                }
                if (method == mcp::methods::INITIALIZE) {
                    std::string session_header =
                        response.headers.count(kMcpSessionIdHeader)
                            ? response.headers[kMcpSessionIdHeader]
                            : "";
                    std::string protocol_header =
                        response.headers.count(kMcpProtocolVersionHeader)
                            ? response.headers[kMcpProtocolVersionHeader]
                            : "";
                    logger->info(
                        "SSE initialize response headers: "
                        "MCP-Protocol-Version=" +
                        protocol_header + ", Mcp-Session-Id=" + session_header);
                    logger->info("SSE initialize response body: " +
                                 response.body);
                }
                return true;
            } catch (const mcp::mcp_exception& e) {
                logger->error(
                    "SSE server request handler returned MCP error: " +
                    std::string(e.what()));
                nlohmann::json error_json;
                error_json["jsonrpc"] = "2.0";
                error_json["id"] = request_json["id"];
                error_json["error"] = {{"code", static_cast<int>(e.code())},
                                       {"message", std::string(e.what())}};
                response.status_code = 200;
                response.body = error_json.dump();
                return true;
            } catch (const std::exception& e) {
                logger->error("SSE server request handler threw exception: " +
                              std::string(e.what()));
                nlohmann::json error_json;
                error_json["jsonrpc"] = "2.0";
                error_json["id"] = request_json["id"];
                error_json["error"] = {
                    {"code", -32603},
                    {"message", std::string("Internal error: ") + e.what()}};
                response.status_code = 500;
                response.body = error_json.dump();
                return true;
            }
        }

    private:
        Impl* impl_;
    };

    // SSE connection request handler adapter
    class SseServerConnectHandlerAdapter
        : public netio::ConnectionRequestHandler {
    public:
        explicit SseServerConnectHandlerAdapter(Impl* impl) : impl_(impl) {}
        static constexpr const char* kSessionIdHeader = "X-Session-Id";
        static constexpr const char* kMcpSessionIdHeader = "Mcp-Session-Id";
        bool handleRequest(const netio::Message& request,
                           netio::Message& response) override {
            auto logger = mcp_logger::getModuleLogger("transport");

            response = netio::Message{};
            response.type = netio::MessageType::RESPONSE;
            response.id = request.id;
            response.timestamp = std::chrono::system_clock::now();

            if (!impl_) {
                logger->error(
                    "SSE connect handler adapter missing implementation "
                    "pointer");
                response.status_code = 500;
                response.headers["Content-Type"] = "application/json";
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32603,"message":"Internal server error"}})";
                return true;
            }

            std::shared_ptr<SseServerConnectHandler> handler;
            {
                std::lock_guard<std::mutex> lock(impl_->mutex_);
                handler = impl_->sse_server_connect_handler_;
            }

            if (!handler) {
                logger->warning(
                    "SSE server connect handler not configured; accepting "
                    "connection with empty response");
                response.status_code = 204;
                response.body.clear();
                return false;
            }

            try {
                // Handle outer core logic first
                TransportHttpMessage transport_request =
                    toTransportHttpMessage(request);
                TransportHttpMessage transport_response =
                    toTransportHttpMessage(response);
                bool result = handler->handleRequest(transport_request,
                                                     transport_response);
                response = toNetioHttpMessage(transport_response);
                response.id = request.id;

                // Attach session id
                std::string session_id;
                try {
                    session_id =
                        impl_->createSessionForSseRequest(transport_request);
                } catch (const std::exception& e) {
                    logger->error("Failed to create session for SSE request: " +
                                  std::string(e.what()));
                }

                mcp_logger::getModuleLogger("transport")
                    ->debug("SSE server connect handler: session_id: " +
                            session_id);
                response.headers[kSessionIdHeader] = session_id;
                response.headers[kMcpSessionIdHeader] = session_id;
                // Add new session_id to SessionManager
                try {
                    // Create attributes; enrich with request data if needed
                    std::unordered_map<std::string, std::string> attributes;
                    if (!transport_request.method.empty())
                        attributes["method"] = transport_request.method;
                    if (!transport_request.path.empty())
                        attributes["path"] = transport_request.path;
                    if (!transport_request.id.empty())
                        attributes["message_id"] = transport_request.id;
                    attributes["created_at"] = impl_->getTimestamp();

                    // Create Session object and add it to SessionManager
                    mcp::Session session;
                    session.id = session_id;
                    session.created_at = std::chrono::steady_clock::now();
                    session.expires_at =
                        session.created_at +
                        impl_->session_manager_.GetSessionTtl();
                    session.attributes = attributes;
                    impl_->session_manager_.AddSession(session_id, session);
                } catch (const std::exception& e) {
                    mcp_logger::getModuleLogger("transport")
                        ->error("Failed to add session to sessionmanager: " +
                                std::string(e.what()));
                }

                // Invoke upper-layer connection callback with connection info
                {
                    std::lock_guard<std::mutex> lock(impl_->mutex_);
                    if (impl_->connection_callback_) {
                        // Update impl_->connection_info_ with allocated
                        // session_id
                        impl_->connection_info_.session_id = session_id;
                        impl_->connection_info_.type = TransportType::kSse;
                        impl_->connection_info_.status =
                            TransportStatus::kConnected;
                        impl_->connection_info_.connected_at =
                            response.timestamp;
                        impl_->connection_info_.last_activity =
                            response.timestamp;
                        mcp_logger::getModuleLogger("transport")
                            ->info(
                                "SSE connect callback invoked with "
                                "session_id: " +
                                session_id);
                        impl_->connection_callback_(impl_->connection_info_);
                    } else {
                        mcp_logger::getModuleLogger("transport")
                            ->error(
                                "SSE connect: connection_callback_ is not "
                                "set.");
                    }
                }
                mcp_logger::getModuleLogger("transport")
                    ->debug(
                        "SSE server connect handler: "
                        "response.headers[kSessionIdHeader]: " +
                        response.headers[kSessionIdHeader]);
                return result;
            } catch (const std::exception& e) {
                logger->error("SSE server connect handler threw exception: " +
                              std::string(e.what()));
                response.status_code = 500;
                response.headers["Content-Type"] = "application/json";
                response.body =
                    R"({"jsonrpc":"2.0","error":{"code":-32000,"message":"Connect handler error"}})";
                return true;
            }
        }

    private:
        Impl* impl_;
    };

    Impl()
        : status_(TransportStatus::kDisconnected),
          type_(TransportType::kAuto),
          config_(),
          connection_id_(generateConnectionId()),
          statistics_(),
          server_request_handler_(nullptr),
          sse_server_connect_handler_(nullptr),
          sse_server_request_handler_(nullptr),
          server_mode_(false),
          logging_enabled_(true),
          log_level_(1),
          log_file_(),
          log_mutex_() {
        // Use global logger
    }

    ~Impl() { stop(); }

    bool start(const TransportConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        try {
            config_ = config;
            logging_enabled_ = config.enable_logging;
            log_level_ = config.log_level;
            server_mode_ = (config.mode == TransportMode::kServer);

            // Initialize logging
            if (logging_enabled_ && !config.log_file_path.empty()) {
                log_file_.open(config.log_file_path, std::ios::app);
            }

            // Determine transport type
            TransportType actual_type = config.type;
            if (actual_type == TransportType::kAuto) {
                // Auto-select: prefer IPC, fallback to HTTP when IPC config is
                // incomplete
                if (!config.ipc.command.empty()) {
                    actual_type = TransportType::kIpc;
                } else if (!config.sse.host.empty()) {
                    actual_type = TransportType::kSse;
                } else {
                    throw TransportError(
                        "Cannot determine transport type: both IPC and SSE "
                        "configs are incomplete");
                }
            }

            type_ = actual_type;
            status_ = TransportStatus::kConnecting;
            // log(1, "Selected transport type: " +
            // log_level_to_string(static_cast<int>(actual_type))); log(1,
            // "Starting transport with type: " +
            // std::to_string(static_cast<int>(actual_type)));
            bool success = false;
            if (actual_type == TransportType::kIpc) {
                mcp_logger::getModuleLogger("transport")
                    ->info("Starting IPC transport");
                success = startIpc();
            } else if (actual_type == TransportType::kSse) {
                mcp_logger::getModuleLogger("transport")
                    ->info("Starting SSE transport");
                success = startSse();
            }

            if (success) {
                status_ = TransportStatus::kConnected;
                connection_info_.connection_id = connection_id_;
                connection_info_.type = actual_type;
                connection_info_.status = status_;
                connection_info_.connected_at =
                    std::chrono::system_clock::now();
                connection_info_.last_activity = connection_info_.connected_at;
                connection_info_.session_id = "";
                mcp_logger::getModuleLogger("transport")
                    ->info("Transport connected successfully");
                updateStatistics("connections", 1);
                mcp_logger::getModuleLogger("transport")
                    ->info("Transport connection info: " +
                           connection_info_.toJson().dump());
                startSessionCleanupThread();
                mcp_logger::getModuleLogger("transport")
                    ->info("Transport connection callback: true");
                if (connection_callback_) {
                    connection_callback_(connection_info_);
                } else {
                    mcp_logger::getModuleLogger("transport")
                        ->error("No connection callback found");
                }

                startReconnectThreadIfNeededLocked();

                mcp_logger::getModuleLogger("transport")
                    ->info("Transport started successfully");
                return true;
            } else {
                status_ = TransportStatus::kError;
                mcp_logger::getModuleLogger("transport")
                    ->error("Failed to start transport");
                return false;
            }
        } catch (const std::exception& e) {
            status_ = TransportStatus::kError;
            mcp_logger::getModuleLogger("transport")
                ->error("Transport start error: " + std::string(e.what()));
            if (error_callback_) {
                error_callback_("Transport start error: " +
                                std::string(e.what()));
            }
            return false;
        }
    }

    bool stop() {
        stopReconnectThread();

        // 先在锁内取出 ipc_comm_/netio_client_ 并把状态置为 kDisconnected，
        // 再在锁外执行真正的 stop（销毁底层通信对象时会 join 读线程，
        // 读线程退出过程中可能回调 handleConnectionLost，而该回调会反向
        // 获取 mutex_，若持锁调用 stop 会造成死锁）。
        std::unique_ptr<mcp::PipeCommunication> ipc_to_stop;
        std::unique_ptr<netio::NetIO> netio_to_stop;
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (status_ == TransportStatus::kDisconnected) {
                return true;
            }

            if (type_ == TransportType::kIpc) {
                ipc_to_stop = std::move(ipc_comm_);
            } else if (type_ == TransportType::kSse) {
                netio_to_stop = std::move(netio_client_);
            }

            // 先翻状态，handleConnectionLost 拿到锁后会因
            // status_ != kConnected 直接返回，避免重复触发回调。
            status_ = TransportStatus::kDisconnected;
            connection_info_.status = status_;
        }

        try {
            // 锁外执行 stop —— 回调中的 handleConnectionLost 可正常获取
            // mutex_，不会死锁。
            if (ipc_to_stop) {
                ipc_to_stop->stop();
                ipc_to_stop.reset();
            }
            if (netio_to_stop) {
                if (server_mode_) {
                    netio_to_stop->stopServer();
                } else {
                    netio_to_stop->disconnect();
                }
                netio_to_stop.reset();
            }

            stopSessionCleanupThread();

            {
                // log_file_ 的其它读写都在 mutex_ 下进行，这里也用 mutex_
                // 短暂持锁；此时通信对象已 stop 完毕，不会再触发回调，
                // 持锁是安全的。
                std::lock_guard<std::mutex> lock(mutex_);
                if (log_file_.is_open()) {
                    log_file_.close();
                }
            }

            mcp_logger::getModuleLogger("transport")->info("Transport stopped");
            return true;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport")
                ->error("Error stopping transport: " + std::string(e.what()));
            return false;
        }
    }

    bool isRunning() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return status_ == TransportStatus::kConnected;
    }

    TransportStatus getStatus() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return status_;
    }

    ConnectionInfo getConnectionInfo() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_info_;
    }

    std::future<TransportMessage> sendRequest(const TransportMessage& message) {
        return std::async(std::launch::async, [this, message]() {
            return sendRequestImpl(message);
        });
    }

    std::future<TransportMessage> sendRequest(
        const std::string& content, const std::string& method,
        const std::string& path,
        const std::map<std::string, std::string>& headers) {
        TransportMessage msg =
            utils::createRequest(content, method, path, headers);
        return sendRequest(msg);
    }

    TransportMessage sendRequestSync(const TransportMessage& message) {
        return sendRequestImpl(message);
    }

    TransportMessage sendRequestSync(
        const std::string& content, const std::string& method,
        const std::string& path,
        const std::map<std::string, std::string>& headers) {
        TransportMessage msg =
            utils::createRequest(content, method, path, headers);
        return sendRequestSync(msg);
    }

    bool sendMessage(const TransportMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (status_ != TransportStatus::kConnected) {
            return false;
        }

        try {
            if (type_ == TransportType::kIpc && ipc_comm_) {
                mcp::Message ipc_msg = toIpcMessage(message);
                const bool ok = ipc_comm_->sendMessage(ipc_msg.content);
                if (ok) {
                    connection_info_.last_activity =
                        std::chrono::system_clock::now();
                    updateStatistics("requests_sent", 1);
                }
                return ok;
            }

            if (type_ == TransportType::kSse && netio_client_) {
                if (server_mode_) {
                    // SSE server mode notification push
                    nlohmann::json payload;
                    try {
                        payload = nlohmann::json::parse(message.content);
                    } catch (...) {
                        return false;
                    }
                    const bool ok = netio_client_->sendSseEvent(payload.dump());
                    if (ok) {
                        connection_info_.last_activity =
                            std::chrono::system_clock::now();
                        updateStatistics("requests_sent", 1);
                    }
                    return ok;
                }

                // SSE client mode one-way send without waiting for a response.
                netio::Message netio_msg = toNetioMessage(message);
                (void)netio_client_->sendRequest(netio_msg);
                connection_info_.last_activity =
                    std::chrono::system_clock::now();
                updateStatistics("requests_sent", 1);
                return true;
            }
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport")
                ->error("sendMessage error: " + std::string(e.what()));
        }

        return false;
    }

    bool hasMessage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type_ == TransportType::kIpc && ipc_comm_) {
            return ipc_comm_->hasMessage();
        }
        return false;  // SSE mode does not support message queue
    }

    TransportMessage getMessage() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type_ == TransportType::kIpc && ipc_comm_) {
            mcp::Message ipc_msg = ipc_comm_->getMessage();
            return fromIpcMessage(ipc_msg);
        }
        return TransportMessage();
    }

    std::vector<TransportMessage> getAllMessages() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<TransportMessage> messages;
        if (type_ == TransportType::kIpc && ipc_comm_) {
            std::vector<mcp::Message> ipc_messages =
                ipc_comm_->getAllMessages();
            for (const auto& ipc_msg : ipc_messages) {
                messages.push_back(fromIpcMessage(ipc_msg));
            }
        }
        return messages;
    }

    void setMessageCallback(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        message_callback_ = callback;
    }

    void setConnectionCallback(ConnectionCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_callback_ = callback;
    }

    void setErrorCallback(ErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_callback_ = callback;
    }

    void setConnectionLostCallback(
        std::function<void(const std::string&)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        on_connection_lost_ = std::move(callback);
    }

    // Server-mode-only method
    void setServerRequestHandler(
        std::shared_ptr<TransportServerRequestHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        server_request_handler_ = handler;
    }

    void SetSseServerConnectHandler(
        std::shared_ptr<SseServerConnectHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        sse_server_connect_handler_ = handler;
    }

    void setSseServerRequestHandler(
        std::shared_ptr<SseServerRequestHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        sse_server_request_handler_ = handler;
    }

    bool sendResponse(const std::string& request_id,
                      const std::string& result) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!server_mode_ || status_ != TransportStatus::kConnected) {
            mcp_logger::getModuleLogger("transport")
                ->error(
                    "Cannot send response: not in server mode or not "
                    "connected");
            return false;
        }

        if (type_ == TransportType::kIpc && ipc_comm_) {
            return ipc_comm_->sendResponse(request_id, result);
        } else if (type_ == TransportType::kSse && netio_client_) {
            mcp_logger::getModuleLogger("transport")
                ->warning(
                    "SSE server responses are returned synchronously; "
                    "sendResponse() call ignored");
            return false;
        }

        return false;
    }

    bool sendError(const std::string& request_id, int error_code,
                   const std::string& error_message) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!server_mode_ || status_ != TransportStatus::kConnected) {
            mcp_logger::getModuleLogger("transport")
                ->error(
                    "Cannot send error: not in server mode or not connected");
            return false;
        }

        if (type_ == TransportType::kIpc && ipc_comm_) {
            return ipc_comm_->sendError(request_id, error_code, error_message);
        } else if (type_ == TransportType::kSse && netio_client_) {
            mcp_logger::getModuleLogger("transport")
                ->warning(
                    "SSE server responses are returned synchronously; "
                    "sendError() call ignored");
            return false;
        }

        return false;
    }

    bool sendNotification(const std::string& method,
                          const std::string& params) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!server_mode_ || status_ != TransportStatus::kConnected) {
            mcp_logger::getModuleLogger("transport")
                ->error(
                    "Cannot send notification: not in server mode or not "
                    "connected");
            return false;
        }

        if (type_ == TransportType::kIpc && ipc_comm_) {
            return ipc_comm_->sendNotification(method, params);
        } else if (type_ == TransportType::kSse && netio_client_) {
            // Push JSON-RPC notification via SSE event stream
            nlohmann::json notification;
            notification["jsonrpc"] = "2.0";
            notification["method"] = method;
            try {
                notification["params"] = nlohmann::json::parse(params);
            } catch (...) {
                notification["params"] = nlohmann::json::object();
            }
            return netio_client_->sendSseEvent(notification.dump());
        }

        return false;
    }

    bool fulfillPendingRequest(const std::string& id, const std::string& body) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        auto it = pending_requests_.find(id);
        if (it != pending_requests_.end()) {
            TransportMessage response;
            response.id = id;
            response.content = body;
            response.status_code = 200;
            response.timestamp = std::chrono::system_clock::now();
            it->second.set_value(std::move(response));
            pending_requests_.erase(it);
            return true;
        }
        return false;
    }

    void updateConfig(const TransportConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
    }

    TransportConfig getConfig() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_;
    }

    nlohmann::json getStatistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return statistics_;
    }

    bool healthCheck() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (type_ == TransportType::kIpc && ipc_comm_) {
            return ipc_comm_->isRunning() && ipc_comm_->isProcessAlive();
        } else if (type_ == TransportType::kSse && netio_client_) {
            return netio_client_->isConnected() && netio_client_->healthCheck();
        }
        return false;
    }

    // Log function is deprecated; use global logger directly

    void setLogLevel(int level) {
        log_level_ = level;
        mcp_logger::getModuleLogger("transport")
            ->info("Log level changed to " + logLevelToString(level));
    }

    void appendLogToFile(int level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!log_file_.is_open()) {
            return;
        }
        log_file_ << "[" << logLevelToString(level) << "] " << message
                  << std::endl;
    }

    void setLogFile(const std::string& file_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.log_file_path = file_path;

        if (log_file_.is_open()) {
            log_file_.close();
        }

        if (file_path.empty()) {
            mcp_logger::getModuleLogger("transport")
                ->info("Transport log file path cleared");
            return;
        }

        log_file_.open(file_path, std::ios::app);
        if (!log_file_.is_open()) {
            mcp_logger::getModuleLogger("transport")
                ->error("Failed to open transport log file: " + file_path);
            return;
        }

        mcp_logger::getModuleLogger("transport")
            ->info("Transport log file path set: " + file_path);
    }

    void enableLogging(bool enable) {
        logging_enabled_ = enable;
        if (enable) {
            mcp_logger::getModuleLogger("transport")->info("Logging enabled");
        }
    }

private:
    mutable std::mutex mutex_;
    TransportStatus status_;
    TransportType type_;
    TransportConfig config_;
    std::string connection_id_;
    ConnectionInfo connection_info_;
    nlohmann::json statistics_;

    // IPC-related fields
    std::unique_ptr<mcp::PipeCommunication> ipc_comm_;

    // SSE-related fields
    std::unique_ptr<netio::NetIO> netio_client_;
    mcp::SessionManager session_manager_;
    std::thread session_cleanup_thread_;
    std::atomic<bool> session_cleanup_running_{false};
    std::condition_variable session_cleanup_cv_;
    std::mutex session_cleanup_mutex_;
    std::chrono::seconds session_cleanup_interval_{std::chrono::seconds(60)};
    std::thread reconnect_thread_;
    std::condition_variable reconnect_cv_;
    std::atomic<bool> stop_reconnect_{false};
    std::mutex status_mutex_;

    // Callback functions
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    ErrorCallback error_callback_;
    std::function<void(const std::string&)> on_connection_lost_;

    // Server-mode-related fields
    std::shared_ptr<TransportServerRequestHandler> server_request_handler_;
    std::shared_ptr<SseServerConnectHandler> sse_server_connect_handler_;
    std::shared_ptr<SseServerRequestHandler> sse_server_request_handler_;
    std::atomic<bool> server_mode_;

    // Pending responses for SSE server-to-client requests
    std::mutex pending_requests_mutex_;
    std::map<std::string, std::promise<TransportMessage>> pending_requests_;
    std::atomic<int> next_sse_request_id_{1};

    // Logging-related fields
    bool logging_enabled_;
    int log_level_;
    std::ofstream log_file_;
    mutable std::mutex log_mutex_;

    bool startIpc() {
        try {
            ipc_comm_ = std::make_unique<mcp::PipeCommunication>();
            ipc_comm_->setConnectionLostCallback(
                [this](const std::string& reason) {
                    handleConnectionLost(reason);
                });

            // Set message handler
            if (message_callback_) {
                auto handler = std::make_shared<TransportMessageHandler>(this);
                ipc_comm_->setMessageHandler(handler);
            }

            // Set server request handler
            if (server_mode_ && server_request_handler_) {
                auto server_handler =
                    std::make_shared<TransportServerRequestHandlerAdapter>(
                        this);
                ipc_comm_->setServerRequestHandler(server_handler);
            }
            // Configure IPC
            mcp::PipeConfig ipc_config;
            ipc_config.mode = server_mode_ ? mcp::CommunicationMode::SERVER
                                           : mcp::CommunicationMode::CLIENT;
            ipc_config.command = config_.ipc.command;
            ipc_config.args = config_.ipc.args;
            ipc_config.env_vars = config_.ipc.env_vars;
            ipc_config.buffer_size = config_.ipc.buffer_size;
            ipc_config.read_timeout_ms = config_.ipc.read_timeout_ms;
            ipc_config.max_retry_count = config_.ipc.max_retry_count;
            ipc_config.enable_logging = config_.ipc.enable_logging;
            ipc_config.log_file_path = config_.ipc.log_file_path;
            ipc_config.log_level = toPipeLogLevel(config_.ipc.log_level);

            // Server-mode configuration
            if (server_mode_) {
                ipc_config.server_name = config_.server.server_name;
                ipc_config.server_version = config_.server.server_version;
            }

            mcp_logger::getModuleLogger("transport")
                ->info(
                    "IPC config: command=" + ipc_config.command +
                    ", arg count=" + std::to_string(ipc_config.args.size()) +
                    ", buffer size=" + std::to_string(ipc_config.buffer_size) +
                    ", timeout=" + std::to_string(ipc_config.read_timeout_ms) +
                    "ms" + ", retry count=" +
                    std::to_string(ipc_config.max_retry_count));

            mcp_logger::getModuleLogger("transport")
                ->info("IPC configuration details:");
            mcp_logger::getModuleLogger("transport")
                ->info("  command: " + ipc_config.command);
            std::string args_str = "  args: ";
            for (const auto& arg : ipc_config.args) {
                args_str += arg + " ";
            }
            mcp_logger::getModuleLogger("transport")->info(args_str);
            mcp_logger::getModuleLogger("transport")
                ->info("  buffer size: " +
                       std::to_string(ipc_config.buffer_size));
            mcp_logger::getModuleLogger("transport")
                ->info("  timeout: " +
                       std::to_string(ipc_config.read_timeout_ms) + "ms");
            mcp_logger::getModuleLogger("transport")
                ->info("  retry count: " +
                       std::to_string(ipc_config.max_retry_count));

            mcp_logger::getModuleLogger("transport")
                ->info("Starting IPC communication...");
            if (!ipc_comm_->start(ipc_config)) {
                mcp_logger::getModuleLogger("transport")
                    ->error("Failed to start IPC communication");
                mcp_logger::getModuleLogger("transport")
                    ->error("IPC startup failed!");
                return false;
            }
            mcp_logger::getModuleLogger("transport")
                ->info("IPC startup succeeded!");

            connection_info_.remote_address = "localhost";
            connection_info_.remote_port = 0;
            connection_info_.local_address = "localhost";
            connection_info_.local_port = 0;
            connection_info_.session_id.clear();

            mcp_logger::getModuleLogger("transport")
                ->info("IPC communication started successfully");
            return true;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport")
                ->error("IPC start error: " + std::string(e.what()));
            return false;
        }
    }

    bool startSse() {
        try {
            netio_client_ = std::make_unique<netio::NetIO>();
            netio_client_->setConnectionLostCallback(
                [this](const std::string& reason) {
                    handleConnectionLost(reason);
                });

            // Set callbacks
            if (message_callback_) {
                netio_client_->setMessageCallback(
                    [this](const netio::Message& msg) {
                        TransportMessage transport_msg = fromNetioMessage(msg);
                        message_callback_(transport_msg);
                    });
            }

            // Set server request handler
            if (server_mode_ && server_request_handler_) {
                auto server_handler =
                    std::make_shared<SseServerRequestHandlerAdapter>(this);
                netio_client_->setServerRequestHandler(server_handler);
            }

            if (sse_server_connect_handler_) {
                auto server_connect_handler =
                    std::make_shared<SseServerConnectHandlerAdapter>(this);
                netio_client_->setConnectionRequestHandler(
                    server_connect_handler);
            }

            if (error_callback_) {
                netio_client_->setErrorCallback(error_callback_);
            }

            // Configure SSE
            netio::NetConfig netio_config;
            netio_config.host = config_.sse.host;
            netio_config.port = config_.sse.port;
            netio_config.protocol = toNetioProtocol(config_.sse.protocol);
            netio_config.streamable_http = config_.sse.streamable_http;
            netio_config.timeout_seconds = config_.sse.timeout_seconds;
            netio_config.max_retries = config_.sse.max_retries;
            netio_config.headers = config_.sse.headers;
            netio_config.user_agent = config_.sse.user_agent;
            netio_config.keep_alive = config_.sse.keep_alive;
            netio_config.auto_reconnect = config_.auto_reconnect;
            netio_config.reconnect_interval = config_.reconnect_interval;
            netio_config.keepalive_interval = config_.sse.keepalive_interval;
            netio_config.endpoint = config_.sse.endpoint;

            bool success = false;
            if (server_mode_) {
                if (netio_config.protocol == netio::Protocol::HTTPS) {
                    const std::string message =
                        "HTTPS server listen is not supported by current SDK. "
                        "Use TransportProtocol::kHttp for server mode.";
                    mcp_logger::getModuleLogger("transport")->error(message);
                    if (error_callback_) {
                        error_callback_(message);
                    }
                    return false;
                }
                // Server mode: start HTTP/HTTPS server
                mcp_logger::getModuleLogger("transport")
                    ->info("Starting SSE server mode: " + netio_config.host +
                           ":" + std::to_string(netio_config.port) +
                           ", protocol: " +
                           std::string(netio_config.protocol ==
                                               netio::Protocol::HTTPS
                                           ? "HTTPS"
                                           : "HTTP") +
                           ", endpoint: " + netio_config.endpoint);

                //
                success = netio_client_->startServer(netio_config);

                if (success) {
                    connection_info_.remote_address = netio_config.host;
                    connection_info_.remote_port = netio_config.port;
                    connection_info_.local_address = netio_config.host;
                    connection_info_.local_port = netio_config.port;
                    connection_info_.session_id.clear();
                    connection_info_.status = TransportStatus::kConnected;
                    connection_info_.type = TransportType::kSse;
                    mcp_logger::getModuleLogger("transport")
                        ->info("SSE server started successfully");
                    if (connection_callback_) {
                        auto logger = mcp_logger::getModuleLogger("transport");
                        logger->info("SSE service callback succeeded");
                        connection_callback_(connection_info_);
                        mcp_logger::getModuleLogger("transport")
                            ->info("SSE service callback succeeded");
                    } else {
                        mcp_logger::getModuleLogger("transport")
                            ->error("SSE service callback failed");
                    }

                } else {
                    mcp_logger::getModuleLogger("transport")
                        ->error("SSE server startup failed");
                }
            } else {
                // Client mode: connect to remote server
                mcp_logger::getModuleLogger("transport")
                    ->info("Starting SSE client mode, connecting to: " +
                           netio_config.host + ":" +
                           std::to_string(netio_config.port));
                mcp_logger::getModuleLogger("transport")
                    ->info("protocol: " +
                           std::string(netio_config.protocol ==
                                               netio::Protocol::HTTPS
                                           ? "HTTPS"
                                           : "HTTP"));

                // Outer call needs connection info and session id, then updates
                // connection_info_
                success = netio_client_->connect(netio_config);
                mcp_logger::getModuleLogger("transport")
                    ->info("SSE client connection: " +
                           (success ? std::string("success")
                                    : std::string("failure")));

                if (success) {
                    auto conn_info = netio_client_->getConnectionInfo();
                    copyConnectionInfoFromNetio(conn_info, connection_info_);
                    connection_info_.session_id.clear();
                    connection_info_.status = TransportStatus::kConnected;
                    connection_info_.type = TransportType::kSse;
                    connection_info_.connected_at =
                        std::chrono::system_clock::now();

                    // Set session id into connection_info_
                    connection_info_.session_id = conn_info.session_id;
                    connection_info_.last_activity =
                        std::chrono::system_clock::now();

                    // Callback reads connection info and session id, then
                    // updates connection_info_
                    if (connection_callback_) {
                        // This callback is for client side
                        connection_callback_(connection_info_);
                    }
                    mcp_logger::getModuleLogger("transport")
                        ->info("SSE client connected successfully");
                } else {
                    mcp_logger::getModuleLogger("transport")
                        ->error("SSE client connection failed");
                }
            }

            return success;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport")
                ->error("SSE start error: " + std::string(e.what()));
            return false;
        }
    }

    TransportMessage sendRequestImpl(const TransportMessage& message) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (status_ == TransportStatus::kReconnecting) {
            return utils::createError("reconnecting", 500);
        }
        if (status_ != TransportStatus::kConnected) {
            return utils::createError("Transport not connected", 500);
        }

        try {
            if (type_ == TransportType::kIpc && ipc_comm_) {
                mcp_logger::getModuleLogger("transport")
                    ->info("Sending IPC request: " +
                           message.content.substr(0, 100) +
                           (message.content.length() > 100 ? "..." : ""));

                mcp::Message ipc_msg = toIpcMessage(message);
                // Use synchronous wait here; disable PipeCommunication's
                // detached timeout helper to avoid lifetime races during
                // teardown.
                auto res_future = ipc_comm_->sendRequest(ipc_msg.content, 0);

                // Wait for response using IPC-specific timeout configuration.
                const int timeout_ms =
                    std::max(1, static_cast<int>(config_.ipc.read_timeout_ms));
                auto timeout = std::chrono::milliseconds(timeout_ms);
                mcp_logger::getModuleLogger("transport")
                    ->info("Waiting for IPC response, timeout: " +
                           std::to_string(timeout_ms) + " ms");
                auto status = res_future.wait_for(timeout);
                if (status != std::future_status::ready) {
                    status = res_future.wait_for(std::chrono::milliseconds(100));
                }
                if (status == std::future_status::ready) {
                    std::string response_content = res_future.get();
                    mcp_logger::getModuleLogger("transport")
                        ->info("Received IPC response: " +
                               response_content.substr(0, 100) +
                               (response_content.length() > 100 ? "..." : ""));

                    TransportMessage response;
                    response.id = message.id;
                    response.content = response_content;
                    response.status_code = 200;
                    response.timestamp = std::chrono::system_clock::now();

                    connection_info_.last_activity = response.timestamp;
                    updateStatistics("requests_sent", 1);
                    updateStatistics("responses_received", 1);

                    return response;
                } else {
                    mcp_logger::getModuleLogger("transport")
                        ->error("IPC request timed out");
                    return utils::createError("Request timeout", 408);
                }
            } else if (type_ == TransportType::kSse && netio_client_) {
                if (server_mode_) {
                    // SSE server mode: send request to client via SSE event
                    // stream
                    std::string req_id =
                        "srv_" + std::to_string(next_sse_request_id_++);

                    nlohmann::json request_json;
                    try {
                        request_json = nlohmann::json::parse(message.content);
                    } catch (...) {
                        return utils::createError(
                            "Invalid request content for SSE push", 400);
                    }
                    // Overwrite id with server-generated unique identifier
                    request_json["id"] = req_id;

                    // Create promise/future and wait for client response
                    std::promise<TransportMessage> promise;
                    auto future = promise.get_future();
                    {
                        std::lock_guard<std::mutex> plock(
                            pending_requests_mutex_);
                        pending_requests_[req_id] = std::move(promise);
                    }

                    // Release main lock before blocking wait to avoid deadlock
                    lock.unlock();

                    bool pushed =
                        netio_client_->sendSseEvent(request_json.dump());
                    if (!pushed) {
                        std::lock_guard<std::mutex> plock(
                            pending_requests_mutex_);
                        pending_requests_.erase(req_id);
                        lock.lock();
                        return utils::createError("No SSE clients connected",
                                                  503);
                    }

                    auto timeout =
                        std::chrono::seconds(config_.sse.timeout_seconds);
                    if (future.wait_for(timeout) == std::future_status::ready) {
                        auto resp = future.get();
                        lock.lock();
                        updateStatistics("requests_sent", 1);
                        updateStatistics("responses_received", 1);
                        return resp;
                    } else {
                        {
                            std::lock_guard<std::mutex> plock(
                                pending_requests_mutex_);
                            pending_requests_.erase(req_id);
                        }
                        lock.lock();
                        return utils::createError(
                            "SSE server request timed out", 408);
                    }
                } else {
                    // SSE client mode: send via HTTP client
                    netio::Message netio_msg = toNetioMessage(message);
                    netio::Message response =
                        netio_client_->sendRequestSync(netio_msg);

                    TransportMessage transport_response =
                        fromNetioMessage(response);
                    transport_response.id = message.id;

                    connection_info_.last_activity =
                        transport_response.timestamp;
                    updateStatistics("requests_sent", 1);
                    updateStatistics("responses_received", 1);

                    return transport_response;
                }
            } else {
                return utils::createError("Invalid transport type", 500);
            }
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport")
                ->error("Send request error: " + std::string(e.what()));
            return utils::createError(
                "Send request error: " + std::string(e.what()), 500);
        }
    }

    void handleConnectionLost(const std::string& reason) {
        ConnectionCallback connection_callback;
        ErrorCallback error_callback;
        std::function<void(const std::string&)> lost_callback;
        ConnectionInfo info;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (status_ != TransportStatus::kConnected) {
                return;
            }

            status_ = autoReconnectEnabledLocked() ? TransportStatus::kReconnecting
                                                   : TransportStatus::kError;
            connection_info_.status = status_;
            connection_info_.last_activity = std::chrono::system_clock::now();
            info = connection_info_;
            connection_callback = connection_callback_;
            error_callback = error_callback_;
            lost_callback = on_connection_lost_;
        }

        failPendingRequests("connection lost: " + reason);
        reconnect_cv_.notify_all();

        if (error_callback) {
            error_callback("connection lost: " + reason);
        }
        if (connection_callback) {
            connection_callback(info);
        }
        if (lost_callback) {
            lost_callback(reason);
        }
    }

    void failPendingRequests(const std::string& reason) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        for (auto& [id, promise] : pending_requests_) {
            TransportMessage response;
            response.id = id;
            response.status_code = 500;
            response.error_message = reason;
            response.content = nlohmann::json{
                {"jsonrpc", "2.0"},
                {"id", id},
                {"error", {{"code", -32603}, {"message", reason}}}
            }.dump();
            response.timestamp = std::chrono::system_clock::now();
            try {
                promise.set_value(std::move(response));
            } catch (const std::future_error&) {
            }
        }
        pending_requests_.clear();
    }

    bool autoReconnectEnabledLocked() const {
        if (server_mode_) {
            return false;
        }
        return type_ == TransportType::kIpc || type_ == TransportType::kSse
                   ? config_.auto_reconnect
                   : false;
    }

    void startReconnectThreadIfNeededLocked() {
        if (!autoReconnectEnabledLocked() || reconnect_thread_.joinable()) {
            return;
        }
        stop_reconnect_.store(false, std::memory_order_release);
        reconnect_thread_ = std::thread([this]() { reconnectLoop(); });
    }

    void stopReconnectThread() {
        stop_reconnect_.store(true, std::memory_order_release);
        reconnect_cv_.notify_all();
        if (reconnect_thread_.joinable() &&
            reconnect_thread_.get_id() != std::this_thread::get_id()) {
            reconnect_thread_.join();
        }
    }

    void reconnectLoop() {
        auto logger = mcp_logger::getModuleLogger("transport");
        logger->info("Transport reconnect thread started");

        while (!stop_reconnect_.load(std::memory_order_acquire)) {
            {
                std::unique_lock<std::mutex> status_lock(status_mutex_);
                reconnect_cv_.wait(status_lock, [this]() {
                    return stop_reconnect_.load(std::memory_order_acquire) ||
                           getStatus() == TransportStatus::kReconnecting;
                });
            }

            if (stop_reconnect_.load(std::memory_order_acquire)) {
                break;
            }

            const int max_attempts =
                std::max(1, config_.max_reconnect_attempts);
            bool connected = false;
            std::string failure_reason = "reconnect budget exhausted";

            for (int attempt = 1; attempt <= max_attempts &&
                                  !stop_reconnect_.load(std::memory_order_acquire);
                 ++attempt) {
                const int delay_seconds = reconnectDelaySeconds(attempt);
                logger->info("Reconnect attempt " + std::to_string(attempt) +
                             "/" + std::to_string(max_attempts) +
                             " after " + std::to_string(delay_seconds) + "s");
                if (waitReconnectDelay(delay_seconds)) {
                    break;
                }
                if (reconnectTransportOnce(failure_reason)) {
                    connected = true;
                    break;
                }
            }

            if (stop_reconnect_.load(std::memory_order_acquire)) {
                break;
            }

            ConnectionCallback connection_callback;
            ErrorCallback error_callback;
            ConnectionInfo info;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                status_ = connected ? TransportStatus::kConnected
                                    : TransportStatus::kError;
                connection_info_.status = status_;
                connection_info_.last_activity =
                    std::chrono::system_clock::now();
                info = connection_info_;
                connection_callback = connection_callback_;
                error_callback = error_callback_;
            }

            if (connected) {
                if (connection_callback) {
                    connection_callback(info);
                }
            } else if (error_callback) {
                error_callback(failure_reason);
            }
        }

        logger->info("Transport reconnect thread stopped");
    }

    int reconnectDelaySeconds(int attempt) const {
        const int base = std::max(1, config_.reconnect_interval);
        const int cap = std::max(base, config_.max_reconnect_interval);
        thread_local std::mt19937 gen{std::random_device{}()};
        std::uniform_int_distribution<int> jitter(0, base);
        return std::min(base * attempt + jitter(gen), cap);
    }

    bool waitReconnectDelay(int seconds) {
        std::unique_lock<std::mutex> lock(status_mutex_);
        return reconnect_cv_.wait_for(lock, std::chrono::seconds(seconds),
                                      [this]() {
                                          return stop_reconnect_.load(
                                              std::memory_order_acquire);
                                      });
    }

    bool reconnectTransportOnce(std::string& failure_reason) {
        TransportType type;
        bool ipc_respawn = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            type = type_;
            ipc_respawn = config_.ipc.respawn_on_disconnect;
        }

        if (type == TransportType::kIpc && !ipc_respawn) {
            failure_reason =
                "IPC respawn disabled; set ipc.respawn_on_disconnect=true";
            return false;
        }

        std::unique_ptr<mcp::PipeCommunication> old_ipc;
        std::unique_ptr<netio::NetIO> old_netio;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            old_ipc = std::move(ipc_comm_);
            old_netio = std::move(netio_client_);
        }
        if (old_ipc) {
            old_ipc->stop();
        }
        if (old_netio) {
            old_netio->disconnect();
        }

        std::lock_guard<std::mutex> lock(mutex_);
        const bool success =
            type == TransportType::kIpc ? startIpc() : startSse();
        if (!success) {
            failure_reason = "reconnect attempt failed";
        }
        return success;
    }

    std::string generateConnectionId() {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);

        return "transport_" + std::to_string(timestamp) + "_" +
               std::to_string(dis(gen));
    }

    void updateStatistics(const std::string& key, int increment) {
        if (statistics_.contains(key)) {
            statistics_[key] = statistics_[key].get<int>() + increment;
        } else {
            statistics_[key] = increment;
        }
        statistics_["last_updated"] =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
    }

    void bindConnectionCallbackLocked() {
        if (!netio_client_) {
            return;
        }

        if (!connection_callback_) {
            netio_client_->setConnectionCallback(nullptr);
            return;
        }

        netio_client_->setConnectionCallback(
            [this](const netio::ConnectionInfo& info) {
                ConnectionCallback callback_copy;
                ConnectionInfo transport_info;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    copyConnectionInfoFromNetio(info, connection_info_);
                    connection_info_.status = static_cast<TransportStatus>(
                        static_cast<int>(info.status));
                    connection_info_.type = type_;
                    callback_copy = connection_callback_;
                    transport_info = connection_info_;
                }

                if (callback_copy) {
                    callback_copy(transport_info);
                }
            });
    }

    std::string getTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

        return ss.str();
    }

    std::string logLevelToString(int level) const {
        switch (level) {
            case 0:
                return "DEBUG";
            case 1:
                return "INFO";
            case 2:
                return "WARNING";
            case 3:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    std::string createSessionForSseRequest(
        const TransportHttpMessage& request) {
        auto logger = mcp_logger::getModuleLogger("transport");

        // Build attributes from request and add timestamp
        std::unordered_map<std::string, std::string> attributes;

        if (!request.method.empty()) {
            attributes.emplace("method", request.method);
        }
        if (!request.path.empty()) {
            attributes.emplace("path", request.path);
        }
        if (!request.id.empty()) {
            attributes.emplace("message_id", request.id);
        }

        // Add timestamp
        attributes.emplace("created_at", getTimestamp());

        const auto session_id = session_manager_.CreateSession(attributes);
        session_manager_.CleanupExpiredSessions();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            updateStatistics("sessions_created", 1);
        }

        logger->info("Created new session id " + session_id +
                     " for SSE connection");

        return session_id;
    }

    void startSessionCleanupThread() {
        if (session_cleanup_running_.load(std::memory_order_acquire)) {
            return;
        }
        session_cleanup_running_.store(true, std::memory_order_release);
        session_cleanup_thread_ = std::thread([this]() {
            auto logger = mcp_logger::getModuleLogger("transport");
            logger->info("Session cleanup thread started");
            while (session_cleanup_running_.load(std::memory_order_acquire)) {
                std::unique_lock<std::mutex> lock(session_cleanup_mutex_);
                if (session_cleanup_cv_.wait_for(
                        lock, session_cleanup_interval_, [this]() {
                            return !session_cleanup_running_.load(
                                std::memory_order_acquire);
                        })) {
                    break;
                }
                lock.unlock();

                try {
                    session_manager_.CleanupExpiredSessions();
                } catch (const std::exception& e) {
                    logger->error("Session cleanup error: " +
                                  std::string(e.what()));
                }
            }
            logger->info("Session cleanup thread stopped");
        });
    }

    void stopSessionCleanupThread() {
        if (!session_cleanup_running_.load(std::memory_order_acquire)) {
            return;
        }
        session_cleanup_running_.store(false, std::memory_order_release);
        session_cleanup_cv_.notify_all();
        if (session_cleanup_thread_.joinable()) {
            session_cleanup_thread_.join();
        }
    }
};

// Transport public interface implementation
Transport::Transport() : impl_(std::make_unique<Impl>()) {}

Transport::~Transport() = default;

bool Transport::start(const TransportConfig& config) {
    return impl_->start(config);
}

bool Transport::stop() { return impl_->stop(); }

bool Transport::isRunning() const { return impl_->isRunning(); }

TransportStatus Transport::getStatus() const { return impl_->getStatus(); }

ConnectionInfo Transport::getConnectionInfo() const {
    return impl_->getConnectionInfo();
}

std::future<TransportMessage> Transport::sendRequest(
    const TransportMessage& message) {
    return impl_->sendRequest(message);
}

std::future<TransportMessage> Transport::sendRequest(
    const std::string& content, const std::string& method,
    const std::string& path,
    const std::map<std::string, std::string>& headers) {
    return impl_->sendRequest(content, method, path, headers);
}

bool Transport::sendMessage(const TransportMessage& message) {
    return impl_->sendMessage(message);
}

TransportMessage Transport::sendRequestSync(const TransportMessage& message) {
    return impl_->sendRequestSync(message);
}

TransportMessage Transport::sendRequestSync(
    const std::string& content, const std::string& method,
    const std::string& path,
    const std::map<std::string, std::string>& headers) {
    return impl_->sendRequestSync(content, method, path, headers);
}

bool Transport::hasMessage() const { return impl_->hasMessage(); }

TransportMessage Transport::getMessage() { return impl_->getMessage(); }

std::vector<TransportMessage> Transport::getAllMessages() {
    return impl_->getAllMessages();
}

void Transport::setMessageCallback(MessageCallback callback) {
    impl_->setMessageCallback(callback);
}

void Transport::setConnectionCallback(ConnectionCallback callback) {
    impl_->setConnectionCallback(callback);
}

void Transport::setErrorCallback(ErrorCallback callback) {
    impl_->setErrorCallback(callback);
}

void Transport::updateConfig(const TransportConfig& config) {
    impl_->updateConfig(config);
}

TransportConfig Transport::getConfig() const { return impl_->getConfig(); }

nlohmann::json Transport::getStatistics() const {
    return impl_->getStatistics();
}

bool Transport::healthCheck() { return impl_->healthCheck(); }

// Transport::log is deprecated; use global logger
void Transport::log(int level, const std::string& message) {
    impl_->appendLogToFile(level, message);
    switch (level) {
        case 0:
            mcp_logger::getModuleLogger("transport")->debug(message);
            break;
        case 1:
            mcp_logger::getModuleLogger("transport")->info(message);
            break;
        case 2:
            mcp_logger::getModuleLogger("transport")->warning(message);
            break;
        case 3:
            mcp_logger::getModuleLogger("transport")->error(message);
            break;
        case 4:
            mcp_logger::getModuleLogger("transport")->fatal(message);
            break;
        default:
            mcp_logger::getModuleLogger("transport")->info(message);
            break;
    }
}

void Transport::setLogLevel(int level) { impl_->setLogLevel(level); }

void Transport::setLogFile(const std::string& file_path) {
    impl_->setLogFile(file_path);
}

void Transport::enableLogging(bool enable) { impl_->enableLogging(enable); }

// Server-mode-only method
void Transport::setServerRequestHandler(
    std::shared_ptr<TransportServerRequestHandler> handler) {
    impl_->setServerRequestHandler(handler);
}

// Server-mode-only method
void Transport::setConnectionRequestHandler(
    std::shared_ptr<SseServerConnectHandler> handler) {
    impl_->SetSseServerConnectHandler(handler);
}

void Transport::setSseServerRequestHandler(
    std::shared_ptr<SseServerRequestHandler> handler) {
    impl_->setSseServerRequestHandler(handler);
}

bool Transport::sendResponse(const std::string& request_id,
                             const std::string& result) {
    return impl_->sendResponse(request_id, result);
}

bool Transport::sendError(const std::string& request_id, int error_code,
                          const std::string& error_message) {
    return impl_->sendError(request_id, error_code, error_message);
}

bool Transport::sendNotification(const std::string& method,
                                 const std::string& params) {
    return impl_->sendNotification(method, params);
}

// Utility function implementation
namespace utils {

TransportConfig createIpcConfig(
    const std::string& command, const std::vector<std::string>& args,
    const std::map<std::string, std::string>& env_vars) {
    TransportConfig config;
    config.type = TransportType::kIpc;
    config.mode = TransportMode::kClient;
    config.ipc.command = command;
    config.ipc.args = args;
    config.ipc.env_vars = env_vars;
    return config;
}

TransportConfig createIpcServerConfig(const std::string& server_name,
                                      const std::string& server_version) {
    TransportConfig config;
    config.type = TransportType::kIpc;
    config.mode = TransportMode::kServer;
    config.server.server_name = server_name;
    config.server.server_version = server_version;
    return config;
}

TransportConfig createSseConfig(const std::string& host, int port,
                                TransportProtocol protocol) {
    TransportConfig config;
    config.type = TransportType::kSse;
    config.mode = TransportMode::kClient;
    config.sse.host = host;
    config.sse.port = port;
    config.sse.protocol = protocol;
    return config;
}

TransportConfig createSseServerConfig(const std::string& host, int port,
                                      TransportProtocol protocol) {
    TransportConfig config;
    config.type = TransportType::kSse;
    config.mode = TransportMode::kServer;
    config.sse.host = host;
    config.sse.port = port;
    config.sse.protocol = protocol;
    return config;
}

TransportConfig createSseSecureConfig(const std::string& host, int port) {
    return createSseConfig(host, port, TransportProtocol::kHttps);
}

TransportMessage createRequest(
    const std::string& content, const std::string& method,
    const std::string& path,
    const std::map<std::string, std::string>& headers) {
    TransportMessage msg;
    msg.id =
        "req_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.content = content;
    msg.method = method;
    msg.path = path;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

TransportMessage createResponse(
    const std::string& content, int status_code,
    const std::map<std::string, std::string>& headers) {
    TransportMessage msg;
    msg.id =
        "resp_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.content = content;
    msg.status_code = status_code;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

TransportMessage createError(
    const std::string& error_message, int status_code,
    const std::map<std::string, std::string>& headers) {
    TransportMessage msg;
    msg.id =
        "err_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.content = error_message;
    msg.status_code = status_code;
    msg.error_message = error_message;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

bool validateConfig(const TransportConfig& config) { return config.validate(); }

bool validateMessage(const TransportMessage& message) {
    return !message.id.empty() && !message.content.empty();
}

std::string jsonToString(const nlohmann::json& j) { return j.dump(2); }

nlohmann::json stringToJson(const std::string& s) {
    try {
        return nlohmann::json::parse(s);
    } catch (const std::exception& e) {
        return nlohmann::json::object();
    }
}

}  // namespace utils

}  // namespace mcp_transport
