//#pragma once

#include "text_det.h"
#include "text_rec.h"
#include <nlohmann/json.hpp>

struct PolygonAndText
{
	std::vector<cv::Point2f> polygon;
	std::string text;
};

class OCRDetector {
public:
    // 构造函数，初始化内部的组件
    OCRDetector(const char* model_location);
    // 析构函数
    ~OCRDetector();
    std::string model_location_;
	
    // 主要的OCR处理函数，负责调用各个子组件完成整个OCR流程
    std::string Ocrdet(cv::Mat& image);

private:
	TextDetector detector_ ;    // 文本检测器
    // TextClassifier classifier_; // 文本分类器
    TextRecognizer recognizer_ ;// 文本识别器
};