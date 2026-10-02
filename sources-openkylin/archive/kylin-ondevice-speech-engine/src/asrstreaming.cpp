#include "asrstreaming.h"

#include "utils.h"

namespace ai_engine::core_ai::speech {

std::unique_ptr<ASRStreamingClient> ASRStreamingClient::create() {
    auto client = std::unique_ptr<ASRStreamingClient>(new ASRStreamingClient());
    if (client->init()) {
        return client;
    }
    return nullptr;
}

std::string ASRStreamingClient::automaticSpeechRecognition(const std::vector<uint8_t> &audio) {
    // Notice: **不要返回**。当语音识别结束时，引擎会传入空语音段，此时我们需要继续往下执行以清空 audioDataCached_。
    // if (audio.empty()) {
    //     return {};
    // }

    // TODO YDS hard coded
    int chunkSamples = 10240;

    std::vector<float> audioData = bytesToFloatArray(audio);

    if (isStreamingAsrEnding_ == true) {
        audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.end());
        audioDataToInfer_ = std::move(audioDataCached_);
        auto result = inference();

        return parseInferResult(result);
    }

    if (audioDataCached_.size() + audioData.size() < chunkSamples) {
        audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.end());
        return {};
    }

    int fillupLength = chunkSamples - audioDataCached_.size();
    audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.begin() + fillupLength);

    audioDataToInfer_ = std::move(audioDataCached_);
    auto result = inference();

    audioDataCached_.clear();
    audioDataCached_.insert(audioDataCached_.begin(), audioData.begin() + fillupLength, audioData.end());

    return parseInferResult(result);
}

bool ASRStreamingClient::automaticSpeechRecognitionAsync(const std::vector<uint8_t>& audio, size_t index) {
    // Notice: **不要返回**。当语音识别结束时，引擎会传入空语音段，此时我们需要继续往下执行以清空 audioDataCached_。
    // if (audio.empty()) {
    //     return true;
    // }

    // TODO YDS hard coded
    int chunkSamples = 10240;

    std::vector<float> audioData = bytesToFloatArray(audio);

    if (isStreamingAsrEnding_ == true) {
        audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.end());
        audioDataToInfer_ = std::move(audioDataCached_);
        int ret = asyncInference();
        {
            std::lock_guard<std::mutex> lock(asyncInferMutex_);
            asyncInferIndexQueue_.push(index);
        }

        return ret;
    }

    if (audioDataCached_.size() + audioData.size() < chunkSamples) {
        audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.end());
        return true;
    }

    int fillupLength = chunkSamples - audioDataCached_.size();
    audioDataCached_.insert(audioDataCached_.end(), audioData.begin(), audioData.begin() + fillupLength);

    audioDataToInfer_ = std::move(audioDataCached_);
    bool ret = asyncInference();
    {
        std::lock_guard<std::mutex> lock(asyncInferMutex_);
        asyncInferIndexQueue_.push(index);
        lastIndex_ = index;
    }

    audioDataCached_.clear();
    audioDataCached_.insert(audioDataCached_.begin(), audioData.begin() + fillupLength, audioData.end());

    return ret;
}

void ASRStreamingClient::setRecognitionResultCallback(RecognitionResultCallback callback) {
    recognitionResultCallback_ = callback;
    setAsyncInferOnCompleteCallback([this](InferResult *result) { asyncInferenceCallback(result); });
}

void ASRStreamingClient::setSequenceStart() {
    isStreamingAsrBegining_ = true;
    streamingAsrSequenceId_ = Utils::generateRandomNumber();
}

void ASRStreamingClient::setSequenceEnd() {
    isStreamingAsrEnding_ = true;
    // 增加这个调用，使调用 setSequenceEnd() 的动作可以立刻触发语音识别推理。
    automaticSpeechRecognitionAsync({}, lastIndex_);
}

void ASRStreamingClient::setSequenceStartToFalseAfterGetOptions() { isStreamingAsrBegining_ = false; }

void ASRStreamingClient::setSequenceEndToFalseAfterGetOptions() { isStreamingAsrEnding_ = false; }

std::vector<std::shared_ptr<ASRStreamingClient::InferInput>> ASRStreamingClient::createInputs() {
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

    static std::vector<int32_t> input1Data;
    input1Data = {static_cast<int32_t>(audioDataToInfer_.size())};
    std::vector<int64_t> input1Shape{1, 1};
    InferInput* input1;
    error = triton::client::InferInput::Create(&input1, "WAV_LENS", input1Shape, "INT32");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create input: " << error.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferInput> input1Ptr(input1);
    input1Ptr->AppendRaw(reinterpret_cast<uint8_t*>(input1Data.data()), input1Data.size() * sizeof(int32_t));

    return {input0Ptr, input1Ptr};
}

std::vector<std::shared_ptr<ASRStreamingClient::InferRequestedOutput>> ASRStreamingClient::createOutputs() {
    InferRequestedOutput *output = nullptr;
    auto error = InferRequestedOutput::Create(&output, "TRANSCRIPTS");
    if (!error.IsOk()) {
        std::cerr << "Failed to create output: " << error.Message() << std::endl;
        return {};
    }

    return {std::shared_ptr<InferRequestedOutput>(output)};
}

ASRStreamingClient::InferOptions ASRStreamingClient::createOptions() {
    triton::client::InferOptions options(inferenceModel_);
    options.model_version_ = "";
    options.client_timeout_ = 0;
    options.sequence_start_ = isStreamingAsrBegining_;
    options.sequence_end_ = isStreamingAsrEnding_;
    options.sequence_id_ = streamingAsrSequenceId_;

    if (isStreamingAsrBegining_ == true) {
        setSequenceStartToFalseAfterGetOptions();
    }
    if (isStreamingAsrEnding_ == true) {
        setSequenceEndToFalseAfterGetOptions();
    }
    return options;
}

void ASRStreamingClient::asyncInferenceCallback(InferResult *inferResult) {
    std::shared_ptr<InferResult> inferResultPtr = std::shared_ptr<InferResult>(inferResult);
    auto sentence = parseInferResult(inferResultPtr);

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

std::string ASRStreamingClient::parseInferResult(const std::shared_ptr<InferResult> &inferResult) {
    if (inferResult == nullptr) {
        return {};
    }

    if (inferResult->RequestStatus().IsOk() == false) {
        std::cerr << "Error status of inference result: " << inferResult->RequestStatus() << std::endl;
        return {};
    }

    std::vector<std::string> results;

    TritonError error = inferResult->StringData("TRANSCRIPTS", &results);
    if (error.IsOk() == false) {
        std::cerr << "unable to get streaming asr result: " << error.Message() << std::endl;
        return {};
    }

    std::string sentence;
    for (auto &result : results) {
        sentence += result;
    }

    return sentence;
}

}  // namespace ai_engine::core_ai::speech