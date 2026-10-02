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

#ifndef SERVICES_ASSISTANT_FILE_INFERENCE_INTERFACE_H
#define SERVICES_ASSISTANT_FILE_INFERENCE_INTERFACE_H

#include <future>
#include <string>

#include "assistant/intentdialoguemanager/dialoguemessage.h"

class FileInferenceInterface {
public:
    virtual ~FileInferenceInterface() = default;

    virtual std::future<DialogueData::FileSearchIdentifier> ragSearchText(const std::string& knowledgeBaseName,
                                                                          const std::string& text, bool useDefault) = 0;

    // 解析文件内容
    virtual DialogueData::CallSuccessIdentifier fileContent(const std::string& knowledgeBaseName,
                                                            const std::string& docFilePath) = 0;

    // 创建知识库
    virtual std::future<DialogueData::KnowledgeBaseResult> createFileKnowledgeBase() = 0;

    // 删除知识库
    virtual void deleteFileKnowledgeBase(const std::string& knowledgeBaseName) = 0;

    // 更新知识库
    virtual DialogueData::FileSearchIdentifier updateFileKnowledgeBase(const std::string& knowledgeBaseName,
                                                                       const std::string& docFilePath) = 0;
};

class KnowledgeBaseInterface {
public:
    virtual ~KnowledgeBaseInterface() = default;

    virtual int init() = 0;

    void setKBChatStreamCallback(const DialogueCallback::KBChatStreamCallback& callback) {
        chatStreamCallback_ = callback;
    }

    virtual void chat(const std::string& question, const std::string& model) = 0;
    virtual void stopChat() = 0;

protected:
    void callChatCallback(const DialogueCallback::KBChatResult& chatData) const {
        if (chatStreamCallback_ != nullptr) {
            chatStreamCallback_(chatData);
        }
    }

protected:
    DialogueCallback::KBChatStreamCallback chatStreamCallback_{nullptr};
};

#endif  // SERVICES_ASSISTANT_FILE_INFERENCE_INTERFACE_H
