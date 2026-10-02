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

#ifndef SERVICES_ASSISTANT_FILE_INFERENCE_H
#define SERVICES_ASSISTANT_FILE_INFERENCE_H

#include <gio/gio.h>

#include <future>

#include "assistant/knowledgebase/knowledgebaseproxy.h"
#include "assistant/rag/ragserverproxy.h"
#include "fileinferenceinterface.h"

namespace assistant::session {
struct DialogueSession;
}

class FileInference : public FileInferenceInterface {
public:
    ~FileInference() override = default;

    std::future<DialogueData::FileSearchIdentifier> ragSearchText(const std::string& knowledgeBaseName,
                                                                  const std::string& text, bool useDefault) override;

    DialogueData::CallSuccessIdentifier fileContent(const std::string& knowledgeBaseName,
                                                    const std::string& filePath) override;

    std::future<DialogueData::KnowledgeBaseResult> createFileKnowledgeBase() override;

    void deleteFileKnowledgeBase(const std::string& knowledgeBaseName) override;

    DialogueData::FileSearchIdentifier updateFileKnowledgeBase(const std::string& knowledgeBaseName,
                                                               const std::string& docFilePath) override;

private:
    RagServerProxy::SimilaritySearchResults searchFileContent(const std::string& question,
                                                              const std::string& knowledgeBaseName,
                                                              RagErrorInfo& error);

private:
    RagServerProxy& ragServerProxy_{RagServerProxy::instance()};
};

// 知识库接口
class KnowledgebaseInference : public KnowledgeBaseInterface {
public:
    ~KnowledgebaseInference() override = default;

    int init() override;
    void chat(const std::string& question, const std::string& model) override;
    void stopChat() override;

private:
    advanced::knowledgebase::KnowledgeBaseProxy& kbServerProxy_{
        advanced::knowledgebase::KnowledgeBaseProxy::instance()};
};

#endif  // SERVICES_ASSISTANT_FILE_INFERENCE_H
