#pragma once

#include <triton/client/grpc_client.h>

#include <memory>
#include <string>
#include <vector>

namespace triton::client {
class Error;
class InferInput;
class InferResult;
class InferOptions;
class InferRequestedOutput;
class InferenceServerGrpcClient;
}  // namespace triton::client

namespace ai_engine::core_ai::speech {

class TritonModelClient {
public:
    using TritonError = triton::client::Error;
    using InferInput = triton::client::InferInput;
    using InferResult = triton::client::InferResult;
    using InferOptions = triton::client::InferOptions;
    using GrpcClient = triton::client::InferenceServerGrpcClient;
    using InferRequestedOutput = triton::client::InferRequestedOutput;

    using AsyncInferOnCompleteCallback = std::function<void(InferResult *)>;

    virtual ~TritonModelClient() = default;

    void setInferenceModel(const std::string& modelName);
    std::shared_ptr<InferResult> inference();

    bool asyncInference();
    void setAsyncInferOnCompleteCallback(AsyncInferOnCompleteCallback callback);

protected:
    TritonModelClient() = default;
    // TODO protected?
    bool init(const std::string& modelName = "");
    virtual std::vector<std::shared_ptr<InferInput>> createInputs() = 0;
    virtual std::vector<std::shared_ptr<InferRequestedOutput>> createOutputs() = 0;
    virtual InferOptions createOptions() = 0;

    std::vector<float> bytesToFloatArray(const std::vector<uint8_t> &audioData);

private:
    bool checkAndLoadModel();

protected:
    std::string inferenceModel_;

private:
    const std::string grpcServerUrl_ = "localhost:8001";
    std::unique_ptr<GrpcClient> client_ = nullptr;

    AsyncInferOnCompleteCallback asyncResultCallback_;
};

}  // namespace ai_engine::core_ai::speech
