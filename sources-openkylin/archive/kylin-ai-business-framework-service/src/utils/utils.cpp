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

#include "utils.h"

#include <uuid/uuid.h>
#include <iostream>
#include <cmath>
#include <regex>

namespace utils {

std::string generateUUID()
{
    uuid_t uuid;
    char uuid_str[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);
    std::string id = uuid_str;

    return id;
}

}

namespace math
{
void normalize(std::vector<float> &vec)
{
    if (vec.empty()) {
        return;
    }

    float normNum = norm(vec);
    if (normNum == 0.0) {
        return;
    }

    for (int i = 0; i < vec.size(); ++i) {
        vec[i] = vec[i] / normNum;
    }
}

// 向量取模
float norm(const std::vector<float>& vec)
{
    float norm = 0.0f;
    for (const float& element : vec) {
        norm += element * element;
    }
    norm = std::sqrt(norm);

    return norm;
}

void normalizeTokens(std::vector<int64_t> &tokens, int length)
{
    if (tokens.size() == length) {
        return;
    } else if (tokens.size() > length) {
        tokens[length - 1] = tokens.back();
        tokens.resize(length);
        std::cerr << "Input tokens exceed " << length
                  << ", the tokens will be cropped" << std::endl;
    } else {
        tokens.resize(52, 0);
    }
}
} // namespace math


namespace text
{
//去除多余的换行和空格、使用递归的方式进行文档切分
void applyRegex(std::string &text) {
    static const std::regex newlineRegex("\\n{3,}");
    static const std::regex spaceRegex("\\s");
    static const std::regex doubleNewlineRegex("\\n\\n");
    static const std::regex singlePunctuationRegex("([;；.!?。！？\\?])([^”’])");
    static const std::regex ellipsisRegex("(\\.{6})([^\"’”」』])");
    static const std::regex chineseEllipsisRegex("(\\…{2})([^\"’”」』])");
    static const std::regex endQuotePunctuationRegex("([;；!?。！？\\?][\"’”」』]{0,2})([^;；!?，。！？\\?])");

    text = std::regex_replace(text, newlineRegex, "\n");
    text = std::regex_replace(text, spaceRegex, " ");
    text = std::regex_replace(text, doubleNewlineRegex, "");
    text = std::regex_replace(text, singlePunctuationRegex, "$1\n$2");
    text = std::regex_replace(text, ellipsisRegex, "$1\n$2");
    text = std::regex_replace(text, chineseEllipsisRegex, "$1\n$2");
    text = std::regex_replace(text, endQuotePunctuationRegex, "$1\n$2");

    text.erase(std::remove_if(text.begin(), text.end(), isspace), text.end());
}

// 根据最大长度分割句子
std::vector<std::string> splitSentences(const std::string &text, int maxLength) {
    std::istringstream iss(text);
    std::string sentence;
    std::vector<std::string> sentences;
    while (std::getline(iss, sentence, '\n')) {
        // 检查句子是否为空，若为空则跳过本次循环
        if (sentence.empty()) {
            continue;
        }
        size_t start = 0;
        while (start < sentence.length()) {
            size_t end = std::min(sentence.length(), start + maxLength);
            // 确保不切分多字节字符
            while (end > start && (sentence[end] & 0xC0) == 0x80) {
                --end;
            }
            // 如果end位置没有移动，则向前寻找一个合法的结束位置
            if (end == start) {
                while (start < sentence.length() && (sentence[start] & 0xC0) == 0x80) {
                    ++start;
                }
                end = std::min(sentence.length(), start + maxLength);
                while (end > start && (sentence[end] & 0xC0) == 0x80) {
                    --end;
                }
            }
            sentences.push_back(sentence.substr(start, end - start));
            start = end;
        }
    }
    return sentences;
}


void segmentTokenize(std::vector<std::string> &texts, int maxLength) {
    auto begin = texts.begin();
    while (begin != texts.end()) {
        applyRegex(*begin);
        std::vector<std::string> sentences = splitSentences(*begin, maxLength);
        if (sentences.empty()) {
            ++begin;
            continue;
        }

        begin = texts.erase(begin);
        begin = texts.insert(begin, sentences.begin(), sentences.end());
        std::advance(begin, sentences.size());
    }

}
}
