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

#ifndef ASSISTANTSERVICEPROXY_H
#define ASSISTANTSERVICEPROXY_H

#include "assistantserviceglue.h"
#include "osassistant/error.h"
#include "osassistant/osassistant.h"
#include "promptidmanager.h"

namespace kyai {
namespace assistant {

class AssistantServiceProxy {
public:
    AssistantServiceProxy();
    ~AssistantServiceProxy();

    void initServerProxy();
    Error init();
    Error initWithChatHistory(const std::vector<Message> &messages,
                              int promptId);
    void setChatResultCallback(ChatResultCallback callback);
    Features supportedFeatures() const;

    void chatAsync(const std::string &message);
    void stopChat();
    void setPromptId(int id);
    void clearContext();
    std::vector<Prompt> prompts();
    bool setPromptOrder(std::list<int> promptIdList);
    int createPrompt(const std::string &name, const std::string &content);
    bool deletePrompt(int promptId);
    bool updatePrompt(int promptId, const std::string &name,
                      const std::string &content);

    std::string getConfiguredNlpModelInfos() const;
    void setCurrentNlpModel(const std::string &modelName) const;

private:
    void connectSignal();
    void createDelegate();
    void handleServerClosed();
    bool retryConnectToServer();
    void setDefaultPromptIdList(const std::vector<Prompt> prompts);
    static void onChatResult(GDBusConnection *connection,
                             const gchar *sender_name, const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name, GVariant *parameters,
                             gpointer user_data);
    static void onCallFinished(GObject *source_object, GAsyncResult *res,
                               gpointer user_data);

    void parseJsonStringToVector(const std::string &jsonString,
                                 std::vector<Prompt> &prompts);
    std::string messageVectorToJson(const std::vector<Message> &messages);
    void adjustPromptOrder(std::vector<Prompt> &prompts);

private:
    struct EngineConfig {
        std::string nlpEngineName;
        std::string nlpConfig;
        std::string visionEngineName;
        std::string visionConfig;
    };

private:
    AiRuntimeAssistant *delegate_{nullptr};
    int sessionId_{0};
    ChatResultCallback chatResultCallback_;
    std::string currentMessage_;
    int currentActorId_{-1};
    PromptIdManager promptIdManager_;
};

}  // namespace assistant
}  // namespace kyai

#endif  // ASSISTANTSERVICEPROXY_H
