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

#include "config.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../utils/logger.h"
#include "jsonexporter.h"

namespace utils {

// OCRConfig 实现
Json::Value OCRConfig::to_json() const {
    Json::Value json;
    json["text_score_threshold"] = text_score_threshold;
    return json;
}

void OCRConfig::from_json(const Json::Value& j) {
    if (j.isMember("text_score_threshold")) {
        text_score_threshold = j["text_score_threshold"].asDouble();
    }
}

// ImageConfig 实现
Json::Value ImageConfig::to_json() const {
    Json::Value json;
    json["slice_height"] = slice_height;
    json["overlap"] = overlap;
    json["binary_threshold"] = binary_threshold;

    Json::Value blur_array(Json::arrayValue);
    blur_array.append(std::get<0>(gaussian_blur_size));
    blur_array.append(std::get<1>(gaussian_blur_size));
    json["gaussian_blur_size"] = blur_array;

    json["merge_distance_factor"] = merge_distance_factor;
    return json;
}

void ImageConfig::from_json(const Json::Value& j) {
    if (j.isMember("slice_height")) {
        slice_height = j["slice_height"].asInt();
    }
    if (j.isMember("overlap")) {
        overlap = j["overlap"].asInt();
    }
    if (j.isMember("binary_threshold")) {
        binary_threshold = j["binary_threshold"].asInt();
    }
    if (j.isMember("gaussian_blur_size") && j["gaussian_blur_size"].isArray()) {
        auto blur_array = j["gaussian_blur_size"];
        if (blur_array.size() >= 2) {
            gaussian_blur_size = {blur_array[0].asInt(), blur_array[1].asInt()};
        }
    }
    if (j.isMember("merge_distance_factor")) {
        merge_distance_factor = j["merge_distance_factor"].asDouble();
    }
}

// AvatarConfig 实现
Json::Value AvatarConfig::to_json() const {
    Json::Value json;
    json["strict_square_ratio_min"] = strict_square_ratio_min;
    json["strict_square_ratio_max"] = strict_square_ratio_max;
    json["iou_threshold"] = iou_threshold;
    json["x_crop_offset"] = x_crop_offset;
    return json;
}

void AvatarConfig::from_json(const Json::Value& j) {
    if (j.isMember("strict_square_ratio_min")) {
        strict_square_ratio_min = j["strict_square_ratio_min"].asDouble();
    }
    if (j.isMember("strict_square_ratio_max")) {
        strict_square_ratio_max = j["strict_square_ratio_max"].asDouble();
    }
    if (j.isMember("iou_threshold")) {
        iou_threshold = j["iou_threshold"].asDouble();
    }
    if (j.isMember("x_crop_offset")) {
        x_crop_offset = j["x_crop_offset"].asInt();
    }
}

// DeduplicationConfig 实现
Json::Value DeduplicationConfig::to_json() const {
    Json::Value json;
    json["ocr_iou_threshold"] = ocr_iou_threshold;
    json["avatar_iou_threshold"] = avatar_iou_threshold;
    return json;
}

void DeduplicationConfig::from_json(const Json::Value& j) {
    if (j.isMember("ocr_iou_threshold")) {
        ocr_iou_threshold = j["ocr_iou_threshold"].asDouble();
    }
    if (j.isMember("avatar_iou_threshold")) {
        avatar_iou_threshold = j["avatar_iou_threshold"].asDouble();
    }
}

// ContentMarkingConfig 实现
Json::Value ContentMarkingConfig::to_json() const {
    Json::Value json;

    auto create_array = [](int a, int b, int c) {
        Json::Value array(Json::arrayValue);
        array.append(a);
        array.append(b);
        array.append(c);
        return array;
    };

    json["green_hsv_lower"] =
        create_array(std::get<0>(green_hsv_lower), std::get<1>(green_hsv_lower), std::get<2>(green_hsv_lower));
    json["green_hsv_upper"] =
        create_array(std::get<0>(green_hsv_upper), std::get<1>(green_hsv_upper), std::get<2>(green_hsv_upper));
    json["green_ratio_threshold"] = green_ratio_threshold;

    json["blue_hsv_lower"] =
        create_array(std::get<0>(blue_hsv_lower), std::get<1>(blue_hsv_lower), std::get<2>(blue_hsv_lower));
    json["blue_hsv_upper"] =
        create_array(std::get<0>(blue_hsv_upper), std::get<1>(blue_hsv_upper), std::get<2>(blue_hsv_upper));
    json["blue_ratio_threshold"] = blue_ratio_threshold;

    json["white_hsv_lower"] =
        create_array(std::get<0>(white_hsv_lower), std::get<1>(white_hsv_lower), std::get<2>(white_hsv_lower));
    json["white_hsv_upper"] =
        create_array(std::get<0>(white_hsv_upper), std::get<1>(white_hsv_upper), std::get<2>(white_hsv_upper));
    json["white_ratio_threshold"] = white_ratio_threshold;

    return json;
}

void ContentMarkingConfig::from_json(const Json::Value& j) {
    auto load_tuple = [](const Json::Value& array) -> std::tuple<int, int, int> {
        if (array.isArray() && array.size() >= 3) {
            return {array[0].asInt(), array[1].asInt(), array[2].asInt()};
        }
        return {0, 0, 0};
    };

    if (j.isMember("green_hsv_lower")) {
        green_hsv_lower = load_tuple(j["green_hsv_lower"]);
    }
    if (j.isMember("green_hsv_upper")) {
        green_hsv_upper = load_tuple(j["green_hsv_upper"]);
    }
    if (j.isMember("green_ratio_threshold")) {
        green_ratio_threshold = j["green_ratio_threshold"].asDouble();
    }
    if (j.isMember("blue_hsv_lower")) {
        blue_hsv_lower = load_tuple(j["blue_hsv_lower"]);
    }
    if (j.isMember("blue_hsv_upper")) {
        blue_hsv_upper = load_tuple(j["blue_hsv_upper"]);
    }
    if (j.isMember("blue_ratio_threshold")) {
        blue_ratio_threshold = j["blue_ratio_threshold"].asDouble();
    }
    if (j.isMember("white_hsv_lower")) {
        white_hsv_lower = load_tuple(j["white_hsv_lower"]);
    }
    if (j.isMember("white_hsv_upper")) {
        white_hsv_upper = load_tuple(j["white_hsv_upper"]);
    }
    if (j.isMember("white_ratio_threshold")) {
        white_ratio_threshold = j["white_ratio_threshold"].asDouble();
    }
}

// OutputConfig 实现
Json::Value OutputConfig::to_json() const {
    Json::Value json;
    json["output_json_dir"] = output_json_dir;
    json["output_images_dir"] = output_images_dir;
    json["debug_dir"] = debug_dir;
    return json;
}

void OutputConfig::from_json(const Json::Value& j) {
    if (j.isMember("output_json_dir")) {
        output_json_dir = j["output_json_dir"].asString();
    }
    if (j.isMember("output_images_dir")) {
        output_images_dir = j["output_images_dir"].asString();
    }
    if (j.isMember("debug_dir")) {
        debug_dir = j["debug_dir"].asString();
    }
}

// Config 主类实现
Config::Config() {
    // 使用默认值初始化所有成员
    // 成员变量已在声明时初始化，无需额外操作
}

Config& Config::getInstance() {
    static Config instance;  // 线程安全的单例实现
    return instance;
}

bool Config::save_to_file(const std::string& config_file) const {
    try {
        std::ofstream file(config_file);
        if (!file.is_open()) {
            LOGE("Cannot create config file: {}", config_file);
            return false;
        }

        Json::Value config_json = to_json();
        file << export_ns::json_dump(config_json, 2);
        file.close();

        LOGD("Config saved to: {}", config_file);
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to save config file: {}", e.what());
        return false;
    }
}

void Config::reset_to_defaults() {
    // 重新构造所有配置对象以恢复默认值
    ocr = OCRConfig{};
    image = ImageConfig{};
    avatar = AvatarConfig{};
    deduplication = DeduplicationConfig{};
    content_marking = ContentMarkingConfig{};
    output = OutputConfig{};
}

bool Config::create_output_directories() {
    // 仅在Debug模式下创建目录
    if (!shouldSaveOutputFiles()) {
        return true;  // Release模式直接返回成功
    }

    try {
        std::vector<std::string> directories = {output.output_json_dir, output.output_images_dir, output.debug_dir};

        for (const auto& dir : directories) {
            if (!create_directory(dir)) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to create output directory: {}", e.what());
        return false;
    }
}

std::vector<std::string> Config::get_time_patterns() const {
    return {
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
        R"(周(一|二|三|四|五|六|日|天))"                                          // 星期
    };
}

std::vector<std::string> Config::get_system_message_patterns() const {
    return {
        R"(欢迎.*加入)",              // 欢迎xxx加入
        R"(.*加入了群聊)",            // xxx加入了群聊
        R"(.*退出了群聊)",            // xxx退出了群聊
        R"(.*邀请.*加入了群聊)",      // xxx邀请yyy加入了群聊
        R"(.*撤回了.*条消息)",        // xxx撤回了x条消息
        R"(群聊名称.*更改为)",        // 群聊名称更改为xxx
        R"(.*开启了.*)",              // xxx开启了群聊邀请确认
        R"(.*关闭了.*)",              // xxx关闭了群聊邀请确认
        R"(.*设置.*为管理员)",        // xxx设置yyy为管理员
        R"(.*移除了.*的管理员身份)",  // xxx移除了yyy的管理员身份
        R"(.*修改了群名称)",          // xxx修改了群名称
        R"(系统消息)",                // 直接的系统消息标识
        R"(以上是历史消息)"           // 历史消息分隔符
    };
}

std::vector<std::string> Config::get_time_exclude_keywords() const {
    return {"报送", "回执", "会议", "参加", "人员", "工作", "通知", "安排", "要求", "地点", "内容",
            "完成", "需要", "前",   "后",   "开始", "结束", "传包", "表格", "填写", "更新", "自测"};
}

std::vector<std::string> Config::get_feishu_keywords() const { return {"云文档", "文件", "消息", "Pin"}; }

bool Config::isDebugMode() const {
#ifdef DEBUG_MODE
    return true;
#else
    return false;
#endif
}

bool Config::shouldSaveOutputFiles() const {
    // 只在Debug模式下保存输出文件
    return isDebugMode();
}

Json::Value Config::to_json() const {
    Json::Value json;
    json["ocr"] = ocr.to_json();
    json["image"] = image.to_json();
    json["avatar"] = avatar.to_json();
    json["deduplication"] = deduplication.to_json();
    json["content_marking"] = content_marking.to_json();
    json["output"] = output.to_json();
    return json;
}

bool Config::from_json(const Json::Value& j) {
    try {
        if (j.isMember("ocr") && j["ocr"].isObject()) {
            ocr.from_json(j["ocr"]);
        }
        if (j.isMember("image") && j["image"].isObject()) {
            image.from_json(j["image"]);
        }
        if (j.isMember("avatar") && j["avatar"].isObject()) {
            avatar.from_json(j["avatar"]);
        }
        if (j.isMember("deduplication") && j["deduplication"].isObject()) {
            deduplication.from_json(j["deduplication"]);
        }
        if (j.isMember("content_marking") && j["content_marking"].isObject()) {
            content_marking.from_json(j["content_marking"]);
        }
        if (j.isMember("output") && j["output"].isObject()) {
            output.from_json(j["output"]);
        }
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to load config from JSON: {}", e.what());
        return false;
    }
}

bool Config::create_directory(const std::string& path) const {
    try {
        if (std::filesystem::exists(path)) {
            return true;  // 目录已存在
        }

        std::filesystem::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to create directory: {} - {}", path, e.what());
        return false;
    }
}

}  // namespace utils