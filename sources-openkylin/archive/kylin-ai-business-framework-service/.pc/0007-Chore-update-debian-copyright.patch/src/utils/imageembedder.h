#ifndef IMAGEEMBEDDER_H
#define IMAGEEMBEDDER_H

#include <onnxruntime/onnxruntime_cxx_api.h>

#include "autotokenizer.h"

namespace cv {
class Mat;
}

class ImageEmbedder {
public:
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
