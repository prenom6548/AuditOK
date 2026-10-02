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

#ifndef SESSION_H
#define SESSION_H

#include <kylin-ai/ai-engine/embeddingengine.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>

#include <memory>
#include <string>
#ifdef SUPPORT_VECTOR_DB
#include "intentdialoguemanager/fileinference/fileinference.h"
#include "intentdialoguemanager/textembedding/textembedding.h"
#endif
#include "intentdialoguemanager/imagehandler/imagehandler.h"
#include "intentdialoguemanager/intentionrecognition.h"
#include "intentdialoguemanager/modelinference/modelinference.h"
#include "intentdialoguemanager/systemoperation/systemoperation.h"
#include "sessionbase.h"

namespace assistant::session {

struct InformationSource {
    std::string name;
    std::string url;
    int id{-1};

    bool operator<(const InformationSource &other) const { return id < other.id; }
};

struct DialogueSession {
    int sessionId{-1};
    std::shared_ptr<TextGenerationEngine> nlpEngine{nullptr};
    std::shared_ptr<TextGenerationEngine> intentEngine{nullptr};
    std::shared_ptr<ImageGenerationEngine> textToImageEngine{nullptr};
    std::shared_ptr<ImageProcessEngine> imageProcessingEngine{nullptr};
    std::shared_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> embeddingEngine{nullptr};
    ModelInference modelInference;
#ifdef SUPPORT_VECTOR_DB
    FileInference fileInference;
    KnowledgebaseInference knowledgeBaseInference;
    // 文本向量化
    TextVectorizationInference textVectorizationInference;
#endif
    ImageHandler imageHandler;
    SystemOperation systemOperation;
    bool intentionRecognitionFirstRound{true};
    IntentionRecognition intentionRecognition;
};

}  // namespace assistant::session

#endif