#include "asrnonstreaming.h"

#include "utils.h"

namespace ai_engine::core_ai::speech {

std::unique_ptr<ASRNonStreamingClient> ASRNonStreamingClient::create() {
    auto client = std::unique_ptr<ASRNonStreamingClient>(new ASRNonStreamingClient());
    if (client->init()) {
        return client;
    }
    return nullptr;
}

void ASRNonStreamingClient::setRecognitionResultCallback(RecognitionResultCallback callback) {
    recognitionResultCallback_ = callback;
    setAsyncInferOnCompleteCallback([this](InferResult *result) { asyncInferenceCallback(result); });
}

std::string ASRNonStreamingClient::automaticSpeechRecognition(const std::vector<uint8_t> &audio) {
    if (audio.empty()) {
        return {};
    }

    audioDataToInfer_ = bytesToFloatArray(audio);

    auto inferResult = inference();

    std::vector<std::string> results = parseInferResult(inferResult);

    return joinResults(results);
}

bool ASRNonStreamingClient::automaticSpeechRecognitionAsync(const std::vector<uint8_t>& audio, size_t index) {
    if (audio.empty()) {
        return true;
    }

    audioDataToInfer_ = bytesToFloatArray(audio);

    bool ret = asyncInference();
    {
        std::lock_guard<std::mutex> lock(asyncInferMutex_);
        asyncInferIndexQueue_.push(index);
    }

    return ret;
}

std::vector<std::shared_ptr<ASRNonStreamingClient::InferInput>> ASRNonStreamingClient::createInputs() {
    std::vector<int64_t> input0Shape = {1, static_cast<int64_t>(audioDataToInfer_.size())};
    InferInput *input0;
    auto error = InferInput::Create(&input0, "WAV", input0Shape, "FP32");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create input: " << error.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferInput> input0Ptr(input0);
    error = input0Ptr->AppendRaw(reinterpret_cast<uint8_t *>(audioDataToInfer_.data()),
                                 audioDataToInfer_.size() * sizeof(float));
    if (error.IsOk() == false) {
        std::cerr << "Failed to set input data: " << error.Message() << std::endl;
        return {};
    }

    std::vector<int32_t> input1Data{static_cast<int32_t>(audioDataToInfer_.size())};
    std::vector<int64_t> input1Shape{1, 1};
    InferInput *input1;
    error = triton::client::InferInput::Create(&input1, "WAV_LENS", input1Shape, "INT32");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create input: " << error.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferInput> input1Ptr(input1);
    // TODO YDS AppendRaw needs input data to be there until Infer is called. Plus WAV_LENS is useless.
    input1Ptr->AppendRaw(reinterpret_cast<uint8_t *>(input1Data.data()), input1Data.size() * sizeof(int32_t));

    return {input0Ptr, input1Ptr};
}

std::vector<std::shared_ptr<ASRNonStreamingClient::InferRequestedOutput>> ASRNonStreamingClient::createOutputs() {
    return {};
}

ASRNonStreamingClient::InferOptions ASRNonStreamingClient::createOptions() {
    InferOptions options(inferenceModel_);
    options.model_version_ = "";
    options.client_timeout_ = 0;
    options.sequence_id_ = Utils::generateRandomNumber();
    return options;
}

void ASRNonStreamingClient::asyncInferenceCallback(InferResult *inferResult) {
    std::shared_ptr<InferResult> inferResultPtr = std::shared_ptr<InferResult>(inferResult);
    auto words = parseInferResult(inferResultPtr);

    std::string sentence = joinResults(words);

    size_t index = 0;
    {
        std::lock_guard<std::mutex> lock(asyncInferMutex_);
        if (!asyncInferIndexQueue_.empty()) {
            index = asyncInferIndexQueue_.front();
            asyncInferIndexQueue_.pop();
        }
    }
    recognitionResultCallback_(sentence, index);
}

std::vector<std::string> ASRNonStreamingClient::parseInferResult(const std::shared_ptr<InferResult> &inferResult) {
    if (inferResult == nullptr) {
        return {};
    }

    std::vector<std::string> results;

    TritonError error = inferResult->StringData("TRANSCRIPTS", &results);
    if (error.IsOk() == false) {
        std::cerr << "unable to get nonstreaming asr result: " << error.Message() << std::endl;
        return {};
    }

    return results;
}

std::string ASRNonStreamingClient::joinResults(const std::vector<std::string>& results) {
    std::string sentence;
    for (const auto& result : results) {
        sentence += result;
    }

    // 去掉 sentence 中两个连续的@符号，词表中的英文词会以两个连续的@符号结尾
    sentence = Utils::removeDoubleAt(sentence);
    return sentence;
}

}  // namespace ai_engine::core_ai::speech
