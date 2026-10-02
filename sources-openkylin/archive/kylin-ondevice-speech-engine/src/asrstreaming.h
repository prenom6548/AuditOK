#pragma once

#include <functional>
#include <memory>
#include <string>

#include "tritonmodelclient.h"

namespace ai_engine::core_ai::speech {

class ASRStreamingClient : public TritonModelClient {
public:
    using RecognitionResultCallback = std::function<void(const std::string&, size_t)>;

    static std::unique_ptr<ASRStreamingClient> create();
    ~ASRStreamingClient() = default;
    ASRStreamingClient(const ASRStreamingClient &) = delete;
    ASRStreamingClient &operator=(const ASRStreamingClient &) = delete;
    ASRStreamingClient(ASRStreamingClient &&) = default;
    ASRStreamingClient &operator=(ASRStreamingClient &&) = default;

    std::string automaticSpeechRecognition(const std::vector<uint8_t> &audio);
    bool automaticSpeechRecognitionAsync(const std::vector<uint8_t>& audio, size_t index);

    void setRecognitionResultCallback(RecognitionResultCallback callback);

    /* 在调用语音识别接口前，应该首先调用该接口作为开始。
     * 例如可以使用 VAD 模型检测语音段的开始，并调用该接口。
     */
    void setSequenceStart();
    /* 语音识别过程中，如果没有需要继续识别的语音段，需要调用该接口作为结束。
     * 例如可以使用 VAD 模型检测语音段的结束，并调用该接口。
     */
    void setSequenceEnd();

private:
    ASRStreamingClient() = default;
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() override;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() override;
    virtual InferOptions createOptions() override;

    void asyncInferenceCallback(InferResult *inferResult);

    std::string parseInferResult(const std::shared_ptr<InferResult> &inferResult);

    void setSequenceStartToFalseAfterGetOptions();
    void setSequenceEndToFalseAfterGetOptions();

private:
    std::vector<float> audioDataToInfer_;
    std::vector<float> audioDataCached_;
    bool isStreamingAsrBegining_ = true;
    bool isStreamingAsrEnding_ = false;
    uint64_t streamingAsrSequenceId_;

    RecognitionResultCallback recognitionResultCallback_;

    std::mutex asyncInferMutex_;
    std::queue<size_t> asyncInferIndexQueue_;

    size_t lastIndex_ = 0;
};

}  // namespace ai_engine::core_ai::speech
