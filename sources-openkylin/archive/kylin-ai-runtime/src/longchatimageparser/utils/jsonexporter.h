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

#ifndef JSON_EXPORTER_H
#define JSON_EXPORTER_H

#include <jsoncpp/json/json.h>

#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "chatmessage.h"
#include "config.h"
#include "ocravatartypes.h"

namespace export_ns {

/**
 * 格式化JSON为UTF-8字符串（支持中文直接输出，不转义为Unicode）
 * @param root JSON对象
 * @param indent 缩进级别（默认2）
 * @return 格式化后的JSON字符串
 */
inline std::string json_dump(const Json::Value& root, int indent = 2) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = std::string(indent, ' ');
    builder["commentStyle"] = "None";
    builder["dropNullPlaceholders"] = false;
    builder["useSpecialFloats"] = false;
    builder["precision"] = 17;
    builder["emitUTF8"] = true;

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream ss;
    writer->write(root, &ss);
    return ss.str();
}

/**
 * JSON导出器
 * 负责将处理结果导出为JSON格式
 */
class JsonExporter {
public:
    /**
     * 构造函数
     *
     * @param config 配置对象
     */
    explicit JsonExporter(const utils::Config& config);

    /**
     * 导出结构化的聊天消息
     *
     * @param session 聊天会话对象
     * @param output_file
     * 输出文件名（可选，默认为"structured_chat_messages.json"）
     * @return 输出文件路径，失败时返回空字符串
     */
    std::string exportChatMessages(const chat::ChatSession& session, const std::string& output_file = "");

    /**
     * 导出标记后的OCR结果
     *
     * @param marked_items 标记后的OCR项列表
     * @param output_file
     * 输出文件名（可选，默认为"marked_ocr_results_original.json"）
     * @return 输出文件路径，失败时返回空字符串
     */
    std::string exportMarkedOCRResults(const std::vector<utils::OCRItem>& marked_items,
                                       const std::string& output_file = "");

    /**
     * 导出汇总数据
     *
     * @param ocr_items 去重后的OCR项列表
     * @param avatar_items 去重后的头像项列表
     * @param marked_items 标记后的OCR项列表
     * @param output_file 输出文件名（可选，默认为"summary_data_original.json"）
     * @return 输出文件路径，失败时返回空字符串
     */
    std::string exportSummaryData(const std::vector<utils::OCRItem>& ocr_items,
                                  const std::vector<utils::AvatarItem>& avatar_items,
                                  const std::vector<utils::OCRItem>& marked_items, const std::string& output_file = "");

private:
    const utils::Config& config_;
    std::filesystem::path output_dir_;

    /**
     * 记录统计信息
     *
     * @param stats 统计信息映射
     */
    void logStatistics(const std::unordered_map<std::string, int>& stats) const;

    /**
     * 创建输出目录
     *
     * @return 是否成功创建
     */
    bool createOutputDirectory() const;

    /**
     * 获取当前时间的ISO字符串
     *
     * @return ISO格式的时间字符串
     */
    std::string getCurrentTimeISO() const;

    /**
     * 写入JSON文件
     *
     * @param output_path 主要输出路径
     * @param data 要写入的JSON数据
     * @param backup_filename 备用文件名
     * @return 实际写入的文件路径，失败时返回空字符串
     */
    std::string writeJsonFile(const std::filesystem::path& output_path, const Json::Value& data,
                              const std::string& backup_filename) const;
};

}  // namespace export_ns

#endif  // JSON_EXPORTER_H
