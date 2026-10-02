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
 * 内容标记处理器模块
 * 负责标记OCR识别结果的类型（时间、昵称、内容等）
 */

#pragma once

#include <opencv2/opencv.hpp>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "../utils/config.h"
#include "../utils/ocravatartypes.h"
#include "../utils/sliceinfo.h"

namespace detection {

/**
 * 内容标记处理器
 * 实现颜色检测(HSV)、内容类型标记和平台特征识别功能
 */
class ContentMarker {
public:
    /**
     * 构造函数
     *
     * @param config 配置对象
     */
    explicit ContentMarker(const utils::Config& config);

    /**
     * 标记内容主入口
     *
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     * @param original_image 原始图像
     * @return 标记后的OCR项列表
     */
    std::vector<utils::OCRItem> markContent(std::vector<utils::OCRItem> ocr_items,
                                            const std::vector<utils::AvatarItem>& avatar_items,
                                            const cv::Mat& original_image);

private:
    // 配置相关
    const utils::Config& config_;

    // 颜色检测参数
    cv::Scalar green_hsv_lower_;
    cv::Scalar green_hsv_upper_;
    double green_ratio_threshold_;

    cv::Scalar blue_hsv_lower_;
    cv::Scalar blue_hsv_upper_;
    double blue_ratio_threshold_;

    cv::Scalar white_hsv_lower_;
    cv::Scalar white_hsv_upper_;
    double white_ratio_threshold_;

    // 模式匹配相关
    std::vector<std::regex> time_patterns_;
    std::vector<std::regex> system_message_patterns_;
    std::vector<std::string> time_exclude_keywords_;
    std::vector<std::string> feishu_keywords_;

    /**
     * 初始化正则表达式模式
     */
    void initializePatterns();

    /**
     * 检测并详细打印平台类型
     *
     * @param ocr_items OCR识别项列表
     * @return 是否为飞书截图
     */
    bool detectAndPrintPlatformType(const std::vector<utils::OCRItem>& ocr_items);

    /**
     * 标记时间内容
     *
     * @param ocr_items OCR识别项列表
     */
    void markTimeContent(std::vector<utils::OCRItem>& ocr_items);

    /**
     * 标记系统消息
     *
     * @param ocr_items OCR识别项列表
     */
    void markSystemMessages(std::vector<utils::OCRItem>& ocr_items);

    /**
     * 微信等平台的昵称和内容标记
     *
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     * @param original_image 原始图像
     */
    void markNicknameAndContentWechat(std::vector<utils::OCRItem>& ocr_items,
                                      const std::vector<utils::AvatarItem>& avatar_items,
                                      const cv::Mat& original_image);

    /**
     * 飞书的昵称和内容标记
     *
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     * @param original_image 原始图像
     */
    void markNicknameAndContentFeishu(std::vector<utils::OCRItem>& ocr_items,
                                      const std::vector<utils::AvatarItem>& avatar_items,
                                      const cv::Mat& original_image);

    /**
     * 在昵称后标记内容
     *
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     */
    void markContentAfterNicknames(std::vector<utils::OCRItem>& ocr_items,
                                   const std::vector<utils::AvatarItem>& avatar_items);

    /**
     * 标记"我的内容"（基于颜色检测）
     *
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     * @param original_image 原始图像
     */
    void markMyContent(std::vector<utils::OCRItem>& ocr_items, const std::vector<utils::AvatarItem>& avatar_items,
                       const cv::Mat& original_image);

    /**
     * 检测文本框区域是否为指定颜色背景（通用函数）
     *
     * @param image 图像
     * @param box 文本框
     * @param hsv_lower HSV颜色空间下限
     * @param hsv_upper HSV颜色空间上限
     * @param ratio_threshold 颜色比例阈值
     * @return 是否为指定颜色背景
     */
    bool detectColorInBox(const cv::Mat& image, const std::vector<std::vector<float>>& box, const cv::Scalar& hsv_lower,
                          const cv::Scalar& hsv_upper, double ratio_threshold);

    /**
     * 检测文本框区域是否为绿色背景
     *
     * @param image 图像
     * @param box 文本框
     * @return 是否为绿色背景
     */
    bool detectGreenContentBox(const cv::Mat& image, const std::vector<std::vector<float>>& box);

    /**
     * 检测文本框区域是否为蓝色背景
     *
     * @param image 图像
     * @param box 文本框
     * @return 是否为蓝色背景
     */
    bool detectBlueContentBox(const cv::Mat& image, const std::vector<std::vector<float>>& box);

    /**
     * 基于位置推理标记相邻的我的内容
     *
     * @param ocr_items OCR识别项列表
     * @param my_content_boxes 我的内容框列表
     * @param avatar_items 头像项列表
     * @param original_image 原始图像
     */
    void markAdjacentMyContent(std::vector<utils::OCRItem>& ocr_items,
                               std::vector<std::pair<int, std::vector<std::vector<float>>>>& my_content_boxes,
                               const std::vector<utils::AvatarItem>& avatar_items, const cv::Mat& original_image);

    /**
     * 检查下一条内容是否应该标记为我的内容
     *
     * @param my_box 我的内容框
     * @param next_box 下一个内容框
     * @param avatar_items 头像项列表
     * @return 是否应该标记为我的内容
     */
    bool isAdjacentMyContent(const std::vector<std::vector<float>>& my_box,
                             const std::vector<std::vector<float>>& next_box,
                             const std::vector<utils::AvatarItem>& avatar_items);

    /**
     * 检查Y坐标范围是否在最近的两个头像框之间
     *
     * @param start_y 起始Y坐标
     * @param end_y 结束Y坐标
     * @param avatar_items 头像项列表
     * @return 是否在头像框之间
     */
    bool is_between_avatars(float start_y, float end_y, const std::vector<utils::AvatarItem>& avatar_items);

    /**
     * 过滤小面积头像（飞书专用）
     *
     * @param avatar_items 头像项列表
     * @return 过滤后的头像项列表
     */
    std::vector<utils::AvatarItem> filter_avatars_by_area(const std::vector<utils::AvatarItem>& avatar_items);

    /**
     * 找到在OCR列表中的插入位置
     *
     * @param ocr_items OCR识别项列表
     * @param y_threshold Y坐标阈值
     * @return 插入位置索引
     */
    int find_insert_index(const std::vector<utils::OCRItem>& ocr_items, float y_threshold);

    /**
     * 检查字符串是否包含指定内容
     *
     * @param text 待检查的文本
     * @param substr 子字符串
     * @return 是否包含
     */
    bool contains(const std::string& text, const std::string& substr);

    /**
     * 替换字符串中的内容
     *
     * @param text 原文本
     * @param from 被替换的内容
     * @param to 替换为的内容
     * @return 替换后的文本
     */
    std::string replace_string(const std::string& text, const std::string& from, const std::string& to);

    /**
     * 判断文本是否为"X条新消息"格式的系统提示
     *
     * @param text 待检查的文本
     * @return 是否为新消息通知
     */
    bool is_new_message_notification(const std::string& text);
};

}  // namespace detection
