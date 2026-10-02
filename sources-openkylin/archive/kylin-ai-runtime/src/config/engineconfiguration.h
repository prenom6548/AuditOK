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

#ifndef ENGINECONFIGURATION_H
#define ENGINECONFIGURATION_H

#include <jsoncpp/json/json.h>

#include <string>

#include "fileutil.h"
#include "prompttemplate.h"

namespace config {

const std::string defaultPromptTemplateConfigFile(const std::string &engineName);
const std::string defaultPromptFilePath(const std::string &engineName, const std::string &model);
const std::string customPromptTemplateConfigFile(const std::string &engineNamel);
const std::string customPromptFilePath(const std::string &engineName, const std::string &model);

using promptType = config::PromptTemplate::PromptContentAccessType;

class EngineConfiguration {
public:
    struct PromptConfigItem {
        int id;
        std::string promptFile;
        std::string description;
        promptType type;
    };

    EngineConfiguration();
    EngineConfiguration(const EngineConfiguration &other) = delete;
    EngineConfiguration &operator=(const EngineConfiguration &rhs) = delete;

    /// \note
    /// Lm - large model
    /// Tm - traditional model
    /// Vd - vector database
    std::string currentLmSpeechEngineName() const;
    std::string currentLmNlpEngineName() const;
    std::string currentLmVisionEngineName() const;
    std::string currentTmOcrEngineName() const;
    std::string currentVdEngineName() const;

    std::string promptTemplateConfigFile() const;
    std::string promptFilePath() const;

    bool createPrompt(int promptId, const std::string &engineName, const std::string &promptName,
                      const std::string &promptContent);
    bool deletePrompt(const std::string &engineName, int promptId);
    bool updatePrompt(int promptId, const std::string &engineName, const std::string &promptName,
                      const std::string &promptContent);

private:
    bool readJsonFile(const std::string &filePath, std::vector<PromptConfigItem> &items);
    void writeJsonFile(const std::string &filePath, const std::vector<PromptConfigItem> &items);
    bool addConfigItem(const std::string &filePath, int promptId, const std::string &promptFile,
                       const std::string &description);

    std::string getPromptNameById(const std::string &filePath, int promptId);
    bool removeConfigItemById(const std::string &filePath, int promptId);
    bool updateConfigItemById(const std::string &filePath, int promptId, const std::string &promptFileName,
                              const std::string &description);

    void loadAiEngineConfig();
    void loadDbEngineConfig();

private:
    FileUtil fileUtil_;
    Json::Value aiEngineConfig_;
    Json::Value dbEngineConfig_;
    const std::string aiEngineConfigFile_{"/etc/kylin-ai/engines/ai-engine/engine.json"};
    const std::string dbEngineConfigFile_{"/etc/kylin-ai/engines/db-engine/engine.json"};
    const std::string promptTemplatesPath{"/etc/kylin-ai/engines/ai-engine/prompt-templates"};
};

}  // namespace config

#endif  // ENGINECONFIGURATION_H
