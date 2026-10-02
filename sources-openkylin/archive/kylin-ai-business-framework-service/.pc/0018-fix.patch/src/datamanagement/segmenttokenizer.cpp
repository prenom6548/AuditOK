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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "segmenttokenizer.h"

namespace SegmentTokenizer {

std::vector<std::string> splitString(
    const std::string& str, const std::string& delimiters) {
    std::vector<std::string> result;

    size_t startPos = 0;
    size_t foundPos = str.find(delimiters);

    while(foundPos != std::string::npos) {
        if (delimiters == "." && !isEnglishPeriod(str, foundPos)) {
            foundPos = str.find(delimiters, foundPos + delimiters.size());
            continue;
        }
        result.emplace_back(str.substr(startPos, foundPos - startPos + delimiters.size()));
        startPos = foundPos + delimiters.size();
        foundPos = str.find(delimiters, startPos);
    }
    std::string lastSegment = str.substr(startPos);
    if (!lastSegment.empty()) {
        result.emplace_back(lastSegment);
    }

    return result;
}

// 按照句号分割句子，如果按照句号分割后仍然过长，则按照逗号进一步分割
std::vector<std::string> splitChineseString(const std::string& str, int maxLength)
{
    std::vector<std::string> sentences = splitString(str, "。");
    for (auto it = sentences.begin(); it != sentences.end();) {
        if (it->size() > maxLength) {
            std::vector<std::string> phrases = splitString(*it, "，");
            it = sentences.erase(it);
            it = sentences.insert(it, phrases.begin(), phrases.end());
        }
        ++it;
    }
    return sentences;
}

std::vector<std::string> splitEnglishString(const std::string& str, int maxLength)
{
    std::vector<std::string> sentences = splitString(str, ".");
    for (auto it = sentences.begin(); it != sentences.end();) {
        if (it->size() > maxLength) {
            std::vector<std::string> phrases = splitString(*it, ",");
            it = sentences.erase(it);
            it = sentences.insert(it, phrases.begin(), phrases.end());
        }
        ++it;
    }
    return sentences;
}

bool isEnglishPeriod(const std::string& text, size_t index) {
    if (index == 0) // 确保不越界
        return false;
    if (index == text.size() - 1) // 文本结尾的'.'认为是英文句号
        return true;

    // 观察`.`周围的内容
    char prevChar = text[index - 1];
    char nextChar = text[index + 1];

    // 判断`.`是否是英文句号
    if (std::isspace(prevChar) && std::isalpha(nextChar)) // 如果`.`前是空格且`.`后是字母
        return true;
    else if (std::isalpha(prevChar) && std::isspace(nextChar)) // 如果`.`前是字母且`.`后是空格
        return true;
    else if (std::ispunct(prevChar) && std::isspace(nextChar)) // 如果`.`前是标点符号且`.`后是空格
        return true;

    return false;
}

bool containsEnglishPeriod(const std::string& text)
{
    for (size_t i = 0; i < text.size(); ++i) {
        if (isEnglishPeriod(text, i))
            return true;
    }
    return false;
}

std::vector<std::string> fixedSizeChunks(const std::string& text, int chunkSize)
{
    std::vector<std::string> chunks;

    size_t startPos = 0;
    while (startPos < text.size()) {
        size_t endPos = startPos + chunkSize;
        // 确保不要在一个 UTF-8 多字节字符的中间分割
        if (endPos < text.size()) {
            while ((text[endPos] & 0xC0) == 0x80) {
                // 如果 endPos 指向的字节是多字节字符的非起始字节
                // 向前移动，直到找到多字节字符的起始字节
                --endPos;
            }
        }

        size_t length = endPos - startPos;
        chunks.push_back(text.substr(startPos, length));
        startPos += length;
    }

    return chunks;
}

// 文本分割，首先按照句号分割，如果按照句号分割完过长，
// 则按逗号进行进一步分割，如果按照逗号分割完仍然过长，则按照固定长度进行分割
void segmentTokenize(std::vector<std::string> &texts, int maxLength)
{
    for (auto begin = texts.begin(); begin != texts.end();) {
        if (begin->size() < maxLength) {
            ++begin;
            continue;
        }

        if (begin->find("。") != std::string::npos) {
            std::vector<std::string> sentences = splitChineseString(*begin, maxLength);
            begin = texts.erase(begin);
            begin = texts.insert(begin, sentences.begin(), sentences.end());
        } else if (containsEnglishPeriod(*begin)) {
            std::vector<std::string> sentences = splitEnglishString(*begin, maxLength);
            begin = texts.erase(begin);
            begin = texts.insert(begin, sentences.begin(), sentences.end());
        }

        if (begin->size() < maxLength) {
            ++begin;
            continue;
        }

        std::vector<std::string> chunks = fixedSizeChunks(*begin, maxLength);
        begin = texts.erase(begin);
        begin = texts.insert(begin, chunks.begin(), chunks.end());
        std::advance(begin, chunks.size());
    }
}

}
