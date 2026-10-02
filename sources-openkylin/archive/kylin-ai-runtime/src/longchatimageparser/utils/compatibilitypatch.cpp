/*
 * Copyright (C) 2025 Kylin Software Co., Ltd.
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

#include "compatibilitypatch.h"

#include <algorithm>
#include <iostream>
#include <regex>

#include "../../utils/logger.h"

namespace utils {
namespace CompatibilityPatch {

// 静态成员定义
const std::vector<std::string>& Patcher::get_special_chars() {
    static const std::vector<std::string> special_chars = {"✨", "★", "☆", "♦", "♠", "♥", "♣"};
    return special_chars;
}

bool Patcher::contains_special_char(const std::string& text) {
    const auto& special_chars = get_special_chars();

    for (const auto& char_str : special_chars) {
        if (text.find(char_str) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool Patcher::is_time_marker(const std::string& text) { return text.find("(时间)") != std::string::npos; }

std::string Patcher::extract_text_from_item(const OCRItem& item) { return item.text; }

std::string Patcher::extract_text_from_json(const Json::Value& item_json) {
    // 支持对象属性（.text）和字典访问（['text']）两种方式
    if (item_json.isMember("text") && item_json["text"].isString()) {
        return item_json["text"].asString();
    }
    return "";
}

bool Patcher::contains_group_keywords(const std::string& content) {
    const std::vector<std::string> keywords = {"沟通", "群", "组"};

    for (const auto& keyword : keywords) {
        if (content.find(keyword) != std::string::npos) {
            // 检查是否同时包含括号（表示群名格式）
            if (content.find("(") != std::string::npos && content.find(")") != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

std::vector<OCRItem> Patcher::patch_ocr_order_for_compatibility(const std::vector<OCRItem>& ocr_items) {
    if (ocr_items.size() < 2) {
        return ocr_items;
    }

    // 创建工作副本
    std::vector<OCRItem> result = ocr_items;

    // 特殊处理：如果前两个项目中有特殊字符和时间，确保特殊字符在前
    for (Json::ArrayIndex i = 0; i < std::min(Json::ArrayIndex(2), static_cast<Json::ArrayIndex>(result.size())); ++i) {
        for (Json::ArrayIndex j = i + 1;
             j < std::min(Json::ArrayIndex(3), static_cast<Json::ArrayIndex>(result.size())); ++j) {
            const std::string& text_i = extract_text_from_item(result[i]);
            const std::string& text_j = extract_text_from_item(result[j]);

            // 如果j是特殊字符而i是时间，交换它们
            if (contains_special_char(text_j) && is_time_marker(text_i)) {
                std::swap(result[i], result[j]);
                LOGD("[CompatibilityPatch] Compatibility adjustment: 交换位置 {} 和 {}", i, j);
            }
        }
    }

    return result;
}

Json::Value Patcher::patch_ocr_order_for_compatibility_json(const Json::Value& ocr_items_json) {
    if (!ocr_items_json.isArray() || ocr_items_json.size() < 2) {
        return ocr_items_json;
    }

    // 创建工作副本
    Json::Value result = ocr_items_json;

    // 特殊处理：如果前两个项目中有特殊字符和时间，确保特殊字符在前
    for (Json::ArrayIndex i = 0; i < std::min(Json::ArrayIndex(2), static_cast<Json::ArrayIndex>(result.size())); ++i) {
        for (Json::ArrayIndex j = i + 1;
             j < std::min(Json::ArrayIndex(3), static_cast<Json::ArrayIndex>(result.size())); ++j) {
            const std::string text_i = extract_text_from_json(result[i]);
            const std::string text_j = extract_text_from_json(result[j]);

            // 如果j是特殊字符而i是时间，交换它们
            if (contains_special_char(text_j) && is_time_marker(text_i)) {
                std::swap(result[i], result[j]);
                LOGD("[CompatibilityPatch] Compatibility adjustment: 交换JSON位置 {} 和 {}", i, j);
            }
        }
    }

    return result;
}

std::vector<std::string> Patcher::patch_marked_texts_for_compatibility(const std::vector<std::string>& marked_texts) {
    if (marked_texts.size() < 2) {
        return marked_texts;
    }

    // 创建工作副本
    std::vector<std::string> result = marked_texts;

    // 检查前几个元素
    for (Json::ArrayIndex i = 0; i < std::min(Json::ArrayIndex(3), static_cast<Json::ArrayIndex>(result.size())); ++i) {
        for (Json::ArrayIndex j = i + 1;
             j < std::min(Json::ArrayIndex(4), static_cast<Json::ArrayIndex>(result.size())); ++j) {
            // 如果j是特殊字符而i包含"(时间)"，交换它们
            if (contains_special_char(result[j]) && is_time_marker(result[i])) {
                std::swap(result[i], result[j]);
                LOGD("[CompatibilityPatch] Compatibility adjustment: 交换文本位置 {} 和 {}", i, j);
            }
        }
    }

    return result;
}

Json::Value Patcher::patch_chat_messages_for_compatibility(const Json::Value& chat_messages) {
    if (!chat_messages.isArray() || chat_messages.size() < 3) {
        return chat_messages;
    }

    // 创建工作副本
    Json::Value result = chat_messages;

    // 1. 确保特殊字符在时间之前
    for (Json::ArrayIndex i = 0; i < std::min(Json::ArrayIndex(2), result.size()); ++i) {
        if (result[i].isMember("type") && result[i]["type"] == "time" && i + 1 < result.size()) {
            if (result[i + 1].isMember("type") && result[i + 1]["type"] == "unknown" &&
                result[i + 1].isMember("content")) {
                std::string content = result[i + 1]["content"].asString();
                if (contains_special_char(content)) {
                    // 交换顺序
                    std::swap(result[i], result[i + 1]);
                    LOGD("[CompatibilityPatch] Compatibility adjustment: 将特殊字符移到时间之前");
                    break;
                }
            }
        }
    }

    // 2. 将包含群聊关键字的unknown类型消息改为group_name
    for (Json::ArrayIndex i = 0; i < std::min(Json::ArrayIndex(5), result.size()); ++i) {
        if (result[i].isMember("type") && result[i]["type"] == "unknown" && result[i].isMember("content")) {
            std::string content = result[i]["content"].asString();
            if (contains_group_keywords(content)) {
                // 转换为group_name类型
                Json::Value new_msg;
                new_msg["type"] = "group_name";
                new_msg["群聊名称"] = content;
                result[i] = new_msg;
                LOGD("[CompatibilityPatch] Compatibility adjustment: 将 '{}' 标记为群聊名称", content);
                break;
            }
        }
    }

    return result;
}

}  // namespace CompatibilityPatch
}  // namespace utils
