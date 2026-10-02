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

#ifndef TYPETRANS_H
#define TYPETRANS_H

#include <jsoncpp/json/json.h>

#include <ctime>
#include <list>
#include <map>
#include <typeinfo>
#include <variant>
#include <vector>

#include "dialoguemessage.h"

namespace TypeTrans {
DialogueData::APINameSearchResult findAPIName(const std::string &searchText, const Json::Value &apiName,
                                              const std::string &attachmentType);

std::string getTimeStr();

std::list<std::string> jsonArrayToStdList(const Json::Value &jsonArray);

Json::Value systemResponseToJsonValue(const std::variant<std::string, std::list<std::string>> &system_response);

Json::Value slotToJsonValue(const std::vector<DialogueData::Slot> &slot);

std::list<std::string> findElementContainingString(const std::vector<std::string> &myList,
                                                   const std::string &targetString);

std::string checkList(const std::list<std::string> &myList);

bool isStringInList(const std::list<std::string> &myList, const std::string &targetString);
Json::Value callSuccessIdentifierToJson(const DialogueData::CallSuccessIdentifier &identifier);

bool checkString(const std::string &str);

bool checkAndSetScaling(std::vector<DialogueData::Slot> &inputVector);

std::vector<std::string> readTxtToList(const std::string &readFilePath);

std::string replaceChinesePunctuation(const std::string &input);

bool isStringInJsonArray(const Json::Value &array, const std::string &str);

bool isPureUTF8ChineseText(const std::string &str);

bool isInteger(const std::string &str);

bool isDecimal(const std::string &str);

std::string removeSuffix(const std::string &str);

std::pair<std::string, std::string> isSlotInRange(const std::string &slotStr, const std::string &intent,
                                                  const std::map<std::string, std::pair<int, int>> &integerScenarios_);

std::pair<std::string, std::string> isSlotInDecimalRange(
    const std::string &slotStr, const std::string &intent,
    const std::map<std::string, std::pair<int, int>> &decimalScenariosScope,
    const std::map<std::string, std::pair<int, int>> &integerScenariosScope);

int countEnglishChars(const std::string &str);

}  // namespace TypeTrans
#endif  // TYPETRANS_H
