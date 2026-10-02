#include "voiceactivitydetection.h"

#include "utils.h"

namespace ai_engine::core_ai::speech {

std::unique_ptr<VoiceActivityDetectionClient> VoiceActivityDetectionClient::create() {
    auto client = std::unique_ptr<VoiceActivityDetectionClient>(new VoiceActivityDetectionClient());
    if (client->init()) {
        return client;
    }
    return nullptr;
}

VoiceActivityDetectionClient::~VoiceActivityDetectionClient() {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    vadResultCallback_ = nullptr;
}

void VoiceActivityDetectionClient::setVADResultCallback(VADResultCallback callback) {
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        vadResultCallback_ = callback;
    }
    setAsyncInferOnCompleteCallback([this](InferResult *result) { asyncInferenceCallback(result); });
}

std::pair<int, int> VoiceActivityDetectionClient::voiceActivityDetect(const std::vector<uint8_t> &audio, bool isStart,
                                                                      bool isEnd) {
    if (audio.empty()) {
        return {};
    }

    audioDataToInfer_ = bytesToFloatArray(audio);

    isStart_ = isStart;
    isEnd_ = isEnd;

    auto inferResult = inference();

    return parseInferResult(inferResult);
}

bool VoiceActivityDetectionClient::voiceActivityDetectAsync(const std::vector<uint8_t> &audio, bool isStart,
                                                            bool isEnd) {
    if (audio.empty()) {
        return true;
    }

    audioDataToInfer_ = bytesToFloatArray(audio);

    isStart_ = isStart;
    isEnd_ = isEnd;

    bool ret = asyncInference();

    return ret;
}

std::vector<std::shared_ptr<VoiceActivityDetectionClient::InferInput>> VoiceActivityDetectionClient::createInputs() {
    std::vector<int64_t> inputShape = {1, static_cast<int64_t>(audioDataToInfer_.size())};

    InferInput *input;
    auto error = InferInput::Create(&input, "WAV", inputShape, "FP32");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create input: " << error.Message() << std::endl;
        return {};
    }

    std::shared_ptr<InferInput> inputPtr(input);
    error = inputPtr->AppendRaw(reinterpret_cast<uint8_t *>(audioDataToInfer_.data()),
                                audioDataToInfer_.size() * sizeof(float));
    if (error.IsOk() == false) {
        std::cerr << "Failed to set input data: " << error.Message() << std::endl;
        return {};
    }

    return {inputPtr};
}

std::vector<std::shared_ptr<VoiceActivityDetectionClient::InferRequestedOutput>>
VoiceActivityDetectionClient::createOutputs() {
    InferRequestedOutput *output;
    auto error = InferRequestedOutput::Create(&output, "OUTPUT0");
    if (!error.IsOk()) {
        std::cerr << "Failed to create output: " << error.Message() << std::endl;
        return {};
    }

    return {std::shared_ptr<InferRequestedOutput>(output)};
}

VoiceActivityDetectionClient::InferOptions VoiceActivityDetectionClient::createOptions() {
    InferOptions options(inferenceModel_);
    static uint64_t sequenceID;
    if (isStart_ == true) {
        sequenceID = Utils::generateRandomNumber();
    }
    options.sequence_id_ = sequenceID;
    options.sequence_start_ = isStart_;
    options.sequence_end_ = isEnd_;

    return options;
}

void VoiceActivityDetectionClient::asyncInferenceCallback(InferResult *inferResult) {
    std::shared_ptr<InferResult> inferResultPtr = std::shared_ptr<InferResult>(inferResult);
    auto result = parseInferResult(inferResultPtr);

    VADResultCallback localCallback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (vadResultCallback_ == nullptr) {
            return;
        }
        localCallback = vadResultCallback_;
    }
    localCallback(result);
}

std::pair<int, int> VoiceActivityDetectionClient::parseInferResult(const std::shared_ptr<InferResult> &inferResult) {
    if (inferResult == nullptr) {
        return {-1, -1};
    }

    size_t outputSizeInByte = 0;
    const uint8_t *outputDataPtr = nullptr;
    auto error = inferResult->RawData("OUTPUT0", &outputDataPtr, &outputSizeInByte);
    if (!error.IsOk()) {
        std::cerr << "Failed to get vad result: " << error.Message() << std::endl;
        return {-1, -1};
    }
    if (outputDataPtr == nullptr) {
        std::cerr << "Failed to get vad result: outputDataPtr is null" << std::endl;
        return {-1, -1};
    }

    size_t count = outputSizeInByte / sizeof(int);
    std::vector<int> results(count);
    if (count > 0 && outputDataPtr != nullptr) {
        std::memcpy(results.data(), outputDataPtr, outputSizeInByte);
    }

    // TODO YDS size == 1 时，是不是没意义
    if (results.size() < 2) {
        return {-1, -1};
    }

    return {results[0], results[1]};
}

}  // namespace ai_engine::core_ai::speech
