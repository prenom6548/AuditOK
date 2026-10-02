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

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "longimageparser.h"

// 包含日志头文件
#include "../../utils/logger.h"

// 包含完整的头文件而不是只使用前向声明
#include "../utils/jsonexporter.h"
#ifdef DEBUG_MODE
#include "../utils/chinesetextrenderer.h"
#include "../utils/visualization.h"
#endif
#include "avatardetector.h"
#include "chatanalyzer.h"
#include "contentmarker.h"
#include "deduplicator.h"
#include "imageslicer.h"
#include "ocrengine.h"

namespace parser {

// 实现类定义
class LongImageParserImpl {
public:
    // 配置和组件 - 注意不同命名空间
    utils::Config& config_;
    std::unique_ptr<parser::ImageSlicer> image_slicer_;
    std::unique_ptr<parser::OcrEngine> ocr_engine_;
    std::unique_ptr<chat::ChatAnalyzer> chat_analyzer_;
    std::unique_ptr<detection::AvatarDetector> avatar_detector_;
    std::unique_ptr<detection::ContentMarker> content_marker_;
    std::unique_ptr<detection::Deduplicator> deduplicator_;
    std::unique_ptr<export_ns::JsonExporter> json_exporter_;
#ifdef DEBUG_MODE
    std::unique_ptr<export_ns::Visualizer> visualizer_;
    std::unique_ptr<export_ns::ChineseTextRenderer> chinese_renderer_;
#endif

    // 处理结果存储
    cv::Mat original_image_;
    std::vector<utils::OCRItem> all_ocr_items_;
    std::vector<utils::AvatarItem> all_avatar_items_;
    std::vector<utils::OCRItem> marked_ocr_items_;
    std::unique_ptr<chat::ChatSession> chat_session_;

    explicit LongImageParserImpl(utils::Config& config) : config_(config) {}

    // 所有私有方法的声明

    std::string getCurrentTimeISO() const;
    void printFinalPlatformDetection(const std::string& platform, bool is_feishu);
    void processSlices(const std::vector<utils::SliceInfo>& slice_infos, std::optional<int> x_crop);
    void deduplicateResults();
    bool isFeishuScreenshot();
    void exportResults();
};

LongImageParser::LongImageParser(
    std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine)
    : pimpl_(std::make_unique<LongImageParserImpl>(utils::Config::getInstance())) {
    // 使用默认硬编码配置，不需要加载外部配置文件
    // 初始化各个组件
    try {
        pimpl_->image_slicer_ = std::make_unique<parser::ImageSlicer>(pimpl_->config_);
        LOGD("[INFO] ImageSlicer initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] ImageSlicer initialization failed: {}", e.what());
        pimpl_->image_slicer_ = nullptr;
    }
    try {
        pimpl_->ocr_engine_ = std::make_unique<parser::OcrEngine>(pimpl_->config_, std::move(visionEngine));
        LOGD("[INFO] OcrEngine initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] OcrEngine initialization failed: {}", e.what());
        pimpl_->ocr_engine_ = nullptr;
    }
    try {
        pimpl_->avatar_detector_ = std::make_unique<detection::AvatarDetector>(pimpl_->config_);
        LOGD("[INFO] AvatarDetector initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] AvatarDetector initialization failed: {}", e.what());
        pimpl_->avatar_detector_ = nullptr;
    }
    try {
        pimpl_->content_marker_ = std::make_unique<detection::ContentMarker>(pimpl_->config_);
        LOGD("[INFO] ContentMarker initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] ContentMarker initialization failed: {}", e.what());
        pimpl_->content_marker_ = nullptr;
    }
    try {
        pimpl_->deduplicator_ = std::make_unique<detection::Deduplicator>(pimpl_->config_);
        LOGD("[INFO] Deduplicator initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] Deduplicator initialization failed: {}", e.what());
        pimpl_->deduplicator_ = nullptr;
    }

    try {
        pimpl_->chat_analyzer_ = std::make_unique<chat::ChatAnalyzer>(pimpl_->config_);
        LOGD("[INFO] ChatAnalyzer initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] ChatAnalyzer initialization failed: {}", e.what());
        pimpl_->chat_analyzer_ = nullptr;
    }

    try {
        pimpl_->json_exporter_ = std::make_unique<export_ns::JsonExporter>(pimpl_->config_);
        LOGD("[INFO] JsonExporter initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] JsonExporter initialization failed: {}", e.what());
        pimpl_->json_exporter_ = nullptr;
    }

#ifdef DEBUG_MODE
    try {
        pimpl_->visualizer_ = std::make_unique<export_ns::Visualizer>(pimpl_->config_);
        LOGD("[INFO] Visualizer initialized successfully");
    } catch (const std::exception& e) {
        LOGD("[ERROR] Visualizer initialization failed: {}", e.what());
        pimpl_->visualizer_ = nullptr;
    }
#endif

#ifdef DEBUG_MODE
    try {
#ifdef USE_FREETYPE
        // 初始化中文字体渲染器（仅在Debug模式下）
        std::string font_path = "../shangshoushisongti.ttf";  // 使用项目中的宋体字体文件
        pimpl_->chinese_renderer_ = std::make_unique<export_ns::ChineseTextRenderer>(font_path, 16);

        if (!pimpl_->chinese_renderer_->isInitialized()) {
            LOGD("[INFO] Chinese font renderer init failed, dedup results will show index only");
        } else {
            LOGD("[INFO] Chinese font renderer initialized successfully");
        }
#else
        pimpl_->chinese_renderer_ = nullptr;
#endif
    } catch (const std::exception& e) {
        LOGD("[ERROR] ChineseTextRenderer initialization failed: {}", e.what());
        pimpl_->chinese_renderer_ = nullptr;
    }
#endif

    // 初始化存储容器
    pimpl_->all_ocr_items_.clear();
    pimpl_->all_avatar_items_.clear();
    pimpl_->marked_ocr_items_.clear();
    pimpl_->chat_session_ = nullptr;

    LOGD("[INFO] Long image OCR processor initialized");
}

LongImageParser::~LongImageParser() = default;

void LongImageParserImpl::printFinalPlatformDetection(const std::string& platform, bool is_feishu) {
    LOGD("\n{}", std::string(60, '*'));
    LOGD("Platform Detection - Final Result");
    LOGD("{}", std::string(60, '*'));

    if (is_feishu) {
        LOGD("Detection result: Feishu");
        LOGD(" Detection status: Successfully identified");
        LOGD("Processing mode: Feishu-specific mode");
        LOGD("Feature recognition: Feishu-specific UI elements detected");
    } else {
        LOGD("Detection result: WeChat/Lanxin/DingTalk");
        LOGD("Detection status: Default identification");
        LOGD("Processing mode: General chat mode");
        LOGD("Feature recognition: No Feishu features, using general processing");
    }

    LOGD("{}", std::string(60, '*'));
    LOGD("[INFO]Final platform type confirmed: {}", platform);
    LOGD("");
}

void LongImageParserImpl::processSlices(const std::vector<utils::SliceInfo>& slice_infos, std::optional<int> x_crop) {
    std::vector<utils::OCRItem> all_ocr_items;
    std::vector<utils::AvatarItem> all_avatar_items;

    for (const auto& slice_info : slice_infos) {
        LOGD("[INFO] Processing slice {}...", slice_info.slice_index);

        // OCR识别
        if (!ocr_engine_) {
            throw std::runtime_error("OcrEngine not available");
        }
        auto slice_ocr_result = ocr_engine_->processocrSlice(slice_info, true, x_crop);

        // 头像检测
        if (!avatar_detector_) {
            throw std::runtime_error("AvatarDetector not available");
        }
        auto avatar_items = avatar_detector_->detectAvatars(slice_info, x_crop);

        // 添加到结果中
        all_ocr_items.insert(all_ocr_items.end(), slice_ocr_result.ocr_items.begin(), slice_ocr_result.ocr_items.end());
        all_avatar_items.insert(all_avatar_items.end(), avatar_items.begin(), avatar_items.end());
    }

    // 保存到实例变量
    all_ocr_items_ = std::move(all_ocr_items);
    all_avatar_items_ = std::move(all_avatar_items);
}

void LongImageParserImpl::deduplicateResults() {
    if (!deduplicator_) {
        throw std::runtime_error("Deduplicator not available");
    }

    size_t before_count = all_ocr_items_.size();
    auto result = deduplicator_->deduplicate(all_ocr_items_, all_avatar_items_);
    all_ocr_items_ = std::move(result.first);
    all_avatar_items_ = std::move(result.second);
    size_t after_count = all_ocr_items_.size();

    LOGD("[INFO] Deduplication completed: {} -> {} (removed {} duplicates)", before_count, after_count,
         (before_count - after_count));
#ifdef DEBUG_MODE
    // 先检查是否需要保存输出文件，Release 模式下直接跳过整个可视化流程
    if (!config_.shouldSaveOutputFiles()) {
        LOGD("[INFO] Deduplication done ({} text boxes) - Release mode skipping visualization", after_count);
        return;
    }

    // Debug 模式下才执行可视化
    try {
        if (original_image_.empty()) {
            LOGD("[WARNING] Original image is empty, skipping dedup visualization");
            return;
        }

        // 复制原图进行可视化
        cv::Mat vis_image = original_image_.clone();

        // 添加标题信息（更大更清晰）
        cv::rectangle(vis_image, cv::Point(10, 10), cv::Point(500, 60), cv::Scalar(0, 0, 0),
                      -1);  // 黑色背景
        cv::putText(vis_image, "Deduplicated OCR Results", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                    cv::Scalar(255, 255, 255), 3);

        // 绘制所有去重后的OCR结果
        for (size_t i = 0; i < all_ocr_items_.size(); ++i) {
            const auto& item = all_ocr_items_[i];

            // 获取边框坐标
            if (item.box.size() >= 4) {
                std::vector<cv::Point> pts;
                for (const auto& point : item.box) {
                    if (point.size() >= 2) {
                        pts.push_back(cv::Point(static_cast<int>(point[0]), static_cast<int>(point[1])));
                    }
                }

                // 绘制边框（绿色，更粗）
                if (pts.size() == 4) {
                    std::vector<std::vector<cv::Point>> contours = {pts};
                    cv::polylines(vis_image, contours, true, cv::Scalar(0, 255, 0), 4);

                    // 添加序号标签和完整文本内容
                    std::string label = std::to_string(i + 1);
                    if (!item.text.empty()) {
                        // 显示完整文字内容，不省略
                        label += ":" + item.text;
                    }

                    // 文本位置
                    cv::Point text_pos(pts[0].x, pts[0].y - 8);
                    text_pos.x = std::max(5, text_pos.x);
                    text_pos.y = std::max(20, text_pos.y);

                    // 绘制白色背景
                    cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, nullptr);
                    cv::rectangle(vis_image, cv::Point(text_pos.x - 2, text_pos.y - text_size.height - 2),
                                  cv::Point(text_pos.x + text_size.width + 2, text_pos.y + 2),
                                  cv::Scalar(255, 255, 255), -1);

                    // 使用中文字体渲染器绘制文本或备用方案
                    if (chinese_renderer_ && chinese_renderer_->isInitialized()) {
                        // 使用中文字体渲染器
                        chinese_renderer_->putText(vis_image, label, text_pos, cv::Scalar(0, 0, 255), 1);
                    } else {
                        // 备用方案：只显示序号（避免中文显示问题）
                        std::string fallback_label = "#" + std::to_string(i + 1);
                        cv::putText(vis_image, fallback_label, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                                    cv::Scalar(0, 0, 255), 1);
                    }
                }
            }
        }

        // 在图像右下角添加统计信息
        std::string stats = "Total: " + std::to_string(after_count) + " texts";
        cv::Size stats_size = cv::getTextSize(stats, cv::FONT_HERSHEY_SIMPLEX, 0.6, 2, nullptr);
        cv::Point stats_pos(vis_image.cols - stats_size.width - 10, vis_image.rows - 20);

        cv::rectangle(vis_image, cv::Point(stats_pos.x - 5, stats_pos.y - stats_size.height - 5),
                      cv::Point(stats_pos.x + stats_size.width + 5, stats_pos.y + 5), cv::Scalar(0, 0, 0), -1);
        cv::putText(vis_image, stats, stats_pos, cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);

        // 保存可视化图像
        std::string output_path = config_.output.output_images_dir + "/dedup_ocr_result.png";
        bool save_success = cv::imwrite(output_path, vis_image);

        if (save_success) {
            LOGD("[DEBUG] Dedup OCR visualization saved: {} ({} text boxes)", output_path, after_count);
        } else {
            LOGD("[WARNING] Failed to save dedup visualization: {}", output_path);
        }

    } catch (const std::exception& viz_error) {
        LOGD("[WARNING] Error creating dedup visualization: {}", viz_error.what());
    }
#endif
}

bool LongImageParserImpl::isFeishuScreenshot() {
    auto keywords = config_.get_feishu_keywords();
    std::set<std::string> detected_keywords;

    for (const auto& item : all_ocr_items_) {
        const std::string& text = item.text;
        for (const auto& keyword : keywords) {
            if (text.find(keyword) != std::string::npos) {
                detected_keywords.insert(keyword);
            }
        }
    }

    bool is_feishu = detected_keywords.size() == keywords.size();

    // 详细记录检测结果（仅Debug模式）
    if (is_feishu) {
        std::string keywords_str;
        for (const auto& keyword : detected_keywords) {
            keywords_str += keyword + " ";
        }
        LOGD("[INFO] Feishu detection success - All keywords found: {}", keywords_str);
    } else {
        std::set<std::string> missing_keywords;
        for (const auto& keyword : keywords) {
            if (detected_keywords.find(keyword) == detected_keywords.end()) {
                missing_keywords.insert(keyword);
            }
        }
        std::string detected_str, missing_str;
        for (const auto& keyword : detected_keywords) {
            detected_str += keyword + " ";
        }
        for (const auto& keyword : missing_keywords) {
            missing_str += keyword + " ";
        }
        LOGD("[INFO] Feishu detection failed - Found: {}, Missing: {}", detected_str, missing_str);
    }

    return is_feishu;
}

void LongImageParserImpl::exportResults() {
    if (!json_exporter_) {
        LOGD("[WARNING] JsonExporter not available, skipping export");
    }

    try {
        // 导出标记后的OCR结果
        if (!marked_ocr_items_.empty()) {
            json_exporter_->exportMarkedOCRResults(marked_ocr_items_);
        }

        // 导出结构化聊天消息
        if (chat_session_) {
            json_exporter_->exportChatMessages(*chat_session_);
        }

        // 导出汇总数据
        json_exporter_->exportSummaryData(all_ocr_items_, all_avatar_items_, marked_ocr_items_);

        LOGD("[INFO] All data export completed");

    } catch (const std::exception& e) {
        LOGD("[ERROR] Error exporting data: {}", e.what());
        throw;
    }
}

std::string LongImageParserImpl::getCurrentTimeISO() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

/**
 * @brief 处理长图并返回错误码
 */
utils::ProcessResult LongImageParser::extractImageContents(const std::string& image_path) {
    auto start_time = std::chrono::steady_clock::now();

    try {
        // 记录开始时间
        LOGD("[INFO] Start processing long image: {}", image_path);

        // 检查参数
        if (image_path.empty()) {
            return utils::ProcessResult::failure(utils::ErrorCode::INVALID_PARAMETER, "Image path is empty");
        }

        // 检查文件是否存在
        if (!std::filesystem::exists(image_path)) {
            return utils::ProcessResult::failure(utils::ErrorCode::FILE_NOT_FOUND,
                                                 "Image file not found: " + image_path);
        }

        // 检查实现是否可用
        if (!pimpl_) {
            return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                 "LongImageParser internal implementation not initialized");
        }

        // ========== 步骤1: 图像切片 ==========
        LOGI("Step 1/9: Slicing long image...");
        std::vector<utils::SliceInfo> slice_infos;
        try {
            if (!pimpl_->image_slicer_) {
                return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                     "ImageSlicer component not initialized");
            }

            auto slice_result = pimpl_->image_slicer_->sliceImage(image_path);
            pimpl_->original_image_ = std::get<0>(slice_result);
            slice_infos = std::get<1>(slice_result);

            if (slice_infos.empty()) {
                return utils::ProcessResult::failure(utils::ErrorCode::IMAGE_SLICE_FAILED,
                                                     "Image slice result is empty");
            }

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(utils::ErrorCode::IMAGE_SLICE_FAILED,
                                                              std::string("Image slicing failed: ") + e.what(),
                                                              typeid(e).name());
        }

        // ========== 步骤2: 计算x_crop值 ==========
        LOGI("Step 2/9: Calculating x_crop value...");
        std::optional<int> x_crop;
        try {
            if (!pimpl_->avatar_detector_) {
                return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                     "AvatarDetector component not initialized");
            }

            x_crop = pimpl_->avatar_detector_->calculateXCrop(slice_infos);
            if (!x_crop.has_value()) {
                return utils::ProcessResult::failure(utils::ErrorCode::XCROP_CALC_FAILED,
                                                     std::string("X crop calculation failed, x_crop is empty"));
            }

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(utils::ErrorCode::XCROP_CALC_FAILED,
                                                              std::string("X crop calculation failed: ") + e.what(),
                                                              typeid(e).name());
        }

        // ========== 步骤3: 头像检测和OCR识别 ==========
        LOGI("Step 3/9: OCR recognition and avatar detection...");
        try {
            if (!pimpl_->ocr_engine_) {
                return utils::ProcessResult::failure(utils::ErrorCode::OCR_ENGINE_INIT_FAILED,
                                                     "OcrEngine component not initialized");
            }

            pimpl_->processSlices(slice_infos, x_crop.value_or(0));

            // 检查头像检测结果
            if (pimpl_->all_avatar_items_.empty()) {
                return utils::ProcessResult::failure(
                    utils::ErrorCode::AVATAR_DETECTION_EMPTY,
                    "Avatar detection result is empty, cannot recognize chat screenshot");
            }

            if (pimpl_->all_ocr_items_.empty()) {
                return utils::ProcessResult::failure(utils::ErrorCode::OCR_RECOGNITION_FAILED,
                                                     "OCR recognition result is empty");
            }

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(
                utils::ErrorCode::OCR_PROCESS_FAILED,
                std::string("OCR or avatar detection processing failed: ") + e.what(), typeid(e).name());
        }

        // ========== 步骤4: 去重处理 ==========
        LOGI("Step 4/9: Deduplication...");
        try {
            if (!pimpl_->deduplicator_) {
                return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                     "Deduplicator component not initialized");
            }

            pimpl_->deduplicateResults();

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(utils::ErrorCode::DEDUP_PROCESS_FAILED,
                                                              std::string("Deduplication failed: ") + e.what(),
                                                              typeid(e).name());
        }

        // ========== 步骤5: 平台类型判断 ==========
        LOGI("Step 5/9: Detecting platform type...");
        try {
            bool is_feishu = pimpl_->isFeishuScreenshot();
            std::string platform = is_feishu ? "Feishu" : "WeChat/Lanxin";
            pimpl_->printFinalPlatformDetection(platform, is_feishu);
        } catch (const std::exception& e) {
            // 平台检测失败不是致命错误，记录但继续
            LOGE("[WARNING] Platform detection failed: {}", e.what());
        }

        // ========== 步骤6: 内容标记 ==========
        LOGI("Step 6/9: Marking content...");
        try {
            if (!pimpl_->content_marker_) {
                return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                     "ContentMarker component not initialized");
            }

            pimpl_->marked_ocr_items_ = pimpl_->content_marker_->markContent(
                pimpl_->all_ocr_items_, pimpl_->all_avatar_items_, pimpl_->original_image_);

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(utils::ErrorCode::CONTENT_MARK_FAILED,
                                                              std::string("Content marking failed: ") + e.what(),
                                                              typeid(e).name());
        }

        // ========== 步骤7: 聊天分析 ==========
        LOGI("Step 7/9: Analyzing chat messages...");
        try {
            if (!pimpl_->chat_analyzer_) {
                return utils::ProcessResult::failure(utils::ErrorCode::INITIALIZATION_FAILED,
                                                     "ChatAnalyzer component not initialized");
            }

            pimpl_->chat_session_ = pimpl_->chat_analyzer_->analyze(pimpl_->marked_ocr_items_);

        } catch (const std::exception& e) {
            return utils::ProcessResult::failureWithException(utils::ErrorCode::CHAT_ANALYZE_FAILED,
                                                              std::string("Chat analysis failed: ") + e.what(),
                                                              typeid(e).name());
        }

        // ========== 步骤8: 导出结果 ==========
        LOGI("Step 8/9: Exporting results...");
#ifdef DEBUG_MODE
        try {
            pimpl_->exportResults();
        } catch (const std::exception& e) {
            // 导出失败不是致命错误，记录但继续
            LOGE("[WARNING] Export failed: {}", e.what());
        }
#endif

        // ========== 步骤9: 创建处理总结 ==========
        LOGI("Step 9/9: Creating process summary...");
        std::string summary;
        try {
#ifdef DEBUG_MODE
            // 创建可视化总结（如果visualizer可用）
            if (pimpl_->visualizer_) {
                pimpl_->visualizer_->createProcessSummaryImage(pimpl_->original_image_, slice_infos,
                                                               static_cast<int>(pimpl_->all_ocr_items_.size()),
                                                               static_cast<int>(pimpl_->all_avatar_items_.size()));
            }
#endif

            // 构造JSON格式的结果
            if (pimpl_->chat_session_) {
                // 准备导出数据
                Json::Value chat_messages = Json::Value(Json::arrayValue);
                for (const auto& msg_ptr : pimpl_->chat_session_->messages) {
                    if (msg_ptr) {
                        auto msg_dict = msg_ptr->to_dict();
                        chat_messages.append(msg_dict);
                    }
                }

                Json::Value export_data;
                Json::Value metadata;
                metadata["export_time"] = pimpl_->getCurrentTimeISO();
                metadata["total_messages"] = static_cast<Json::UInt>(chat_messages.size());
                metadata["description"] = "Structured chat messages - organized by nickname, content, time";
                export_data["metadata"] = metadata;
                export_data["chat_messages"] = chat_messages;

                summary = export_ns::json_dump(export_data, 2);  // 2个空格缩进的JSON字符串
            } else {
                // 没有聊天消息时返回空的JSON结构
                Json::Value empty_data;
                Json::Value metadata;
                metadata["export_time"] = pimpl_->getCurrentTimeISO();
                metadata["total_messages"] = 0;
                metadata["description"] = "No chat messages";
                empty_data["metadata"] = metadata;
                empty_data["chat_messages"] = Json::Value(Json::arrayValue);

                summary = export_ns::json_dump(empty_data, 2);
            }
        } catch (const std::exception& e) {
            // 总结创建失败不影响整体成功
            summary = "Processing completed but summary creation failed: " + std::string(e.what());
        }

        // 计算处理耗时
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // 创建成功结果
        auto result = utils::ProcessResult::success(summary);
        result.setProcessingTime(duration);

        LOGD("[SUCCESS] Processing completed, time elapsed: {} ms", duration.count());

        return result;

    } catch (const std::exception& e) {
        // 捕获所有未预期的异常
        return utils::ProcessResult::failureWithException(
            utils::ErrorCode::UNKNOWN_ERROR, std::string("Unexpected error: ") + e.what(), typeid(e).name());
    } catch (...) {
        // 捕获非标准异常
        return utils::ProcessResult::failure(utils::ErrorCode::UNKNOWN_ERROR, "Unknown exception occurred");
    }
}

}  // namespace parser