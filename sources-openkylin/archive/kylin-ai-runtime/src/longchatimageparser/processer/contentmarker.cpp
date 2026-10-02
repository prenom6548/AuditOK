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

/*
 * 内容标记处理器模块实现
 * 负责标记OCR识别结果的类型（时间、昵称、内容等）
 */

#include "contentmarker.h"

#include <algorithm>
#include <iostream>
#include <map>

#include "../../utils/logger.h"
#include "../utils/compatibilitypatch.h"
#include "deduplicator.h"

// 本地定义 BoxBounds
namespace {
struct BoxBounds {
    float min_x;
    float min_y;
    float max_x;
    float max_y;
    BoxBounds(float minX, float minY, float maxX, float maxY) : min_x(minX), min_y(minY), max_x(maxX), max_y(maxY) {}
};
}  // anonymous namespace

// 使用相关命名空间中的函数
using detection::Deduplicator;

// 帮助函数：获取边界框的边界值
BoxBounds getBoxBounds(const utils::Config& config, const std::vector<std::vector<float>>& box) {
    // 简化计算：直接从OCR格式的边界框计算边界
    std::vector<float> x_coords, y_coords;

    for (const auto& point : box) {
        if (point.size() >= 2) {
            x_coords.push_back(point[0]);
            y_coords.push_back(point[1]);
        }
    }

    if (x_coords.empty()) {
        return BoxBounds(0, 0, 0, 0);
    }

    float min_x = *std::min_element(x_coords.begin(), x_coords.end());
    float max_x = *std::max_element(x_coords.begin(), x_coords.end());
    float min_y = *std::min_element(y_coords.begin(), y_coords.end());
    float max_y = *std::max_element(y_coords.begin(), y_coords.end());

    return BoxBounds(min_x, min_y, max_x, max_y);
}

namespace detection {

// 时间文本最大长度限制
constexpr int TIME_TEXT_MAX_LENGTH = 30;
// 日志显示文本截断长度
constexpr size_t DISPLAY_TEXT_MAX_LENGTH = 30;

ContentMarker::ContentMarker(const utils::Config& config) : config_(config) {
    // 初始化颜色检测参数
    auto content_config = config_.content_marking;

    green_hsv_lower_ =
        cv::Scalar(std::get<0>(content_config.green_hsv_lower), std::get<1>(content_config.green_hsv_lower),
                   std::get<2>(content_config.green_hsv_lower));
    green_hsv_upper_ =
        cv::Scalar(std::get<0>(content_config.green_hsv_upper), std::get<1>(content_config.green_hsv_upper),
                   std::get<2>(content_config.green_hsv_upper));
    green_ratio_threshold_ = content_config.green_ratio_threshold;

    blue_hsv_lower_ = cv::Scalar(std::get<0>(content_config.blue_hsv_lower), std::get<1>(content_config.blue_hsv_lower),
                                 std::get<2>(content_config.blue_hsv_lower));
    blue_hsv_upper_ = cv::Scalar(std::get<0>(content_config.blue_hsv_upper), std::get<1>(content_config.blue_hsv_upper),
                                 std::get<2>(content_config.blue_hsv_upper));
    blue_ratio_threshold_ = content_config.blue_ratio_threshold;

    white_hsv_lower_ =
        cv::Scalar(std::get<0>(content_config.white_hsv_lower), std::get<1>(content_config.white_hsv_lower),
                   std::get<2>(content_config.white_hsv_lower));
    white_hsv_upper_ =
        cv::Scalar(std::get<0>(content_config.white_hsv_upper), std::get<1>(content_config.white_hsv_upper),
                   std::get<2>(content_config.white_hsv_upper));
    white_ratio_threshold_ = content_config.white_ratio_threshold;

    // 初始化关键词和排除词
    time_exclude_keywords_ = config_.get_time_exclude_keywords();
    feishu_keywords_ = config_.get_feishu_keywords();

    // 初始化正则表达式模式
    initializePatterns();
}

void ContentMarker::initializePatterns() {
    // 初始化时间匹配模式
    auto time_pattern_strings = config_.get_time_patterns();
    for (const auto& pattern_str : time_pattern_strings) {
        try {
            time_patterns_.emplace_back(pattern_str);
        } catch (const std::regex_error& e) {
            LOGE("Time regex error: {} - {}", pattern_str, e.what());
        }
    }

    // 初始化系统消息匹配模式
    auto system_pattern_strings = config_.get_system_message_patterns();
    for (const auto& pattern_str : system_pattern_strings) {
        try {
            system_message_patterns_.emplace_back(pattern_str);
        } catch (const std::regex_error& e) {
            LOGE("System message regex error: {} - {}", pattern_str, e.what());
        }
    }
}

std::vector<utils::OCRItem> ContentMarker::markContent(std::vector<utils::OCRItem> ocr_items,
                                                       const std::vector<utils::AvatarItem>& avatar_items,
                                                       const cv::Mat& original_image) {
    LOGD("Starting content marking...");

    // 创建工作副本（保存原始文本）
    for (auto& item : ocr_items) {
        item.original_text = item.text;  // 保存原始文本
    }

    // 排序 - 直接按Y坐标排序，不进行同一行判断
    std::sort(ocr_items.begin(), ocr_items.end(), [](const auto& a, const auto& b) {
        return a.get_center_y() < b.get_center_y();  // 按Y坐标从上到下排序
    });

    // 对头像也进行排序
    auto sorted_avatars = avatar_items;
    std::sort(sorted_avatars.begin(), sorted_avatars.end(),
              [](const auto& a, const auto& b) { return a.center_y() < b.center_y(); });

    // 判断是否为飞书截图并详细打印
    bool is_feishu = detectAndPrintPlatformType(ocr_items);

    // 1. 标记时间
    markTimeContent(ocr_items);

    // 2. 标记系统消息（仅飞书）
    if (is_feishu) {
        markSystemMessages(ocr_items);
    }

    // 3. 基于头像位置标记昵称和内容
    std::string mode = is_feishu ? "feishu" : "common";
    LOGI("Using {} processing mode", mode);
    if (is_feishu) {
        markNicknameAndContentFeishu(ocr_items, sorted_avatars, original_image);
    } else {
        markNicknameAndContentWechat(ocr_items, sorted_avatars, original_image);
    }

    // 4. 标记"我的内容"（基于颜色检测）
    if (!is_feishu) {  // 飞书不需要颜色检测
        LOGD("Enabling color detection for my content");
        markMyContent(ocr_items, sorted_avatars, original_image);
    } else {
        LOGD("Skipping color detection for Feishu platform");
    }

    LOGD("Content marking completed");
    return ocr_items;
}

bool ContentMarker::detectAndPrintPlatformType(const std::vector<utils::OCRItem>& ocr_items) {
    std::set<std::string> detected_keywords;
    std::vector<std::map<std::string, std::string>> keyword_positions;

    // 检测飞书关键词
    for (size_t i = 0; i < ocr_items.size(); i++) {
        const std::string& text = ocr_items[i].text;
        for (const std::string& keyword : feishu_keywords_) {
            if (text.find(keyword) != std::string::npos) {
                detected_keywords.insert(keyword);
                std::map<std::string, std::string> pos_info;
                pos_info["keyword"] = keyword;
                pos_info["text"] = text;
                pos_info["position"] = std::to_string(i);
                keyword_positions.push_back(pos_info);
            }
        }
    }

    bool is_feishu = (detected_keywords.size() == feishu_keywords_.size());

    LOGD("\n{}", std::string(50, '-'));
    LOGD("Content Marker - Platform Detection");
    LOGD("{}", std::string(50, '-'));

    // 收集所有文本用于分析
    LOGD("Recognized text count: {}", ocr_items.size());

    // 详细打印检测结果
    LOGD("Feishu keyword detection ({}/{}):", detected_keywords.size(), feishu_keywords_.size());

    for (const std::string& keyword : feishu_keywords_) {
        if (detected_keywords.count(keyword)) {
            LOGD(" '{}' - Found", keyword);
        } else {
            LOGD(" '{}' - Not found", keyword);
        }
    }

    if (!keyword_positions.empty()) {
        LOGD("\n Keyword positions:");
        for (size_t i = 0; i < std::min(keyword_positions.size(), size_t(3)); i++) {  // 只显示前3个
            auto& pos_info = keyword_positions[i];
            std::string display_text = pos_info["text"];
            if (display_text.length() > DISPLAY_TEXT_MAX_LENGTH) {
                display_text = display_text.substr(0, DISPLAY_TEXT_MAX_LENGTH) + "...";
            }
            LOGD("  - '{}' in text: '{}'", pos_info["keyword"], display_text);
        }
    }

    // 输出最终判断
    if (is_feishu) {
        LOGD("\nResult: Feishu screenshot");
        LOGD("All required keywords detected");
    } else {
        std::set<std::string> missing_keywords;
        for (const std::string& keyword : feishu_keywords_) {
            if (!detected_keywords.count(keyword)) {
                missing_keywords.insert(keyword);
            }
        }
        std::string missing_str;
        for (auto it = missing_keywords.begin(); it != missing_keywords.end(); ++it) {
            if (it != missing_keywords.begin()) missing_str += ", ";
            missing_str += *it;
        }
        LOGD("\nResult: WeChat/Lanxin/DingTalk");
        LOGD(" Missing keywords: {}", missing_str);
    }

    LOGD("{}\n", std::string(50, '-'));

    return is_feishu;
}

void ContentMarker::markTimeContent(std::vector<utils::OCRItem>& ocr_items) {
    int marked_count = 0;

    for (auto& item : ocr_items) {
        std::string text = item.text;
        // 移除首尾空白
        text.erase(text.begin(),
                   std::find_if(text.begin(), text.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        text.erase(std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
                   text.end());

        // 排除过长的文本
        if (utils::utf8_char_count(text) > TIME_TEXT_MAX_LENGTH) {
            continue;
        }

        // 排除包含非时间关键词的文本
        bool has_exclude_keyword = false;
        for (const std::string& keyword : time_exclude_keywords_) {
            if (text.find(keyword) != std::string::npos) {
                has_exclude_keyword = true;
                break;
            }
        }
        if (has_exclude_keyword) {
            continue;
        }

        // 检查时间模式
        bool is_time = false;
        for (size_t i = 0; i < time_patterns_.size(); ++i) {
            const auto& pattern = time_patterns_[i];
            std::smatch match;
            if (std::regex_search(text, match, pattern)) {
                double matched_length = utils::utf8_char_count(match.str());
                double match_ratio = matched_length / utils::utf8_char_count(text);

                // 对于复合时间格式，降低阈值要求
                double threshold = 0.6;  // 默认阈值

                // 获取对应的模式字符串（从Config获取）
                auto pattern_strs = config_.get_time_patterns();
                if (i < pattern_strs.size()) {
                    const std::string& pattern_str = pattern_strs[i];

                    // 检查是否为复合时间格式（昨天|今天等）- 索引2和10
                    if (i == 2 || i == 3 || i == 10) {  // (昨天|今天)类模式
                        threshold = 0.4;
                    }
                    // 检查是否为年份格式 - 索引0和1
                    else if (i == 0 || i == 1) {  // 年份模式
                        threshold = 0.7;
                    }
                }

                if (match_ratio >= threshold) {
                    is_time = true;
                    break;
                }
            }
        }

        if (is_time) {
            item.text = text + "(时间)";
            marked_count++;
        }
    }

    LOGD("Marked {} time items", marked_count);
}

void ContentMarker::markSystemMessages(std::vector<utils::OCRItem>& ocr_items) {
    int marked_count = 0;

    for (auto& item : ocr_items) {
        const std::string& text = item.text;

        // 跳过已经标记过的内容
        if (utils::str_contains(text, "(时间)") || utils::str_contains(text, "(昵称)") ||
            utils::str_contains(text, "(内容)") || utils::str_contains(text, "(我的内容)") ||
            utils::str_contains(text, "(系统消息)")) {
            continue;
        }

        // 检查是否匹配系统消息模式
        for (const auto& pattern : system_message_patterns_) {
            if (std::regex_search(text, pattern)) {
                item.text = text + "(系统消息)";
                marked_count++;
                break;
            }
        }
    }

    LOGD("Marked {} system messages", marked_count);
}

bool ContentMarker::is_new_message_notification(const std::string& text) {
    // 判断是否为"X条新消息"格式的系统提示
    // 支持匹配："1条新消息"、"10条新消息"等
    std::regex new_message_pattern(R"(\d+\s*条\s*新\s*消\s*息)");
    return std::regex_search(text, new_message_pattern);
}

void ContentMarker::markNicknameAndContentWechat(std::vector<utils::OCRItem>& ocr_items,
                                                 const std::vector<utils::AvatarItem>& avatar_items,
                                                 const cv::Mat& original_image) {
    LOGD("Using WeChat mode to mark nicknames and content...");

    // 特殊处理：如果没有头像，将第一个非时间的文本标记为昵称
    if (avatar_items.empty()) {
        LOGD("No avatar detected, marking first non-time text as nickname");
        for (auto& ocr_item : ocr_items) {
            if (!utils::str_contains(ocr_item.text, "(时间)") && !utils::str_contains(ocr_item.text, "(昵称)")) {
                ocr_item.text = ocr_item.text + "(昵称)";
                LOGD("Marked first non-time text as nickname: {}", ocr_item.text);
                break;
            }
        }
        // 如果没有头像，直接返回，不插入虚拟昵称
        return;
    }

    std::vector<std::pair<int, utils::OCRItem>> virtual_nicknames_to_insert;

    for (size_t i = 0; i < avatar_items.size(); i++) {
        const auto& avatar = avatar_items[i];

        // 寻找在当前头像Y范围内的文本作为昵称
        bool nickname_found = false;

        for (auto& ocr_item : ocr_items) {
            if (utils::str_contains(ocr_item.text, "(时间)")) {
                continue;
            }

            float box_y_min = ocr_item.get_min_y();

            // 检查是否在头像Y范围内
            if (avatar.y() <= box_y_min && box_y_min <= avatar.y() + avatar.height()) {
                // 额外检查：避免将内容误标记为昵称
                //     ocr_item.text = ocr_item.text + "(昵称)";
                //     nickname_found = true;
                ocr_item.text = ocr_item.text + "(昵称)";
                nickname_found = true;
                break;
            }
        }

        // 如果没有找到昵称，创建虚拟昵称
        if (!nickname_found) {
            int insert_index = find_insert_index(ocr_items, avatar.y() + avatar.height());

            utils::OCRItem virtual_nickname;
            virtual_nickname.text = "未知用户" + std::to_string(i + 1) + "(昵称)";
            virtual_nickname.box = {{avatar.x(), avatar.y()},
                                    {avatar.x() + avatar.width(), avatar.y()},
                                    {avatar.x() + avatar.width(), avatar.y() + avatar.height()},
                                    {avatar.x(), avatar.y() + avatar.height()}};
            virtual_nickname.score = 0.0f;
            virtual_nickname.slice_index = avatar.slice_index;
            virtual_nickname.is_virtual = true;

            virtual_nicknames_to_insert.emplace_back(insert_index, virtual_nickname);
        }
    }

    // 插入虚拟昵称（按索引倒序插入）
    std::sort(virtual_nicknames_to_insert.begin(), virtual_nicknames_to_insert.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (const auto& [insert_index, virtual_nickname] : virtual_nicknames_to_insert) {
        ocr_items.insert(ocr_items.begin() + insert_index, virtual_nickname);
    }

    // 标记内容
    markContentAfterNicknames(ocr_items, avatar_items);
}

void ContentMarker::markNicknameAndContentFeishu(std::vector<utils::OCRItem>& ocr_items,
                                                 const std::vector<utils::AvatarItem>& avatar_items,
                                                 const cv::Mat& original_image) {
    LOGD("Using Feishu mode to mark nicknames and content...");

    // 过滤小面积头像
    auto filtered_avatars = filter_avatars_by_area(avatar_items);

    std::vector<std::map<std::string, std::vector<int>>> nickname_operations;
    std::vector<std::map<std::string, int>> virtual_nicknames_to_insert;

    for (size_t i = 0; i < filtered_avatars.size(); i++) {
        const auto& avatar = filtered_avatars[i];

        // 寻找在当前头像Y范围内的文本作为昵称
        std::vector<std::string> nickname_texts;
        std::vector<int> nickname_indices;

        for (size_t j = 0; j < ocr_items.size(); j++) {
            const auto& ocr_item = ocr_items[j];
            if (utils::str_contains(ocr_item.text, "(时间)") || utils::str_contains(ocr_item.text, "(系统消息)")) {
                continue;
            }

            float box_y_min = ocr_item.get_min_y();

            // 检查是否在头像Y范围内
            if (avatar.y() <= box_y_min && box_y_min <= avatar.y() + avatar.height()) {
                nickname_texts.push_back(ocr_item.text);
                nickname_indices.push_back(static_cast<int>(j));
            }
        }

        if (!nickname_indices.empty()) {
            // 记录昵称合并操作
            std::map<std::string, std::vector<int>> operation;
            operation["indices"] = nickname_indices;
            nickname_operations.push_back(operation);

            // 合并昵称文本
            std::string merged_text;
            for (size_t k = 0; k < nickname_texts.size(); k++) {
                if (k > 0) merged_text += " ";
                merged_text += nickname_texts[k];
            }
            merged_text += "(昵称)";
            operation["merged_text"] = {static_cast<int>(merged_text.length())};  // 存储长度作为标识
        } else {
            // 创建虚拟昵称
            int insert_index = find_insert_index(ocr_items, avatar.y() + avatar.height());

            std::map<std::string, int> virtual_item;
            virtual_item["avatar_index"] = static_cast<int>(i);
            virtual_item["insert_index"] = insert_index;
            virtual_nicknames_to_insert.push_back(virtual_item);
        }
    }

    // 执行昵称合并（逆序处理）
    for (auto it = nickname_operations.rbegin(); it != nickname_operations.rend(); ++it) {
        const auto& operation = *it;
        const auto& indices = operation.at("indices");

        if (indices.empty()) continue;

        // 创建合并文本（添加边界检查）
        std::string merged_text;
        for (size_t k = 0; k < indices.size(); k++) {
            int idx = indices[k];
            if (idx >= 0 && idx < static_cast<int>(ocr_items.size())) {
                if (k > 0) merged_text += " ";
                merged_text += ocr_items[idx].text;
            }
        }
        merged_text += "(昵称)";

        // 更新第一个昵称项（添加边界检查）
        int first_index = indices[0];
        if (first_index >= 0 && first_index < static_cast<int>(ocr_items.size())) {
            ocr_items[first_index].text = merged_text;
        }

        // 删除多余的昵称项（按倒序删除避免索引失效）
        if (indices.size() > 1) {
            std::vector<int> indices_to_delete(indices.begin() + 1, indices.end());
            // 按倒序排序
            std::sort(indices_to_delete.begin(), indices_to_delete.end(), std::greater<int>());
            // 倒序删除
            for (int idx : indices_to_delete) {
                if (idx >= 0 && idx < static_cast<int>(ocr_items.size())) {
                    ocr_items.erase(ocr_items.begin() + idx);
                }
            }
        }
    }

    // 重新计算虚拟昵称的插入位置并插入
    for (auto& virtual_item : virtual_nicknames_to_insert) {
        int avatar_index = virtual_item["avatar_index"];
        const auto& avatar = filtered_avatars[avatar_index];

        int insert_index = find_insert_index(ocr_items, avatar.y() + avatar.height());
        virtual_item["insert_index"] = insert_index;
    }

    // 按插入位置倒序排序并插入
    std::sort(virtual_nicknames_to_insert.begin(), virtual_nicknames_to_insert.end(),
              [](const auto& a, const auto& b) { return a.at("insert_index") > b.at("insert_index"); });

    for (const auto& virtual_item : virtual_nicknames_to_insert) {
        int insert_index = virtual_item.at("insert_index");
        int avatar_index = virtual_item.at("avatar_index");
        const auto& avatar = filtered_avatars[avatar_index];

        utils::OCRItem virtual_nickname;
        virtual_nickname.text = "未知用户" + std::to_string(avatar_index + 1) + "(昵称)";
        virtual_nickname.box = {{avatar.x(), avatar.y()},
                                {avatar.x() + avatar.width(), avatar.y()},
                                {avatar.x() + avatar.width(), avatar.y() + avatar.height()},
                                {avatar.x(), avatar.y() + avatar.height()}};
        virtual_nickname.score = 0.0f;
        virtual_nickname.slice_index = avatar.slice_index;
        virtual_nickname.is_virtual = true;

        ocr_items.insert(ocr_items.begin() + insert_index, virtual_nickname);
    }

    // 标记内容
    markContentAfterNicknames(ocr_items, filtered_avatars);
}

void ContentMarker::markContentAfterNicknames(std::vector<utils::OCRItem>& ocr_items,
                                              const std::vector<utils::AvatarItem>& avatar_items) {
    // 添加输入验证
    if (avatar_items.empty() || ocr_items.empty()) {
        return;
    }

    for (size_t i = 0; i < avatar_items.size(); i++) {
        const auto& avatar = avatar_items[i];

        // 找到下一个头像的Y位置边界（添加边界检查）
        int next_boundary =
            (i + 1 < avatar_items.size()) ? static_cast<int>(avatar_items[i + 1].y()) : std::numeric_limits<int>::max();

        // 找到对应的昵称
        int nickname_index = -1;
        for (size_t j = 0; j < ocr_items.size(); j++) {
            if (utils::str_contains(ocr_items[j].text, "(昵称)")) {
                float box_y_min = ocr_items[j].get_min_y();
                if (avatar.y() <= box_y_min && box_y_min <= avatar.y() + avatar.height() - 1) {
                    nickname_index = static_cast<int>(j);
                    break;
                }
            }
        }

        if (nickname_index >= 0) {
            // 标记该昵称后的内容
            for (size_t k = nickname_index + 1; k < ocr_items.size(); k++) {
                auto& next_ocr = ocr_items[k];
                if (utils::str_contains(next_ocr.text, "(时间)") || utils::str_contains(next_ocr.text, "(昵称)") ||
                    utils::str_contains(next_ocr.text, "(系统消息)")) {
                    continue;
                }

                float next_box_y_min = next_ocr.get_min_y();

                // 检查是否在当前头像区域内且未到达下一个头像边界
                if (next_box_y_min > avatar.y() && next_box_y_min < next_boundary) {
                    if (!utils::str_contains(next_ocr.text, "(内容)")) {
                        next_ocr.text = next_ocr.text + "(内容)";
                    }
                } else if (next_box_y_min >= next_boundary) {
                    break;
                }
            }
        }
    }
}

int ContentMarker::find_insert_index(const std::vector<utils::OCRItem>& ocr_items, float y_threshold) {
    // 找到第一个Y坐标大于阈值的位置
    for (size_t idx = 0; idx < ocr_items.size(); idx++) {
        float item_y_min = ocr_items[idx].get_min_y();
        if (item_y_min > y_threshold) {
            return static_cast<int>(idx);
        }
    }
    return static_cast<int>(ocr_items.size());
}

std::vector<utils::AvatarItem> ContentMarker::filter_avatars_by_area(
    const std::vector<utils::AvatarItem>& avatar_items) {
    if (avatar_items.empty()) {
        return {};
    }

    // 计算面积均值
    float total_area = 0.0f;
    for (const auto& item : avatar_items) {
        total_area += item.area();
    }
    float mean_area = total_area / avatar_items.size();

    // 筛选面积大于等于均值的头像
    std::vector<utils::AvatarItem> filtered;
    for (const auto& item : avatar_items) {
        if (item.area() >= mean_area) {
            filtered.push_back(item);
        }
    }

    LOGD("Avatar filtering: {} -> {}", avatar_items.size(), filtered.size());
    return filtered;
}

void ContentMarker::markMyContent(std::vector<utils::OCRItem>& ocr_items,
                                  const std::vector<utils::AvatarItem>& avatar_items, const cv::Mat& original_image) {
    if (original_image.empty()) {
        LOGD("Original image not available, skipping color detection");
        return;
    }

    LOGD("Starting color-based my content detection...");

    // 第一轮：基于颜色检测
    std::vector<std::pair<int, std::vector<std::vector<float>>>> my_content_boxes;

    for (size_t i = 0; i < ocr_items.size(); i++) {
        auto& item = ocr_items[i];
        if (utils::str_contains(item.text, "(内容)")) {
            const auto& box = item.box;

            // 检测绿色和蓝色背景
            bool is_green = detectGreenContentBox(original_image, box);
            bool is_blue = detectBlueContentBox(original_image, box);

            if (is_green || is_blue) {
                // 检查是否为"X条新消息"的系统提示
                if (is_green && is_new_message_notification(item.original_text)) {
                    // 跳过绿色背景的"新消息"提示，不标记为"我的内容"
                    //     (绿色背景但是'新消息'通知)"
                    LOGD("Skipping system notification: {} (green bg but 'new message')", item.text);
                } else {
                    item.text = utils::str_replace(item.text, "(内容)", "(我的内容)");
                    my_content_boxes.emplace_back(static_cast<int>(i), box);
                    std::string reason = is_green ? "green background" : "blue background";
                    LOGD("Marked as my content: {} (reason: {})", item.text, reason);
                }
            }
        }
    }

    // 第二轮：基于位置推理
    markAdjacentMyContent(ocr_items, my_content_boxes, avatar_items, original_image);
}

bool ContentMarker::detectColorInBox(const cv::Mat& image, const std::vector<std::vector<float>>& box,
                                     const cv::Scalar& hsv_lower, const cv::Scalar& hsv_upper, double ratio_threshold) {
    try {
        // 输入验证
        if (image.empty() || box.empty()) {
            LOGE("Warning: Image or bounding box is empty");
            return false;
        }

        // 获取文本框区域
        auto bounds = getBoxBounds(config_, box);
        int min_x = std::max(0, static_cast<int>(bounds.min_x));
        int max_x = std::min(image.cols, static_cast<int>(bounds.max_x));
        int min_y = std::max(0, static_cast<int>(bounds.min_y));
        int max_y = std::min(image.rows, static_cast<int>(bounds.max_y));

        // 边界检查
        if (max_x <= min_x || max_y <= min_y || min_x < 0 || min_y < 0 || max_x > image.cols || max_y > image.rows) {
            LOGE(
                "Warning: Bounding box exceeds image bounds - min_x:{} max_x:{} min_y:{} max_y:{} "
                "image size:{}x{}",
                min_x, max_x, min_y, max_y, image.cols, image.rows);
            return false;
        }

        // 提取区域图像
        cv::Mat roi = image(cv::Rect(min_x, min_y, max_x - min_x, max_y - min_y));
        if (roi.empty()) {
            return false;
        }

        // 转换为HSV颜色空间
        cv::Mat hsv;
        cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);

        // 创建颜色掩码
        cv::Mat mask;
        cv::inRange(hsv, hsv_lower, hsv_upper, mask);

        // 计算颜色像素的比例
        int color_pixels = cv::countNonZero(mask);
        int total_pixels = roi.rows * roi.cols;

        if (total_pixels > 0) {
            double color_ratio = static_cast<double>(color_pixels) / total_pixels;
            return color_ratio > ratio_threshold;
        }

        return false;

    } catch (const std::exception& e) {
        LOGE("Error detecting color box: {}", e.what());
        return false;
    }
}

bool ContentMarker::detectGreenContentBox(const cv::Mat& image, const std::vector<std::vector<float>>& box) {
    return detectColorInBox(image, box, green_hsv_lower_, green_hsv_upper_, green_ratio_threshold_);
}

bool ContentMarker::detectBlueContentBox(const cv::Mat& image, const std::vector<std::vector<float>>& box) {
    // 先检测蓝色
    if (!detectColorInBox(image, box, blue_hsv_lower_, blue_hsv_upper_, blue_ratio_threshold_)) {
        return false;
    }

    // 蓝色检测还需要额外检查白色背景比例
    try {
        auto bounds = getBoxBounds(config_, box);
        int min_x = std::max(0, static_cast<int>(bounds.min_x));
        int max_x = std::min(image.cols, static_cast<int>(bounds.max_x));
        int min_y = std::max(0, static_cast<int>(bounds.min_y));
        int max_y = std::min(image.rows, static_cast<int>(bounds.max_y));

        if (max_x <= min_x || max_y <= min_y) {
            return false;
        }

        cv::Mat roi = image(cv::Rect(min_x, min_y, max_x - min_x, max_y - min_y));
        if (roi.empty()) {
            return false;
        }

        cv::Mat hsv;
        cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);

        // 检测白色和蓝色比例
        cv::Mat white_mask, blue_mask;
        cv::inRange(hsv, white_hsv_lower_, white_hsv_upper_, white_mask);
        cv::inRange(hsv, blue_hsv_lower_, blue_hsv_upper_, blue_mask);

        int white_pixels = cv::countNonZero(white_mask);
        int blue_pixels = cv::countNonZero(blue_mask);
        int total_pixels = roi.rows * roi.cols;

        if (total_pixels > 0) {
            double white_ratio = static_cast<double>(white_pixels) / total_pixels;
            double blue_ratio = static_cast<double>(blue_pixels) / total_pixels;
            return white_ratio < white_ratio_threshold_ && blue_ratio > white_ratio;
        }

        return false;

    } catch (const std::exception& e) {
        LOGE("Error detecting blue box: {}", e.what());
        return false;
    }
}

void ContentMarker::markAdjacentMyContent(
    std::vector<utils::OCRItem>& ocr_items,
    std::vector<std::pair<int, std::vector<std::vector<float>>>>& my_content_boxes,
    const std::vector<utils::AvatarItem>& avatar_items, const cv::Mat& original_image) {
    LOGD("Starting position inference: {} my content boxes", my_content_boxes.size());

    if (my_content_boxes.empty() || avatar_items.empty()) {
        return;
    }

    // 使用安全的方式处理动态修改，避免容器修改时的问题
    std::vector<std::pair<int, std::vector<std::vector<float>>>> newly_marked_content;
    size_t original_size = my_content_boxes.size();

    for (size_t i = 0; i < original_size; i++) {
        // 安全检查数组边界
        if (i >= my_content_boxes.size()) {
            break;
        }

        int my_index = my_content_boxes[i].first;
        const auto& my_box = my_content_boxes[i].second;
        auto my_bounds = getBoxBounds(config_, my_box);
        float my_y_max = my_bounds.max_y;

        // 查找下一条内容
        for (size_t next_index = my_index + 1; next_index < ocr_items.size(); next_index++) {
            auto& next_item = ocr_items[next_index];

            // 跳过已经标记为我的内容的
            if (utils::str_contains(next_item.text, "(我的内容)")) {
                continue;
            }

            // 只处理内容标记
            if (!utils::str_contains(next_item.text, "(内容)")) {
                continue;
            }

            const auto& next_box = next_item.box;
            auto next_bounds = getBoxBounds(config_, next_box);
            float next_y_min = next_bounds.min_y;

            // 检查是否已通过颜色检测
            bool is_green = detectGreenContentBox(original_image, next_box);
            bool is_blue = detectBlueContentBox(original_image, next_box);
            if (is_green || is_blue) {
                continue;
            }

            // 检查位置条件
            if (isAdjacentMyContent(my_box, next_box, avatar_items)) {
                next_item.text = utils::str_replace(next_item.text, "(内容)", "(我的内容)");
                LOGD("Marked as my content by position inference: {}", next_item.text);
                // 收集新标记的内容，稍后统一添加
                newly_marked_content.emplace_back(static_cast<int>(next_index), next_box);
            }
        }
    }

    // 将新标记的内容统一添加到列表中
    for (const auto& new_content : newly_marked_content) {
        my_content_boxes.push_back(new_content);
    }
}

bool ContentMarker::isAdjacentMyContent(const std::vector<std::vector<float>>& my_box,
                                        const std::vector<std::vector<float>>& next_box,
                                        const std::vector<utils::AvatarItem>& avatar_items) {
    auto my_bounds = getBoxBounds(config_, my_box);
    auto next_bounds = getBoxBounds(config_, next_box);

    // 检查是否在最近的两个头像框之间
    return is_between_avatars(my_bounds.max_y, next_bounds.min_y, avatar_items);
}

bool ContentMarker::is_between_avatars(float start_y, float end_y, const std::vector<utils::AvatarItem>& avatar_items) {
    if (avatar_items.empty()) {
        return true;
    }

    // 找到包含start_y的头像对
    for (size_t i = 0; i < avatar_items.size() - 1; i++) {
        const auto& avatar1 = avatar_items[i];
        const auto& avatar2 = avatar_items[i + 1];

        float avatar1_y_max = avatar1.y() + avatar1.height() - 1;
        float avatar2_y_min = avatar2.y();

        // 检查是否在这两个头像之间
        if (avatar1_y_max <= start_y && end_y <= avatar2_y_min) {
            return true;
        }
    }

    return false;
}

}  // namespace detection
