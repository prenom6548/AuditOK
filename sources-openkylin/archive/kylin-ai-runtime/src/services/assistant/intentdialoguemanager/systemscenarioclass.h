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

#ifndef SYSTEM_SCENARIO_H
#define SYSTEM_SCENARIO_H

#include <jsoncpp/json/json.h>

#include <filesystem>
#include <map>
#include <string>

#include "dialoguemessage.h"

// 定义基类B
class BaseFunction {
public:
    // 构造函数
    BaseFunction();
    ~BaseFunction();
    std::string getPrompt(const std::string &intentPromptFile);
    static Json::Value loadJsonFile(const std::string &jsonFile);

    std::string readTxtFile(const std::string &file_path);

    void getpromptReplaceText(std::string &newReplaceText);

private:
    std::string intentPrompt_;
    std::string promptReplaceText_;
};

// 会话记忆
class DialogueMemory : public BaseFunction {
public:
    bool isJsonLoaded = false;

public:
    DialogueMemory();
    ~DialogueMemory();

    void loadDialogueData(std::vector<std::filesystem::path> foundDialogueFile, const std::string &targetString,
                          std::string &dialogueId);

    void updateHistorySessionToFile(const bool &isFirstRound, const DialogueData::Dialogue &currentDialogue);

    // // 根据当前轮次意图检索历史对话
    DialogueData::Intent retrieveHistoryDialogue(DialogueData::Dialogue &allHistoryDialogue,
                                                 DialogueData::Intent &curTurnIntent);

    Json::Value getDialogueHistoryData();
    std::vector<DialogueData::DialogueTurn> getRelevantHistoryData();

private:
    Json::Value historyDialogueMerge;
    std::vector<DialogueData::DialogueTurn> relevantHistoryData;
};

void findFilesWithString(const std::filesystem::path &fileDirectory, const std::string &targetString,
                         std::string &dialogueId, DialogueMemory &dialogueMemory);

#endif  // SYSTEM_SCENARIO_H
