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

#include "promptmanager.h"

#include <filesystem>
#include <memory>

PromptManager::PromptManager(const std::string &engineName, const std::string &model)
    : engineName_(engineName), model_(model) {
    initPromptTemplate();
}

void PromptManager::initPromptTemplate() {
    if (modelHasPrompts()) {
        defaultPrompt_ = std::make_shared<config::PromptTemplate>(config::defaultPromptTemplateConfigFile(engineName_),
                                                                  config::defaultPromptFilePath(engineName_, model_));
    } else {
        defaultPrompt_ =
            std::make_shared<config::PromptTemplate>(config::defaultPromptTemplateConfigFile(customEngineName_),
                                                     config::defaultPromptFilePath(customEngineName_, customModel_));
    }

    customPrompt_ = std::make_shared<config::PromptTemplate>(config::customPromptTemplateConfigFile(engineName_),
                                                             config::customPromptFilePath(engineName_, model_));
}

bool PromptManager::modelHasPrompts() {
    std::string path = config::defaultPromptFilePath(engineName_, model_);
    return std::filesystem::exists(path) && std::filesystem::is_directory(path) &&
           std::filesystem::directory_iterator(path) != std::filesystem::directory_iterator();
}

config::PromptTemplate::Prompt PromptManager::prompt(const int promptId) const {
    int minPromptId = int(config::PromptTemplate::PromptId::Default);
    int maxPromptId = config::PromptTemplate::maxDefaultPromptId;
    if (promptId >= minPromptId && promptId <= maxPromptId) {
        return defaultPrompt_->prompt(promptId);
    } else {
        return customPrompt_->prompt(promptId);
    }
}

std::vector<config::PromptTemplate::Prompt> PromptManager::prompts() {
    std::vector<config::PromptTemplate::Prompt> defaultPrompts = defaultPrompt_->prompts();
    std::vector<config::PromptTemplate::Prompt> customPrompts = customPrompt_->prompts();

    std::vector<config::PromptTemplate::Prompt> allPrompts = defaultPrompts;
    auto it = customPrompts.begin();
    while (it != customPrompts.end()) {
        allPrompts.push_back(*it);
        ++it;
    }
    return allPrompts;
}

int PromptManager::createPrompt(const std::set<std::string> &engineNameVec, const std::string &promptName,
                                const std::string &promptContent) {
    int promptId = customPrompt_->createPrompt(promptName, promptContent);
    for (const auto &engineName : engineNameVec) {
        if (!engineConfiguration_.createPrompt(promptId, engineName, promptName, promptContent)) {
            return -1;
        }
    }
    return promptId;
}

bool PromptManager::deletePrompt(const std::set<std::string> &engineNameVec, int promptId) {
    if (!customPrompt_->deletePrompt(promptId)) {
        return false;
    }
    for (const auto &engineName : engineNameVec) {
        if (!engineConfiguration_.deletePrompt(engineName, promptId)) {
            return false;
        }
    }
    return true;
}

bool PromptManager::updatePrompt(const std::set<std::string> &engineNameVec, int promptId, std::string promptName,
                                 const std::string &promptContent) {
    if (!customPrompt_->updatePrompt(promptId, promptName, promptContent)) {
        return false;
    }
    for (const auto &engineName : engineNameVec) {
        if (!engineConfiguration_.updatePrompt(promptId, engineName, promptName, promptContent)) {
            return false;
        }
    }

    return true;
}
