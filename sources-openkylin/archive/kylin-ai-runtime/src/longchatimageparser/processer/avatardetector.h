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
 * 头像检测器模块
 * 负责检测聊天界面中的头像位置
 */

#pragma once

#include <map>
#include <memory>
#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

#include "../utils/config.h"
#include "../utils/ocravatartypes.h"
#include "../utils/sliceinfo.h"

namespace detection {

/**
 * 边界框结构体
 * 用于表示矩形区域,采用左上角坐标+宽高格式
 */
struct BoundingBox {
    int x;       // 左上角X坐标
    int y;       // 左上角Y坐标
    int width;   // 宽度
    int height;  // 高度

    BoundingBox() : x(0), y(0), width(0), height(0) {}
    BoundingBox(int x_, int y_, int w, int h) : x(x_), y(y_), width(w), height(h) {}

    int area() const { return width * height; }
    int right() const { return x + width; }
    int bottom() const { return y + height; }
};

/**
 * 带切片索引的边界框
 */
struct BoundingBoxWithSlice {
    int x;
    int y;
    int width;
    int height;
    int slice_index;

    BoundingBoxWithSlice() : x(0), y(0), width(0), height(0), slice_index(-1) {}
    BoundingBoxWithSlice(int x_, int y_, int w, int h, int slice_idx)
        : x(x_), y(y_), width(w), height(h), slice_index(slice_idx) {}
    BoundingBoxWithSlice(const BoundingBox& box, int slice_idx)
        : x(box.x), y(box.y), width(box.width), height(box.height), slice_index(slice_idx) {}

    BoundingBox toBoundingBox() const { return BoundingBox(x, y, width, height); }
};

/**
 * 头像检测器
 * 实现轮廓检测算法、正方形判定和头像框合并功能
 */
class AvatarDetector {
public:
    /**
     * 构造函数
     *
     * @param config 配置对象
     */
    explicit AvatarDetector(const utils::Config& config);

    /**
     * 检测切片中的头像
     *
     * @param slice_info 切片信息
     * @param x_crop X方向裁剪值（如果为nullopt，则自动计算）
     * @return 检测到的头像列表
     */
    std::vector<utils::AvatarItem> detectAvatars(const utils::SliceInfo& slice_info,
                                                 std::optional<int> x_crop = std::nullopt);

    /**
     * 基于多个切片计算合适的x_crop值
     *
     * @param slice_infos 切片信息列表
     * @return 计算得到的x_crop值
     */
    std::optional<int> calculateXCrop(const std::vector<utils::SliceInfo>& slice_infos);

private:
    // 配置相关
    const utils::Config& config_;
    std::string debug_dir_;

    // 头像检测参数
    int binary_threshold_;
    std::pair<int, int> gaussian_blur_size_;
    double strict_square_ratio_min_;
    double strict_square_ratio_max_;
    double iou_threshold_;
    int x_crop_offset_;

    /**
     * 预处理图像：转灰度、模糊、二值化
     *
     * @param image 输入图像
     * @return 处理后的二值图像
     */
    cv::Mat preprocessImage(const cv::Mat& image);

    /**
     * 提取轮廓并计算外接矩形
     *
     * @param binary_img 二值图像
     * @param original_img 原始图像
     * @return 矩形列表
     */
    std::vector<BoundingBox> extractContoursAndRects(const cv::Mat& binary_img, const cv::Mat& original_img);

    /**
     * 为单个切片找到target_box
     *
     * @param slice_info 切片信息
     * @return target_box，如果未找到则为nullopt
     */
    std::optional<BoundingBox> findTargetBoxForSlice(const utils::SliceInfo& slice_info);

    /**
     * 选择最合适的target_box
     *
     * @param rects 矩形列表
     * @return 最合适的target_box，如果未找到则为nullopt
     */
    std::optional<BoundingBox> selectTargetBox(const std::vector<BoundingBox>& rects);

    /**
     * 从所有框中找到最合适的目标框
     *
     * @param all_boxes 所有框的列表
     * @return 最合适的目标框，如果未找到则为nullopt
     */
    std::optional<BoundingBoxWithSlice> findBestTargetBox(const std::vector<BoundingBoxWithSlice>& all_boxes);

    /**
     * 根据切片数量决定处理策略
     *
     * @param slice_infos 切片信息列表
     * @return 需要处理的切片列表
     */
    std::vector<utils::SliceInfo> getSlicesToProcess(const std::vector<utils::SliceInfo>& slice_infos);

    /**
     * 应用非最大抑制算法去除重叠框
     *
     * @param rects 矩形列表
     * @return 去重后的矩形列表
     */
    std::vector<BoundingBox> applyNms(const std::vector<BoundingBox>& rects);

    /**
     * 计算两个框的IOU
     *
     * @param box1 框1
     * @param box2 框2
     * @return IOU值
     */
    double calculateIou(const BoundingBox& box1, const BoundingBox& box2);

    /**
     * 根据最大框计算合并阈值
     *
     * @param rects 矩形列表
     * @return 合并阈值
     */
    double calculateMergeThreshold(const std::vector<BoundingBox>& rects);

    /**
     * 合并相邻的边界框
     *
     * @param rects 矩形列表
     * @param merge_threshold 合并阈值
     * @return 合并后的矩形列表
     */
    std::vector<BoundingBox> mergeNearbyBoxes(const std::vector<BoundingBox>& rects, double merge_threshold);

    /**
     * 判断两个框是否应该合并
     *
     * @param box1 框1
     * @param box2 框2
     * @param distance_threshold 距离阈值
     * @return 是否应该合并
     */
    bool shouldMerge(const BoundingBox& box1, const BoundingBox& box2, double distance_threshold);

    /**
     * 合并一组框为一个最小外接框
     *
     * @param boxes 框组列表
     * @return 合并后的框，如果失败则为nullopt
     */
    std::optional<BoundingBox> mergeBoxes(const std::vector<BoundingBox>& boxes);

    /**
     * 保存调试图像
     *
     * @param image 原图像
     * @param rects 矩形列表
     * @param slice_index 切片索引
     */
    void saveDebugImage(const cv::Mat& image, const std::vector<BoundingBox>& rects, int slice_index);

    /**
     * 判断矩形是否为严格正方形
     *
     * @param rect 矩形
     * @return 是否为严格正方形
     */
    bool isStrictSquare(const BoundingBox& rect);
};

}  // namespace detection
