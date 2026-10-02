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

#include "jsonexporter.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include "../../utils/logger.h"
#include "compatibilitypatch.h"

namespace export_ns {

JsonExporter::JsonExporter(const utils::Config& config) : config_(config), output_dir_(config.output.output_json_dir) {
    // 仅在Debug模式下创建输出目录
    if (config_.shouldSaveOutputFiles()) {
        createOutputDirectory();
    }
}

bool JsonExporter::createOutputDirectory() const {
    try {
        if (!std::filesystem::exists(output_dir_)) {
            return std::filesystem::create_directories(output_dir_);
        }
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to create output directory: {}", e.what());
        return false;
    }
}

std::string JsonExporter::getCurrentTimeISO() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

std::string JsonExporter::writeJsonFile(const std::filesystem::path& output_path, const Json::Value& data,
                                        const std::string& backup_filename) const {
    // 检查是否应该保存文件（仅在Debug模式下）
    if (!config_.shouldSaveOutputFiles()) {
        // 只在Debug配置时输出日志，避免Release模式下的多余输出
        LOGD("[INFO] JSON data ready but not saved to disk");
        return output_path.string();  // 返回路径但不实际保存
    }

    // 尝试写入主要路径
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing");
        }

        // 保持JSON字段顺序的输出，使用JsonCompat::dump输出UTF-8中文
        file << json_dump(data, 2) << std::endl;
        file.close();

        LOGD("[DEBUG] JSON file exported to: {}", output_path.string());
        return output_path.string();

    } catch (const std::exception& e) {
        LOGE("JSON file write failed: {}, error: {}", output_path.string(), e.what());

        // 尝试备用路径
        try {
            auto temp_dir = std::filesystem::temp_directory_path();
            auto backup_path = temp_dir / ("backup_" + backup_filename);

            std::ofstream backup_file(backup_path);
            if (!backup_file.is_open()) {
                throw std::runtime_error("Cannot open backup file for writing");
            }

            backup_file << json_dump(data, 2) << std::endl;
            backup_file.close();

            LOGE("Saved using backup path: {}", backup_path.string());
            return backup_path.string();

        } catch (const std::exception& backup_error) {
            LOGE("Backup path write also failed: {}", backup_error.what());
            return "";
        }
    }
}

void JsonExporter::logStatistics(const std::unordered_map<std::string, int>& stats) const {
    for (const auto& [key, value] : stats) {
        if (key == "chat") {
            LOGD("  - Chat messages: {} items", value);
        } else if (key == "time") {
            LOGD("  - Time messages: {} items", value);
        } else if (key == "my_chat") {
            LOGD("  - My messages: {} items", value);
        } else if (key == "group_name") {
            LOGD("  - Group name: {} items", value);
        } else if (key == "retract_message") {
            LOGD("  - Retracted messages: {} items", value);
        } else if (key == "unknown") {
            LOGD("  - Unknown content: {} items", value);
        }
    }
}

std::string JsonExporter::exportChatMessages(const chat::ChatSession& session, const std::string& output_file) {
    std::string filename = output_file.empty() ? "structured_chat_messages.json" : output_file;
    auto output_path = output_dir_ / filename;

    // 获取统计信息
    auto stats = session.getStatistics();

    // 准备导出数据
    Json::Value chat_messages = Json::Value(Json::arrayValue);
    for (const auto& msg_ptr : session.messages) {
        if (msg_ptr) {
            auto msg_dict = msg_ptr->to_dict();
            chat_messages.append(msg_dict);
        }
    }

    // 统计各类消息数量
    int nickname_messages = 0, time_messages = 0, my_messages = 0;
    int group_name_messages = 0, retract_messages = 0, unknown_messages = 0;

    for (const auto& msg : chat_messages) {
        std::string type = msg.isMember("type") ? msg["type"].asString() : "";
        if (type == "chat")
            nickname_messages++;
        else if (type == "time")
            time_messages++;
        else if (type == "my_chat")
            my_messages++;
        else if (type == "group_name")
            group_name_messages++;
        else if (type == "retract_message")
            retract_messages++;
        else if (type == "unknown")
            unknown_messages++;
    }

    Json::Value export_data;
    Json::Value metadata;
    metadata["export_time"] = getCurrentTimeISO();
    metadata["total_messages"] = static_cast<Json::UInt>(chat_messages.size());
    metadata["description"] = "Structured chat messages - organized by nickname, content, time";
    export_data["metadata"] = metadata;

    export_data["chat_messages"] = chat_messages;

    Json::Value statistics;
    statistics["nickname_messages"] = nickname_messages;
    statistics["time_messages"] = time_messages;
    statistics["my_messages"] = my_messages;
    statistics["group_name_messages"] = group_name_messages;
    statistics["retract_messages"] = retract_messages;
    statistics["unknown_messages"] = unknown_messages;
    export_data["statistics"] = statistics;

    // 写入文件
    std::string result_path = writeJsonFile(output_path, export_data, filename);
    if (!result_path.empty()) {
        LOGD("Structured chat messages exported to: {}", result_path);
        logStatistics(stats);
    }

    return result_path;
}

std::string JsonExporter::exportMarkedOCRResults(const std::vector<utils::OCRItem>& marked_items,
                                                 const std::string& output_file) {
    std::string filename = output_file.empty() ? "marked_ocr_results_original.json" : output_file;
    auto output_path = output_dir_ / filename;

    if (marked_items.empty()) {
        LOGE("No marked OCR results to export");
        return "";
    }

    // 提取所有text字段
    std::vector<std::string> text_results;
    text_results.reserve(marked_items.size());
    for (const auto& item : marked_items) {
        text_results.push_back(item.text);
    }
    // 统计各类标记
    int time_count = 0, nickname_count = 0, content_count = 0, my_content_count = 0;
    for (const auto& text : text_results) {
        if (text.find("(时间)") != std::string::npos)
            time_count++;
        else if (text.find("(昵称)") != std::string::npos)
            nickname_count++;
        else if (text.find("(内容)") != std::string::npos)
            content_count++;
        else if (text.find("(My content)") != std::string::npos)
            my_content_count++;
    }

    int unmarked_count = text_results.size() - time_count - nickname_count - content_count - my_content_count;

    Json::Value export_data;
    Json::Value metadata;
    metadata["export_time"] = getCurrentTimeISO();
    metadata["total_items"] = static_cast<Json::UInt>(text_results.size());
    metadata["description"] = "Marked OCR text results - text content only";
    export_data["metadata"] = metadata;

    Json::Value marked_texts_array(Json::arrayValue);
    for (const auto& text : text_results) {
        marked_texts_array.append(text);
    }
    export_data["marked_texts"] = marked_texts_array;

    Json::Value statistics;
    statistics["time_items"] = time_count;
    statistics["nickname_items"] = nickname_count;
    statistics["content_items"] = content_count;
    statistics["my_content_items"] = my_content_count;
    statistics["unmarked_items"] = unmarked_count;
    export_data["statistics"] = statistics;

    // 写入文件
    std::string result_path = writeJsonFile(output_path, export_data, filename);
    if (!result_path.empty()) {
        LOGD("Marked OCR results exported to: {}", result_path);
        LOGD("  - Time marks: {} items", time_count);
        LOGD("  - Nickname marks: {} items", nickname_count);
        LOGD("  - Content marks: {} items", content_count);
        LOGD("  - My content: {} items", my_content_count);
        LOGD("  - Unmarked: {} items", unmarked_count);
    }

    return result_path;
}

std::string JsonExporter::exportSummaryData(const std::vector<utils::OCRItem>& ocr_items,
                                            const std::vector<utils::AvatarItem>& avatar_items,
                                            const std::vector<utils::OCRItem>& marked_items,
                                            const std::string& output_file) {
    std::string filename = output_file.empty() ? "summary_data_original.json" : output_file;
    auto output_path = output_dir_ / filename;

    // 准备导出数据
    Json::Value ocr_results_original = Json::Value(Json::arrayValue);
    for (const auto& item : ocr_items) {
        ocr_results_original.append(item.to_dict());
    }

    Json::Value avatar_positions_original = Json::Value(Json::arrayValue);
    for (const auto& item : avatar_items) {
        avatar_positions_original.append(item.to_dict());
    }

    Json::Value marked_ocr_results_original = Json::Value(Json::arrayValue);
    for (const auto& item : marked_items) {
        marked_ocr_results_original.append(item.to_dict());
    }

    // 统计处理过的切片数量
    std::set<int> processed_slices;
    for (const auto& item : ocr_items) {
        processed_slices.insert(item.slice_index);
    }

    Json::Value export_data;
    export_data["ocr_results_original"] = ocr_results_original;
    export_data["avatar_positions_original"] = avatar_positions_original;
    export_data["marked_ocr_results_original"] = marked_ocr_results_original;

    Json::Value statistics;
    statistics["total_ocr_items"] = static_cast<Json::UInt>(ocr_items.size());
    statistics["total_avatars"] = static_cast<Json::UInt>(avatar_items.size());
    statistics["total_marked_ocr_items"] = static_cast<Json::UInt>(marked_items.size());
    statistics["processed_slices"] = static_cast<Json::UInt>(processed_slices.size());
    export_data["statistics"] = statistics;

    // 写入文件
    std::string result_path = writeJsonFile(output_path, export_data, filename);
    if (!result_path.empty()) {
        LOGD("Summary data exported to: {}", result_path);
    }

    return result_path;
}

}  // namespace export_ns
