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

#include "systemscenarioclass.h"

#include <jsoncpp/json/json.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "dialoguemessage.h"
#include "utils/logger.h"
#include "utils/util.h"
#include "variabletypeconversion.h"

using namespace DialogueData;
using namespace DialogueConfig;
using namespace TypeTrans;

// baseFunction
BaseFunction::BaseFunction() {};
BaseFunction::~BaseFunction() {};

std::string BaseFunction::getPrompt(const std::string &intentPromptFile) {
    std::string promptStr;
    std::ifstream file(intentPromptFile);
    if (!file.is_open()) {
        LOGE("Failed to open prompt file: {}", intentPromptFile);
        return promptStr;
    }
    LOGD("{}", intentPromptFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    promptStr = buffer.str();
    const std::string replacedString{"{intent-definitions}"};

    size_t foundPos = promptStr.find(replacedString);
    if (foundPos != std::string::npos) {
        promptStr.replace(promptStr.find(replacedString), replacedString.size(), promptReplaceText_);
    }
    LOGI("promptStr = {}", promptStr);
    return promptStr;
}

Json::Value BaseFunction::loadJsonFile(const std::string &jsonFilePath) {  //   arrayValue
    // 创建输入文件流对象
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        LOGE("Open Json file failed: {}", jsonFilePath);
        return Json::Value();
    }

    Json::Reader jsonReader;
    Json::Value rootValue;

    bool success = jsonReader.parse(jsonFile, rootValue);
    if (!success) {
        LOGE("The json file failed to be parsed. {}", jsonFilePath);
    }

    // 关闭文件流，读取操作已完成
    jsonFile.close();

    // Json::Value vJson;
    Json::StreamWriterBuilder jswBuilder;
    jswBuilder["emitUTF8"] = true;
    std::unique_ptr<Json::StreamWriter> jsWriter(jswBuilder.newStreamWriter());

    std::ostringstream os;
    jsWriter->write(rootValue, &os);
    std::string strJson = os.str();

    return rootValue;
}

std::string BaseFunction::readTxtFile(const std::string &file_path) {
    std::ifstream txt_file(file_path);
    if (txt_file.is_open()) {
        std::stringstream buffer;
        buffer << txt_file.rdbuf();
        std::string file_content = buffer.str();
        txt_file.close();
        return file_content;
    } else {
        std::cerr << "无法打开txt文件: " << file_path << std::endl;
        return "无法解析txt文件的内容";
    }
}

// 会话记忆
DialogueMemory::DialogueMemory() {};
DialogueMemory::~DialogueMemory() {};

void DialogueMemory::loadDialogueData(std::vector<std::filesystem::path> foundDialogueFile,
                                      const std::string &targetString, std::string &dialogueId) {
    Json::Value tempDialogueData;  // 存储筛选出的文件的内容

    // 输出找到的文件路径
    if (!foundDialogueFile.empty()) {
        historyDialogueMerge["dialogue_id"] = dialogueId;

        // for (const auto& path : foundFiles) {
        for (size_t i = 0; i < foundDialogueFile.size(); ++i) {
            LOGI("\t找到的文件路径: {}", foundDialogueFile[i].string());

            tempDialogueData = loadJsonFile(foundDialogueFile[i].string());

            if (i == 0) {
                historyDialogueMerge["dialogue"] = tempDialogueData["dialogue"];
            } else {
                if (tempDialogueData.isObject() && (tempDialogueData["dialogue"].isArray())) {
                    const Json::Value &dialogueArray = tempDialogueData["dialogue"];

                    LOGI("\tThe value of 'dialogue' key is:");
                    for (size_t i = 0; i < dialogueArray.size(); ++i) {
                        // 明确将索引操作转换为使用整数索引获取数组元素
                        const Json::Value &turnValue = dialogueArray[Json::ArrayIndex(i)];
                        historyDialogueMerge["dialogue"].append(turnValue);
                    }
                }
            }
        }

    } else {
        LOGI("\t`foundFiles`为空，未找到符合【{}】的文件.", targetString);
    }

    isJsonLoaded = true;
}

Json::Value DialogueMemory::getDialogueHistoryData() { return historyDialogueMerge; }

std::vector<DialogueData::DialogueTurn> DialogueMemory::getRelevantHistoryData() { return relevantHistoryData; }

void DialogueMemory::updateHistorySessionToFile(const bool &isFirstRound,
                                                const DialogueData::Dialogue &curTurnDialogue) {
    if (curTurnDialogue.dialogue.empty()) {
        LOGI("当前对话轮次信息为空，无法进行更新操作。");
        return;
    }

    LOGI("正在异步更新历史会话文件，当前轮次信息: {}", curTurnDialogue.dialogue_id);

    std::filesystem::path newDialogueFile =
        DialogueConfig::dialogueHistoryFile / (curTurnDialogue.dialogue_id + ".json");
    Json::Value tempTurnAll;
    Json::Value writeJson;

    tempTurnAll["dialogue_id"] = curTurnDialogue.dialogue_id;

    std::vector<Json::Value> tempTurnMerge;

    for (size_t i = 0; i < curTurnDialogue.dialogue.size(); ++i) {
        Json::Value tempTurn;
        // Json::Value currentSlot;
        tempTurn["turn_num"] = curTurnDialogue.dialogue[i].turn_num;
        tempTurn["first_turn_flag"] = curTurnDialogue.dialogue[i].first_turn_flag;
        tempTurn["role"] = curTurnDialogue.dialogue[i].role;
        tempTurn["user_input"] = curTurnDialogue.dialogue[i].user_input.c_str();
        tempTurn["intent_category"] = curTurnDialogue.dialogue[i].intent_category.c_str();
        tempTurn["system_response"] = systemResponseToJsonValue(curTurnDialogue.dialogue[i].system_response);
        tempTurn["turn_keywords"] = systemResponseToJsonValue(curTurnDialogue.dialogue[i].turn_keywords);

        tempTurn["dialogue_state"]["slots"] = slotToJsonValue(curTurnDialogue.dialogue[i].dialogue_state.slots);
        tempTurn["dialogue_state"]["system_acts"] =
            systemResponseToJsonValue(curTurnDialogue.dialogue[i].dialogue_state.system_acts);
        tempTurn["dialogue_state"]["user_acts"] =
            systemResponseToJsonValue(curTurnDialogue.dialogue[i].dialogue_state.user_acts);

        tempTurnMerge.push_back(tempTurn);
    }

    for (const auto &element : tempTurnMerge) {
        tempTurnAll["dialogue"].append(element);
    }

    if (isFirstRound) {  // 第一轮，创建新的json文件
        writeJson = tempTurnAll;
        LOGI("第一轮，创建新的json文件");

    } else {
        Json::Value modifiedHistoryDialogueMerge = getDialogueHistoryData();

        // 将tempTurnAll更新到historyDialogueMerge中
        for (size_t i = 0; i < tempTurnAll["dialogue"].size(); ++i) {
            // 明确将索引操作转换为使用整数索引获取数组元素
            const Json::Value &turnValue = tempTurnAll["dialogue"][Json::ArrayIndex(i)];
            modifiedHistoryDialogueMerge["dialogue"].append(turnValue);
        }

        modifiedHistoryDialogueMerge["dialogue_id"] = curTurnDialogue.dialogue_id;
        writeJson = modifiedHistoryDialogueMerge;
    }

    // 打开文件，写入当前轮次会话
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;

    std::ofstream outputFile(newDialogueFile);
    if (outputFile.is_open()) {
        outputFile << Json::writeString(builder, writeJson);

        outputFile.close();
        LOGI("新的json文件已创建并写入成功。");
    } else {
        LOGE("无法打开文件以创建新的json文件: {}", newDialogueFile.string());
    }
}

DialogueData::Intent DialogueMemory::retrieveHistoryDialogue(DialogueData::Dialogue &allHistoryDialogue,
                                                             DialogueData::Intent &curTurnIntent) {
    int count = 0;
    for (const auto &perTurnDialogue : allHistoryDialogue.dialogue) {
        if (perTurnDialogue.intent_category == curTurnIntent.name && count < DialogueConfig::maxRetrieveRounds) {
            relevantHistoryData.push_back(perTurnDialogue);

            // 补充槽位缺失的信息,遍历Intent的slot,如果有空的,就从历史中查找并赋值,如果不为空则不处理
            for (auto &eleSlot : curTurnIntent.slots) {
                if (eleSlot.values.empty()) {
                    for (auto &perSlot : perTurnDialogue.dialogue_state.slots) {
                        if (perSlot.name == eleSlot.name && !perSlot.values.empty()) {
                            eleSlot.values = perSlot.values;
                        }
                    }
                }
            }
            ++count;
        }
    }

    return curTurnIntent;
}

// 函数用于遍历目录并查找包含指定字符串的文件，返回找到的文件路径列表
void findFilesWithString(const std::filesystem::path &fileDirectory, const std::string &targetString,
                         std::string &dialogueId, DialogueMemory &dialogueMemory) {
    std::vector<std::filesystem::path> foundFiles;

    try {
        // 遍历目录中的每个条目（文件或子目录）
        for (const auto &entry : std::filesystem::directory_iterator(fileDirectory)) {
            if (entry.is_regular_file()) {
                // 获取文件名
                std::string fileName = entry.path().filename().string();

                // 获取文件名的后缀
                std::string fileExtension = fileName.substr(fileName.find_last_of('.'));

                // 检查文件名是否包含指定字符串且后缀为.json
                if (fileName.find(targetString) != std::string::npos && fileExtension == ".json") {
                    // 如果文件名包含指定字符串，将其路径添加到结果列表中
                    foundFiles.push_back(entry.path());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        LOGI("遍历目录时出错: {}", e.what());
    }

    // 加载查找到的会话记录文件
    dialogueMemory.loadDialogueData(foundFiles, targetString, dialogueId);

    // // 打印historyDialogueAll的内容
    // 遍历historyDialogueAll获取key为dialogue_id的value
    Json::Value::Members members = dialogueMemory.getDialogueHistoryData().getMemberNames();
    for (const auto &member : members) {
        LOGI("\tThe value of member is: {}", member);

        if (member == "dialogue_id") {
            std::ostringstream oss;
            oss << dialogueMemory.getDialogueHistoryData()[member];
            LOGI("\tThe value of dialogue_id is: {}", oss.str());
        }
    }

    // 获取并打印key为"dialogue"的值
    if (dialogueMemory.getDialogueHistoryData().isObject() &&
        dialogueMemory.getDialogueHistoryData()["dialogue"].isArray()) {
        Json::Value dialogueArray = dialogueMemory.getDialogueHistoryData()["dialogue"];

        LOGI("\tThe value of 'dialogue' key is:");

        for (size_t i = 0; i < dialogueArray.size(); ++i) {  // arrayValue
            const Json::Value &turnValue = dialogueArray[Json::ArrayIndex(i)];
            std::ostringstream oss;
            oss << "\t\tturn_num: " << turnValue["turn_num"] << "\n";
            oss << "\t\tfirst_turn_flag: " << turnValue["first_turn_flag"] << "\n";
            oss << "\t\trole: " << turnValue["role"] << "\n";
            oss << "\t\tuser_input: " << turnValue["user_input"] << "\n";
            oss << "\t\tintent_category: " << turnValue["intent_category"] << "\n";
            LOGI("{}", oss.str());

            const Json::Value &turnSysResponse = turnValue["system_response"];

            LOGI("\t\tsystem_response: ");
            for (size_t j = 0; j < turnSysResponse.size(); ++j) {
                LOGI("\t\t\t{}", turnSysResponse[Json::ArrayIndex(j)].asString());
            }

            const Json::Value &turnKeywords = turnValue["turn_keywords"];
            LOGI("\t\tturn_keywords: ");
            for (size_t j = 0; j < turnKeywords.size(); ++j) {
                LOGI("{}\t", turnKeywords[Json::ArrayIndex(j)].asString());
            }

            const Json::Value &dialogueState = turnValue["dialogue_state"];
            LOGI("\t\tdialogue_state:\n");
            LOGI("\t\t  slots:\n");
            const Json::Value &dialogueStateArray = dialogueState["slots"];
            for (size_t j = 0; j < dialogueStateArray.size(); ++j) {
                LOGI("\t\t\tname: {},  isRequired: {}", dialogueStateArray[Json::ArrayIndex(j)]["name"].asString(),
                     dialogueStateArray[Json::ArrayIndex(j)]["isRequired"].asBool());

                for (size_t jjj = 0; jjj < dialogueStateArray[Json::ArrayIndex(j)]["values"].size(); ++jjj) {
                    LOGI("{}", dialogueStateArray[Json::ArrayIndex(j)]["values"][Json::ArrayIndex(jjj)].asString());
                }
            }

            LOGI("\t\t  system_acts: ");
            const Json::Value &dialogueStateSysArray = dialogueState["system_acts"];
            for (size_t j = 0; j < dialogueStateSysArray.size(); ++j) {
                LOGI("{}", dialogueStateSysArray[Json::ArrayIndex(j)].asString());
            }

            LOGI("\t\t  user_acts: ");
            const Json::Value &dialogueStateUserArray = dialogueState["user_acts"];
            for (size_t j = 0; j < dialogueStateUserArray.size(); ++j) {
                LOGI("{}", dialogueStateUserArray[Json::ArrayIndex(j)].asString());
            }
        }
    } else {
        LOGI("The 'dialogue' key does not exist or its value is not an array.");
    }
}
