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

#include "chinesetextrenderer.h"

#include <algorithm>
#include <iostream>

#include "../../utils/logger.h"

#ifdef DEBUG_MODE

namespace export_ns {

ChineseTextRenderer::ChineseTextRenderer(const std::string& font_path, int font_size)
    : font_path_(font_path), font_size_(font_size), initialized_(false) {
#ifdef USE_FREETYPE
    initialized_ = initialize();
#else
    LOGD(
        "[ChineseTextRenderer] FreeType support disabled - text rendering will "
        "use fallback");
    initialized_ = false;
#endif
}

ChineseTextRenderer::~ChineseTextRenderer() {
#ifdef USE_FREETYPE
    if (initialized_) {
        FT_Done_Face(face_);
        FT_Done_FreeType(library_);
    }
#endif
}

bool ChineseTextRenderer::initialize() {
#ifdef USE_FREETYPE
    // 初始化FreeType库
    if (FT_Init_FreeType(&library_)) {
        LOGE("[ChineseTextRenderer] Unable to initialize FreeType library");
        return false;
    }

    // 加载字体文件
    if (FT_New_Face(library_, font_path_.c_str(), 0, &face_)) {
        LOGE("[ChineseTextRenderer] Unable to load font file: {}", font_path_);
        FT_Done_FreeType(library_);
        return false;
    }

    // 设置字体大小
    if (FT_Set_Pixel_Sizes(face_, 0, font_size_)) {
        LOGE("[ChineseTextRenderer] Unable to set font size: {}", font_size_);
        FT_Done_Face(face_);
        FT_Done_FreeType(library_);
        return false;
    }

    LOGD("[ChineseTextRenderer] Successfully initialized font: {}, size: {}", font_path_, font_size_);
    return true;
#else
    return false;
#endif
}

void ChineseTextRenderer::putText(cv::Mat& image, const std::string& text, const cv::Point& position,
                                  const cv::Scalar& color, int thickness) {
#ifdef USE_FREETYPE
    if (!initialized_) {
        LOGE("[ChineseTextRenderer] Renderer not initialized");
        return;
    }

    // 将UTF-8转换为UTF-32
    std::vector<uint32_t> utf32_chars = utf8ToUtf32(text);

    int x = position.x;
    int y = position.y;

    // 渲染每个字符
    for (uint32_t character : utf32_chars) {
        // 加载字符
        if (FT_Load_Char(face_, character, FT_LOAD_RENDER)) {
            LOGE("[ChineseTextRenderer] Unable to load character: {}", character);
            continue;
        }

        renderCharacter(image, character, x, y, color, thickness);

        // 移动到下一个字符位置
        x += (face_->glyph->advance.x >> 6);
    }
#else
    // Fallback: 使用OpenCV默认字体（不支持中文）
    cv::putText(image, text, position, cv::FONT_HERSHEY_SIMPLEX, 0.5, color, thickness);
#endif
}

void ChineseTextRenderer::renderCharacter(cv::Mat& image, uint32_t character, int x, int y, const cv::Scalar& color,
                                          int thickness) {
#ifdef USE_FREETYPE
    FT_GlyphSlot glyph = face_->glyph;
    FT_Bitmap& bitmap = glyph->bitmap;

    // 计算字符在图像中的位置
    int char_x = x + glyph->bitmap_left;
    int char_y = y - glyph->bitmap_top;

    // 确保字符在图像边界内
    if (char_x < 0 || char_y < 0 || char_x + bitmap.width >= image.cols || char_y + bitmap.rows >= image.rows) {
        return;
    }

    // 渲染位图到图像
    for (unsigned int row = 0; row < bitmap.rows; ++row) {
        for (unsigned int col = 0; col < bitmap.width; ++col) {
            int img_x = char_x + col;
            int img_y = char_y + row;

            if (img_x >= 0 && img_x < image.cols && img_y >= 0 && img_y < image.rows) {
                unsigned char pixel_value = bitmap.buffer[row * bitmap.pitch + col];

                if (pixel_value > 0) {
                    // 混合颜色（简单的alpha混合）
                    double alpha = pixel_value / 255.0;

                    cv::Vec3b& img_pixel = image.at<cv::Vec3b>(img_y, img_x);

                    for (int c = 0; c < 3; ++c) {
                        img_pixel[c] = static_cast<uchar>((1.0 - alpha) * img_pixel[c] + alpha * color[c]);
                    }
                }
            }
        }
    }
#endif
}

cv::Size ChineseTextRenderer::getTextSize(const std::string& text) {
#ifdef USE_FREETYPE
    if (!initialized_) {
        return cv::Size(0, 0);
    }

    std::vector<uint32_t> utf32_chars = utf8ToUtf32(text);

    int width = 0;
    int height = font_size_;

    for (uint32_t character : utf32_chars) {
        if (FT_Load_Char(face_, character, FT_LOAD_DEFAULT)) {
            continue;
        }
        width += (face_->glyph->advance.x >> 6);
        height = std::max(height, static_cast<int>(face_->glyph->bitmap.rows));
    }

    return cv::Size(width, height);
#else
    // Fallback: 返回估算大小
    return cv::Size(text.length() * 10, 16);
#endif
}

void ChineseTextRenderer::setFontSize(int size) {
#ifdef USE_FREETYPE
    if (!initialized_) {
        return;
    }

    font_size_ = size;
    FT_Set_Pixel_Sizes(face_, 0, font_size_);
#else
    font_size_ = size;
#endif
}

std::vector<uint32_t> ChineseTextRenderer::utf8ToUtf32(const std::string& utf8_str) {
    std::vector<uint32_t> utf32_chars;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(utf8_str.data());
    size_t len = utf8_str.length();

    for (size_t i = 0; i < len;) {
        uint32_t codepoint = 0;
        int char_len = 0;

        if ((data[i] & 0x80) == 0) {
            // 1字节字符 (ASCII)
            codepoint = data[i];
            char_len = 1;
        } else if ((data[i] & 0xE0) == 0xC0) {
            // 2字节字符
            if (i + 1 < len) {
                codepoint = ((data[i] & 0x1F) << 6) | (data[i + 1] & 0x3F);
                char_len = 2;
            }
        } else if ((data[i] & 0xF0) == 0xE0) {
            // 3字节字符（大部分中文字符）
            if (i + 2 < len) {
                codepoint = ((data[i] & 0x0F) << 12) | ((data[i + 1] & 0x3F) << 6) | (data[i + 2] & 0x3F);
                char_len = 3;
            }
        } else if ((data[i] & 0xF8) == 0xF0) {
            // 4字节字符
            if (i + 3 < len) {
                codepoint = ((data[i] & 0x07) << 18) | ((data[i + 1] & 0x3F) << 12) | ((data[i + 2] & 0x3F) << 6) |
                            (data[i + 3] & 0x3F);
                char_len = 4;
            }
        }

        if (char_len > 0 && codepoint > 0) {
            utf32_chars.push_back(codepoint);
            i += char_len;
        } else {
            // 跳过无效字节
            i++;
        }
    }

    return utf32_chars;
}

}  // namespace export_ns

#endif  // DEBUG_MODE
