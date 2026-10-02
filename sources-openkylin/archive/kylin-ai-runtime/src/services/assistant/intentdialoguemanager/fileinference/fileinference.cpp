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

#include "fileinference.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <string>
#include <vector>

#include "assistant/handlers/requesthandler.h"
#include "assistant/message.h"
#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"

std::future<DialogueData::FileSearchIdentifier> FileInference::ragSearchText(const std::string& knowledgeBaseName,
                                                                             const std::string& text, bool useDefault) {
    return cpr::async([knowledgeBaseName, text, useDefault, this] {
        LOGD("text {}", text);
        LOGD("knowledgeBaseName {}", knowledgeBaseName);
        DialogueData::FileSearchIdentifier ret{};
        RagErrorInfo error;
        auto fileContents = searchFileContent(text, knowledgeBaseName, error);
        std::string contents;

        if (error.errorCode) {
            ret.successRes = false;
            ret.errorCode = int(assistant::Error::getErrorCodeFromFileKnowledgeBase(error.errorCode));
            ret.errorMessage = error.errorMessage;
            return ret;
        }
        for (const auto& fileContent : fileContents) {
            std::string filePath = fileContent.filePath;
            std::string content = fileContent.fileContent;
            contents += content + "。";
            if (useDefault) {
                const assistant::session::InformationSource informationSource =
                    assistant::message::getInformationSourceFromUserGuidePath(filePath);
                if (informationSource.id != -1) {
                    DialogueData::InformationSource source{
                        informationSource.name,
                        informationSource.url,
                        informationSource.id,
                    };

                    ret.sources.insert(source);
                }
            }
        }
        ret.successRes = true;
        ret.responseContent = contents;
        return ret;
    });
}

DialogueData::CallSuccessIdentifier FileInference::fileContent(const std::string& knowledgeBaseName,
                                                               const std::string& filePath) {
    DialogueData::CallSuccessIdentifier ret{};

    if (filePath.empty()) {
        ret.errorCode = int(assistant::ErrorCode::FileNotFound);
        ret.errorMessage = "文件路径为空";
        return ret;
    }

    if (knowledgeBaseName.empty()) {
        ret.errorCode = int(assistant::ErrorCode::ParamError);
        ret.errorMessage = "AiRuntime内部参数错误";
        return ret;
    }

    std::string message = ragServerProxy_.getFileContent(knowledgeBaseName, filePath);
    ret.successRes = true;
    ret.responseContent = message;
    return ret;
}

std::future<DialogueData::KnowledgeBaseResult> FileInference::createFileKnowledgeBase() {
    return cpr::async([this] {
        DialogueData::KnowledgeBaseResult result;
        RagErrorInfo error;
        result.knowledgeBaseName = ragServerProxy_.createKnowledgeBase(error);
        if (error.errorCode) {
            result.errorCode = int(assistant::Error::getErrorCodeFromFileKnowledgeBase(error.errorCode));
            result.errorMessage = error.errorMessage;
            LOGE("createFileKnowledgeBase error: {}", error.errorMessage);
        }
        return result;
    });
}

void FileInference::deleteFileKnowledgeBase(const std::string& knowledgeBaseName) {
    ragServerProxy_.deleteKnowledgeBase(knowledgeBaseName);
}

DialogueData::FileSearchIdentifier FileInference::updateFileKnowledgeBase(const std::string& knowledgeBaseName,
                                                                          const std::string& docFilePath) {
    auto result = ragServerProxy_.addFiles(knowledgeBaseName, {docFilePath});
    int errorCode = 0;
    if (result.errorCode) {
        errorCode = int(assistant::Error::getErrorCodeFromFileKnowledgeBase(result.errorCode));
    }
    return {errorCode, result.errorMessage};
}

RagServerProxy::SimilaritySearchResults FileInference::searchFileContent(const std::string& question,
                                                                         const std::string& knowledgeBaseName,
                                                                         RagErrorInfo& error) {
    return ragServerProxy_.similaritySearch(knowledgeBaseName, question, error);
}

int KnowledgebaseInference::init() { return kbServerProxy_.init(); }

void KnowledgebaseInference::chat(const std::string& question, const std::string& model) {
    auto callback = [this](const advanced::knowledgebase::KBChatResponse& result) {
        DialogueCallback::KBChatResult chatData{};
        chatData.model = result.model;

        if (result.errorCode) {
            chatData.errorCode = result.errorCode;
            chatData.response = "";
            chatData.isFinished = true;
            callChatCallback(chatData);
            return;
        }

        chatData.errorCode = 0;
        chatData.reasoningContent = result.reasoningContent;
        chatData.response = result.response;
        chatData.isFinished = result.isFinished;

        for (const auto& src : result.sources) {
            const auto info = assistant::message::getInformationSourceFromUserGuidePath(src.filePath);
            if (info.id != -1) {
                chatData.sources.insert(DialogueCallback::KBChatInfo{
                    info.name,
                    info.url,
                    info.id,
                });
            }
            if (src.toolIndex != -1) {
                DialogueCallback::KBChatInfo toolInfo{};
                toolInfo.toolCmd = src.toolPath;
                toolInfo.toolCmdId = src.toolIndex;
                chatData.sources.insert(std::move(toolInfo));
            }
        }

        callChatCallback(chatData);
    };
    kbServerProxy_.chat(question, model, callback);
}

void KnowledgebaseInference::stopChat() { kbServerProxy_.stopChat(); }
