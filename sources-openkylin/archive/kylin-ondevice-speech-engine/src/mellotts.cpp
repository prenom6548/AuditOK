#include "mellotts.h"

#include "utils.h"

#include <cmath>

namespace ai_engine::core_ai::speech {

std::unique_ptr<MelloTTSClient> MelloTTSClient::create() {
    auto client = std::unique_ptr<MelloTTSClient>(new MelloTTSClient());
    if (client->init()) {
        return client;
    }
    return nullptr;
}

void MelloTTSClient::setSpeaker(int32_t speaker) { speaker_ = speaker; }

std::string MelloTTSClient::textToSpeech(const std::string &text) {
    if (text.empty() == true) {
        std::cout << "No text for synthesizing speech." << std::endl;
        return {};
    }

    textToSynthesize_ = text;

    auto inferResult = inference();

    std::string base64AudioData = parseInferResult(inferResult);

    return base64AudioData;
}

bool MelloTTSClient::textToSpeechAsync(const std::string &text) {
    if (text.empty()) {
        return true;
    }

    textToSynthesize_ = text;

    // 发起异步推理请求
    bool ret = asyncInference();

    return ret;
}

void MelloTTSClient::setSynthesisResultCallback(SynthesisResultCallback callback) {
    synthesisResultCallback_ = callback;
    // 设置底层异步推理完成后的回调函数
    setAsyncInferOnCompleteCallback([this](InferResult *result) { asyncInferenceCallback(result); });
}

void MelloTTSClient::asyncInferenceCallback(InferResult *inferResult) {
    std::shared_ptr<InferResult> inferResultPtr = std::shared_ptr<InferResult>(inferResult);
    // 增加获取每条id的代码，并转换成数字
    std::string resultStrId;
    triton::client::Error error = inferResultPtr->Id(&resultStrId);
    uint64_t resultId = 0;
    // 只有在resultStrId非空时才进行转换
    if (!resultStrId.empty()) {
        resultId = std::stoull(resultStrId);
    }

    std::string base64AudioData = parseInferResult(inferResultPtr);

    // 调用上层设置的回调函数传递结果
    if (synthesisResultCallback_) {
        synthesisResultCallback_(base64AudioData, resultId, sequenceIds_);
    }
}

std::vector<std::shared_ptr<MelloTTSClient::InferInput>> MelloTTSClient::createInputs() {
    std::vector<int64_t> textShape{1, 1};
    InferInput *textInput;
    auto textError = InferInput::Create(&textInput, "text", textShape, "BYTES");
    if (textError.IsOk() == false) {
        std::cerr << "Failed to create input text: " << textError.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferInput> textInputPtr(textInput);
    textError = textInputPtr->AppendFromString({textToSynthesize_});
    if (textError.IsOk() == false) {
        std::cerr << "Failed to set input text: " << textError.Message() << std::endl;
        return {};
    }

    std::vector<int64_t> speakerShape{1, 1};
    InferInput *speakerInput;
    auto speakerError = InferInput::Create(&speakerInput, "speaker", speakerShape, "INT32");
    if (speakerError.IsOk() == false) {
        std::cerr << "Failed to create input speaker: " << speakerError.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferInput> speakerInputPtr(speakerInput);
    // Notice: AppendRaw DOESN'T copy input.
    speakerError = speakerInputPtr->AppendRaw(reinterpret_cast<uint8_t *>(&speaker_), sizeof(int32_t));
    if (speakerError.IsOk() == false) {
        std::cerr << "Failed to set speaker: " << speakerError.Message() << std::endl;
        return {};
    }

    return {textInputPtr, speakerInputPtr};
}

std::vector<std::shared_ptr<MelloTTSClient::InferRequestedOutput>> MelloTTSClient::createOutputs() {
    InferRequestedOutput *audioOutput;
    auto error = InferRequestedOutput::Create(&audioOutput, "Audio");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create output: " << error.Message() << std::endl;
        return {};
    }
    std::shared_ptr<InferRequestedOutput> audioOutputPtr(audioOutput);

    return {audioOutputPtr};
}

MelloTTSClient::InferOptions MelloTTSClient::createOptions() {
    InferOptions options(inferenceModel_);
    options.sequence_id_ = Utils::generateRandomNumber();
    sequenceIds_.push_back(options.sequence_id_);
    options.sequence_start_ = true;
    options.sequence_end_ = true;  // 一次性请求，既是开始，也是结束
    return options;
}

std::string MelloTTSClient::parseInferResult(const std::shared_ptr<InferResult> &inferResult) {
    if (inferResult == nullptr) {
        std::cerr << "Empty inference results." << std::endl;
        return {};
    }

    const uint8_t *audioResult = nullptr;
    size_t audioResultSize = 0;
    auto error = inferResult->RawData("Audio", &audioResult, &audioResultSize);
    if (error.IsOk() == false) {
        std::cerr << "unable to get text to speech result: " << error.Message() << std::endl;
        return {};
    }

    size_t count = audioResultSize / sizeof(float);
    std::vector<float> audioData(reinterpret_cast<const float *>(audioResult),
                                 reinterpret_cast<const float *>(audioResult) + count);

    std::vector<uint8_t> audio(audioData.size() * sizeof(int16_t));
    for (size_t i = 0; i < audioData.size(); ++i) {
        int16_t sample = static_cast<int16_t>(round(std::clamp(audioData[i], -1.0f, 1.0f) * 32767.0f));
        memcpy(audio.data() + i * sizeof(int16_t), &sample, sizeof(int16_t));
    }

    std::vector<uint8_t> audioNormalized = Utils::normalizePcm16Rms(audio, -20.0f, 60.0f);
    std::vector<uint8_t> audioResampled = Utils::resampleAudio(audioNormalized, 44100, 16000);
    std::string audioStr(reinterpret_cast<const char *>(audioResampled.data()), audioResampled.size());

    std::string result = Utils::base64Encode(audioStr);
    return result;
}

}  // namespace ai_engine::core_ai::speech
