/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <iostream>
#include "libkyimageproc.h"

extern "C" 
{
    void* create_image_pixelmap(int width, int height)
    {
        cv::Mat *img = new cv::Mat(height, width, CV_8UC3);
        if(img->empty())
        {
            // std::cerr << "Error: Failed to create image." << std::endl;
            delete img;
            return nullptr;
        }

        return static_cast<void*>(img);
    }

    // 函数声明，用于加载图像并返回图像的宽度和高度
    void* load_image(char * srcpath)
    {
        // std::cerr << srcpath << std::endl;
        cv::Mat* img = new cv::Mat(cv::imread(srcpath, cv::IMREAD_UNCHANGED));
        if (img->empty()) {
            // std::cerr << "Error: Could not open or find the image." << std::endl;
            delete img;
            return nullptr;
        }
        return static_cast<void*>(img);
    }

    kdk_image_info *get_image_info(void *handle)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return nullptr;
        }

        kdk_image_info *info = nullptr;
        info = (kdk_image_info *)calloc(1, sizeof(kdk_image_info));
        if(!info)
            return nullptr;

        info->size.width = img->cols;
        info->size.height = img->rows;
        return info;
    }

    int get_image_bytes(void *handle)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return -1;
        }
        int ret = -1;
        ret = img->step;
        return ret;
    }

    int get_image_bytes_total(void *handle)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return -1;
        }
        int ret = -1;
        ret = img->total() * img->elemSize();
        return ret;
    }

    void set_image_opacity(void *handle, float rate)
    {
        if (rate < 0.0 || rate > 1.0) {
            // std::cerr << "Error: rate must be between 0 and 1." << std::endl;
            return;
        }

        // 将 void* 转换为 cv::Mat*
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return;
        }

        // std::cerr << img->channels() << std::endl;

        // 检查图像是否已经有 alpha 通道
        if (img->channels() == 3) {
            // 创建具有 alpha 通道的新图像
            cv::Mat imgWithAlpha;
            cv::cvtColor(*img, imgWithAlpha, cv::COLOR_BGR2BGRA);
            *img = imgWithAlpha;
            // std::cerr << "Error: no alpha" << std::endl;
        }

        // 设置透明度
        for (int y = 0; y < img->rows; y++) {
            for (int x = 0; x < img->cols; x++) {
                cv::Vec4b& pixel = img->at<cv::Vec4b>(y, x);
                pixel[3] = static_cast<uchar>(pixel[3] * rate);
            }
        }
        return;
    }

    void set_image_scale(void *handle, int xrate, int yrate)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);

        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return;
        }

        // // 计算新的尺寸
        // int new_width = img->cols * xrate / 100;
        // int new_height = img->rows * yrate / 100;

        // 缩放图像
        cv::resize(*img, *img, cv::Size(xrate, yrate));
    }

    void set_image_rotate(void *handle, int angle)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return;
        }

        // 获取图像中心
        cv::Point2f center(img->cols / 2.0, img->rows / 2.0);

        // 计算旋转矩阵
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

        // 计算旋转后的图像边界
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), img->size(), angle).boundingRect2f();

        // 调整旋转矩阵以考虑平移
        rot.at<double>(0, 2) += bbox.width / 2.0 - img->cols / 2.0;
        rot.at<double>(1, 2) += bbox.height / 2.0 - img->rows / 2.0;

        // 旋转图像
        cv::warpAffine(*img, *img, rot, bbox.size());
    }

    void set_image_flip(void *handle, bool horizontal, bool vertical)
    {
        if(!horizontal && !vertical) {
            return ;
        }

        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return;
        }

        // 设置翻转代码
        int flipCode = 0; // 0 表示垂直翻转
        if (horizontal && vertical) {
            flipCode = -1; // 同时水平和垂直翻转
        } else if (horizontal) {
            flipCode = 1; // 水平翻转
        }

        // 翻转图像
        cv::flip(*img, *img, flipCode);
    }

    void set_image_crop(void *handle, int x, int y, int height, int width)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return;
        }

        // 检查剪裁区域是否超出图像边界
        if (x < 0 || y < 0 || x + width > img->cols || y + height > img->rows) {
            // std::cerr << "Error: crop region is out of image bounds." << std::endl;
            return;
        }

        // 剪裁图像
        cv::Rect cropRegion(x, y, width, height);
        *img = (*img)(cropRegion);
    }

    int image_save(void *handle, char *dstpath)
    {
        cv::Mat* img = static_cast<cv::Mat*>(handle);
        if (img == nullptr || img->empty()) {
            // std::cerr << "Error: invalid image handle." << std::endl;
            return -1;
        }
        bool result = cv::imwrite(dstpath, *img);
        delete static_cast<cv::Mat*>(img);
        if (result)
        {
            //  std::cerr << "Image saved successfully!" << std::endl;
            return 0;
        } 
        else{
            //  std::cerr << "Failed to save image!" << std::endl;
            return -1;
        }
    }
}
