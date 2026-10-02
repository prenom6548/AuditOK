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

#ifndef OCR_MODELS_UTILS_CHINESE_TEXT_RENDERER_H
#define OCR_MODELS_UTILS_CHINESE_TEXT_RENDERER_H

#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

#ifdef USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace export_ns {

#ifdef DEBUG_MODE

/**
 * 中文字体渲染器
 * 用于在OpenCV图像上渲染中文字符
 */
class ChineseTextRenderer {
public:
    /**
     * 构造函数
     *
     * @param font_path 字体文件路径
     * @param font_size 字体大小（像素）
     */
    ChineseTextRenderer(const std::string& font_path, int font_size = 20);

    /**
     * 析构函数
     */
    ~ChineseTextRenderer();

    /**
     * 在图像上绘制中文文本
     *
     * @param image 目标图像
     * @param text 要绘制的文本（UTF-8编码）
     * @param position 文本位置
     * @param color 文本颜色
     * @param thickness 文本粗细（可选，默认为1）
     */
    void putText(cv::Mat& image, const std::string& text, const cv::Point& position, const cv::Scalar& color,
                 int thickness = 1);

    /**
     * 计算文本的大小
     *
     * @param text 文本内容
     * @return 文本大小 (width, height)
     */
    cv::Size getTextSize(const std::string& text);

    /**
     * 设置字体大小
     *
     * @param size 新的字体大小
     */
    void setFontSize(int size);

    /**
     * 检查是否成功初始化
     *
     * @return true if initialized successfully
     */
    bool isInitialized() const { return initialized_; }

private:
#ifdef USE_FREETYPE
    FT_Library library_;
    FT_Face face_;
#endif
    std::string font_path_;
    int font_size_;
    bool initialized_;

    /**
     * 初始化FreeType库和字体
     *
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * 渲染单个字符到图像
     *
     * @param image 目标图像
     * @param character UTF-32字符码
     * @param x 字符x坐标
     * @param y 字符y坐标
     * @param color 字符颜色
     * @param thickness 粗细
     */
    void renderCharacter(cv::Mat& image, uint32_t character, int x, int y, const cv::Scalar& color, int thickness);

    /**
     * 将UTF-8字符串转换为UTF-32
     *
     * @param utf8_str UTF-8字符串
     * @return UTF-32字符向量
     */
    std::vector<uint32_t> utf8ToUtf32(const std::string& utf8_str);
};

#endif  // DEBUG_MODE

}  // namespace export_ns

#endif  // OCR_MODELS_UTILS_CHINESE_TEXT_RENDERER_H
