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

#include "dialoguemanager.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

#include "assistant/message.h"
#include "assistant/session.h"
#include "config/prompttemplate.h"
#include "utils/datetime.h"
#include "utils/logger.h"
#include "utils/util.h"

DialogueManager::DialogueManager(GDBusConnection &connection) : connection_(connection) {}

DialogueManager::~DialogueManager() {
    for (auto &[sessionId, session] : sessions_) {
        session.intentionRecognition.stopDialogue();
        session.intentionRecognition.setOutputCallback(nullptr);
        if (session.imageProcessingEngine) {
            session.imageProcessingEngine->setImageBinaryResultCallback(nullptr);
            session.imageProcessingEngine->setMattingResultCallback(nullptr);
        }
    }
}

void DialogueManager::addSession(int sessionId) {
    sessions_[sessionId].sessionId = sessionId;
    requestHandlerManagers_[sessionId] = RequestHandlerManager{};
}

void DialogueManager::setBaseNlpEngine(int sessionId, std::shared_ptr<TextGenerationEngine> baseNlpEngine,
                                       EngineSelector &engineSelector_, bool isDefaultMode) {
    sessions_[sessionId].nlpEngine = baseNlpEngine;
    sessions_[sessionId].modelInference.initChatEngine(sessions_[sessionId].nlpEngine, engineSelector_, isDefaultMode);
}

void DialogueManager::setIntentNlpEngine(int sessionId, std::shared_ptr<TextGenerationEngine> intentEngine) {
    sessions_[sessionId].intentEngine = intentEngine;
    sessions_[sessionId].modelInference.initIntentEngine(sessions_[sessionId].intentEngine);
}

void DialogueManager::setVisionEngine(int sessionId, std::unique_ptr<ImageGenerationEngine> visionEngine) {
    sessions_[sessionId].textToImageEngine = std::move(visionEngine);
    sessions_[sessionId].imageHandler.initImageGenerationEngine(sessions_[sessionId].textToImageEngine);
}

void DialogueManager::setImageProcessingEngine(int sessionId,
                                               std::unique_ptr<ImageProcessEngine> imageProcessingEngine) {
    if (imageProcessingEngine == nullptr) {
        return;
    }
    imageProcessingEngine->setMattingResultCallback(
        [sessionId, this](const ai_engine::core_ai::vision::ImageResult &imageData) {
            this->callImageProcessResultCallback(sessionId, imageData);
        });
    imageProcessingEngine->setImageBinaryResultCallback(
        [sessionId, this](const ai_engine::core_ai::vision::ImageResult &imageData) {
            callImageProcessResultCallback(sessionId, imageData);
        });
    sessions_[sessionId].imageProcessingEngine = std::move(imageProcessingEngine);
}

void DialogueManager::setEmbeddingEngine(
    int sessionId, std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> embeddingEngine) {
    if (embeddingEngine == nullptr) {
        return;
    }
    sessions_[sessionId].embeddingEngine = std::move(embeddingEngine);
#ifdef SUPPORT_VECTOR_DB
    sessions_[sessionId].textVectorizationInference.initEmbeddingEngine(sessions_[sessionId].embeddingEngine);
#endif
}

assistant::ErrorCode DialogueManager::handleRequest(int sessionId, const std::string &userMessage) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::onHandleChat1: Session not found.{}", sessionId);
        const std::string errorMessage = assistant::message::createErrorMessageResult(1, "Session not found.");
        RequestHandler::sendChatResult(&connection_, sessionId, errorMessage);
        return assistant::ErrorCode::ParamError;
    }
    if (requestHandlerManagers_.find(sessionId) == requestHandlerManagers_.end()) {
        LOGE("Assistant::onHandleChat1: Session not found.{}", sessionId);
        const std::string errorMessage = assistant::message::createErrorMessageResult(1, "Session not found.");
        RequestHandler::sendChatResult(&connection_, sessionId, errorMessage);
        return assistant::ErrorCode::ParamError;
    }

    return requestHandlerManagers_[sessionId].handleRequest(&connection_, userMessage, sessions_[sessionId]);
}

void DialogueManager::sendChatError(int sessionId, int errorCode, const std::string &errorMsg) {
    const std::string errorMessage = assistant::message::createErrorMessageResult(errorCode, errorMsg);
    RequestHandler::sendChatResult(&connection_, sessionId, errorMessage);
}

void DialogueManager::setChatSystemRole(int sessionId, int promptId, const std::string &promptName,
                                        std::string promptContent) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::onHandleChat2: Session not found.{}", sessionId);
        return;
    }
    chatSystemRoleReplace(sessionId, promptId, promptContent);
    sessions_[sessionId].intentionRecognition.setScenarioInstructionPrompt(promptId, promptContent, promptName);
}

void DialogueManager::setChatContextMessages(int sessionId, int actorId, const std::string &contextMessages) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::setChatContextMessages: Session not found.{}", sessionId);
        return;
    }
    sessions_[sessionId].intentionRecognition.loadHistoryDialogue(contextMessages);
    sessions_[sessionId].intentionRecognitionFirstRound = false;
    // TODO 需要定义广义的历史消息的结构体，包含系统操作、文生图结果、RAG结果、提示词ID、提示词信息等
    //      需要意图识别提供设置历史消息的接口
}

void DialogueManager::clearContext(int sessionId) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::clearContext3: Session not found.{}", sessionId);
        return;
    }
    sessions_[sessionId].intentionRecognition.clearHistoryDialogue();
    sessions_[sessionId].intentionRecognitionFirstRound = true;
}

void DialogueManager::stopChat(int sessionId) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::stopChat4: Session not found.{}", sessionId);
        return;
    }
    if (sessions_[sessionId].nlpEngine) {
        sessions_[sessionId].intentionRecognition.stopDialogue();
    }
    if (sessions_[sessionId].intentEngine) {
        sessions_[sessionId].intentEngine->stopChat();
    }
}

void DialogueManager::stopGenerateImage(int sessionId) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::stopGenerateImage: Session not found.{}", sessionId);
        return;
    }
    if (sessions_[sessionId].textToImageEngine) {
        sessions_[sessionId].textToImageEngine->stopImageGeneration();
    }
}

void DialogueManager::initIntentionRecognition(int sessionId) {
    if (sessions_.find(sessionId) == sessions_.end()) {
        LOGE("Assistant::onHandleChat: Session not found.{}", sessionId);
        return;
    }
    sessions_[sessionId].intentionRecognition.setOutputCallback(
        [sessionId, this](DialogueCallback::Type type, const std::string &message) {
            RequestHandler::sendChatResult(&this->connection_, sessionId, message);
        });

    sessions_[sessionId].imageHandler.initImageProcessEngine(sessions_[sessionId].imageProcessingEngine);

    sessions_[sessionId].intentionRecognition.setModelInferenceInterface(&sessions_[sessionId].modelInference);
#ifdef SUPPORT_VECTOR_DB
    sessions_[sessionId].intentionRecognition.setFileInferenceInterface(&sessions_[sessionId].fileInference);
    // 文本向量化
    sessions_[sessionId].intentionRecognition.setTextEmbeddingInterface(
        &sessions_[sessionId].textVectorizationInference);
    sessions_[sessionId].intentionRecognition.setKnowledgeBaseInterface(&sessions_[sessionId].knowledgeBaseInference);
#endif
    sessions_[sessionId].intentionRecognition.setImageHandlerInterface(&sessions_[sessionId].imageHandler);
    sessions_[sessionId].intentionRecognition.setSystemOperationInterface(&sessions_[sessionId].systemOperation);
}

void DialogueManager::chatSystemRoleReplace(int sessionId, int promptId, std::string &promptContent) {
    if (sessions_[sessionId].nlpEngine == nullptr) {
        return;
    }

    // 此函数不必判断sessionId是否存在，因为调用之前判断过
    std::set<config::PromptTemplate::PromptId> needReplacePromptIdSet{
        config::PromptTemplate::PromptId::TodoListGeneration,            // 代办
        config::PromptTemplate::PromptId::WeeklyReportGeneration,        // 周报
        config::PromptTemplate::PromptId::MeetingInformationExtraction,  // 日程
    };
    auto thePromptId = config::PromptTemplate::PromptId(promptId);
    if (needReplacePromptIdSet.find(thePromptId) == needReplacePromptIdSet.end()) {
        return;
    }

    // 端侧模型格式举例如下：
    // 日期格式：2025-01-10
    // 星期格式：星期五；（不能是周几，不能是英文）
    // 年份格式：2025

    // 云端和自定义模型格式举例如下：
    // 日期格式：2025-01-10 14:41:07
    // 星期格式：星期五；（不能是周几，不能是英文）
    // 年份格式：2025

    DateTime dateTime{};

    // 替换日期
    std::string oldDateTime = "{curDate}";
    std::string newDateTime;

    if (sessions_[sessionId].nlpEngine->isCloud() || sessions_[sessionId].nlpEngine->isCustomModelEngine()) {
        // 云端和自定义模型
        newDateTime = fmt::format("{}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", dateTime.year(), dateTime.month(),
                                  dateTime.day(), dateTime.hour(), dateTime.min(), dateTime.sec());
    } else {
        // 端侧模型
        newDateTime = fmt::format("{}-{:02d}-{:02d}", dateTime.year(), dateTime.month(), dateTime.day());
    }

    if (size_t pos = promptContent.find(oldDateTime); pos != std::string::npos) {
        promptContent.replace(pos, oldDateTime.length(), newDateTime);
    }

    // 替换星期
    std::string oldWeekDay = "{curWeek}";
    std::string newWeekDay = fmt::format("星期{}", dateTime.weekday());
    if (size_t pos = promptContent.find(oldWeekDay); pos != std::string::npos) {
        promptContent.replace(pos, oldWeekDay.length(), newWeekDay);
    }

    // 替换年份
    std::string oldYear = "{curYear}";
    std::string newYear = fmt::format("{}", dateTime.year());
    if (size_t pos = promptContent.find(oldYear); pos != std::string::npos) {
        promptContent.replace(pos, oldYear.length(), newYear);
    }
}

void DialogueManager::callImageProcessResultCallback(int sessionId,
                                                     const ai_engine::core_ai::vision::ImageResult &result) {
    assistant::message::ImageData imageData{};
    imageData.success = result.success;
    imageData.imageBase64Data = result.imageBase64Data;
    imageData.imageFormat = result.imageFormat;
    imageData.modelName = result.modelName;
    imageData.errorCode = result.error.getCode();
    imageData.errorMessage = result.error.getMessage();
    RequestHandler::sendChatResult(&connection_, sessionId,
                                   assistant::message::createImageDataMessage(imageData, false));
}