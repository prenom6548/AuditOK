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

#ifndef PROMPTMANAGER_H
#define PROMPTMANAGER_H

#include <memory>
#include <set>
#include <vector>

#include "config/engineconfiguration.h"
#include "config/prompttemplate.h"

class PromptManager {
public:
    PromptManager(const std::string &engineName, const std::string &model);

    config::PromptTemplate::Prompt prompt(const int promptId) const;

    std::vector<config::PromptTemplate::Prompt> prompts();
    int createPrompt(const std::set<std::string> &engineNameVec, const std::string &promptName,
                     const std::string &promptContent);
    bool deletePrompt(const std::set<std::string> &engineNameVec, int promptId);
    bool updatePrompt(const std::set<std::string> &engineNameVec, int promptId, std::string promptName,
                      const std::string &promptContent);

private:
    void initPromptTemplate();
    bool modelHasPrompts();

private:
    // 1-100默认提示词Id
    std::shared_ptr<config::PromptTemplate> defaultPrompt_;
    // 大于100为自定义提示词Id
    std::shared_ptr<config::PromptTemplate> customPrompt_;

    std::string engineName_;
    std::string model_;
    config::EngineConfiguration engineConfiguration_;

    const std::string customEngineName_ = "custom";
    const std::string customModel_ = "custom";
};

#endif  // PROMPTMANAGER_H
