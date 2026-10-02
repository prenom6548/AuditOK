#ifndef TEXTEMBEDDER_H
#define TEXTEMBEDDER_H

#include "autotokenizer.h"

#include <onnxruntime/onnxruntime_cxx_api.h>

#include <vector>
#include <string>

class TextEmbedder {
public:
    TextEmbedder();
    std::vector<std::vector<float>> batchEmbeddingTexts(const std::vector<std::string>& texts);
    std::vector<float> embeddingText(const std::string& text);
    std::vector<std::vector<float>> embeddingTexts(const std::vector<std::string>& texts);

private:
    AutoTokenizer tokenizer_;
    Ort::Session session_;

    std::vector<float> inference(Token &token, int batch);
    Token tokenize(const std::vector<std::string>& texts);
    int longestTokensSize(const std::vector<Token>& tokens) const;
    void resizeTokens(std::vector<Token>& tokens, size_t maxLength) const;
    Token mergeTokens(const std::vector<Token>& tokens) const;

    std::vector<Ort::Value> createInputTensors(Token& inputTokens, int batch, const std::vector<const char *>& inputNames) const;
    Ort::Value createOutputTensor(std::vector<float>& outputs) const;
};

#endif // TEXTEMBEDDER_H
