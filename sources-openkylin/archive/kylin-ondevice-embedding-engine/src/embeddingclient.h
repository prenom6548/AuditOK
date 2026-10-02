/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#pragma once

#include <grpc_client.h>
#include <kylin-ai/ai-engine/embeddingengine.h>

#include <memory>
#include <mutex>
#include <opencv4/opencv2/opencv.hpp>
#include <string>

namespace ai_engine::core_ai::embedding {

enum ERROR_CODE : int {
    SYSTEM_ERR = -1,
    SUCCESS = 0,
    LOAD_MODEL_ERROR,
    CREATE_TRITON_CLIENT_ERROR,
    TRITON_INFER_ERROR,
    ADD_EMBEDDING_TASK_ERROR,
};
typedef struct EmbeddingClientError {
    int code;
    std::string message;
} EmbeddingClientError;

typedef struct EmbeddingClientResult {
    bool success;
    EmbeddingClientError error;
    std::vector<float> res;
    EmbeddingClientResult() {}
    EmbeddingClientResult(bool success, std::vector<float> res)
        : success(success), res(res), error{SUCCESS, "Success"} {}

    EmbeddingClientResult(bool success, int code, std::string message)
        : success(success), res({}), error({code, message}) {}

} EmbeddingClientResult;

struct InferChannelInfo {
    std::string display_name;        // 对外显示的模型名
    std::string manage_name;         // sdk传入的模型名
    std::string model_capability;    // 模型能力
    std::string channel_capability;  // 模型的具体功能，如图像向量化或文本向量化
    std::string model_name;          // triton端的模型名
    int model_priority;              // 模型优先级
    std::string channel_input_name;  // input_name
    std::string channel_output_name;  // output_name
    int channel_output_dim;           // 输出维度
};

class EmbeddingClient {
public:
    EmbeddingClient();
    result::AiEngineBaseResult init();
    bool createTritonClient();

    std::string modelInfo();
    void setEmbeddingResultCallback(EmbeddingResultCallback callback);

    std::string currentTextEmbeddingModel();
    result::AiEngineBaseResult setCurrentTextEmbeddingModel(
        const std::string& manageName);

    std::string currentImageEmbeddingModel();
    result::AiEngineBaseResult setCurrentImageEmbeddingModel(
        const std::string& manageName);

    EmbeddingClientResult embeddingTextInfer(const std::string& text);
    EmbeddingClientResult embeddingImageInfer(
        const std::string& fileData,
        int fileDataType);  // dataType
                            // 区分图片数据类型，0:输入为图片路径，1：输入base64图片格式
    EmbeddingClientResult embeddingTextInferWithImageModel(
        const std::string& text);

    result::AiEngineBaseResult embeddingText(const std::string& text);
    result::AiEngineBaseResult embeddingImageFromImageFile(
        const std::string& filePath);
    result::AiEngineBaseResult embeddingImageFromBase64ImageData(
        const std::string& fileData);
    result::AiEngineBaseResult embeddingTextWithImageModel(
        const std::string& text);
    void stopImageEmbedding();
    void stopTextEmbedding();

private:
    // int modelSwitch
    // 由于文本和图像模型对于输出shape的检查方式不同，所以用这个参数区分一下
    // modelSwitch 0:text model , 1: image model
    bool validateShapeAndDatatype(
        const std::string& name,
        std::shared_ptr<triton::client::InferResult> result,
        const std::string& expectedDataType, const int modelSwitch);
    std::vector<unsigned char> base64Decode(const std::string& input);
    std::vector<float> imageToFloatVector(const cv::Mat& image);
    std::vector<float> preprocessVision(cv::Mat& image);
    std::string getConfigValue(const char* key, const char* engine);
    std::string buildChannelCapabilityJson();
    bool parseJsonToModelInfo(const std::string& jsonStr);
    bool updateModelInfo();

private:
    bool initTritonClient_ = false;
    InferChannelInfo textModelChannel_;
    InferChannelInfo imageModelTextChannel_;
    InferChannelInfo imageModelImageChannel_;

private:
    const std::string grpcServerUrl_ = "localhost:8001";
    std::unique_ptr<triton::client::InferenceServerGrpcClient> grpcClient_;
    std::vector<InferChannelInfo> inferChannelInfoList_;
    EmbeddingResultCallback callback_ = nullptr;

private:
    std::atomic<bool> embeddingTextRunning_;
    std::atomic<bool> embeddingImageRunning_;
    std::mutex callBackMutex_;  // 用于保护回调函数变量的互斥锁
};

}  // namespace ai_engine::core_ai::embedding