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

#ifndef CHAT_ANALYZER_H
#define CHAT_ANALYZER_H

#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "../utils/chatmessage.h"
#include "../utils/config.h"
#include "../utils/ocravatartypes.h"

namespace chat {

/**
 * 聊天分析器
 * 负责将标记后的OCR结果组织成结构化的聊天消息
 */
class ChatAnalyzer {
public:
    /**
     * 构造函数
     * @param config 配置对象
     */
    explicit ChatAnalyzer(const utils::Config& config);

    /**
     * 析构函数
     */
    ~ChatAnalyzer();

    /**
     * 分析标记后的OCR结果，组织成结构化聊天消息
     * @param marked_ocr_items 标记后的OCR项列表
     * @return 聊天会话对象
     */
    std::unique_ptr<chat::ChatSession> analyze(const std::vector<utils::OCRItem>& marked_ocr_items);

private:
    // 时间匹配正则模式
    std::vector<std::regex> time_patterns_;

    // 排除关键词
    std::vector<std::string> time_exclude_keywords_;

    /**
     * 组织聊天消息
     * @param marked_ocr_items 标记后的OCR项列表
     * @param session 聊天会话对象
     */
    void organizeMessages(const std::vector<utils::OCRItem>& marked_ocr_items, chat::ChatSession& session);

    /**
     * 按空间位置对内容片段进行排序（从上到下，从左到右）
     * @param marked_ocr_items 标记后的OCR项列表
     * @param indices 要排序的OCR项索引列表
     * @return 排序后的索引列表
     */
    std::vector<int> sortContentFragmentsByPosition(const std::vector<utils::OCRItem>& marked_ocr_items,
                                                    const std::vector<int>& indices);

    /**
     * 移除文本中的所有标记，获取纯净内容
     * @param text 带标记的文本
     * @return 去除标记后的纯净文本
     */
    std::string getCleanText(const std::string& text);

    /**
     * 判断文本是否为群聊名称
     * @param text 待判断的文本
     * @return 是否为群聊名称
     */
    bool isGroupName(const std::string& text);

    /**
     * 检查文本是否为时间内容
     * @param text 文本内容
     * @return 是否为时间内容
     */
    bool isTimeContent(const std::string& text);

    /**
     * 初始化时间匹配模式
     */
    void initializeTimePatterns();

    /**
     * 去除字符串首尾空白字符
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    std::string trim(const std::string& str);

    /**
     * 检查字符串是否包含指定子串
     * @param str 主字符串
     * @param substr 子字符串
     * @return 是否包含
     */
    bool contains(const std::string& str, const std::string& substr);

    /**
     * 替换字符串中的子串
     * @param str 源字符串
     * @param from 要替换的子串
     * @param to 替换为的子串
     * @return 替换后的字符串
     */
    std::string replace(const std::string& str, const std::string& from, const std::string& to);
};

}  // namespace chat

#endif  // CHAT_ANALYZER_H
