#pragma once

#include <string>

#include "tritonmodelclient.h"

namespace ai_engine::core_ai::speech {

class PunctuationClient : public TritonModelClient {
public:
    static std::unique_ptr<PunctuationClient> create();
    virtual ~PunctuationClient() override = default;
    PunctuationClient(const PunctuationClient &) = delete;
    PunctuationClient &operator=(const PunctuationClient &) = delete;
    PunctuationClient(PunctuationClient &&) = default;
    PunctuationClient &operator=(PunctuationClient &&) = default;

    std::string addPunctuation(const std::string &text);

private:
    PunctuationClient() = default;
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() override;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() override;
    virtual InferOptions createOptions() override;

    std::vector<std::string> parseInferResult(const std::shared_ptr<InferResult> &inferResult);

private:
    std::string textInput_;
};

}  // namespace ai_engine::core_ai::speech
