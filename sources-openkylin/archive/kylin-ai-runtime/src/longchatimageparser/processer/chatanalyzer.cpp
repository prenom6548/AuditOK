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

#include "chatanalyzer.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include "../../utils/logger.h"
#include "../utils/compatibilitypatch.h"

namespace chat {

using namespace utils;

constexpr int GROUP_LENGTH = 100;

ChatAnalyzer::ChatAnalyzer(const utils::Config& config) {
    (void)config;  // 参数保留用于未来扩展
    initializeTimePatterns();

    // 初始化排除关键词列表
    time_exclude_keywords_ = {"报送", "回执", "会议", "参加", "人员", "工作", "通知", "安排", "要求", "地点", "内容",
                              "完成", "需要", "前",   "后",   "开始", "结束", "传包", "表格", "填写", "更新", "自测"};

    LOGD("[ChatAnalyzer] Initialized, time pattern count: {}", time_patterns_.size());
}

ChatAnalyzer::~ChatAnalyzer() = default;

std::unique_ptr<chat::ChatSession> ChatAnalyzer::analyze(const std::vector<utils::OCRItem>& marked_ocr_items) {
    LOGD("[ChatAnalyzer] Starting chat message analysis...");

    // 创建聊天会话
    auto session = std::make_unique<chat::ChatSession>();

    // 组织chat_messages
    organizeMessages(marked_ocr_items, *session);

    // 获取统计信息
    auto stats = session->getStatistics();
    LOGD(
        "[ChatAnalyzer] Analysis completed: total_messages={}, chat_messages={}, time_messages={}, "
        "my_messages={}",
        stats["total"], stats["chat"], stats["time"], stats["my_chat"]);

    return session;
}

void ChatAnalyzer::organizeMessages(const std::vector<utils::OCRItem>& marked_ocr_items, chat::ChatSession& session) {
    int i = 0;

    while (i < static_cast<int>(marked_ocr_items.size())) {
        std::string text = str_trim(marked_ocr_items[i].text);
        if (text.empty()) {
            i++;
            continue;
        }

        // 在前10条消息中检查群聊名称（通过文本模式匹配）
        if (i < 10 && isGroupName(text)) {
            // 移除标记，获取纯净的群聊名称
            std::string clean_group_name = getCleanText(text);

            auto message = std::make_unique<chat::ChatMessage>();
            message->type = chat::MessageType::GROUP_NAME;
            message->content = clean_group_name;
            session.addMessage(std::move(message));
            i++;
            continue;
        }

        // 处理时间标记
        if (str_contains(text, "(时间)")) {
            std::string time_text = str_trim(str_replace(text, "(时间)", ""));
            auto message = std::make_unique<chat::ChatMessage>();
            message->type = chat::MessageType::TIME;
            message->time = time_text;
            session.addMessage(std::move(message));
            i++;
            continue;
        }

        // 处理我的内容标记
        if (str_contains(text, "(我的内容)")) {
            std::vector<int> my_content_indices;
            int j = i;

            // 收集连续的"我的内容"标记
            while (j < static_cast<int>(marked_ocr_items.size())) {
                std::string current_text = str_trim(marked_ocr_items[j].text);
                if (current_text.empty()) {
                    j++;
                    continue;
                }

                if (str_contains(current_text, "(我的内容)")) {
                    std::string content = str_trim(str_replace(current_text, "(我的内容)", ""));
                    if (!content.empty()) {
                        my_content_indices.push_back(j);
                    }
                    j++;
                } else {
                    break;
                }
            }

            // 创建消息
            if (!my_content_indices.empty()) {
                // 按空间position排序内容片段
                std::vector<int> sorted_indices = sortContentFragmentsByPosition(marked_ocr_items, my_content_indices);

                std::string combined_content;
                for (size_t k = 0; k < sorted_indices.size(); ++k) {
                    if (k > 0) combined_content += " ";
                    std::string content =
                        str_trim(str_replace(marked_ocr_items[sorted_indices[k]].text, "(我的内容)", ""));
                    combined_content += content;
                }

                auto message = std::make_unique<chat::ChatMessage>();
                message->type = chat::MessageType::MY_CHAT;
                message->nickname = "我";
                message->content = combined_content;
                session.addMessage(std::move(message));
            }

            i = j;
            continue;
        }

        // 处理昵称标记
        if (str_contains(text, "(昵称)")) {
            std::string nickname = str_trim(str_replace(text, "(昵称)", ""));

            // 收集后续内容
            std::vector<int> content_indices;
            std::vector<std::string> retract_messages;
            std::vector<int> untagged_indices;  // 未标记的内容索引
            int j = i + 1;

            while (j < static_cast<int>(marked_ocr_items.size())) {
                std::string next_text = str_trim(marked_ocr_items[j].text);
                if (next_text.empty()) {
                    j++;
                    continue;
                }

                // 遇到新的标记则停止
                if (str_contains(next_text, "(昵称)") || str_contains(next_text, "(时间)") ||
                    str_contains(next_text, "(我的内容)")) {
                    break;
                }

                // 收集内容
                if (str_contains(next_text, "(内容)")) {
                    std::string content = str_trim(str_replace(next_text, "(内容)", ""));
                    if (!content.empty()) {
                        // 检查特殊内容
                        if (str_contains(content, "撤回了一条消息")) {
                            retract_messages.push_back(content);
                        } else if (isTimeContent(content)) {
                            // 时间内容
                            auto time_message = std::make_unique<chat::ChatMessage>();
                            time_message->type = chat::MessageType::TIME;
                            time_message->time = content;
                            session.addMessage(std::move(time_message));
                        } else {
                            content_indices.push_back(j);
                        }
                    }
                } else {
                    // 未标记的内容也收集
                    untagged_indices.push_back(j);
                }

                j++;
            }

            // 创建消息
            if (!content_indices.empty() || !untagged_indices.empty()) {
                // 合并所有内容索引并按空间position排序
                std::vector<int> all_content_indices = content_indices;
                all_content_indices.insert(all_content_indices.end(), untagged_indices.begin(), untagged_indices.end());

                std::vector<int> sorted_indices = sortContentFragmentsByPosition(marked_ocr_items, all_content_indices);

                std::string combined_content;
                for (size_t k = 0; k < sorted_indices.size(); ++k) {
                    if (k > 0) combined_content += " ";

                    std::string fragment_text = marked_ocr_items[sorted_indices[k]].text;
                    // 如果是内容标记，去掉标记
                    if (str_contains(fragment_text, "(内容)")) {
                        fragment_text = str_trim(str_replace(fragment_text, "(内容)", ""));
                    }
                    combined_content += fragment_text;
                }

                auto message = std::make_unique<chat::ChatMessage>();
                message->type = chat::MessageType::CHAT;
                message->nickname = nickname;
                message->content = combined_content;
                session.addMessage(std::move(message));
            } else {
                LOGD("[ChatAnalyzer] Warning: Found nickname without content: {}  position：{}", nickname, i);
            }

            // 添加撤回消息
            for (const auto& retract_content : retract_messages) {
                auto message = std::make_unique<chat::ChatMessage>();
                message->type = chat::MessageType::RETRACT_MESSAGE;
                message->content = retract_content;
                session.addMessage(std::move(message));
            }

            i = j;
            continue;
        }

        // 处理孤立的内容标记
        if (str_contains(text, "(内容)")) {
            std::string content = str_trim(str_replace(text, "(内容)", ""));

            auto message = std::make_unique<chat::ChatMessage>();
            if (str_contains(content, "撤回了一条消息")) {
                message->type = chat::MessageType::RETRACT_MESSAGE;
                message->content = content;
            } else if (isTimeContent(content)) {
                message->type = chat::MessageType::TIME;
                message->time = content;
            } else {
                message->type = chat::MessageType::CHAT;
                message->nickname = "未知";
                message->content = content;
            }
            session.addMessage(std::move(message));
            i++;
            continue;
        }

        // 处理系统消息
        if (str_contains(text, "(系统消息)")) {
            std::string system_content = str_trim(str_replace(text, "(系统消息)", ""));
            auto message = std::make_unique<chat::ChatMessage>();
            message->type = chat::MessageType::SYSTEM_MESSAGE;
            message->content = system_content;
            session.addMessage(std::move(message));
        } else {
            // 未知内容
            auto message = std::make_unique<chat::ChatMessage>();
            message->type = chat::MessageType::UNKNOWN;
            message->content = text;
            session.addMessage(std::move(message));
        }

        i++;
    }
}

std::string ChatAnalyzer::getCleanText(const std::string& text) {
    // 移除所有可能的标记，获取原始文本内容
    std::string clean_text = str_trim(text);
    // 移除各种标记
    clean_text = str_replace(clean_text, "(昵称)", "");
    clean_text = str_replace(clean_text, "(内容)", "");
    clean_text = str_replace(clean_text, "(时间)", "");
    clean_text = str_replace(clean_text, "(我的内容)", "");
    clean_text = str_replace(clean_text, "(系统消息)", "");
    clean_text = str_replace(clean_text, "(群名)", "");
    clean_text = str_replace(clean_text, "(群聊)", "");
    return str_trim(clean_text);  // 去除标记后重新trim
}

// 基于文本模式判断群聊名称，用于前10条消息的群名识别
bool ChatAnalyzer::isGroupName(const std::string& text) {
    // 基于文本模式判断是否为群聊名称
    std::string clean_text = getCleanText(text);

    // 检查群聊特征模式：包含 (数字) 格式，表示群成员数
    std::regex group_pattern(R"(\(\d+\))");
    if (!std::regex_search(clean_text, group_pattern)) {
        return false;
    }

    // 群名长度合理性检查（避免过长的文本被误判）
    if (utf8_char_count(clean_text) > GROUP_LENGTH) {
        return false;
    }

    // 排除明显不是群名的模式
    if (str_contains(clean_text, "撤回了一条消息")) {
        return false;
    }

    LOGD("[ChatAnalyzer] Detected group name: {} ()", clean_text);
    return true;
}

bool ChatAnalyzer::isTimeContent(const std::string& text) {
    std::string trimmed_text = str_trim(text);

    if (utf8_char_count(trimmed_text) > 30) {
        return false;
    }

    for (const auto& keyword : time_exclude_keywords_) {
        if (str_contains(trimmed_text, keyword)) {
            return false;
        }
    }

    // 检查时间模式
    for (const auto& pattern : time_patterns_) {
        std::smatch match;
        if (std::regex_search(trimmed_text, match, pattern)) {
            int matched_length = utf8_char_count(match[0].str());
            double match_ratio = static_cast<double>(matched_length) / utf8_char_count(trimmed_text);

            // 获取模式字符串进行特殊判断
            if (match_ratio >= 0.4) {
                return true;
            }
        }
    }

    return false;
}

void ChatAnalyzer::initializeTimePatterns() {
    std::vector<std::string> pattern_strings = {
        R"(\d{4}年\d{1,2}月\d{1,2}日\d{1,2}:\d{2})",                              // 2025年6月17日9:10
        R"(\d{4}年\d{1,2}月\d{1,2}日)",                                           // 2025年6月17日
        R"((昨天|今天|前天|明天)(早上|上午|中午|下午|晚上|凌晨)?\d{1,2}:\d{2})",  // 昨天晚上6:23
        R"((上午|下午|早上|中午|晚上|凌晨)\d{1,2}:\d{2})",                        // 上午9:17
        R"(\d{1,2}:\d{2})",                                                       // 5:51
        R"(\d{4}-\d{1,2}-\d{1,2})",                                               // 年-月-日
        R"(\d{1,2}/\d{1,2})",                                                     // 月/日
        R"(\d{1,2}月\d{1,2}日)",                                                  // 月日
        R"(\d{1,2}月\d{1,2}日\d{1,2}:\d{2})",                                     // 7月28日14:53
        R"(\d{1,2}:\d{2}:\d{2})",                                                 // 时:分:秒
        R"((昨天|今天|前天|明天))",                                               // 相对日期
        R"(周(一|二|三|四|五|六|日|天))",                                         // 星期
    };

    time_patterns_.reserve(pattern_strings.size());
    for (const auto& pattern_str : pattern_strings) {
        try {
            time_patterns_.emplace_back(pattern_str);
        } catch (const std::regex_error& e) {
            LOGE("[ChatAnalyzer] Regex compilation failed: {}, Error: {}", pattern_str, e.what());
        }
    }
}

std::vector<int> ChatAnalyzer::sortContentFragmentsByPosition(const std::vector<utils::OCRItem>& marked_ocr_items,
                                                              const std::vector<int>& indices) {
    /**
     * 按空间position对内容片段进行排序（从上到下，从左到右）
     * 使用OCRItem的box坐标信息进行排序
     */

    // 创建索引副本进行排序
    std::vector<int> sorted_indices = indices;

    // 排序逻辑：首先按center_y排序（从上到下），对于相近的Y坐标，按min_x排序（从左到右）
    std::sort(sorted_indices.begin(), sorted_indices.end(), [&marked_ocr_items](int idx1, int idx2) {
        const utils::OCRItem& item1 = marked_ocr_items[idx1];
        const utils::OCRItem& item2 = marked_ocr_items[idx2];

        double center_y1 = item1.get_center_y();
        double center_y2 = item2.get_center_y();

        // 定义行高阈值，如果两个文本的center_y差距小于此值，认为在同一行
        double line_threshold = 20.0;

        if (std::abs(center_y1 - center_y2) < line_threshold) {
            // 在同一行内，按X坐标从左到右排序
            return item1.get_min_x() < item2.get_min_x();
        } else {
            // 不在同一行，按Y坐标从上到下排序
            return center_y1 < center_y2;
        }
    });

    return sorted_indices;
}

}  // namespace chat
