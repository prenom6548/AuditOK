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

#include <map>
#include <set>
#include <string>
#include <vector>

struct SlotDefinition {
    std::string name;
    std::string description;
    std::vector<std::string> values;
    bool required{false};
};

struct IntentDefinition {
    std::string primary_intent_name;
    std::string description;
    std::vector<SlotDefinition> slots;
    std::map<std::string, std::vector<std::string>> entities;
    double score_threshold;
};

class IntentSlotRules {
public:
    IntentSlotRules();
    ~IntentSlotRules() = default;
    void loadIntentSlotData(const std::string& file_path = "");
    std::map<std::string, IntentDefinition> getIntentSlotDataMap() const { return intentSlotDataMap_; }
    std::map<std::string, std::map<std::string, std::vector<std::string>>> getIncludeEntitiesMap() const {
        return includeEntitiesMap_;
    }
    std::map<std::string, std::vector<std::string>> getExcludeEntitiesMap() const { return excludeEntitiesMap_; }
    std::vector<std::string> getIntentListWithoutSlots() const { return withoutSlotIntents_; }

private:
    std::set<std::string> primaryIntentList_;
    std::vector<std::string> intentList_;
    std::map<std::string, IntentDefinition> intentSlotDataMap_;
    std::map<std::string, std::map<std::string, std::vector<std::string>>> includeEntitiesMap_;
    std::map<std::string, std::vector<std::string>> excludeEntitiesMap_;
    std::vector<std::string> withoutSlotIntents_;
};