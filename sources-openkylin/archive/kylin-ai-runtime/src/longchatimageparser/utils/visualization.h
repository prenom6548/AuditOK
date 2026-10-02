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

#ifndef OCR_MODELS_UTILS_VISUALIZATION_H
#define OCR_MODELS_UTILS_VISUALIZATION_H

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "config.h"
#include "sliceinfo.h"

namespace export_ns {

#ifdef DEBUG_MODE

/**
 * 可视化工具类
 * 用于生成调试和展示用的可视化图像
 */
class Visualizer {
public:
    /**
     * 构造函数
     *
     * @param config 配置对象
     */
    explicit Visualizer(const utils::Config& config);

    /**
     * 创建处理过程总结图像
     *
     * @param original_image 原始图像
     * @param slice_infos 切片信息列表
     * @param ocr_count OCR识别结果数量
     * @param avatar_count 头像检测数量
     * @param output_file 输出文件名
     */
    void createProcessSummaryImage(const cv::Mat& original_image, const std::vector<utils::SliceInfo>& slice_infos,
                                   int ocr_count, int avatar_count,
                                   const std::string& output_file = "process_summary.jpg");

private:
    const utils::Config& config_;
    std::filesystem::path output_dir_;

    /**
     * 初始化输出目录
     */
    void init_output_directory();
};

#endif  // DEBUG_MODE

}  // namespace export_ns

#endif  // OCR_MODELS_UTILS_VISUALIZATION_H
