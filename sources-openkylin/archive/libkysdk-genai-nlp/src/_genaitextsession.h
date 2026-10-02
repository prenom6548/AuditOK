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

#ifndef GENAI_TEXT_SESSION_H
#define GENAI_TEXT_SESSION_H

#include <string>

#include "_chatmodelconfig.h"
#include "_textmodelinfo.h"
#include "_toolchoice.h"
#include "chat.h"
#include "config.h"
#include "genainlpserviceproxy.h"

class _GenAiTextSession {
public:
    _GenAiTextSession();

    int init();

    void init(GenAiTextInitCallback callback, void *userData);

    void destroy();

    void setResultCallback(const ChatResultCallback &callback, void *userData);

    void setChatModelConfig(const ChatModelConfig &config);

    void chat(const std::string &question);

    int chatWithRequestId(const std::string &question,
                          const std::string &requestId);

    void generateContent(const std::string &question);

    void chatWithHistoryMessages(const ChatMessage &message);

    void chatWithHistoryMessagesWithRequestId(const ChatMessage &message, const std::string &requestId);

    void setChatSystemPrompt(const std::string &message);

    void setChatSystemPromptId(PromptId promptId);

    void stopChat();

    int stopChatWithRequestId(const std::string &requestId);

    void clearChatMessage();

    TextModelList *getTextModelList(int *errorCode);

    void enableInternalEventLoop(bool enable);

    int registerTool(const std::string &toolName, GenAiToolType toolType, const std::string &jsonSchema);

    int clearTools();

    void setToolHandler(GenAiToolHandler handler, void *userData);

    int submitToolResult(const std::string &toolCallsJson, const std::string &resultJson);

    void setToolChoiceConfig(const GenAiToolChoiceConfig &config);

private:
    bool initServer();
    int initSession();
    void initSessionAsync();
    static void onSessionInited(int sessionId, int errorCode, void *userData);

private:
    kyai::genai::nlp::GenAiNlpServiceProxy serviceProxy_;
    int sessionId_{0};
    ChatModelConfig chatModelConfig_{};
    GenAiToolChoiceConfig toolChoiceConfig_{};
    TextModelList textModelList_{};

    GenAiTextInitCallback initCallback_{nullptr};
    void *initUserData_{nullptr};
};

#endif  // GENAI_TEXT_SESSION_H
