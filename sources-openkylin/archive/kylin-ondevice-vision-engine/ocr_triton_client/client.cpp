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

#include "client.h"

#include <nlohmann/json.hpp>

using namespace std;
namespace nl = nlohmann;
namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

vision::TextRecognitionResult parseJsonToTextRecognitionResult(const std::string& jsonString) {
    vision::TextRecognitionResult result;
    result.success = true;  // 假设成功时 errorCode 为 -1

    try {
        nl::json j = nl::json::parse(jsonString);

        std::ostringstream allTextStream;

        for (const auto& item : j) {
            vision::TextLine line;
            line.text = item.at("text").get<std::string>();
            line.conf = item.at("confidence").get<float>();

            // 累积 allText，用换行符分割
            if (!allTextStream.str().empty()) {
                allTextStream << "\n";
            }
            allTextStream << line.text;

            // 解析多边形点
            for (const auto& point : item.at("polygon")) {
                vision::PixelPoint pixel;
                pixel.x = point[0].get<int>();
                pixel.y = point[1].get<int>();
                line.points.push_back(pixel);
            }

            result.lines.push_back(line);
        }

        result.allText = allTextStream.str();
    } catch (const std::exception& e) {
        result.success = false;  // 假设错误时 errorCode 为 -1
        result.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError, e.what());
        // result.errorMessage = e.what();
    }

    return result;
}

OcrTritonClient::OcrTritonClient(const string& url) {
    // Create the Triton client once during object construction
    FAIL_IF_ERR(tc::InferenceServerHttpClient::Create(&client_, url, false), "unable to create http client");
    OcrRunning_.store(false, std::memory_order_release);
}

void OcrTritonClient::setTextRecognitionResultCallback(TextRecognitionResultCallback callback) {
    ocrResultCallback_ = std::move(callback);
}

result::AiEngineBaseResult OcrTritonClient::OcrRequestByPath(string ImagePath) {
    std::unique_lock<std::mutex> lock(ocrMutex_);
    OcrRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    vision::TextRecognitionResult res;

    // 使用 ifstream 检查文件是否存在
    std::ifstream file(ImagePath);
    if (!file.is_open()) {
        // 如果文件无法打开，返回错误信息
        res.success = false;  // 假设错误时 errorCode 为 -1
        res.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::ImageFileReadError,
                                           "[Ocr] Can not open file");
        OcrRunning_.store(false, std::memory_order_release);
        return res;
    }

    cv::Mat src_img = cv::imread(ImagePath);
    res = OcrRequestByImage(src_img);
    if (!res.success) {
        error.success = false;
        error.error = ai_engine::EngineError((int)VisionEngineErrorCode::ImageFileReadError,
                                             "[OcrTriton]: failed to Request Ocr by img path.");
    }
    std::lock_guard<std::mutex> callBackLock(callBackMutex_);
    if (OcrRunning_.load(std::memory_order_acquire) == true) {
        if (ocrResultCallback_ != nullptr) {
            ocrResultCallback_(res);
        }
    }
    OcrRunning_.store(false, std::memory_order_release);

    return res;
}

result::AiEngineBaseResult OcrTritonClient::OcrRequestByBase64(const char* ImageData) {
    std::unique_lock<std::mutex> lock(ocrMutex_);
    OcrRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    vision::TextRecognitionResult res;
    cv::Mat Image = Base64ToMat(ImageData);
    res = OcrRequestByImage(Image);
    if (!res.success) {
        error.success = false;
        error.error = ai_engine::EngineError((int)VisionEngineErrorCode::ImageFileReadError,
                                             "[OcrTriton]: failed to Request Ocr by base64.");
    }
    std::lock_guard<std::mutex> callBackLock(callBackMutex_);
    if (OcrRunning_.load(std::memory_order_acquire) == true) {
        if (ocrResultCallback_ != nullptr) {
            ocrResultCallback_(res);
        }
    }
    OcrRunning_.store(false, std::memory_order_release);

    return error;
}

std::string OcrTritonClient::GetConfigValue(const char* key, const char* engine) {
    std::string model_name = "model_bank";
    tc::Error err = client_->LoadModel(model_name);
    tc::InferOptions options(model_name);
    options.model_version_ = "";
    options.priority_ = 1;

    std::vector<std::string> input_key_data{std::string(key)};
    std::vector<std::string> input_engine_data{std::string(engine)};

    tc::InferInput *input0, *input1;
    std::vector<int64_t> shape{1};
    tc::InferInput::Create(&input0, "key", shape, "BYTES");
    tc::InferInput::Create(&input1, "engine", shape, "BYTES");
    std::shared_ptr<tc::InferInput> input0_ptr, input1_ptr;
    input0_ptr.reset(input0);
    input1_ptr.reset(input1);
    input0_ptr->AppendFromString(input_key_data);
    input1_ptr->AppendFromString(input_engine_data);
    std::vector<tc::InferInput*> inputs = {input0_ptr.get(), input1_ptr.get()};

    tc::InferRequestedOutput* output0;
    tc::Error err_out0 = tc::InferRequestedOutput::Create(&output0, "value");
    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);
    std::vector<const tc::InferRequestedOutput*> outputs = {output0_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        std::cout << __LINE__ << " err_infer.Message(): " << err_infer.Message() << std::endl;
        return "";
    }

    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* output0_data_ptr;
    size_t recv_output0_byte_size;

    std::vector<std::string> text_output_data;
    tc::Error err_paser = results_ptr->StringData("value", &text_output_data);
    if (!err_paser.IsOk()) {
        std::cout << __LINE__ << " err_paser.Message(): " << err_infer.Message() << std::endl;
        return "";
    }
    return text_output_data[0];
}

vision::TextRecognitionResult OcrTritonClient::OcrRequestByImage(cv::Mat Image) {
    vision::TextRecognitionResult res;
    bool ready = false;
    tc::Error error = client_->IsModelReady(&ready, model_name_);
    if (!error.IsOk()) {
        res.success = false;
        res.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError,
                                           "[Ocr] model status check failed");
        std::cerr << "[OCR]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return res;
    }
    if (!ready) {
        std::cerr << "[OCR]Model error: model is not ready, we have to LoadModel "
                     "model : "
                  << model_name_ << std::endl;
        error = client_->LoadModel(model_name_);
        if (!error.IsOk()) {
            res.success = false;
            res.error =
                ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError, "[Ocr] model load failed");
            std::cerr << "[OCR]:Triton error: unable to LoadModel model." << model_name_
                      << "error message:" << error.Message() << std::endl;
            return res;
        }
    }
    if (Image.empty()) {
        res.success = false;
        res.error =
            ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::ImageFileReadError, "[Ocr] Image is empty");
        std::cout << __LINE__ << " res.errorMessage: " << res.error.getMessage() << std::endl;
        return res;
    }

    // Resize image if needed (currently retains original size)
    cv::Mat img_resize;
    cv::resize(Image, img_resize, Image.size());

    // Prepare input data
    std::vector<uint8_t> input_data(img_resize.total() * img_resize.channels());
    std::memcpy(input_data.data(), img_resize.data, input_data.size());

    // Perform inference
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int64_t> shape{img_resize.rows, img_resize.cols, 3};

    // Initialize the inputs with the data.
    tc::InferInput* input0;

    tc::Error err_ino = tc::InferInput::Create(&input0, "IN0", shape, "UINT8");
    if (!err_ino.IsOk()) {
        res.success = false;  // 假设错误时 errorCode 为 -1
        res.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError, err_ino.Message());
        // model error "unable to get product_reviews"
        std::cout << __LINE__ << " res.errorMessage: " << res.error.getMessage() << std::endl;
        return res;
    }

    std::shared_ptr<tc::InferInput> input0_ptr;
    input0_ptr.reset(input0);
    input0_ptr->SetBinaryData(true);
    input0_ptr->AppendRaw(input_data.data(), input_data.size());

    tc::InferRequestedOutput* output0;

    tc::Error err_out0 = tc::InferRequestedOutput::Create(&output0, "OUT0");
    if (!err_out0.IsOk()) {
        res.success = false;  // 假设错误时 errorCode 为 -1
        res.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError, err_out0.Message());
        // req error "unable to get unnorm_text_features"
        std::cout << __LINE__ << " res.errorMessage: " << res.error.getMessage() << std::endl;
        return res;
    }

    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);

    std::string model_version = "";
    // The inference settings. Will be using default for now.
    tc::InferOptions options(model_name_);
    options.model_version_ = model_version;
    options.priority_ = 1;

    std::vector<tc::InferInput*> inputs = {input0_ptr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {output0_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        res.success = false;  // 假设错误时 errorCode 为 -1
        res.error = ai_engine::EngineError((int)ai_engine::VisionEngineErrorCode::OcrModelError, err_infer.Message());
        // Infer error "unable to run model"
        std::cout << __LINE__ << " res.errorMessage: " << res.error.getMessage() << std::endl;
        return res;
    }
    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* output0_data_ptr;
    size_t recv_output0_byte_size;
    results_ptr->RawData("OUT0", (const uint8_t**)&output0_data_ptr, &recv_output0_byte_size);

    auto end_time = std::chrono::high_resolution_clock::now();

    res = parseJsonToTextRecognitionResult(output0_data_ptr);

    return res;
}

void OcrTritonClient::init() {
    model_name_ = GetConfigValue("ocr", "ocr");
    if (model_name_.size() < 1) {
        printf("line:%d model_name.size():%ld\n", __LINE__, model_name_.size());
        model_name_ = "ocr_ppocr";
    }

    return;
}

result::AiEngineBaseResult OcrTritonClient::stopTextRecognition() {
    if (OcrRunning_.load(std::memory_order_acquire)) {
        OcrRunning_.store(false, std::memory_order_release);
        vision::TextRecognitionResult res;
        res.success = true;
        res.modelName = model_name_;
        res.allText = "";
        res.lines.clear();
        if (ocrResultCallback_ != nullptr) {
            ocrResultCallback_(res);
        }
    }
    result::AiEngineBaseResult error;
    return error;
}

}  // namespace ai_engine::core_ai::vision