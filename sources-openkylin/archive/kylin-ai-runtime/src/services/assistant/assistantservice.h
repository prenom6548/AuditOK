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

#ifndef ASSISTANTSERVICE_H
#define ASSISTANTSERVICE_H

#include <gio/gio.h>
#include <jsoncpp/json/value.h>
#include <kylin-ai/ai-engine/embeddingengine.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

#include <memory>
#include <set>

#include "assistant/dialoguemanager/dialoguemanager.h"
#include "assistant/error.h"
#include "assistantserviceglue.h"
#include "config/prompttemplate.h"
#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#include "promptmanager.h"

class AssistantService : public std::enable_shared_from_this<AssistantService> {
private:
    using INlpEngine = ai_engine::gen_ai::nlp::AbstractTextGenerationEngine;

public:
    AssistantService(GDBusConnection &connection, ai_engine::AiEnginePluginManager &enginePluginManager,
                     EngineSelector &engineSelector);

    ~AssistantService();

    void stop();

private:
    enum Feature { TextChat = 1, TextToImage = 2, ImageProcessing = 4, DocumentQA = 8 };

private:
    void exportSkeleton();
    void unexportSkeleton();
    void connectHandlers();

    int generateRandomSessionId();

    std::shared_ptr<INlpEngine> initBaseNlpEngine(int sessionId);

    assistant::Error initNlpEngines(int sessionId);

    assistant::Error initVisionEngine(int sessionId);

    assistant::Error initImageProcessingEngine(int sessionId, const std::string &config);

    void stopNlpEngine(int sessionId);
    void stopVisionEngine(int sessionId);
    void setNlpEnginePromptId(int sessionId, int promptId);

    std::string aiassistantPrompts(const std::vector<config::PromptTemplate::Prompt> &prompts, int sessionId) const;

    // 默认模式（智能模式）使用控制面板的模型配置选项
    void tryToChangeNlpModelWithDefaultMode(int sessionId);

    // 用户模式使用助手应用的模型配置选项
    void tryToChangeNlpModelWithUserMode(int sessionId, bool isDefaultMode);

    void tryToChangeVisionModel(int sessionId);

private:
    static bool onHandleInit(AiRuntimeAssistant *object, GDBusMethodInvocation *invocation, gpointer userData);
    static bool onHandleInitWithChatHistory(AiRuntimeAssistant *object, GDBusMethodInvocation *invocation,
                                            const gchar *messages, gint promptId, gpointer userData);
    static bool onHandleSupportedFeatures(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                          gint sessionId, gpointer userData);
    static bool onHandleChat(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gchar *message,
                             gint sessionId, gpointer userData);
    static bool onHandleStopChat(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                 gpointer userData);
    static bool onHandleSetPromptId(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                    gint promptId, gpointer userData);
    static bool onHandleClearContext(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                     gpointer userData);
    static bool onHandlePrompts(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                gpointer userData);
    static bool onHandleCreatePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gchar *promptName,
                                     gchar *promptContent, gpointer userData);
    static bool onHandleDeletePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint promptId,
                                     gpointer userData);
    static bool onHandleUpdatePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint promptId,
                                     gchar *promptName, gchar *promptContent, gpointer userData);

    static bool onHandleGetConfiguredNlpModelList(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                                  gint sessionId, gpointer userData);
    static bool onHandleSetCurrentNlpModel(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                           gint sessionId, gchar *modelName, gpointer userData);

    assistant::ErrorCode doHandleChat(int sessionId, const std::string &message);
    void clearContext(int sessionId);

    std::string currentModelName(int sessionId) const;

    bool isDefaultMode(int sessionId) const;

    std::string getAllPrompts(int sessionId) const;

private:
    bool isExported_{false};
    AiRuntimeAssistant *delegate_{nullptr};
    GDBusConnection &connection_;
    std::shared_ptr<DialogueManager> dialogueManager_{nullptr};

    std::set<int> sessionIdSet_;

    std::unique_ptr<PromptManager> promptManager_;  // TODO 放session里

    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    EngineSelector &engineSelector_;
    std::map<int, std::atomic_bool> defaultModeNlpModelChangedMap_;
    std::map<int, std::atomic_bool> visionModelChangedMap_;
    std::vector<gulong> handlerIds_;
    std::map<int, std::string> currentModels_;
    std::map<int, std::atomic_bool> userModeNlpModelChangedMap_;
};

#endif  // ASSISTANTSERVICE_H
