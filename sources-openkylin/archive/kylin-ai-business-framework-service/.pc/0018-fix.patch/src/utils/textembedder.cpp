/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "textembedder.h"
#include "utils/utils.h"

#include <iostream>
#include <fstream>

static const char* MODEL_FOLDER = "/usr/share/kylin-datamanagement-models/bge-m3-onnx";
static const char* MODEL_PATH = "/usr/share/kylin-datamanagement-models/bge-m3-onnx/model.onnx";

TextEmbedder::TextEmbedder()
    : tokenizer_(MODEL_FOLDER)
    , session_(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Default"),
               MODEL_PATH, Ort::SessionOptions(nullptr))
{}

bool TextEmbedder::isModelInstalled()
{
    std::ifstream modelFile(MODEL_PATH);
    if (!modelFile.good()) {
        return false;
    }
    return true;
}

bool TextEmbedder::isModelSupport()
{
    return true;
}

int TextEmbedder::longestTokensSize(const std::vector<Token> &tokens) const
{
    int longestLength = 0;

    for (const auto& mapItem : tokens) {
        for (const auto& pair : mapItem) {
            longestLength = std::max<int>(longestLength, pair.second.size());
        }
    }

    return longestLength;
}

std::vector<Ort::Value> TextEmbedder::createInputTensors(Token &inputTokens, int batch, const std::vector<const char*>& inputNames) const
{
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
    std::vector<Ort::Value> inputTensors;

    for (int i = 0; i < inputNames.size(); ++i) {
        // tokens 的顺序需要和 names 一一对应
        const std::string& inputName = inputNames[i];
        std::vector<int64_t>& inputValue = inputTokens[inputName];
        const int length = inputValue.size() / batch;
        const std::vector<int64_t> shape = { batch, length };

        Ort::Value inputTensor = Ort::Value::CreateTensor<int64_t>(
            memoryInfo, inputValue.data(), inputValue.size(), shape.data(), shape.size());
        inputTensors.emplace_back(std::move(inputTensor));
    }

    return inputTensors;
}

Ort::Value TextEmbedder::createOutputTensor(std::vector<float>& outputs) const
{
    const int batch = outputs.size() / 1024;
    const std::vector<int64_t> outputShape = { batch, 1024 };
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);

    Ort::Value outputTensor = Ort::Value::CreateTensor<float_t>(
        memoryInfo, outputs.data(), outputs.size(), outputShape.data(), outputShape.size());

    return outputTensor;
}

void TextEmbedder::resizeTokens(std::vector<Token> &tokens, size_t maxLength) const
{
    for (auto& token : tokens) {
        for (auto& pair : token) {
            std::vector<int64_t>& values = pair.second;
            if (values.size() < maxLength) {
                values.resize(maxLength, 0);
            }
        }
    }
}

Token TextEmbedder::mergeTokens(const std::vector<Token> &tokens) const
{
    Token result;

    for (const auto& token : tokens) {
        for (const auto& pair : token) {
            const std::string& key = pair.first;
            const std::vector<int64_t>& value = pair.second;

            std::vector<int64_t>& resultValue = result[key];
            resultValue.reserve(resultValue.size() + value.size());
            resultValue.insert(resultValue.end(), value.begin(), value.end());
        }
    }

    return result;
}

Token TextEmbedder::tokenize(const std::vector<std::string> &texts)
{
    std::vector<Token> tokenList;
    for (const std::string& text : texts) {
        auto tokenize = tokenizer_.tokenize(text);
        if (!tokenize.empty()) {
            tokenList.emplace_back(std::move(tokenize));
        }
    }

    // 在进行批处理时首先要将 tokens 对齐，让所有 tokens 长度一致，
    // 对于不够长的可以补 0 并在 attention_mask 进行标记
    // 然后需要将所有token合并成一个 token, 如果只合并了 token 没有对齐会导致推理结果错误
    int maxLength = longestTokensSize(tokenList);
    std::cout << "Tokens max length is: " << maxLength
              << " tokens batch: " << tokenList.size() << std::endl;

    resizeTokens(tokenList, maxLength);
    return mergeTokens(tokenList);
}

std::vector<std::vector<float>> TextEmbedder::batchEmbeddingTexts(const std::vector<std::string>& texts)
{
    if (texts.empty()) {
        return std::vector<std::vector<float>>();
    }

    Token tokens = tokenize(texts);
    const int batch = static_cast<int>(texts.size());

    std::vector<float> output = inference(tokens, batch);
    if (output.empty() || output.size() / batch != 1024) {
        return std::vector<std::vector<float>>();
    }

    std::vector<std::vector<float>> outputs;
    for (int i = 0; i < batch; ++i) {
        std::vector<float> temp(output.begin() + i * 1024, output.begin() + (i + 1) * 1024);
        outputs.push_back(temp);
    }

    return outputs;
}

std::vector<float> TextEmbedder::embeddingText(const std::string &text)
{
    if (text.empty()) {
        return std::vector<float>();
    }

    Token token = tokenizer_.tokenize(text);
    const int batch = 1;

    return inference(token, batch);
}

std::vector<std::vector<float>> TextEmbedder::embeddingTexts(const std::vector<std::string> &texts)
{
    std::vector<std::vector<float>> outputs;
    if (texts.empty()) {
        return outputs;
    }

    for (const auto& text : texts) {
        std::vector<float> output = embeddingText(text);
        if (output.empty()) {
            continue;
        }

        outputs.emplace_back(std::move(output));
    }

    return outputs;
}

std::vector<float> TextEmbedder::inference(Token& token, int batch)
{
    if (token.empty() || batch < 1) {
        std::cerr << "Tokens is empty or invalid batch, cancel Bgem3 inference" << std::endl;
        return std::vector<float>();
    }

    const std::vector<const char*> inputNames = {"input_ids", "attention_mask"};
    const std::vector<const char*> outputNames = {"dense_vecs"};
    std::vector<float> output(batch * 1024);

    std::vector<Ort::Value> inputTensors = createInputTensors(token, batch, inputNames);

    Ort::Value outputTensor = createOutputTensor(output);

    Ort::RunOptions runOpts(nullptr);
    session_.Run(runOpts, inputNames.data(), inputTensors.data(), inputTensors.size(),
                 outputNames.data(), &outputTensor, 1);
    math::normalize(output);


    return output;
}
