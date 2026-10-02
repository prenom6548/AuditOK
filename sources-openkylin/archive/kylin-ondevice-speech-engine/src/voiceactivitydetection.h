#pragma once

#include <string>

#include "tritonmodelclient.h"

namespace ai_engine::core_ai::speech {

class VoiceActivityDetectionClient : public TritonModelClient {
public:
    using VADResultCallback = std::function<void(std::pair<int, int> result)>;

    static std::unique_ptr<VoiceActivityDetectionClient> create();
    virtual ~VoiceActivityDetectionClient() override;
    VoiceActivityDetectionClient(const VoiceActivityDetectionClient &) = delete;
    VoiceActivityDetectionClient &operator=(const VoiceActivityDetectionClient &) = delete;
    VoiceActivityDetectionClient(VoiceActivityDetectionClient &&) = default;
    VoiceActivityDetectionClient &operator=(VoiceActivityDetectionClient &&) = default;

    void setVADResultCallback(VADResultCallback callback);
    std::pair<int, int> voiceActivityDetect(const std::vector<uint8_t> &audio, bool isStart, bool isEnd);
    bool voiceActivityDetectAsync(const std::vector<uint8_t> &audio, bool isStart, bool isEnd);

private:
    VoiceActivityDetectionClient() = default;
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() override;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() override;
    virtual InferOptions createOptions() override;

    void asyncInferenceCallback(InferResult *inferResult);

    std::pair<int, int> parseInferResult(const std::shared_ptr<InferResult> &inferResult);

private:
    std::vector<float> audioDataToInfer_;
    bool isStart_ = false;
    bool isEnd_ = false;

    std::mutex callbackMutex_;
    VADResultCallback vadResultCallback_;
};

}  // namespace ai_engine::core_ai::speech
