#include "punctuation.h"

namespace ai_engine::core_ai::speech {

std::unique_ptr<PunctuationClient> PunctuationClient::create() {
    auto client = std::unique_ptr<PunctuationClient>(new PunctuationClient());
    if (client->init()) {
        return client;
    }
    return nullptr;
}

std::string PunctuationClient::addPunctuation(const std::string &text) {
    if (text.empty() == true) {
        std::cout << "No text to add punctuation." << std::endl;
        return {};
    }

    textInput_ = text;

    auto inferResult = inference();

    std::string textWithPunc;
    std::vector<std::string> results = parseInferResult(inferResult);
    for (auto &result : results) {
        textWithPunc += result;
    }

    if (textWithPunc.empty() == true) {
        std::cerr << "Failed to add punctuation to the text." << text << std::endl;
        textWithPunc = text;
    }

    return textWithPunc;
}

std::vector<std::shared_ptr<PunctuationClient::InferInput>> PunctuationClient::createInputs() {
    std::vector<int64_t> inputShape{1, 1};
    InferInput *input = nullptr;
    auto error = InferInput::Create(&input, "text_no_punc", inputShape, "BYTES");
    if (error.IsOk() == false) {
        std::cerr << "Failed to create input: " << error.Message() << std::endl;
        return {};
    }

    std::shared_ptr<InferInput> inputPtr(input);
    inputPtr->SetBinaryData(false);
    error = inputPtr->AppendFromString({textInput_});
    if (error.IsOk() == false) {
        std::cerr << "Failed to set input data: " << error.Message() << std::endl;
        return {};
    }

    return {inputPtr};
}

std::vector<std::shared_ptr<PunctuationClient::InferRequestedOutput>> PunctuationClient::createOutputs() {
    InferRequestedOutput *output = nullptr;
    InferRequestedOutput::Create(&output, "OUTPUT0");

    std::shared_ptr<InferRequestedOutput> outputPtr(output);
    outputPtr->SetBinaryData(false);

    return {outputPtr};
}

PunctuationClient::InferOptions PunctuationClient::createOptions() { return InferOptions(inferenceModel_); }

std::vector<std::string> PunctuationClient::parseInferResult(const std::shared_ptr<InferResult> &inferResult) {
    if (inferResult == nullptr) {
        std::cerr << "Empty inference results." << std::endl;
        return {};
    }

    std::vector<std::string> results;

    TritonError error = inferResult->StringData("OUTPUT0", &results);
    if (error.IsOk() == false) {
        std::cerr << "unable to get punctuation result: " << error.Message() << std::endl;
        return {};
    }

    return results;
}

}  // namespace ai_engine::core_ai::speech
