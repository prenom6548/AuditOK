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

#ifndef OCR_MODELS_UTILS_COMPATIBILITY_PATCH_H
#define OCR_MODELS_UTILS_COMPATIBILITY_PATCH_H

#include <jsoncpp/json/json.h>

#include <string>
#include <vector>

#include "ocravatartypes.h"

namespace utils {

/**
 * 检查字符串是否包含子串
 *
 * @param text 待检查的文本
 * @param substr 子字符串
 * @return 是否包含
 */
inline bool str_contains(const std::string& text, const std::string& substr) {
    return text.find(substr) != std::string::npos;
}

/**
 * 计算 UTF-8 字符串中的实际字符数量
 *
 * @param str UTF-8 编码的字符串
 * @return 字符数量（非字节数）
 */
inline size_t utf8_char_count(const std::string& str) {
    size_t count = 0;
    for (size_t i = 0; i < str.length();) {
        unsigned char c = str[i];
        if (c <= 0x7F) {
            i += 1;  // ASCII字符（1字节）
        } else if ((c & 0xE0) == 0xC0) {
            i += 2;  // 2字节UTF-8字符
        } else if ((c & 0xF0) == 0xE0) {
            i += 3;  // 3字节UTF-8字符（包括中文）
        } else if ((c & 0xF8) == 0xF0) {
            i += 4;  // 4字节UTF-8字符
        } else {
            i += 1;  // 无效字符，跳过
        }
        count++;
    }
    return count;
}

/**
 * 去除字符串首尾空白字符
 *
 * @param str 输入字符串
 * @return 去除首尾空白后的字符串
 */
inline std::string str_trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

/**
 * 替换字符串中所有匹配的子串
 *
 * @param str 输入字符串
 * @param from 要替换的子串
 * @param to 替换后的内容
 * @return 替换后的新字符串
 */
inline std::string str_replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

namespace CompatibilityPatch {
class Patcher {
public:
    /**
     * @param ocr_items OCR项目列表
     * @return 调整顺序后的OCR项目列表
     */
    static std::vector<OCRItem> patch_ocr_order_for_compatibility(const std::vector<OCRItem>& ocr_items);

    /**
     *
     * @param ocr_items_json JSON格式的OCR项目列表
     * @return 调整顺序后的JSON格式OCR项目列表
     */
    static Json::Value patch_ocr_order_for_compatibility_json(const Json::Value& ocr_items_json);

    /**
     *
     * @param marked_texts 标记后的文本列表
     * @return 调整顺序后的文本列表
     */
    static std::vector<std::string> patch_marked_texts_for_compatibility(const std::vector<std::string>& marked_texts);

    /**

     * @param chat_messages 聊天消息列表（JSON格式）
     * @return 调整后的聊天消息列表
     */
    static Json::Value patch_chat_messages_for_compatibility(const Json::Value& chat_messages);

    /**
     * 获取特殊字符列表
     *
     * @return 特殊字符向量
     */
    static const std::vector<std::string>& get_special_chars();

    /**
     * 检查文本中是否包含特殊字符
     *
     * @param text 要检查的文本
     * @return 是否包含特殊字符
     */
    static bool contains_special_char(const std::string& text);

    /**
     * 检查文本是否为时间标记
     *
     * @param text 要检查的文本
     * @return 是否为时间标记
     */
    static bool is_time_marker(const std::string& text);

    /**
     * 从OCRItem中提取文本内容
     *
     * @param item OCR项目
     * @return 文本内容
     */
    static std::string extract_text_from_item(const OCRItem& item);

    /**
     * 从JSON对象中提取文本内容
     * 支持不同的JSON结构（对象属性或字典访问）
     *
     * @param item_json JSON格式的项目
     * @return 文本内容
     */
    static std::string extract_text_from_json(const Json::Value& item_json);

    /**
     * 检查消息内容是否包含群聊关键字
     *
     * @param content 消息内容
     * @return 是否包含群聊关键字
     */
    static bool contains_group_keywords(const std::string& content);
};

}  // namespace CompatibilityPatch
}  // namespace utils

#endif  // OCR_MODELS_UTILS_COMPATIBILITY_PATCH_H
