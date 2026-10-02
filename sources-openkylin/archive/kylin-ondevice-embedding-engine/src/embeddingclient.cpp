#include "embeddingclient.h"

#include <getopt.h>
#include <unistd.h>

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "utils.h"

namespace tc = triton::client;
using json = nlohmann::json;

namespace ai_engine::core_ai::embedding {
EmbeddingClient::EmbeddingClient() {}

result::AiEngineBaseResult EmbeddingClient::init() {
    createTritonClient();

    bool succ = updateModelInfo();
    if (!succ) {
        std::cerr << "[Embedding server]:init failed, model info update error!"
                  << std::endl;
        return result::AiEngineBaseResult{
            false,
            ai_engine::EngineError(SYSTEM_ERR, "Model info update error")};
    }
    return result::AiEngineBaseResult{true,
                                      ai_engine::EngineError(0, "Success")};
}

bool EmbeddingClient::createTritonClient() {
    if (initTritonClient_) return true;
    tc::Error error = tc::InferenceServerGrpcClient::Create(
        &grpcClient_, grpcServerUrl_, true /* verbose */);

    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Create http client error,"
                  << error.Message() << std::endl;
        return false;
    }
    initTritonClient_ = true;
    return true;
}

void EmbeddingClient::setEmbeddingResultCallback(
    EmbeddingResultCallback callback) {
    std::lock_guard<std::mutex> callBackLock(callBackMutex_);
    callback_ = std::move(callback);
}

std::string EmbeddingClient::modelInfo() {
    createTritonClient();
    bool succ = updateModelInfo();
    if (!succ) {
        return "";
    }
    // 用于存储已经处理过的display_name，以避免重复
    std::set<std::string> processedNames;
    json outputJson;

    // 收集所有需要排序的条目
    std::vector<json> entriesToSort;

    for (const auto& item : inferChannelInfoList_) {
        std::string displayName = item.display_name;

        // 检查是否已处理当前display_name
        if (processedNames.find(displayName) == processedNames.end()) {
            // 如果未处理，则添加到集合中
            processedNames.insert(displayName);

            // 创建新条目
            json newEntry;
            newEntry["modelName"] = item.manage_name;
            newEntry["name"] = item.manage_name;
            newEntry["displayName"] = item.display_name;
            newEntry["model"] = item.manage_name;
            newEntry["priority"] = item.model_priority;
            newEntry["dim"] = item.channel_output_dim;
            newEntry["ondevice"] = true;
            newEntry["model_capability"] = item.model_capability;

            entriesToSort.push_back(newEntry);
        }
    }

    // 对entriesToSort根据priority进行排序
    std::sort(entriesToSort.begin(), entriesToSort.end(),
              [](const json& a, const json& b) -> bool {
                  return a["priority"].get<int>() < b["priority"].get<int>();
              });

    // 将排序后的条目添加到outputJson
    for (const auto& entry : entriesToSort) {
        outputJson["models"].push_back(entry);
    }
    outputJson["model_catalog"] = json::parse(buildChannelCapabilityJson());

    return outputJson.dump(4);
}

std::string EmbeddingClient::currentTextEmbeddingModel() {
    return textModelChannel_.manage_name;
}
result::AiEngineBaseResult EmbeddingClient::setCurrentTextEmbeddingModel(
    const std::string& manageName) {
    bool found = false;
    for (auto& InferchannelInfo : inferChannelInfoList_) {
        if (InferchannelInfo.manage_name == manageName) {
            textModelChannel_ = InferchannelInfo;
            found = true;
        }
    }
    if (!found) {
        return result::AiEngineBaseResult{
            false,
            ai_engine::EngineError(0, "Text model not found: " + manageName)};
    }
    return result::AiEngineBaseResult{true,
                                      ai_engine::EngineError(0, "Success")};
}

std::string EmbeddingClient::currentImageEmbeddingModel() {
    return imageModelImageChannel_.manage_name;
}
result::AiEngineBaseResult EmbeddingClient::setCurrentImageEmbeddingModel(
    const std::string& manageName) {
    bool found = false;
    for (auto& InferchannelInfo : inferChannelInfoList_) {
        if (InferchannelInfo.manage_name == manageName) {
            if (InferchannelInfo.channel_capability == "IMAGE") {
                imageModelImageChannel_ = InferchannelInfo;
                found = true;
            } else if (InferchannelInfo.channel_capability == "TEXT") {
                imageModelTextChannel_ = InferchannelInfo;
                found = true;
            }
        }
    }
    if (!found) {
        return result::AiEngineBaseResult{
            false,
            ai_engine::EngineError(0, "Image model not found: " + manageName)};
    }
    return result::AiEngineBaseResult{true,
                                      ai_engine::EngineError(0, "Success")};
}

EmbeddingClientResult EmbeddingClient::embeddingTextInfer(
    const std::string& text) {
    bool ready = false;
    tc::Error error =
        grpcClient_->IsModelReady(&ready, textModelChannel_.model_name);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    if (!ready) {
        error = grpcClient_->LoadModel(textModelChannel_.model_name);
        if (!error.IsOk()) {
            std::cerr
                << "[EmbeddingClient]:Triton error: unable to LoadModel model."
                << textModelChannel_.model_name
                << "error message:" << error.Message() << std::endl;
            return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                         error.Message());
        }
    }
    std::vector<std::string> inputData(1);
    inputData[0] = text;

    std::vector<int64_t> shape{1};

    // Initialize the inputs with the data.
    tc::InferInput* input0;
    error = tc::InferInput::Create(&input0, "product_reviews", shape, "BYTES");
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error: unable to get product_reviews,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferInput> inputPtr;
    inputPtr.reset(input0);
    inputPtr->SetBinaryData(true);

    error = inputPtr->AppendFromString(inputData);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to set data for "
                     "product_reviews,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    // Generate the outputs to be requested.
    tc::InferRequestedOutput* output0;

    error = tc::InferRequestedOutput::Create(&output0, "logits");
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get logits,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferRequestedOutput> outputPtr;
    outputPtr.reset(output0);
    outputPtr->SetBinaryData(true);

    // The inference settings. Will be using default for now.
    tc::InferOptions options(textModelChannel_.model_name);
    options.model_version_ = "";

    std::vector<tc::InferInput*> inputs = {inputPtr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {outputPtr.get()};

    tc::InferResult* results;
    error = grpcClient_->Infer(&results, options, inputs, outputs);
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton infer error: unable to run model,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferResult> resultsPtr;
    resultsPtr.reset(results);

    // Validate the results...
    if (!validateShapeAndDatatype("logits", resultsPtr, "FP32", 0)) {
        std::cerr << "[EmbeddingClient]: Trion infer error: embeddingText "
                     "validateShapeAndDatatype failed."
                  << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     "Trion infer error: embeddingText "
                                     "validateShapeAndDatatype failed.");
    }

    std::vector<int64_t> shapeOut;
    resultsPtr->Shape("logits", &shapeOut);

    float* outputDataPtr;

    const size_t outputByteSize =
        textModelChannel_.channel_output_dim * 4 * shapeOut[1] *
        shapeOut[0];  // 输出 1*756*768  会输出756(会变化)个768，取第一个768
                      // 所以是 768 * 4 * shapeOut[1] 字节

    size_t recvOutputByteSize;
    error = resultsPtr->RawData("logits", (const uint8_t**)&outputDataPtr,
                                &recvOutputByteSize);
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error: unable to get data for logits,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    if (recvOutputByteSize != outputByteSize) {
        std::cerr << "[EmbeddingClient]:Error: received incorrect byte size "
                     "for 'logits': "
                  << recvOutputByteSize << std::endl;
        return EmbeddingClientResult(
            false, TRITON_INFER_ERROR,
            "Trion infer error: embeddingText received "
            "incorrect byte size.");
    }
    std::vector<float> tokens;

    for (size_t i = 0; i < textModelChannel_.channel_output_dim; ++i) {
        tokens.push_back(outputDataPtr[i]);
    }

    normalize(tokens);

    return EmbeddingClientResult(true, tokens);
}
EmbeddingClientResult EmbeddingClient::embeddingImageInfer(
    const std::string& fileData, int fileDataType) {
    bool ready = false;
    tc::Error error =
        grpcClient_->IsModelReady(&ready, imageModelImageChannel_.model_name);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    if (!ready) {
        error = grpcClient_->LoadModel(imageModelImageChannel_.model_name);
        if (!error.IsOk()) {
            std::cerr
                << "[EmbeddingClient]:Triton error: unable to LoadModel model."
                << imageModelImageChannel_.model_name
                << "error message:" << error.Message() << std::endl;
            return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                         error.Message());
        }
    }
    cv::Mat image;
    try {
        if (fileDataType == 0)
            image = cv::imread(fileData, cv::IMREAD_COLOR);
        else if (fileDataType == 1) {
            std::vector<unsigned char> decodedImageData =
                base64Decode(fileData);
            if (decodedImageData.empty()) {
                return EmbeddingClientResult(false, SYSTEM_ERR,
                                             "Decoded image data is empty.");
            }
            image = cv::imdecode(decodedImageData, cv::IMREAD_COLOR);
        }
    } catch (const cv::Exception& e) {
        std::cerr << "[EmbeddingClient]:OpenCV Error:" << e.what() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     "Trion infer error: embeddingImage "
                                     "preprocessed image failed.");
    }

    if (image.empty()) {
        if (fileDataType == 0) {
            // 输入文件路径的才打印，否则base64数据太多了
            std::cerr << "[EmbeddingClient]:Error:Unable to load image from:"
                      << fileData << std::endl;
        }
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     "Trion infer error: embeddingImage "
                                     "preprocessed image is empty.");
    }
    std::vector<float> input0_data = preprocessVision(image);

    std::vector<int64_t> shape = {3, 224, 224};

    // Initialize the inputs with the data.
    tc::InferInput* input0;
    error = tc::InferInput::Create(&input0, "image", shape, "FP32");
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error:unable to get image,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    std::shared_ptr<tc::InferInput> inputPtr;
    inputPtr.reset(input0);
    error = inputPtr->AppendRaw(reinterpret_cast<uint8_t*>(&input0_data[0]),
                                input0_data.size() * sizeof(float));
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error:unable to set data for image,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    // The inference settings. Will be using default for now.
    tc::InferOptions options(imageModelImageChannel_.model_name);
    options.model_version_ = "";

    std::vector<tc::InferInput*> inputs = {inputPtr.get()};
    // Empty output vector will request data for all the output tensors from
    // the server.
    std::vector<const tc::InferRequestedOutput*> outputs = {};

    tc::InferResult* results;
    error = grpcClient_->Infer(&results, options, inputs, outputs);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error:unable to run model,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferResult> resultsPtr;
    resultsPtr.reset(results);

    // Validate the results...
    if (!validateShapeAndDatatype("unnorm_image_features", resultsPtr, "FP32",
                                  1)) {
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     "Trion infer error: embeddingImage "
                                     "validateShapeAndDatatype failed.");
    }

    // Get pointers to the result returned...
    float* outputData;
    size_t outputByteSize;
    error = resultsPtr->RawData("unnorm_image_features",
                                (const uint8_t**)&outputData, &outputByteSize);
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error:unable to get result data for "
               "unnorm_image_features,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    if (outputByteSize !=
        imageModelImageChannel_.channel_output_dim * sizeof(float)) {
        std::cerr << "[EmbeddingClient]:Triton error:received incorrect byte "
                     "size for "
                     "'unnorm_image_features':"
                  << outputByteSize << std::endl;
        return EmbeddingClientResult(
            false, TRITON_INFER_ERROR,
            "Trion infer error: embeddingImage received "
            "incorrect byte size.");
    }

    std::vector<float> output0_data_vector(
        outputData, outputData + imageModelImageChannel_.channel_output_dim);
    normalize(output0_data_vector);

    return EmbeddingClientResult{true, output0_data_vector};
}

EmbeddingClientResult EmbeddingClient::embeddingTextInferWithImageModel(
    const std::string& text) {
    bool ready = false;
    tc::Error error =
        grpcClient_->IsModelReady(&ready, imageModelTextChannel_.model_name);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    if (!ready) {
        error = grpcClient_->LoadModel(imageModelTextChannel_.model_name);
        if (!error.IsOk()) {
            std::cerr
                << "[EmbeddingClient]:Triton error: unable to LoadModel model."
                << imageModelTextChannel_.model_name
                << "error message:" << error.Message() << std::endl;
            return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                         error.Message());
        }
    }
    std::vector<std::string> input0_data(1);
    input0_data[0] = text;

    std::vector<int64_t> shape{1};

    // Initialize the inputs with the data.
    tc::InferInput* input0;
    error = tc::InferInput::Create(&input0, "product_reviews", shape, "BYTES");
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error: unable to get product_reviews,"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferInput> inputPtr;
    inputPtr.reset(input0);
    inputPtr->SetBinaryData(true);
    error = inputPtr->AppendFromString(input0_data);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to set data for "
                     "product_reviews,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    // Generate the outputs to be requested.
    tc::InferRequestedOutput* output0;
    error = tc::InferRequestedOutput::Create(&output0, "unnorm_text_features");
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get "
                     "unnorm_text_features,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }
    std::shared_ptr<tc::InferRequestedOutput> outputPtr;
    outputPtr.reset(output0);
    outputPtr->SetBinaryData(true);

    // The inference settings. Will be using default for now.
    tc::InferOptions options(imageModelTextChannel_.model_name);
    options.model_version_ = "";
    // options.priority_ = 3;

    std::vector<tc::InferInput*> inputs = {inputPtr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {outputPtr.get()};

    tc::InferResult* results;
    error = grpcClient_->Infer(&results, options, inputs, outputs);
    if (!error.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to run model,"
                  << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    std::shared_ptr<tc::InferResult> resultsPtr;
    resultsPtr.reset(results);

    // Validate the results...
    if (!validateShapeAndDatatype("unnorm_text_features", resultsPtr, "FP32",
                                  1)) {
        return EmbeddingClientResult(
            false, TRITON_INFER_ERROR,
            "Trion infer error: embeddingTextWithImageModel "
            "validateShapeAndDatatype failed.");
    }

    float* outputDataPtr;

    size_t outputByteSize = imageModelImageChannel_.channel_output_dim *
                            4;  // 输出 1024 个 float 所以是 1024 * 4 字节

    size_t recvOutputByteSize;
    error = resultsPtr->RawData("unnorm_text_features",
                                (const uint8_t**)&outputDataPtr,
                                &recvOutputByteSize);
    if (!error.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error: unable to get result data for "
               "unnorm_text_features"
            << "error message:" << error.Message() << std::endl;
        return EmbeddingClientResult(false, TRITON_INFER_ERROR,
                                     error.Message());
    }

    if (recvOutputByteSize != outputByteSize) {
        std::cerr
            << "[EmbeddingClient]:Error: received incorrect byte size for "
               "'unnorm_text_features': "
            << recvOutputByteSize << std::endl;
        return EmbeddingClientResult(
            false, TRITON_INFER_ERROR,
            "Trion infer error: embeddingTextWithImageModel received "
            "incorrect byte size");
    }
    std::vector<float> tokens;

    //   std::cout << "unnorm_text_features: ";
    for (size_t i = 0; i < imageModelImageChannel_.channel_output_dim; ++i) {
        // std::cout << outputDataPtr[i] << " ";
        tokens.push_back(outputDataPtr[i]);
    }
    //   std::cout << std::endl;

    normalize(tokens);
    return EmbeddingClientResult{true, tokens};
}

bool EmbeddingClient::validateShapeAndDatatype(
    const std::string& name, std::shared_ptr<tc::InferResult> result,
    const std::string& expectedDataType, const int modelSwitch) {
    std::vector<int64_t> shape;
    tc::Error err;
    err = result->Shape(name, &shape);
    if (!err.IsOk()) {
        std::cerr << "[EmbeddingClient]:Triton error: unable to get shape for"
                  << name << std::endl;
        return false;
    }
    // Validate shape
    if (modelSwitch == 0) {
        if ((shape.size() != 3) ||
            (shape[2] != textModelChannel_.channel_output_dim)) {
            std::cerr
                << "[EmbeddingClient]:Error: received incorrect shapes for "
                << name << std::endl;
            return false;
        }
    } else if (modelSwitch == 1) {
        if (shape !=
            std::vector<int64_t>{imageModelImageChannel_.channel_output_dim}) {
            std::cerr
                << "[EmbeddingClient]:Error: received incorrect shapes for "
                << name << std::endl;
            return false;
        }
    }

    std::string dataType;
    err = result->Datatype(name, &dataType);
    if (!err.IsOk()) {
        std::cerr
            << "[EmbeddingClient]:Triton error: unable to get dataType for"
            << name << std::endl;
        return false;
    }

    // Validate dataType
    if (dataType.compare(expectedDataType) != 0) {
        std::cerr << "[EmbeddingClient]:Error: received incorrect dataType for "
                  << name << ": " << dataType << std::endl;
        return false;
    }
    return true;
}
std::vector<unsigned char> EmbeddingClient::base64Decode(
    const std::string& input) {
    std::vector<unsigned char> output;
    const std::string base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int val = 0, valb = -8;
    for (auto c : input) {
        if (std::isspace(c)) {
            continue;
        }
        if (c == '=') {
            break;
        }
        const auto ch = std::find(base64Chars.begin(), base64Chars.end(), c);
        if (ch == base64Chars.end()) {
            std::cerr << "[EmbeddingClient]:parse base64Decode error!"
                      << std::endl;
            return {};
        }

        val = (val << 6) + (ch - base64Chars.begin());
        valb += 6;
        if (valb >= 0) {
            output.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return output;
}
std::vector<float> EmbeddingClient::imageToFloatVector(const cv::Mat& image) {
    std::vector<float> floatVector;

    // 确保图像不为空且为三通道图像
    if (image.empty() || image.channels() != 3) {
        std::cerr << "[EmbeddingClient]:imageToFloatVector error:Image is "
                     "empty or not 3 channels, "
                     "can't convert to vector"
                  << std::endl;
        return floatVector;
    }

    // 将每个通道的像素值添加到向量中
    for (int c = 0; c < image.channels(); ++c) {
        for (int i = 0; i < image.cols; ++i) {
            for (int j = 0; j < image.rows; ++j) {
                float pixelValue = image.at<cv::Vec3f>(i, j)[c];
                floatVector.push_back(pixelValue);
            }
        }
    }

    return floatVector;
}
std::vector<float> EmbeddingClient::preprocessVision(cv::Mat& image) {
    const int width = 224;
    const int height = 224;
    const cv::Scalar mean = cv::Scalar(0.48145466, 0.4578275, 0.40821073);
    const cv::Scalar deviation = cv::Scalar(0.26862954, 0.26130258, 0.27577711);

    cv::resize(image, image, cv::Size(width, height), cv::INTER_CUBIC);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    // Normalize
    image.convertTo(image, CV_32FC3, 1.0 / 255.0);
    cv::subtract(image, mean, image);
    cv::divide(image, deviation, image);

    return imageToFloatVector(image);
}

std::string EmbeddingClient::getConfigValue(const char* key,
                                            const char* engine) {
    std::string model_name = "model_bank";
    tc::InferOptions options(model_name);
    options.model_version_ = "";
    options.priority_ = 1;
    tc::Error err_create = grpcClient_->LoadModel(model_name);
    if (!err_create.IsOk()) {
        std::cout << __LINE__
                  << " err_create.Message(): " << err_create.Message()
                  << std::endl;
        return "";
    }

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
    tc::Error err_infer =
        grpcClient_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        std::cout << __LINE__ << " err_infer.Message(): " << err_infer.Message()
                  << std::endl;
        return "";
    }

    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* output0_data_ptr;
    size_t recv_output0_byte_size;

    std::vector<std::string> text_output_data;
    tc::Error err_paser = results_ptr->StringData("value", &text_output_data);
    if (!err_paser.IsOk()) {
        std::cout << __LINE__ << " err_paser.Message(): " << err_infer.Message()
                  << std::endl;
        return "";
    }
    return text_output_data[0];
}

bool EmbeddingClient::parseJsonToModelInfo(const std::string& jsonStr) {
    try {
        auto j = nlohmann::json::parse(jsonStr);

        if (!j.contains("models") || !j["models"].is_array()) {
            std::cerr << "Invalid JSON: 'models' array not found.\n";
            return false;
        }

        for (const auto& model : j["models"]) {
            if (!model.contains("othersInfo") ||
                !model["othersInfo"].contains("channel_info")) {
                std::cerr << "Invalid JSON: 'channel_info' not found.\n";
                continue;
            }
            for (const auto& channel_info :
                 model["othersInfo"]["channel_info"]) {
                InferChannelInfo info;
                info.display_name = model["othersInfo"]["display_name"];
                info.manage_name = model["othersInfo"]["manage_name"];
                info.model_capability = model["othersInfo"]["embd_type"];
                info.channel_capability = channel_info["channel_type"];
                info.model_name = model["model"];
                info.model_priority = channel_info["priority"];
                info.channel_input_name = channel_info["input_name"];
                info.channel_output_name = channel_info["output_name"];
                info.channel_output_dim = channel_info["output_size"];

                inferChannelInfoList_.push_back(info);
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << '\n';
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return false;
    }
    return true;
}

std::string EmbeddingClient::buildChannelCapabilityJson() {
    // 最终结果：map<model_capability, vector<{name, models}>>
    std::unordered_map<std::string, std::vector<json>> capabilityMap;

    for (const auto& info : inferChannelInfoList_) {
        const std::string& cap = info.model_capability;
        const std::string& name = info.manage_name;
        const std::string& model = info.model_name;

        // 查找当前 capability 下是否已有相同 name 的条目
        auto& groupList = capabilityMap[cap];
        bool found = false;
        for (auto& group : groupList) {
            if (group["name"] == name) {
                group["models"].push_back(model);
                found = true;
                break;
            }
        }

        if (!found) {
            json newGroup;
            newGroup["name"] = name;
            newGroup["models"] = json::array({model});
            groupList.push_back(std::move(newGroup));
        }
    }

    // 转换为最终 json 对象
    json result;
    for (const auto& [cap, groups] : capabilityMap) {
        result[cap] = groups;
    }

    std::cout << "model_catalog result: " << result << std::endl;

    return result.dump();
}

bool EmbeddingClient::updateModelInfo() {
    std::string embd_model_list = getConfigValue("model_info_list", "embd");
    std::string ensemble_embd_model_list =
        getConfigValue("model_info_list", "ensemble-embd");
    std::cout << "embd_model_list: " << embd_model_list << std::endl;
    std::cout << "ensemble_embd_model_list: " << ensemble_embd_model_list
              << std::endl;

    nlohmann::json embd_model_list_json =
        nlohmann::json::parse(embd_model_list);
    nlohmann::json ensemble_embd_model_list_json =
        nlohmann::json::parse(ensemble_embd_model_list);
    inferChannelInfoList_.clear();
    bool succ = parseJsonToModelInfo(embd_model_list);
    if (!succ) {
        return false;
    }
    succ = parseJsonToModelInfo(ensemble_embd_model_list);
    if (!succ) {
        return false;
    }

    for (auto& info : inferChannelInfoList_) {
        std::cout << "-----------------------------" << std::endl;
        std::cout << "display_name: " << info.display_name << std::endl;
        std::cout << "manage_name: " << info.manage_name << std::endl;
        std::cout << "model_capability: " << info.model_capability << std::endl;
        std::cout << "channel_capability: " << info.channel_capability
                  << std::endl;
        std::cout << "model_name: " << info.model_name << std::endl;
        std::cout << "model_priority: " << info.model_priority << std::endl;
        std::cout << "channel_input_name: " << info.channel_input_name
                  << std::endl;
        std::cout << "channel_output_name: " << info.channel_output_name
                  << std::endl;
        std::cout << "channel_output_dim: " << info.channel_output_dim
                  << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }
    return true;
}

result::AiEngineBaseResult EmbeddingClient::embeddingText(
    const std::string& text) {
    embeddingTextRunning_.store(true, std::memory_order_release);
    if (!callback_) {
        std::cerr << "[Embedding server]:Callback is nullptr!" << std::endl;
        embeddingTextRunning_.store(false, std::memory_order_release);
        return result::AiEngineBaseResult{
            false, ai_engine::EngineError(ADD_EMBEDDING_TASK_ERROR,
                                          "Callback is nullptr!")};
    }

    std::cout << "[Embedding server]:Embedding text start" << std::endl;
    EmbeddingClientResult result = embeddingTextInfer(text);
    ai_engine::EngineError error(result.error.code, result.error.message);

    if (embeddingTextRunning_.load(std::memory_order_acquire) == true) {
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{result.success, error,
                                      "ondevice embedding engine", result.res});
        }
    }
    embeddingTextRunning_.store(false, std::memory_order_release);
    return result::AiEngineBaseResult{result.success, error};
}

result::AiEngineBaseResult EmbeddingClient::embeddingImageFromImageFile(
    const std::string& filePath) {
    embeddingImageRunning_.store(true, std::memory_order_release);
    if (!callback_) {
        std::cerr << "[Embedding server]:Callback is nullptr!" << std::endl;
        embeddingImageRunning_.store(false, std::memory_order_release);
        return result::AiEngineBaseResult{
            false, ai_engine::EngineError(ADD_EMBEDDING_TASK_ERROR,
                                          "Callback is nullptr!")};
    }
    std::cout << "[Embedding server]:Embedding image from imageFile start"
              << std::endl;
    EmbeddingClientResult result = embeddingImageInfer(filePath, 0);
    ai_engine::EngineError error(result.error.code, result.error.message);

    if (embeddingImageRunning_.load(std::memory_order_acquire) == true) {
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{result.success, error,
                                      "ondevice embedding engine", result.res});
        }
    }
    embeddingImageRunning_.store(false, std::memory_order_release);
    return result::AiEngineBaseResult{result.success, error};
}
result::AiEngineBaseResult EmbeddingClient::embeddingImageFromBase64ImageData(
    const std::string& fileData) {
    embeddingImageRunning_.store(true, std::memory_order_release);
    if (!callback_) {
        std::cerr << "[Embedding server]:Callback is nullptr!" << std::endl;
        embeddingImageRunning_.store(false, std::memory_order_release);
        return result::AiEngineBaseResult{
            false, ai_engine::EngineError(ADD_EMBEDDING_TASK_ERROR,
                                          "Callback is nullptr!")};
    }
    std::cout << "[Embedding server]:Embedding image from "
                 "base6ImageData start"
              << std::endl;
    EmbeddingClientResult result = embeddingImageInfer(fileData, 1);
    ai_engine::EngineError error(result.error.code, result.error.message);

    if (embeddingImageRunning_.load(std::memory_order_acquire) == true) {
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{result.success, error,
                                      "ondevice embedding engine", result.res});
        }
    }
    embeddingImageRunning_.store(false, std::memory_order_release);
    return result::AiEngineBaseResult{result.success, error};
}
result::AiEngineBaseResult EmbeddingClient::embeddingTextWithImageModel(
    const std::string& text) {
    embeddingImageRunning_.store(true, std::memory_order_release);
    if (!callback_) {
        std::cerr << "[Embedding server]:Callback is nullptr!" << std::endl;
        embeddingImageRunning_.store(false, std::memory_order_release);
        return result::AiEngineBaseResult{
            false, ai_engine::EngineError(ADD_EMBEDDING_TASK_ERROR,
                                          "Callback is nullptr!")};
    }
    std::cout << "[Embedding server]:Embedding text with imageModel start"
              << std::endl;
    EmbeddingClientResult result = embeddingTextInferWithImageModel(text);
    ai_engine::EngineError error(result.error.code, result.error.message);

    if (embeddingImageRunning_.load(std::memory_order_acquire) == true) {
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{result.success, error,
                                      "ondevice embedding engine", result.res});
        }
    }
    embeddingImageRunning_.store(false, std::memory_order_release);
    return result::AiEngineBaseResult{result.success, error};
}

void EmbeddingClient::stopImageEmbedding() {
    if (embeddingImageRunning_.load(std::memory_order_acquire)) {
        embeddingImageRunning_.store(false, std::memory_order_release);
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{true,
                                      ai_engine::EngineError(),
                                      "ondevice embedding engine",
                                      {}});
        }
    }
}
void EmbeddingClient::stopTextEmbedding() {
    if (embeddingTextRunning_.load(std::memory_order_acquire)) {
        embeddingTextRunning_.store(false, std::memory_order_release);
        std::lock_guard<std::mutex> callBackLock(callBackMutex_);
        if (callback_ != nullptr) {
            callback_(EmbeddingResult{true,
                                      ai_engine::EngineError(),
                                      "ondevice embedding engine",
                                      {}});
        }
    }
}

}  // namespace ai_engine::core_ai::embedding
