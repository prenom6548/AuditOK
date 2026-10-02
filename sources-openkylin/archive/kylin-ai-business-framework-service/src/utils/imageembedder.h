/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IMAGEEMBEDDER_H
#define IMAGEEMBEDDER_H

#include <onnxruntime/onnxruntime_cxx_api.h>

#include "autotokenizer.h"

namespace cv {
class Mat;
}

class ImageEmbedder {
public:
    static bool isModelSupport();
    static bool isModelInstalled();

    ImageEmbedder();

    std::vector<float> embeddingText(const std::string &text);
    std::vector<float> embeddingImage(const std::string &filePath);

private:
    Ort::Session textSession_;
    Ort::Session imageSession_;
    AutoTokenizer tokenizer_;

    std::vector<float> preprocessVision(const std::string& filePath) const;
    std::vector<float> imageToFloatVector(const cv::Mat& image) const;

};

#endif // IMAGEEMBEDDER_H
