/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#pragma once

#include <jsoncpp/json/json.h>

#include <map>
#include <string>

#include "intenttypes.h"
#include "rulebasedintentretriever.h"
#include "slotextractor.h"

class TextEmbeddingInterface;

class IntentSlotRecognizer {
public:
    IntentSlotRecognizer();
    ~IntentSlotRecognizer() = default;

    IntentRecognitionResult recognizeIntentAndSlots(const std::string& user_input);
    void setTextEmbeddingInterface(TextEmbeddingInterface* interface);
    Json::Value getSupplementedCandidateIntents(const Json::Value& baseIntents) const;

private:
    Json::Value buildCandidateIntentSlots(const std::vector<intent::IntentResult>& predict_intents);

private:
    std::shared_ptr<IntentSlotRules> intentSlotRules_;
    RuleBasedIntentRetriever ruleBasedIntentRetriever_;
    SlotExtractor slotExtractor_;
    const std::vector<std::string> candidateIntentSupplementaryList = {
    "闲聊", "百科知识问答"};

};
