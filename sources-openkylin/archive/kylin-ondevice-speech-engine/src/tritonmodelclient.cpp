#include "tritonmodelclient.h"

#include <iostream>

namespace ai_engine::core_ai::speech {

bool TritonModelClient::init(const std::string &modelName) {
    TritonError error;

    bool verbose = false;
    error = GrpcClient::Create(&client_, grpcServerUrl_, verbose);
    if (error.IsOk() == false) {
        std::cerr << __func__ << ": " << error.Message() << std::endl;
        return false;
    }

    (void)modelName;

    return true;
}

void TritonModelClient::setInferenceModel(const std::string& modelName) {
    std::cout << "Inference model set to: " << modelName << std::endl;
    inferenceModel_ = modelName;
}

std::shared_ptr<TritonModelClient::InferResult> TritonModelClient::inference() {
    std::vector<InferInput *> inputs;
    auto inputPtr = createInputs();
    for (auto &input : inputPtr) {
        inputs.push_back(input.get());
    }

    std::vector<const InferRequestedOutput *> outputs;
    auto outputPtr = createOutputs();
    for (auto &output : outputPtr) {
        outputs.push_back(output.get());
    }

    InferOptions options = createOptions();

    if (checkAndLoadModel() == false) {
        return nullptr;
    }

    InferResult *inferResult = nullptr;
    TritonError inferError = client_->Infer(&inferResult, options, inputs, outputs);
    if (inferError.IsOk() == false) {
        std::cerr << "Failed to Infer: " << inferError.Message() << std::endl;
        return nullptr;
    }

    return std::shared_ptr<InferResult>(inferResult);
}

bool TritonModelClient::asyncInference() {
    std::vector<InferInput *> inputs;
    auto inputPtr = createInputs();
    for (auto &input : inputPtr) {
        inputs.push_back(input.get());
    }

    std::vector<const InferRequestedOutput *> outputs;
    auto outputPtr = createOutputs();
    for (auto &output : outputPtr) {
        outputs.push_back(output.get());
    }

    InferOptions options = createOptions();
    options.request_id_ = std::to_string(options.sequence_id_);

    if (checkAndLoadModel() == false) {
        return false;
    }

    TritonError inferError = client_->AsyncInfer(asyncResultCallback_, options, inputs, outputs);

    if (inferError.IsOk() == false) {
        std::cerr << "Failed to Infer: " << inferError.Message() << std::endl;
        return false;
    }

    return true;
}

void TritonModelClient::setAsyncInferOnCompleteCallback(AsyncInferOnCompleteCallback callback) {
    asyncResultCallback_ = callback;
}

std::vector<float> TritonModelClient::bytesToFloatArray(const std::vector<uint8_t> &audioData) {
    const size_t num_samples = audioData.size() / 2;
    if (num_samples == 0) {
        return std::vector<float>();
    }

    std::vector<float> output(num_samples);

    std::vector<int16_t> temp(num_samples);
    std::memcpy(temp.data(), audioData.data(), num_samples * 2);

    std::transform(temp.begin(), temp.end(), output.begin(),
                   [](int16_t sample) { return static_cast<float>(sample) / 32768.0f; });

    return output;
}

bool TritonModelClient::checkAndLoadModel() {
    bool ready = false;
    auto modelReadyError = client_->IsModelReady(&ready, inferenceModel_);
    if (modelReadyError.IsOk() == false) {
        std::cerr << "Failed to check model ready: " << modelReadyError.Message() << std::endl;
        // 检查模型是否 ready 的请求失败，不代表模型未加载，所以这里不返回。
    }

    if (ready == false) {
        std::cerr << "Inference model isn't ready: " << inferenceModel_ << std::endl;
        auto error = client_->LoadModel(inferenceModel_);
        if (error.IsOk()) {
            std::cout << "Model loaded: " << inferenceModel_ << std::endl;
            return true;
        } else {
            std::cerr << "Failed to load model: " << error.Message() << std::endl;
            return false;
        }
    }
    return true;
}

}  // namespace ai_engine::core_ai::speech