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

#include <memory>
#include <string>
#include <vector>

class IntentSlotRules;

struct ExtractedSlot {
    std::string slotName;
    std::string slotValue;
    bool isRequired;
};

class SlotExtractor {
public:
    SlotExtractor(std::shared_ptr<IntentSlotRules> rules);
    ~SlotExtractor();
    std::vector<ExtractedSlot> extractSlots(const std::string& user_input, const std::string& predict_intent_name);

private:
    std::shared_ptr<IntentSlotRules> intentSlotRules_;
    class SlotExtractorImpl;
    std::unique_ptr<SlotExtractorImpl> pImpl_;
};