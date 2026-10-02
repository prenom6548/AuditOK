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

#pragma once

#include <jsoncpp/json/json.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace utils {

/**
 * OCR相关配置
 */
struct OCRConfig {
    double text_score_threshold = 0.00;

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 图像处理相关配置
 */
struct ImageConfig {
    int slice_height = 1200;
    int overlap = 200;
    int binary_threshold = 230;
    std::tuple<int, int> gaussian_blur_size = {5, 5};
    double merge_distance_factor = 1.0;  // 合并距离因子

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 头像检测相关配置
 */
struct AvatarConfig {
    double strict_square_ratio_min = 0.8;
    double strict_square_ratio_max = 1.2;
    double iou_threshold = 0.0;
    int x_crop_offset = 3;

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 去重相关配置
 */
struct DeduplicationConfig {
    double ocr_iou_threshold = 0.3;
    double avatar_iou_threshold = 0.0;

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 内容标记相关配置
 */
struct ContentMarkingConfig {
    std::tuple<int, int, int> green_hsv_lower = {35, 40, 40};
    std::tuple<int, int, int> green_hsv_upper = {85, 255, 255};
    double green_ratio_threshold = 0.2;

    std::tuple<int, int, int> blue_hsv_lower = {100, 30, 80};
    std::tuple<int, int, int> blue_hsv_upper = {130, 180, 255};
    double blue_ratio_threshold = 0.3;

    std::tuple<int, int, int> white_hsv_lower = {0, 0, 200};
    std::tuple<int, int, int> white_hsv_upper = {180, 30, 255};
    double white_ratio_threshold = 0.5;

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 输出相关配置
 */
struct OutputConfig {
    std::string output_json_dir = "./output_json";
    std::string output_images_dir = "./output_images";
    std::string debug_dir = "./output_images/debug";

    // 转换为JSON
    Json::Value to_json() const;

    // 从JSON加载
    void from_json(const Json::Value& j);
};

/**
 * 主配置类
 * 包含所有子配置模块
 */
class Config {
public:
    // 单例访问
    static Config& getInstance();

    // 禁用拷贝构造和赋值
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 配置子模块
    OCRConfig ocr;
    ImageConfig image;
    AvatarConfig avatar;
    DeduplicationConfig deduplication;
    ContentMarkingConfig content_marking;
    OutputConfig output;

    // 保存配置到文件（JSON格式）
    bool save_to_file(const std::string& config_file) const;

    // 重置为默认配置
    void reset_to_defaults();

    // 创建必要的输出目录
    bool create_output_directories();

    // 获取时间匹配模式列表
    std::vector<std::string> get_time_patterns() const;

    // 获取系统消息匹配模式列表
    std::vector<std::string> get_system_message_patterns() const;

    // 获取时间排除关键词列表
    std::vector<std::string> get_time_exclude_keywords() const;

    // 获取飞书截图识别关键词
    std::vector<std::string> get_feishu_keywords() const;

    // 获取当前构建模式（Debug/Release）
    bool isDebugMode() const;

    // 是否应该保存输出文件（Debug模式保存，Release模式不保存）
    bool shouldSaveOutputFiles() const;

    // 转换为JSON格式
    Json::Value to_json() const;

    // 从JSON格式加载
    bool from_json(const Json::Value& j);

private:
    Config();  // 私有构造函数

    // 创建单个目录
    bool create_directory(const std::string& path) const;
};

}  // namespace utils
