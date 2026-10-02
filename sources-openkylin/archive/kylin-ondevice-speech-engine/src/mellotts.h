#pragma once

#include <string>
#include <vector>

#include "tritonmodelclient.h"

namespace ai_engine::core_ai::speech {

class MelloTTSClient : public TritonModelClient {
public:
    using SynthesisResultCallback =
        std::function<void(const std::string &, const uint64_t, const std::vector<uint64_t> &)>;
    static std::unique_ptr<MelloTTSClient> create();
    virtual ~MelloTTSClient() override = default;
    MelloTTSClient(const MelloTTSClient &) = delete;
    MelloTTSClient &operator=(const MelloTTSClient &) = delete;
    MelloTTSClient(MelloTTSClient &&) = default;
    MelloTTSClient &operator=(MelloTTSClient &&) = default;

    void setSpeaker(int32_t speaker);
    std::string textToSpeech(const std::string &text);

    bool textToSpeechAsync(const std::string &text);
    void setSynthesisResultCallback(SynthesisResultCallback callback);

private:
    MelloTTSClient() = default;
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() override;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() override;
    virtual InferOptions createOptions() override;

    void asyncInferenceCallback(InferResult *inferResult);

    std::string parseInferResult(const std::shared_ptr<InferResult> &inferResult);

private:
    int32_t speaker_ = 0;
    std::string textToSynthesize_;
    // 添加回调函数成员变量
    SynthesisResultCallback synthesisResultCallback_;
    // 用于跟踪sequence_id，改为使用数组存储所有的sequence_id
    std::vector<uint64_t> sequenceIds_;
};

}  // namespace ai_engine::core_ai::speech
