#pragma once

#include <memory>
#include <queue>
#include <string>

#include "tritonmodelclient.h"

namespace ai_engine::core_ai::speech {

class ASRNonStreamingClient : public TritonModelClient {
public:
    using RecognitionResultCallback = std::function<void(const std::string&, size_t)>;
    static std::unique_ptr<ASRNonStreamingClient> create();
    virtual ~ASRNonStreamingClient() override = default;
    ASRNonStreamingClient(const ASRNonStreamingClient &) = delete;
    ASRNonStreamingClient &operator=(const ASRNonStreamingClient &) = delete;
    ASRNonStreamingClient(ASRNonStreamingClient &&) = default;
    ASRNonStreamingClient &operator=(ASRNonStreamingClient &&) = default;

    void setRecognitionResultCallback(RecognitionResultCallback callback);
    std::string automaticSpeechRecognition(const std::vector<uint8_t> &audio);
    bool automaticSpeechRecognitionAsync(const std::vector<uint8_t>& audio, size_t index);

private:
    ASRNonStreamingClient() = default;
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() override;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() override;
    virtual InferOptions createOptions() override;

    void asyncInferenceCallback(InferResult *inferResult);

    std::vector<std::string> parseInferResult(const std::shared_ptr<InferResult> &inferResult);

    std::string joinResults(const std::vector<std::string>& results);

private:
    std::vector<float> audioDataToInfer_;

    RecognitionResultCallback recognitionResultCallback_;

    std::mutex asyncInferMutex_;
    std::queue<size_t> asyncInferIndexQueue_;
};

}  // namespace ai_engine::core_ai::speech
