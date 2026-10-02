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

#ifndef GENAI_NLP_SERVICE_PROXY_H
#define GENAI_NLP_SERVICE_PROXY_H

#include <jsoncpp/json/json.h>
#include <toolchoice.h>

#include <atomic>
#include <functional>
#include <thread>

#include "_chatmodelconfig.h"
#include "_textmodelinfo.h"
#include "chat.h"
#include "genainlpserver.h"
#include "modelinfo.h"
#include "protocolversion.h"

namespace kyai::genai::nlp {

using ReconnectedCallback = std::function<void()>;

class GenAiNlpServiceProxy {
private:
    using InitCallback = std::function<void(int sessionId, int errorCode, void *userData)>;

public:
    ~GenAiNlpServiceProxy();

    bool connectToServer();
    bool reconnectServer();

    int initEngine(const ChatModelConfig &chatModelConfig, int *sessionId);

    void initEngineAsync(const ChatModelConfig &chatModelConfig, const InitCallback &callback, void *userData);

    void destroyEngine(int sessionId);

    void setChatResultCallback(const ChatResultCallback &callback, void *userData);

    void setToolHandler(const GenAiToolHandler &toolHandler, void *toolHandlerUserData);

    void setReconnectedCallback(const ReconnectedCallback &callback);

    void chat(int sessionId, const ChatModelConfig &chatModelConfig, const GenAiToolChoiceConfig &toolChoiceConfig,
              const std::string &question);

    int chatWithRequestId(int sessionId, const ChatModelConfig &chatModelConfig, const std::string &question,
                          const std::string &requestId);

    void generateContent(int sessionId, const ChatModelConfig &chatModelConfig, const std::string &prompt);

    void stopChat(int sessionId);

    int stopChatWithRequestId(int sessionId, const std::string &requestId);

    void clearChatMessage(int sessionId);

    void setChatSystemPromptId(int sessionId, PromptId promptId);

    void addSystemMessage(int sessionId, const std::string &message);

    void chatWithHistoryMessages(int sessionId, const ChatModelConfig &chatModelConfig,
                                 const GenAiToolChoiceConfig &toolChoiceConfig, const std::string &messages);

    int chatWithHistoryMessagesWithRequestId(int sessionId, const ChatModelConfig &chatModelConfig,
                                             const GenAiToolChoiceConfig &toolChoiceConfig, const std::string &messages,
                                             const std::string &requestId);

    TextModelList getModelList(int sessionId, int *errorCode);

    void enableInternalEventLoop(bool enable);

    int registerTool(int sessionId, const std::string &toolName, GenAiToolType toolType, const std::string &jsonSchema);

    int clearTools(int sessionId);

    int submitToolResult(int sessionId, const ChatModelConfig &chatModelConfig, const std::string &toolCallsJson,
                         const std::string &resultJson);

private:
    void startInternalEventLoop();
    void stopInternalEventLoop();

    void connectChatResultSignals(int sessionId);
    void connectToolHandlerResultSignals(int sessionId);

    static void onChatResultCallback(GDBusConnection *connection, const gchar *senderName, const gchar *objectPath,
                                     const gchar *interfaceName, const gchar *signalName, GVariant *parameters,
                                     gpointer userData);
    static void onToolHandlerResultCallback(GDBusConnection *connection, const gchar *senderName,
                                            const gchar *objectPath, const gchar *interfaceName,
                                            const gchar *signalName, GVariant *parameters, gpointer userData);
    static void onChatFinishedCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    static void onGenerateContentFinishedCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    static void onInitFinished(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    void handleErrorOccurred(int errorCode);
    void handleServerClosed();
    void handleChatTimeout();
    void handleGeneralError();
    void handleMessagesEmptyError();

    void startCallbackTimer();
    void removeCallbackTimerIfExists();
    void resetCallbackTimer();
    void handleCallbackTimeout();
    void refreshRuntimeProtocolVersion();
    bool ensureProtocolSupportedForApi(const char *apiName, int *errorCode = nullptr);

private:
    ChatResultCallback chatResultCallback_{nullptr};
    ReconnectedCallback reconnectedCallback_{nullptr};
    void *userData_{nullptr};
    GenAiToolHandler toolHandlerResultCallback_{nullptr};
    void *toolHandlerUserData_{nullptr};

    AiRuntimeGenAiNlpService *proxy_{nullptr};
    GenAiNlpServer *genNlpServer_{nullptr};
    int currentSessionId_{0};
    guint subscriptionId_{0};
    guint timeoutId_{0};
    int reconnectAttempts_{0};
    static constexpr int maxReconnectAttempts = 10;
    std::atomic<bool> internalLoopRequested_{false};
    std::thread eventLoopThread_;
    GMainContext *eventLoopContext_{nullptr};

    InitCallback initCallback_{nullptr};
    void *initUserData_{nullptr};
};

}  // namespace kyai::genai::nlp
#endif  // GENAI_NLP_SERVICE_PROXY_H
