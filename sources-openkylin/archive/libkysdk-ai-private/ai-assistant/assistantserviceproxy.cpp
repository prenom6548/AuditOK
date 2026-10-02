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

#include "assistantserviceproxy.h"

#include <jsoncpp/json/json.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <list>
#include <thread>

#include "assistantserviceglue.h"
#include "osassistant/error.h"
#include "serverproxy.h"

namespace {
const char *serverCloseErrorMessage = R"(
    {
        "error_code": 2,
        "error_message": "服务异常，请检查后台服务是否正常运行。"
    }
)";
}

namespace kyai {
namespace assistant {

const char *objectPath = "/com/kylin/AiRuntime/Assistant";
const char *assistantResultInterface = "com.kylin.AiRuntime.Assistant";

void AssistantServiceProxy::onChatResult(
    GDBusConnection *connection, const gchar *sender_name,
    const gchar *object_path, const gchar *interface_name,
    const gchar *signal_name, GVariant *parameters, gpointer user_data) {
    auto *proxy = static_cast<AssistantServiceProxy *>(user_data);

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *char_result = g_variant_iter_next_value(&iter);
    GVariant *int_errorCode = g_variant_iter_next_value(&iter);

    if (!char_result || !int_errorCode) {
        fprintf(stderr, "set chat result callback error: result is nullptr!\n");
        return;
    }

    char *result = (char *)g_variant_get_data(char_result);
    int *errorCode = (int *)g_variant_get_data(int_errorCode);
    if (!result || !errorCode) {
        fprintf(stderr,
                "set chat result callback error: error code is nullptr!\n");
        return;
    }

    if (proxy->chatResultCallback_) {
        proxy->chatResultCallback_(result);
    }
}

AssistantServiceProxy::AssistantServiceProxy() { initServerProxy(); }

AssistantServiceProxy::~AssistantServiceProxy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

void AssistantServiceProxy::initServerProxy() {
    if (!ServerProxy::getInstance().available()) {
        std::string errMessage{
            "Error creating sssistant service proxy: Server proxy "
            "connection is unavailable.\n"};
        std::fprintf(stderr, "%s", errMessage.c_str());
        if (!retryConnectToServer()) {
            return;
        }
    }
    createDelegate();
}

Error AssistantServiceProxy::init() {
    gint errorCode;
    gchar *errorMessage{nullptr};
    GError *gError{nullptr};

    std::fprintf(stdout, "Init assistant service proxy...\n");

    if (!delegate_) {
        return {ErrorCode::ServerError, "Server delegate no inited."};
    }

    bool ret = ai_runtime_assistant_call_init_sync(
        delegate_, &sessionId_, &errorCode, &errorMessage, nullptr, &gError);

    if (sessionId_ > 0) {
        std::fprintf(stdout, "Init assistant service proxy success...%d\n",
                     sessionId_);
        connectSignal();
    }

    if (gError) {
        std::fprintf(stderr, "Error: %s\n", gError->message);
        const std::string errMessage{gError->message};
        g_error_free(gError);
        return {ErrorCode::ServerError, errMessage};
    }

    std::fprintf(stdout, "Session ID: %d\n", sessionId_);

    if (errorCode != (int)ErrorCode::Ok) {
        std::fprintf(stderr, "Error: %s\n", errorMessage);
        return {ErrorCode(errorCode), errorMessage};
    }

    return {ErrorCode::Ok};
}

Error AssistantServiceProxy::initWithChatHistory(
    const std::vector<Message> &messages, int promptId) {
    if (!delegate_) {
        return {ErrorCode::ServerError, "Server delegate no inited."};
    }

    gint errorCode;
    gchar *errorMessage{nullptr};
    GError *gError{nullptr};

    gboolean success = ai_runtime_assistant_call_init_with_chat_history_sync(
        delegate_, messageVectorToJson(messages).c_str(), promptId, &sessionId_,
        &errorCode, &errorMessage, nullptr, &gError);
    if (gError) {
        std::fprintf(stderr, "Error: %s\n", gError->message);
        const std::string errMessage{gError->message};
        g_error_free(gError);
        return {ErrorCode::ServerError, errMessage};
    }

    if (!success) {
        std::fprintf(stderr, "Error: %s\n", "initWithChatHistory failed.");
        return {ErrorCode(errorCode), errorMessage};
    }

    std::fprintf(stdout, "Session ID: %d\n", sessionId_);

    if (errorCode != (int)ErrorCode::Ok) {
        std::fprintf(stderr, "Error: %s\n", errorMessage);
        return {ErrorCode(errorCode), errorMessage};
    }

    connectSignal();
    return {ErrorCode::Ok};
}

void AssistantServiceProxy::setChatResultCallback(ChatResultCallback callback) {
    chatResultCallback_ = std::move(callback);
}

Features AssistantServiceProxy::supportedFeatures() const {
    if (!delegate_) {
        return Features{0};
    }

    gint features{0};
    GError *error{nullptr};

    gboolean success = ai_runtime_assistant_call_supported_features_sync(
        delegate_, sessionId_, &features, nullptr, &error);
    if (error) {
        std::fprintf(stderr, "Error calling supportedFuntions: %s\n",
                     error->message);
        g_error_free(error);
        return Features{0};
    }

    if (!success) {
        std::fprintf(
            stderr,
            "Call to supportedFeatures failed without an error message.\n");
        return Features{0};
    }

    return Features{features};
}

void AssistantServiceProxy::onCallFinished(GObject *source_object,
                                           GAsyncResult *res,
                                           gpointer user_data) {
    GError *error = nullptr;
    GDBusProxy *proxy = G_DBUS_PROXY(source_object);
    GVariant *result;

    result = g_dbus_proxy_call_finish(proxy, res, &error);

    if (!error) {
        return;
    }

    g_printerr("Error during call: %s\n", error->message);
    AssistantServiceProxy *asssistantServiceproxy =
        static_cast<AssistantServiceProxy *>(user_data);
    // server closed
    if (error->code == 18) {
        asssistantServiceproxy->handleServerClosed();
    }
    g_error_free(error);
}

void AssistantServiceProxy::chatAsync(const std::string &message) {
    if (!delegate_) {
        return;
    }

    currentMessage_ = message;
    ai_runtime_assistant_call_chat(delegate_, message.c_str(), sessionId_,
                                   nullptr, onCallFinished, this);
}

void AssistantServiceProxy::stopChat() {
    if (!delegate_) {
        return;
    }

    ai_runtime_assistant_call_stop_chat(delegate_, sessionId_, nullptr, nullptr,
                                        nullptr);
}

void AssistantServiceProxy::setPromptId(int id) {
    if (!delegate_) {
        return;
    }

    currentActorId_ = id;
    ai_runtime_assistant_call_set_prompt_id(delegate_, sessionId_, id, nullptr,
                                            onCallFinished, this);
}

void AssistantServiceProxy::clearContext() {
    if (!delegate_) {
        return;
    }

    currentActorId_ = -1;
    currentMessage_.clear();
    ai_runtime_assistant_call_clear_context(delegate_, sessionId_, nullptr,
                                            nullptr, nullptr);
}

std::vector<Prompt> AssistantServiceProxy::prompts() {
    if (!delegate_) {
        return {};
    }

    gchar *allPrompts = nullptr;
    GError *error = nullptr;
    std::vector<Prompt> promptVector{};
    gboolean success = ai_runtime_assistant_call_prompts_sync(
        delegate_, sessionId_, &allPrompts, nullptr, &error);

    if (!success) {
        std::fprintf(stderr,
                     "Call to prompts failed without an error message.\n");
        return {};
    }

    if (error) {
        std::fprintf(stderr, "Error calling prompts: %s\n", error->message);
        g_error_free(error);
        return {};
    }

    parseJsonStringToVector(allPrompts, promptVector);
    if (promptIdManager_.promptIdList().empty()) {
        setDefaultPromptIdList(promptVector);
    }
    adjustPromptOrder(promptVector);
    return promptVector;
}

bool AssistantServiceProxy::setPromptOrder(std::list<int> promptIdList) {
    if (!delegate_) {
        return false;
    }
    return promptIdManager_.updatePromptIdOrder(promptIdList);
}

int AssistantServiceProxy::createPrompt(const std::string &name,
                                        const std::string &content) {
    if (!delegate_) {
        return -1;
    }

    gint promptId = -1;
    GError *error = nullptr;
    gboolean success = ai_runtime_assistant_call_create_prompt_sync(
        delegate_, name.c_str(), content.c_str(), &promptId, nullptr, &error);

    if (!success) {
        std::fprintf(stderr,
                     "Call to createPrompt failed without an error "
                     "message.\n");
        return -1;
    }

    if (error) {
        std::fprintf(stderr, "Error calling createPrompt: %s\n",
                     error->message);
        g_error_free(error);
        return -1;
    }

    if (promptId == -1) {
        return -1;
    }
    if (!promptIdManager_.addPromptId(promptId)) {
        return -1;
    }
    return (int)promptId;
}

bool AssistantServiceProxy::deletePrompt(int promptId) {
    if (!delegate_) {
        return false;
    }
    GError *error = nullptr;
    gboolean success = false;
    if (!promptIdManager_.deletePromptId(promptId)) {
        return false;
    }

    gboolean ret = ai_runtime_assistant_call_delete_prompt_sync(
        delegate_, promptId, &success, nullptr, &error);

    if (error) {
        std::fprintf(stderr, "Error calling deletePrompt: %s\n",
                     error->message);
        g_error_free(error);
        return false;
    }

    if (!ret) {
        std::fprintf(stderr,
                     "Call to deletePrompt failed without an error "
                     "message.\n");
        return false;
    }

    return success;
}

bool AssistantServiceProxy::updatePrompt(int promptId, const std::string &name,
                                         const std::string &content) {
    if (!delegate_) {
        return false;
    }
    GError *error = nullptr;
    gboolean success = false;
    gboolean ret = ai_runtime_assistant_call_update_prompt_sync(
        delegate_, promptId, name.c_str(), content.c_str(), &success, nullptr,
        &error);
    if (error) {
        std::fprintf(stderr, "Error calling updatePrompt: %s\n",
                     error->message);
        g_error_free(error);
        return false;
    }

    if (!ret) {
        std::fprintf(stderr,
                     "Call to updatePrompt failed without an error "
                     "message.\n");
        return false;
    }

    return success;
}

void AssistantServiceProxy::connectSignal() {
    auto *connection = ServerProxy::getInstance().getConnection();
    std::string interfaceName =
        assistantResultInterface + std::to_string(sessionId_);

    g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), "ChatResult", objectPath,
        NULL, G_DBUS_SIGNAL_FLAGS_NONE, onChatResult, this, nullptr);
}

void AssistantServiceProxy::createDelegate() {
    GError *error = nullptr;
    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = ai_runtime_assistant_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath, nullptr,
        &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating speech processor proxy %s: %s\n", objectPath,
                   error->message);
        g_error_free(error);
    }
}

void AssistantServiceProxy::handleServerClosed() {
    if (retryConnectToServer()) {
        if (currentActorId_ != -1) {
            setPromptId(currentActorId_);
        }
        if (!currentMessage_.empty()) {
            chatAsync(currentMessage_);
        }
    } else {
        if (chatResultCallback_) {
            chatResultCallback_(serverCloseErrorMessage);
        }
    }
}

bool AssistantServiceProxy::retryConnectToServer() {
    std::fprintf(stdout, "Server closed, retrying to connect...\n");
    int retryCount = 0;
    while (retryCount < 10) {
        ServerProxy::getInstance().init();
        if (ServerProxy::getInstance().available()) {
            createDelegate();
            init();
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        retryCount++;
    }

    return false;
}

void AssistantServiceProxy::setDefaultPromptIdList(
    const std::vector<Prompt> prompts) {
    std::list<int> idList;
    for (auto prompt : prompts) {
        idList.push_back(prompt.id);
    }
    promptIdManager_.setDefaultPromptIdList(idList);
}

void AssistantServiceProxy::parseJsonStringToVector(
    const std::string &jsonString, std::vector<Prompt> &prompts) {
    if (jsonString.empty()) {
        fprintf(stderr,
                "AssistantServiceProxy parseJsonStringToVector: jsonString is "
                "empty!\n");
        return;
    }
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();

    std::string errors;
    bool parsingSuccessful =
        reader->parse(jsonString.c_str(),
                      jsonString.c_str() + jsonString.size(), &root, &errors);

    delete reader;

    if (!parsingSuccessful) {
        fprintf(stderr,
                "AssistantServiceProxy parseJsonStringToVector: failed to "
                "parse JSON: %s\n",
                errors.c_str());
        return;
    }

    prompts.clear();
    for (const auto &item : root) {
        Prompt newPrompt;
        newPrompt.id = item["id"].asInt();
        newPrompt.name = item["name"].asString();
        newPrompt.content = item["content"].asString();
        newPrompt.type =
            (kyai::assistant::PromptContentAccessType)item["type"].asInt();
        newPrompt.active = item["active"].asBool();
        prompts.push_back(newPrompt);
    }
}

std::string AssistantServiceProxy::messageVectorToJson(
    const std::vector<Message> &messages) {
    Json::Value jsonArray(Json::arrayValue);

    for (const auto &message : messages) {
        Json::Value jsonMessage;
        jsonMessage["user"] = message.user;
        jsonMessage["assistant"] = message.assistant;
        jsonArray.append(jsonMessage);
    }

    Json::StyledWriter writer;
    return writer.write(jsonArray);
}

void AssistantServiceProxy::adjustPromptOrder(std::vector<Prompt> &prompts) {
    std::list<int> promptIdList = promptIdManager_.promptIdList();
    if (promptIdList.empty()) {
        return;
    }
    std::vector<Prompt> newPrompts{};
    for (int promptId : promptIdList) {
        for (const auto &prompt : prompts) {
            if (prompt.id != promptId) {
                continue;
            }
            newPrompts.push_back(prompt);
        }
    }

    // 找到没有在promptIdList中的prompt
    for (const auto &prompt : prompts) {
        if (promptIdList.end() ==
            std::find(promptIdList.begin(), promptIdList.end(), prompt.id)) {
            newPrompts.push_back(prompt);
        }
    }

    prompts = newPrompts;
}

std::string AssistantServiceProxy::getConfiguredNlpModelInfos() const {
    gchar *configuredNlpModelInfos{nullptr};
    GError *error = nullptr;

    ai_runtime_assistant_call_get_configured_nlp_model_list_sync(
        delegate_, sessionId_, &configuredNlpModelInfos, nullptr, &error);
    if (error != nullptr) {
        std::fprintf(stderr, "Error calling getConfiguredNlpModelInfos: %s\n",
                     error->message);
        g_error_free(error);
        return "";
    }
    if (configuredNlpModelInfos == nullptr) {
        return "";
    }
    std::string ret(configuredNlpModelInfos);
    g_free(configuredNlpModelInfos);
    return ret;
}

void AssistantServiceProxy::setCurrentNlpModel(
    const std::string &modelName) const {
    GError *error = nullptr;
    ai_runtime_assistant_call_set_current_nlp_model_sync(
        delegate_, sessionId_, modelName.c_str(), nullptr, &error);
    if (error != nullptr) {
        std::fprintf(stderr, "Error calling setCurrentNlpModel: %s\n",
                     error->message);
        g_error_free(error);
    }
}

}  // namespace assistant
}  // namespace kyai
