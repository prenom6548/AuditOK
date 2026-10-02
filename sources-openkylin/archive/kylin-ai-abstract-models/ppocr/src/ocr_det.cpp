#include "ocr.h"

// 构造函数，初始化内部的组件
OCRDetector::OCRDetector(const char* model_location)
	: 	detector_(model_location),
  		recognizer_(model_location){
		model_location_ = model_location;
	}

// 析构函数
OCRDetector::~OCRDetector() = default;

std::string formatResultsToString(const std::vector<std::pair<PolygonAndText, float>>& ocrResults) {
    nlohmann::json jsonArray = nlohmann::json::array(); // 创建一个JSON数组

    for (const auto& [polygon_text, confidence] : ocrResults) {
        nlohmann::json jsonObj; // 每个OCR结果为一个JSON对象
        
        // 处理多边形的点
        nlohmann::json pointsArray = nlohmann::json::array();
        for (const auto& point : polygon_text.polygon) {
            pointsArray.push_back({point.x, point.y});
        }
        
        jsonObj["polygon"] = pointsArray;
        jsonObj["text"] = polygon_text.text; // 自动处理特殊字符
        jsonObj["confidence"] = confidence;

        jsonArray.push_back(jsonObj);
    }

    return jsonArray.dump(2); // 返回格式化后的JSON字符串，缩进2格
}


// 主要的OCR处理函数
std::string OCRDetector::Ocrdet(cv::Mat& image)
{
	std::vector<std::pair<PolygonAndText, float>> Ocrresults;
	std::pair<string, float> recognize_result;
	PolygonAndText polygon_text;
	
	vector< vector<Point2f> > results_ori = detector_.detect(image);
	vector<vector<Point2f>> results = detector_.mergeBoundingBoxes(results_ori);
	int index = 0;

	for (size_t i = 0; i < results.size(); i++)
	{
		Mat textimg = detector_.get_rotate_crop_image(image, results[i]);
		recognize_result = recognizer_.predict_text(textimg);
		if (recognize_result.first == "" &&
            recognize_result.second < 0.00001f) {
            continue;
        }

		polygon_text.polygon=results[i];
		polygon_text.text=recognize_result.first;
		Ocrresults.push_back(std::make_pair(polygon_text, recognize_result.second));
		index++;
	}

    std::string resultString = formatResultsToString(Ocrresults);

    return resultString;
}
