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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "ondeviceengine.h"
#include "../ondevicevisioncapabilityengine.h"
#include <opencv2/opencv.hpp>
#include "base64_transformer.h"
#include <iostream>
#include <chrono>
#include <thread>
void ocrPrintResult(const ai_engine::core_ai::vision::TextRecognitionResult &result) {
    printf("result.allText:%s\n",result.allText.c_str());
    
   printf("-----printResult:result.lines.size():%ld\n",result.lines.size());
    for(auto line : result.lines) {
        std::cout << line.text << std::endl;

    }
}

void mattingPrintResult(const ai_engine::core_ai::vision::ImageResult result) {
    base64_to_file(result.imageBase64Data, "../test_data/matting_out.png");
}


void imgBinPrintResult(const ai_engine::core_ai::vision::ImageResult result) {
    base64_to_file(result.imageBase64Data, "../test_data/bin_out.png");
}

int main(int argc, char **argv) {
    ai_engine::core_ai::vision::OndeviceVisionImageProcessEngine *vision_ImageProcess_ = new ai_engine::core_ai::vision::OndeviceVisionImageProcessEngine();
    ai_engine::core_ai::vision::OndeviceVisionTextRecognitionEngine *vision_TextRecognition_ = new ai_engine::core_ai::vision::OndeviceVisionTextRecognitionEngine();

    std::string img_path_ocr = "../test_data/ocr.jpg";
    std::string img_path_matting = "../test_data/matting.jpeg";
    std::string img_path_ddcp = "../test_data/ddcp.jpg";
    std::string img_bin_ddcp = "../test_data/bin.jpg";

    // ai_engine::EngineError error;
    ai_engine::result::AiEngineBaseResult res;
    res = vision_TextRecognition_->init();
    res = vision_ImageProcess_->initMattingsModule();
    vision_TextRecognition_->setTextRecognitionResultCallback(ocrPrintResult);
    vision_ImageProcess_->setMattingResultCallback(mattingPrintResult);

    vision_ImageProcess_->setImageBinaryResultCallback(imgBinPrintResult);
    std::string model_info = vision_TextRecognition_->modelInfo();
    printf("vision_TextRecognition_:model_info:%s\n",model_info.c_str());
    std::string model_info2 = vision_ImageProcess_->modelInfo();
    printf("vision_ImageProcess_:model_info:%s\n",model_info2.c_str());
    int index = 0;

    std::string image_data_ocr = file_to_base64(img_path_ocr);
    std::string image_data_matting = file_to_base64(img_path_matting);
    // std::string image_data_ddcp = file_to_base64(img_path_ddcp);
    std::string image_data_bin = file_to_base64(img_path_ddcp);
    ai_engine::EngineError error2;
    res = vision_TextRecognition_->recognizeTextFromBase64ImageData(image_data_ocr.c_str());
    res = vision_ImageProcess_->mattingFromBase64ImageData(image_data_matting.c_str());
    res = vision_ImageProcess_->binarizeImageFromBase64ImageData(image_data_bin.c_str());
}