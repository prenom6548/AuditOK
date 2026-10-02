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

#include "client.h"

#include <google/protobuf/util/json_util.h>
#include <grpcpp/grpcpp.h>
#include <sys/socket.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "stub_manager.h"
using grpc::ClientContext;
using json = nlohmann::json;
using grpc::ClientAsyncResponseReaderInterface;
#define LOG_INFO(msg)                                                 \
    {                                                                 \
        time_t t = time(0);                                           \
        char mbstr[100];                                              \
        strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %X", localtime(&t)); \
        std::cout << mbstr << " - INFO - " << msg << std::endl;       \
    }
#define LOG_ERROR(msg)                                                \
    {                                                                 \
        time_t t = time(0);                                           \
        char mbstr[100];                                              \
        strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %X", localtime(&t)); \
        std::cerr << mbstr << " - ERROR - " << msg << std::endl;      \
    }

namespace document {

std::string serviceName = "kylin-ai-document-service.service";

std::string ProtoMessageToJsonString(const google::protobuf::Message &message);
template <typename T>
T JsonStringToProtoMessage(const std::string &json_string);
std::string ProtoMessageToJsonString(const google::protobuf::Message &message) {
    std::string json_string;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(message, &json_string, options);
    return json_string;
}
template <typename T>
T JsonStringToProtoMessage(const std::string &json_string) {
    T message;
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;
    google::protobuf::util::JsonStringToMessage(json_string, &message, options);
    return message;
}

static std::string ConstructSockPath(const std::string &base_path, const std::string &suffix) {
    uid_t uid = getuid();  // 获取当前用户的UID
    return base_path + std::to_string(uid) + suffix;
}

std::shared_ptr<grpc::Channel> CreateChannel(const std::string &target) {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    auto channel = grpc::CreateCustomChannel(target, grpc::InsecureChannelCredentials(), args);
    // 拿到一个可用的channel 最多等待3s
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(3);
    if (!channel->WaitForConnected(deadline)) {
        LOG_ERROR("Failed to connect to channel within 3 seconds");
    }

    return channel;
}

std::string escapeUnitName(const std::string &name) {
    std::string escaped;
    for (char c : name) {
        if (c == '-')
            escaped += "_2d";
        else if (c == '.')
            escaped += "_2e";
        else
            escaped += c;
    }
    return escaped;
}

bool checkServiceActive(sd_bus *bus, const std::string &serviceName, sd_bus_error *error) {
    // true: active
    char *activeState = nullptr;
    int ret = sd_bus_get_property_string(bus, "org.freedesktop.systemd1",
                                         ("/org/freedesktop/systemd1/unit/" + escapeUnitName(serviceName)).c_str(),
                                         "org.freedesktop.systemd1.Unit", "ActiveState", error, &activeState);

    if (ret < 0) {
        LOG_ERROR(std::string("Failed to get service state:") + error->message);
        if (activeState) free(activeState);
        return false;
    }

    bool isActive = (strcmp(activeState, "active") == 0);
    free(activeState);
    return isActive;
}

bool startUserService(const std::string &serviceName) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    sd_bus *bus = nullptr;

    // 连接到用户 D-Bus
    if (sd_bus_open_user(&bus) < 0) {
        LOG_ERROR("Failed to connect to user bus");
        return false;
    }

    // 检查服务是否已运行
    if (checkServiceActive(bus, serviceName, &error)) {
        LOG_INFO("Service is already running: " + serviceName);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return true;
    }

    // 启动服务
    int ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                                 "org.freedesktop.systemd1.Manager", "StartUnit", &error, &reply, "ss",
                                 serviceName.c_str(), "replace");
    if (ret < 0) {
        LOG_ERROR(std::string("Failed to start service: ") + error.message);
        sd_bus_message_unref(reply);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    /*
    *** 影响域：只会影响系统启动第一次调用文档处理服务
    */
    LOG_INFO("Service started successfully: " + serviceName);
    sd_bus_message_unref(reply);
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
    return false;
}

document::DocumentChunkClient::DocumentChunkClient() : stub_manager_(nullptr) { startUserService(serviceName); }

DocumentChunkClient::~DocumentChunkClient() {}

DocumentChunkResponse DocumentChunkClient::SyncChunkDocument(const std::vector<std::string> &file_paths,
                                                             const std::vector<std::string> &chunker_types,
                                                             const std::vector<int32_t> &chunk_sizes) {
    ChunkRequest request;
    for (size_t i = 0; i < file_paths.size(); ++i) {
        request.add_file_paths(file_paths[i]);
        request.add_chunker_types(chunker_types[i]);
        request.add_chunk_sizes(chunk_sizes[i]);
        // 默认max_doc_chars_count设置为-1 表示不限制
        request.add_max_doc_chars_count(-1);
    }
    std::string request_json = ProtoMessageToJsonString(request);
    LOG_INFO("request: " + request_json);

    /*
    重试机制 重试3次 每300ms重试一次 如果一直用不了，那等待时间最多10s
    */
    int max_retries = 3;
    DocumentChunkResponse response_json;
    grpc::Status status;
    std::string response_json_str;
    ChunkedResponse response;

    for (int attempt = 0; attempt < max_retries; attempt++) {
        ClientContext context;
        if (!stub_manager_) {
            stub_manager_ = std::make_unique<ChunkStubManager>(
                CreateChannel(ConstructSockPath("unix:///tmp/document_service_sync_", ".sock")), nullptr);
        }

        status = stub_manager_->GetSyncStub()->SyncChunkDocument(&context, request, &response);
        // 服务不可用时进行重试
        if (status.error_code() != grpc::StatusCode::UNAVAILABLE) {
            break;
        }

        LOG_INFO("Service unavailable, retry attempt: " + std::to_string(attempt + 1));
        usleep(300 * 1000);
        response.Clear();
        stub_manager_.reset();
    }

    if (!status.ok()) {
        LOG_ERROR("Service error:errorCode:" + std::to_string(status.error_code()) +
                  ",errorMsg:" + status.error_message());
    }

    LOG_INFO("Chunked over: " + request_json);
    response_json_str = ProtoMessageToJsonString(response);
    response_json.data = response_json_str;
    response_json.errorCode = status.error_code();
    response_json.errorMsg = status.error_message();
    return response_json;
}

DocumentChunkResponse DocumentChunkClient::SyncChunkDocument(const std::vector<std::string> &file_paths,
                                                             const std::vector<std::string> &chunker_types,
                                                             const std::vector<int32_t> &chunk_sizes,
                                                             const std::vector<int32_t> &max_doc_chars_count) {
    /* 文档解析服务端会通过max_doc_chars_count参数和文档解析后的字符数数比较后，决定是否要执行分块
        max_doc_chars_count > 解析文档的字符数 会继续执行切分流程，否则是立刻返回错误
    */
    ChunkRequest request;
    for (size_t i = 0; i < file_paths.size(); ++i) {
        request.add_file_paths(file_paths[i]);
        request.add_chunker_types(chunker_types[i]);
        request.add_chunk_sizes(chunk_sizes[i]);
        request.add_max_doc_chars_count(max_doc_chars_count[i]);
    }
    std::string request_json = ProtoMessageToJsonString(request);
    LOG_INFO("request: " + request_json);

    /*
    重试机制 重试3次 每300ms重试一次 如果一直用不了，那等待时间最多10s
    */
    int max_retries = 3;
    DocumentChunkResponse response_json;
    grpc::Status status;
    std::string response_json_str;
    ChunkedResponse response;

    for (int attempt = 0; attempt < max_retries; attempt++) {
        ClientContext context;
        if (!stub_manager_) {
            stub_manager_ = std::make_unique<ChunkStubManager>(
                CreateChannel(ConstructSockPath("unix:///tmp/document_service_sync_", ".sock")), nullptr);
        }

        status = stub_manager_->GetSyncStub()->SyncChunkDocument(&context, request, &response);
        // 服务不可用时进行重试
        if (status.error_code() != grpc::StatusCode::UNAVAILABLE) {
            break;
        }

        LOG_INFO("Service unavailable, retry attempt: " + std::to_string(attempt + 1));
        usleep(300 * 1000);
        response.Clear();
        stub_manager_.reset();
    }

    if (!status.ok()) {
        LOG_ERROR("Service error:errorCode:" + std::to_string(status.error_code()) +
                  ",errorMsg:" + status.error_message());
    }

    LOG_INFO("Chunked over: " + request_json);
    response_json_str = ProtoMessageToJsonString(response);
    response_json.data = response_json_str;
    response_json.errorCode = status.error_code();
    response_json.errorMsg = status.error_message();
    return response_json;
}

void DocumentChunkClient::AsyncChunkDocument(const std::vector<std::string> &file_paths,
                                             const std::vector<std::string> &chunker_types,
                                             const std::vector<int32_t> &chunk_sizes, void *callback_data,
                                             DocumentChunkCallback callback) {
    ChunkRequest request;
    ChunkedResponse response;
    for (size_t i = 0; i < file_paths.size(); ++i) {
        request.add_file_paths(file_paths[i]);
        request.add_chunker_types(chunker_types[i]);
        request.add_chunk_sizes(chunk_sizes[i]);
    }
    std::string request_json = ProtoMessageToJsonString(request);
    LOG_INFO("request: " + request_json);
    ClientContext context;
    auto *async_stub = stub_manager_->GetAsyncStub();
    auto wrapped_callback = [this, callback, callback_data](::grpc::Status status) {
        ChunkedResponse response;
        DocumentChunkResponse response_json;
        if (status.ok()) {
            std::string response_json_str = ProtoMessageToJsonString(response);
            response_json.data = response_json_str;
            response_json.errorCode = status.error_code();
            response_json.errorMsg = status.error_message();
        } else {
            LOG_ERROR("Async request failed: " + status.error_message());
            response_json.data = nullptr;
            response_json.errorCode = status.error_code();
            response_json.errorMsg = status.error_message();
        }
        callback(response_json, callback_data);
    };
    async_stub->experimental_async()->AsyncChunkDocument(&context, &request, &response, wrapped_callback);
}
}  // namespace document