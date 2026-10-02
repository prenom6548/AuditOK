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

#include "engineconfiguration.h"

#include <jsoncpp/json/json.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <fstream>

#include "utils/logger.h"

namespace config {

std::filesystem::path defaultPromptTemplatesPath = std::filesystem::path(CONFIGS_PATH) / "engines" / "ai-engines";
const std::string customPromptTemplatesPath = std::string(getenv("HOME")) + "/.config/kylin-ai/engines/ai-engines";

namespace utils {

Json::Value readJsonValue(const std::string &jsonFile) {
    std::ifstream ifs(jsonFile);
    if (!ifs.is_open()) {
        return Json::Value();
    }
    Json::Value root;
    ifs >> root;
    return root;
}

std::string readJsonMultiKeysStringValue(const Json::Value &value, std::initializer_list<std::string> keys) {
    Json::Value currentJsonObj = value;

    for (const auto &key : keys) {
        if (currentJsonObj.isMember(key)) {
            currentJsonObj = currentJsonObj[key];
        }
    }

    return currentJsonObj.isString() ? currentJsonObj.asString() : "";
}

}  // namespace utils

EngineConfiguration::EngineConfiguration() {
    loadAiEngineConfig();
    loadDbEngineConfig();
}

std::string config::EngineConfiguration::currentLmSpeechEngineName() const {
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, {"large-model", "current", "speech"});
}

std::string EngineConfiguration::currentLmNlpEngineName() const {
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, {"large-model", "current", "nlp"});
}

std::string EngineConfiguration::currentLmVisionEngineName() const {
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, {"large-model", "current", "vision"});
}

std::string EngineConfiguration::currentTmOcrEngineName() const {
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, {"traditional-model", "current", "ocr"});
}

std::string EngineConfiguration::currentVdEngineName() const {
    return utils::readJsonMultiKeysStringValue(dbEngineConfig_, {"traditional-model", "current", "ocr"});
}

std::string EngineConfiguration::promptTemplateConfigFile() const {
    return promptTemplatesPath + "/" + currentLmNlpEngineName() + "/prompt.json";
}

std::string EngineConfiguration::promptFilePath() const {
    return promptTemplatesPath + "/" + currentLmNlpEngineName() + "/prompts";
}

bool EngineConfiguration::createPrompt(int promptId, const std::string &engineName, const std::string &promptName,
                                       const std::string &promptContent) {
    std::string promptConfigFileDir = std::string(customPromptTemplatesPath) + "/" + engineName;
    std::string promptsFileDir = promptConfigFileDir + "/prompts";
    if (!fileUtil_.createDirectory(promptsFileDir)) {
        LOGE("EngineConfiguration createPrompt: failed to create directory !");
        return false;
    }

    std::string promptConfigFilePath = promptConfigFileDir + "/prompt.json";
    std::string promptFileName = promptName + std::to_string(promptId);
    if (!fileUtil_.createFile(promptConfigFilePath)) {
        return false;
    }
    if (!addConfigItem(promptConfigFilePath, promptId, promptFileName, promptName)) {
        return false;
    }

    std::string promptFilePath = promptConfigFileDir + "/prompts/" + promptFileName;
    if (!fileUtil_.createFileWriteContent(promptFilePath, promptContent)) {
        return false;
    }
    return true;
}

bool EngineConfiguration::deletePrompt(const std::string &engineName, int promptId) {
    std::string promptConfigFileDir = std::string(customPromptTemplatesPath) + "/" + engineName;
    std::string promptConfigFilePath = promptConfigFileDir + "/prompt.json";
    std::string promptFileName = getPromptNameById(promptConfigFilePath, promptId);
    if (promptFileName.empty()) {
        LOGE("EngineConfiguration deletePrompt: id does not exist !");
        return false;
    }

    std::string promptFilePath = promptConfigFileDir + "/prompts/" + promptFileName;
    if (!fileUtil_.deleteFile(promptFilePath)) {
        return false;
    }
    if (!removeConfigItemById(promptConfigFilePath, promptId)) {
        return false;
    }
    return true;
}

bool EngineConfiguration::updatePrompt(int promptId, const std::string &engineName, const std::string &promptName,
                                       const std::string &promptContent) {
    std::string promptConfigFileDir = std::string(customPromptTemplatesPath) + "/" + engineName;
    std::string promptConfigFilePath = promptConfigFileDir + "/prompt.json";
    std::string oldPromptFileName = getPromptNameById(promptConfigFilePath, promptId);
    if (oldPromptFileName.empty()) {
        LOGE("EngineConfiguration updatePrompt: id does not exist !");
        return false;
    }

    std::string oldPromptFilePath = promptConfigFileDir + "/prompts/" + oldPromptFileName;
    if (!fileUtil_.deleteFile(oldPromptFilePath)) {
        return false;
    }

    std::string newPromptFileName = promptName + std::to_string(promptId);
    std::string newPromptFilePath = promptConfigFileDir + "/prompts/" + newPromptFileName;

    if (!fileUtil_.createFileWriteContent(newPromptFilePath, promptContent)) {
        return false;
    }
    if (!updateConfigItemById(promptConfigFilePath, promptId, newPromptFileName, promptName)) {
        return false;
    }
    return true;
}

bool EngineConfiguration::readJsonFile(const std::string &filePath, std::vector<PromptConfigItem> &items) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGE("EngineConfiguration: failed to open file: {}", filePath);
        return false;
    }

    // 文件不能为空
    Json::Value root;
    file >> root;
    file.close();

    items.clear();
    for (const auto &item : root) {
        PromptConfigItem newItem;
        newItem.id = item["id"].asInt();
        newItem.promptFile = item["prompt-file"].asString();
        newItem.description = item["description"].asString();
        newItem.type = (promptType)item["type"].asInt();

        items.push_back(newItem);
    }
    return true;
}

void EngineConfiguration::writeJsonFile(const std::string &filePath, const std::vector<PromptConfigItem> &items) {
    Json::Value root(Json::arrayValue);

    for (const auto &item : items) {
        Json::Value newItem;
        newItem["id"] = item.id;
        newItem["prompt-file"] = item.promptFile;
        newItem["description"] = item.description;
        newItem["type"] = (int)item.type;

        root.append(newItem);
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        LOGE("EngineConfiguration: Failed to open file: {}", filePath);
        return;
    }

    file << root;
    file.close();
}

bool EngineConfiguration::addConfigItem(const std::string &filePath, int promptId, const std::string &promptFileName,
                                        const std::string &description) {
    std::vector<PromptConfigItem> items;

    if (!fileUtil_.isFileEmpty(filePath) && !readJsonFile(filePath, items)) {
        LOGE("EngineConfiguration addConfigItem: failed to read prompt config file {}", filePath);
        return false;
    }

    PromptConfigItem newItem;
    newItem.id = promptId;
    newItem.promptFile = promptFileName;
    newItem.description = description;
    newItem.type = promptType::ReadWrite;

    auto it = items.begin();
    while (it != items.end()) {
        if (it->id == promptId) {
            LOGE("EngineConfiguration addConfigItem: prompt config item with ID {} already exists.", promptId);
            return false;
        }
        if (it->id > promptId) {
            items.insert(it, newItem);
            writeJsonFile(filePath, items);
            return true;
        }
        ++it;
    }
    items.push_back(newItem);
    writeJsonFile(filePath, items);
    return true;
}

std::string EngineConfiguration::getPromptNameById(const std::string &filePath, int promptId) {
    std::vector<PromptConfigItem> items;
    if (!fileUtil_.isFileEmpty(filePath) && !readJsonFile(filePath, items)) {
        LOGE("EngineConfiguration getPromptNameById: failed to read prompt config file {}", filePath);
        return "";
    }

    auto it = items.begin();
    while (it != items.end()) {
        if (it->id == promptId) {
            return it->promptFile;
        }
        ++it;
    }
    return "";
}

bool EngineConfiguration::removeConfigItemById(const std::string &filePath, int promptId) {
    std::vector<PromptConfigItem> items;
    if (!fileUtil_.isFileEmpty(filePath) && !readJsonFile(filePath, items)) {
        LOGE("EngineConfiguration removeConfigItemById: failed to read prompt config file {}", filePath);
        return false;
    }

    auto it = items.begin();
    while (it != items.end()) {
        if (it->id == promptId) {
            items.erase(it);
            break;
        }
        ++it;
    }
    writeJsonFile(filePath, items);
    return true;
}

bool EngineConfiguration::updateConfigItemById(const std::string &filePath, int promptId,
                                               const std::string &promptFileName, const std::string &description) {
    std::vector<PromptConfigItem> items;
    if (!fileUtil_.isFileEmpty(filePath) && !readJsonFile(filePath, items)) {
        LOGE("EngineConfiguration updateConfigItemById: failed to read prompt config file {}", filePath);
        return false;
    }

    bool found = false;
    for (auto &item : items) {
        if (item.id == promptId) {
            item.promptFile = promptFileName;
            item.description = description;
            item.type = promptType::ReadWrite;
            found = true;
            break;
        }
    }

    if (!found) {
        LOGE("EngineConfiguration updateConfigItemById: prompt config update item with ID {} not found.", promptId);
        return false;
    }

    writeJsonFile(filePath, items);
    return true;
}

void EngineConfiguration::loadAiEngineConfig() { aiEngineConfig_ = utils::readJsonValue(aiEngineConfigFile_); }

void EngineConfiguration::loadDbEngineConfig() { dbEngineConfig_ = utils::readJsonValue(dbEngineConfigFile_); }

const std::string defaultPromptTemplateConfigFile(const std::string &engineName) {
    return std::string(defaultPromptTemplatesPath) + "/" + engineName + "/prompt.json";
}

const std::string defaultPromptFilePath(const std::string &engineName, const std::string &model) {
    return std::string(defaultPromptTemplatesPath) + "/" + engineName + "/" + model + "/prompts";
}

const std::string customPromptTemplateConfigFile(const std::string &engineName) {
    return std::string(customPromptTemplatesPath) + "/" + engineName + "/prompt.json";
}

const std::string customPromptFilePath(const std::string &engineName, const std::string &model) {
    return std::string(customPromptTemplatesPath) + "/" + engineName + "/prompts";
}

}  // namespace config
