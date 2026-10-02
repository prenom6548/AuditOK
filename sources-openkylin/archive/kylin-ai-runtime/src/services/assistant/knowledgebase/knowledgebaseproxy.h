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

#ifndef KNOWLEDGEBASEPROXY_H
#define KNOWLEDGEBASEPROXY_H

#include <gio/gio.h>

#include <functional>
#include <memory>
#include <string>

#include "advaiknowledgebaseserviceglue.h"
#include "jsoncpp/json/json.h"

namespace advanced::knowledgebase {

struct KBChatSource {
    std::string filePath;
    std::string fileContent;
    std::string toolPath;
    int toolIndex{-1};
    double confidence;
};

struct KBChatResponse {
    bool isFinished;
    std::string model;
    std::string reasoningContent;
    std::string response;
    int errorCode;
    std::vector<KBChatSource> sources;
};
using KBChatStreamCallback = std::function<void(const KBChatResponse &response)>;

enum KBErrorCode {
    SUCCESS = 0,  // 成功

    // 知识库
    KB_ID_INVALID = 1,  // 无效的知识库ID
    KB_CREATE_FAILED,   // 知识库创建失败
    KB_ALREADY_EXISTS,  // 知识库已存在
    KB_NOT_FOUND,       // 知识库不存在
    KB_ACCESS_DENIED,   // 知识库访问被拒
    KB_CORRUPTED,       // 知识库损坏

    // 检索
    RETRIEVAL_FAILED = 201,  // 检索失败
    RETRIEVAL_NO_RESULTS,    // 未检索到结果

    // 聊天
    CHAT_MODEL_NOT_EXIST = 301,         // 聊天模型未安装
    CHAT_MODEL_UNAVAILABLE,             // 聊天模型不可用
    CHAT_MODEL_NET_ERROR,               // 云端聊天模型无法连接
    CHAT_MODEL_AUTHENTICATION_FAILURE,  // 聊天模型鉴权失败
    CHAT_MODEL_RUNTIME_ERROR,           // 聊天模型服务异常
    CHAT_MODEL_TOO_MANY_REQUESTS,       // 聊天模型请求过多
    CHAT_MODEL_SERVICE_ERROR,           // 聊天模型服务错误
    CHAT_MODEL_SERVICE_TIMEOUT,         // 聊天模型服务超时
    CHAT_MODEL_BAD_REQUEST,             // 聊天模型请求错误
    CHAT_MODEL_RUN_FAILED,              // 聊天模型运行失败
    CHAT_MODEL_RUN_TIME_OUT,            // 聊天模型运行超时
    CHAT_MODEL_INVALID_SESSION,         // 聊天模型无效会话
    CHAT_SSL_CERTIFICATE_ERROR,         // 系统时间异常或数字证书过期

    // 通用错误
    INVALID_PARAMETER = 901,  // 无效参数
    OPERATION_TIMEOUT,        // 操作超时
    INVALID_SESSION_ID,       // 无效会话ID
    SERVER_ERROR,             // 服务错误

    UNKNOWN_ERROR = 1000  // 未知错误
};

struct KnowledgebaseErrorInfo {
    int errorCode;
    std::string errorMessage;
};

class KnowledgeBaseProxy {
public:
    static KnowledgeBaseProxy &instance();
    KnowledgeBaseProxy(const KnowledgeBaseProxy &) = delete;
    KnowledgeBaseProxy &operator=(const KnowledgeBaseProxy &) = delete;
    ~KnowledgeBaseProxy();

    int init();
    void destroyKBSession() const;
    void chat(const std::string &question, const std::string &model, KBChatStreamCallback callback);
    void stopChat();

private:
    KnowledgeBaseProxy();
    bool initServer(const std::string &unixPath);
    int initSession();
    void destroy(int sessionId) const;

    void connectChatResultSignals();
    static void onChatResultCallback(GDBusConnection *connection, const gchar *senderName, const gchar *objectPath,
                                     const gchar *interfaceName, const gchar *signalName, GVariant *parameters,
                                     gpointer userData);
    std::string buildChatJsonString(const std::string &question, const std::string &model) const;
    static int parseErrorCode(const Json::Value &json);

private:
    GDBusConnection *connection_{nullptr};
    AiBusinessKnowledgeBaseService *proxy_{nullptr};
    guint chatSubscriptionId_{0};
    int sessionId_{0};
    KBChatStreamCallback chatResultCallback_{nullptr};
    std::string chatRequestId_;
};
}  // namespace advanced::knowledgebase

#endif  // KNOWLEDGEBASEPROXY_H
