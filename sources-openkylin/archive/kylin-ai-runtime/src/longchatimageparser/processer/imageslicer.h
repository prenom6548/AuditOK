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

#ifndef OCR_MODELS_CORE_IMAGE_SLICER_H  // 头文件保护
#define OCR_MODELS_CORE_IMAGE_SLICER_H

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <tuple>
#include <vector>

#include "../utils/config.h"
#include "../utils/sliceinfo.h"

namespace parser {

/**
 * 图像切片器
 * 负责将长图切分为多个切片，对应Python版本的ImageSlicer类
 */
class ImageSlicer {
public:
    /**
     * 构造函数
     *
     * @param config 配置对象
     */
    explicit ImageSlicer(const utils::Config& config);

    /**
     * 切分长图 串联所有逻辑：读取图片-选择处理策略-执行图像切分
     * @param image_path 图像路径
     * @return std::tuple<cv::Mat, std::vector<SliceInfo>> 原始图像和切片信息列表
     * @throws std::runtime_error 当无法读取图像时
     */
    std::tuple<cv::Mat, std::vector<utils::SliceInfo>> sliceImage(const std::string& image_path);

    /**
     * 根据切片数量决定处理策略
     * 处理策略：
     * - 1个切片：处理所有切片
     * - 2个切片：只处理第一个切片
     * - >=3个切片：只处理中间的切片（排除首尾）
     *
     * @param slice_infos 所有切片信息列表
     * @return 需要处理的切片列表
     */
    std::vector<utils::SliceInfo> getSlicesToProcess(const std::vector<utils::SliceInfo>& slice_infos);

private:
    /**
     * 执行图像切分
     *
     * @param image 原始图像
     * @return 切片信息列表
     */
    std::vector<utils::SliceInfo> performSlicing(const cv::Mat& image);

    /**
     * 保存切片图像
     *
     * @param slice_info 切片信息
     */
    void saveSlice(const utils::SliceInfo& slice_info);

    /**
     * 诊断图像读取失败的具体原因
     *
     * @param image_path 图像路径
     * @return 描述性错误信息
     */
    std::string diagnoseImageReadFailure(const std::string& image_path);

    // 配置对象引用
    const utils::Config& config_;

    // 切片参数
    int slice_height_;
    int overlap_;

    // 输出目录路径
    std::filesystem::path output_dir_;
};

}  // namespace parser

#endif  // OCR_MODELS_CORE_IMAGE_SLICER_H
