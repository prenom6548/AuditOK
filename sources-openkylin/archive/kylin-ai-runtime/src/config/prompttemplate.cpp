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

#include "prompttemplate.h"

#include <jsoncpp/json/json.h>

#include <fstream>

#include "utils/logger.h"

namespace config {

std::string readPrompt(const std::string &file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return content;
}

PromptTemplate::PromptTemplate(const std::string &promptConfigFile, const std::string &promptFilePath) {
    LOGI("Prompt config file: {}, prompt file path:{}", promptConfigFile, promptFilePath);
    dissectPrompts(promptConfigFile, promptFilePath);
}

PromptTemplate::Prompt PromptTemplate::prompt(const int id) const {
    auto iter = prompts_.find(id);
    if (iter != prompts_.end()) {
        return iter->second;
    }
    return Prompt{};
}

std::vector<PromptTemplate::Prompt> PromptTemplate::prompts() {
    std::vector<Prompt> allPrompts;
    for (const auto &pair : prompts_) {
        allPrompts.push_back(Prompt{pair.second.id, pair.second.name, pair.second.content, pair.second.type});
    }

    return allPrompts;
}

int PromptTemplate::createPrompt(const std::string &name, const std::string &content) {
    int promptId = getIdleId();
    prompts_[promptId] = Prompt{promptId, name, content, PromptContentAccessType::ReadWrite};
    return promptId;
}

bool PromptTemplate::deletePrompt(int promptId) {
    if (prompts_.find(promptId) == prompts_.end()) {
        LOGE("Failed to delete prompt, {} not exists !", promptId);
        return false;
    }
    prompts_.erase(promptId);
    return true;
}

bool PromptTemplate::updatePrompt(int promptId, const std::string &name, const std::string &content) {
    if (prompts_.find(promptId) == prompts_.end()) {
        LOGE("Failed to update prompt, {} not exists !", promptId);
        return false;
    }
    prompts_[promptId] = Prompt{promptId, name, content, config::PromptTemplate::ReadWrite};
    return true;
}

void PromptTemplate::dissectPrompts(const std::string &promptFile, const std::string &promptFilePath) {
    std::ifstream ifs(promptFile);
    if (!ifs.is_open()) {
        return;
    }
    Json::Value root;
    ifs >> root;
    if (!root.isArray()) {
        LOGE("Prompt file is not an array of objects.");
        return;
    }
    for (const auto &item : root) {
        if (!item.isObject()) {
            LOGE("Prompt file is not an array of objects.");
            continue;
        }

        if (!item.isMember("id") || !item.isMember("prompt-file") || !item.isMember("description") ||
            !item.isMember("type")) {
            LOGE(
                "Prompt file is not an array of objects with id, prompt "
                "and description.");
            continue;
        }

        if (!item["id"].isInt() || !item["prompt-file"].isString() || !item["description"].isString() ||
            !item["type"].isInt()) {
            LOGE("Prompt file's key type is incorrect.");
            continue;
        }

        int id = item["id"].asInt();
        const std::string promptFileName = item["prompt-file"].asString();
        if (promptFileName.empty()) {
            continue;
        }
        const std::string promptFileDescription = item["description"].asString();
        if (promptFileDescription.empty()) {
            continue;
        }

        const std::string promptContent = readPrompt(promptFilePath + "/" + promptFileName);
        prompts_[id] = {id, promptFileDescription, promptContent, (PromptContentAccessType)item["type"].asInt()};
    }
}

int PromptTemplate::getIdleId() {
    int id = 101;
    while (1) {
        if (prompts_.find(id) == prompts_.end()) {
            return id;
        }
        ++id;
    }
}

}  // namespace config
