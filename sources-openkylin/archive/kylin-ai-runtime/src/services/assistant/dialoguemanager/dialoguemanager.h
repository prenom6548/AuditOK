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

#ifndef DIALOGUEMANAGER_H
#define DIALOGUEMANAGER_H

#include <gio/gio.h>
#include <kylin-ai/ai-engine/embeddingengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>

#include <memory>
#include <string>

#include "assistant/handlers/requesthandlermanager.h"
#ifdef SUPPORT_VECTOR_DB
#include "assistant/rag/ragserverproxy.h"
#endif
#include "assistant/session.h"
#include "engineselector/engineselector.h"

class DialogueManager {
public:
    explicit DialogueManager(GDBusConnection &connection);
    ~DialogueManager();

    using TextGenerationEngine = ai_engine::gen_ai::nlp::AbstractTextGenerationEngine;
    using ImageGenerationEngine = ai_engine::gen_ai::vision::AbstractImageGenerationEngine;
    using ImageProcessEngine = ai_engine::core_ai::vision::AbstractImageProcessEngine;

    void addSession(int sessionId);

    void setBaseNlpEngine(int sessionId, std::shared_ptr<TextGenerationEngine> baseNlpEngine,
                          EngineSelector &engineSelector_, bool isDefaultMode);

    void setIntentNlpEngine(int sessionId, std::shared_ptr<TextGenerationEngine> intentEngine);

    void setVisionEngine(int sessionId, std::unique_ptr<ImageGenerationEngine> visionEngine);

    void setImageProcessingEngine(int sessionId, std::unique_ptr<ImageProcessEngine> imageProcessingEngine);

    void setEmbeddingEngine(int sessionId,
                            std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> embeddingEngine);

    assistant::ErrorCode handleRequest(int sessionId, const std::string &userMessage);
    void sendChatError(int sessionId, int errorCode, const std::string &errorMsg);

    void setChatSystemRole(int sessionId, int promptId, const std::string &promptName, std::string promptContent);
    void setChatContextMessages(int sessionId, int actorId, const std::string &contextMessages);

    void clearContext(int sessionId);

    void stopChat(int sessionId);

    void stopGenerateImage(int sessionId);

    void initIntentionRecognition(int sessionId);

private:
    void chatSystemRoleReplace(int sessionId, int promptId, std::string &promptContent);

    void callImageProcessResultCallback(int sessionId, const ai_engine::core_ai::vision::ImageResult &imageData);

private:
    GDBusConnection &connection_;
    std::map<int, assistant::session::DialogueSession> sessions_;
    std::map<int, RequestHandlerManager> requestHandlerManagers_;
};

#endif
