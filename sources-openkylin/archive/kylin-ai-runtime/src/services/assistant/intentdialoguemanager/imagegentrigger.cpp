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

#include "imagegentrigger.h"

#include "utils/logger.h"

namespace {
// 定义句子结束符：英文 .?! 和 中文 。？！
const std::vector<std::string> delimiters = {".", "?", "!", ",", ";", "。", "？", "！", "，", "；", "\n"};
}  // namespace

bool ImageGenTrigger::shouldTriggerImageGeneration(const std::string& input) {
    if (input.empty()) {
        return false;
    }

    // 1. 将输入分割成句子列表
    std::vector<std::string> sentences = splitIntoSentences(input);

    // 2. 逐句检查
    for (const std::string& sentence : sentences) {
        if (checkSingleSentence(sentence)) {
            LOGI("Text to image sentence: {}", sentence);
            return true;  // 只要有一句话命中，就触发
        }
    }

    return false;
}

bool ImageGenTrigger::checkSingleSentence(const std::string& sentence) const {
    // 查找动作词
    size_t actionIndex = std::string::npos;
    for (const std::string& word : wordJudgment_) {
        if (size_t pos = sentence.find(word); pos != std::string::npos) {
            actionIndex = pos;
            break;
        }
    }
    if (actionIndex == std::string::npos) {
        return false;
    }
    // 查找名词
    size_t nounIndex = std::string::npos;
    for (const std::string& word : mainFont_) {
        if (size_t pos = sentence.find(word); pos != std::string::npos) {
            nounIndex = pos;
            break;
        }
    }
    if (nounIndex == std::string::npos) {
        return false;
    }
    // 动作词必须出现在名词之前才触发图像生成
    if (actionIndex < nounIndex) {
        if (nounIndex + 6 == sentence.length()) {
            return true;
        }
        std::string tempText = sentence.substr(nounIndex + 6);
        for (const std::string& delimiter : delimiters) {
            if (tempText.find(delimiter) == 0) {
                return true;
            }
        }
        return false;
    }
    return false;
}

std::vector<std::string> ImageGenTrigger::splitIntoSentences(const std::string& text) {
    std::vector<std::string> sentences;
    // 简单的 UTF-8 遍历，遇到标点切分
    // 注意：这里为了简化代码，使用了字符串匹配标点。
    // 在 UTF-8 中，中文字符占3字节，简单遍历需小心，但find匹配是安全的。
    // 这种手动遍历在处理多字节字符时比较繁琐，
    // 这里采用更健壮的方式：查找最近的标点符号位置

    std::string tempText = text;
    while (!tempText.empty()) {
        size_t minPos = std::string::npos;
        size_t delimLen = 0;

        // 寻找最早出现的标点符号
        for (const auto& delim : delimiters) {
            size_t pos = tempText.find(delim);
            if (pos != std::string::npos) {
                if (minPos == std::string::npos || pos < minPos) {
                    minPos = pos;
                    delimLen = delim.length();
                }
            }
        }

        if (minPos == std::string::npos) {
            // 没有标点符号了，剩下的全是最后一句话
            if (!tempText.empty()) {
                sentences.push_back(tempText);
            }
            break;
        } else {
            // 截取句子 (包含标点，或者不包含均可，这里不包含标点以便匹配纯净文本)
            std::string sent = tempText.substr(0, minPos);
            if (!sent.empty()) {
                sentences.push_back(sent);
            }
            // 移动 text 指针
            tempText = tempText.substr(minPos + delimLen);
        }
    }
    return sentences;
}
