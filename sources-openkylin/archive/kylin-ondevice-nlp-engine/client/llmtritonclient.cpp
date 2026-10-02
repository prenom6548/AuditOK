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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "llmtritonclient.h"

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <nlohmann/json.hpp>

namespace ai_engine::gen_ai::nlp {

// 静态成员变量定义 - 全局单例监控相关
std::string LlmTritonClient::model_repository_path_ =
    "/opt/appdata/kylin-ai/model-repository/";
std::string LlmTritonClient::grpc_server_url_ = "localhost:8001";
std::unique_ptr<tc::InferenceServerGrpcClient> LlmTritonClient::global_client_ =
    nullptr;
std::vector<std::string> LlmTritonClient::monitored_paths_;
std::atomic<bool> LlmTritonClient::monitor_running_{false};
std::thread LlmTritonClient::monitor_thread_;
int LlmTritonClient::epoll_fd_ = -1;
int LlmTritonClient::inotify_fd_ = -1;
int LlmTritonClient::model_dir_watch_fd_ = -1;
std::mutex LlmTritonClient::monitor_mutex_;
std::atomic<int> LlmTritonClient::instance_count_{0};
std::unordered_set<LlmTritonClient *> LlmTritonClient::registered_instances_;
std::mutex LlmTritonClient::instance_registry_mutex_;

#define FAIL_IF_ERR(X, MSG)                                                    \
    {                                                                          \
        tc::Error err = (X);                                                   \
        if (!err.IsOk()) {                                                     \
            std::cerr << "error: " << (MSG) << ": " << err << std::endl;       \
        }                                                                      \
    }

template <typename T>
std::shared_ptr<tc::InferInput>
createInput(const std::string &name, const std::vector<int64_t> &shape,
            const std::string &datatype, const T &data) {
    tc::InferInput *raw_input;
    FAIL_IF_ERR(tc::InferInput::Create(&raw_input, name.c_str(), shape,
                                       datatype.c_str()),
                "unable to create '" + name + "'");

    std::shared_ptr<tc::InferInput> input_ptr(raw_input);
    FAIL_IF_ERR(input_ptr->Reset(), "unable to reset '" + name + "'");

    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        input_ptr->SetBinaryData(true);
        FAIL_IF_ERR(input_ptr->AppendFromString(data),
                    "unable to set data for '" + name + "'");
    } else if constexpr (std::is_same_v<T, std::vector<float>>) {
        FAIL_IF_ERR(
            input_ptr->AppendRaw(reinterpret_cast<const uint8_t *>(data.data()),
                                 sizeof(float) * data.size()),
            "unable to set data for '" + name + "'");
    } else {
        FAIL_IF_ERR(input_ptr->AppendRaw(
                        reinterpret_cast<const uint8_t *>(&data), sizeof(T)),
                    "unable to set data for '" + name + "'");
    }

    LOGI("Created input: {}", input_ptr->Name());
    return input_ptr;
}

TextGenerationResult
LlmTritonClient::createTextGenerationResult(const std::string &result,
                                            const std::string &finish_reason,
                                            const bool &is_end) {
    const std::string start_thinking_str = "\u003cthink\u003e";
    const std::string end_thinking_str = "\u003c/think\u003e";
    if (result == start_thinking_str) {
        isThinking_ = true;
    }

    if (result == end_thinking_str) {
        isThinking_ = false;
    }
    nlohmann::json body;
    body["is_end"] = is_end;
    if (isThinking()) {
        if (result == start_thinking_str || result == end_thinking_str) {
            body["reasoning_content"] = "";
        } else {
            body["reasoning_content"] = result;
        }
        body["result"] = "";
    } else {
        if (result == start_thinking_str || result == end_thinking_str) {
            body["result"] = "";
        } else {
            body["result"] = result;
        }
    }
    body["finish_reason"] = finish_reason;

    TextGenerationResult output;
    output.success = true;
    output.error = ai_engine::EngineError(); // 默认构造无错误
    output.modelName = getModelDisplayName();

    try {
        output.text = body.dump();
    } catch (const nlohmann::json::type_error &e) {
        body["reasoning_content"] = "";
        body["result"] = "";
        body["finish_reason"] = finish_reason;
        body["is_end"] = is_end;
        LOGE("捕获到类型错误（通常是编码问题）: {}", e.what());
    } catch (const std::exception &e) {
        body["reasoning_content"] = "";
        body["result"] = "";
        body["finish_reason"] = finish_reason;
        body["is_end"] = is_end;
        LOGE("其他错误: {}", e.what());
    }

    return output;
}

LlmTritonClient::LlmTritonClient() {
    tc::Headers http_headers;
    uint32_t stream_timeout = 0;
    LOGI("LlmTritonClient");

    FAIL_IF_ERR(tc::InferenceServerGrpcClient::Create(
                    &local_client_, grpc_server_url_, false, false,
                    tc::SslOptions(), tc::KeepAliveOptions(), true),
                "unable to create client");
    FAIL_IF_ERR(tc::InferenceServerGrpcClient::Create(
                    &share_client_, grpc_server_url_, false, false,
                    tc::SslOptions(), tc::KeepAliveOptions(), true),
                "unable to create client");

    client_ = (use_local_model_ ? local_client_.get() : share_client_.get());

    // 注册实例，仅在第一个实例时启动监控线程
    registerInstance();
}

LlmTritonClient::~LlmTritonClient() {
    // 注销实例，仅在最后一个实例销毁时停止监控线程
    LOGI("~LlmTritonClient request_id_:{}", request_id_);
    std::unique_lock<std::mutex> lock(chat_client_mutex_);
    unregisterInstance();
    LOGI("~LlmTritonClient over request_id_:{}", request_id_);
}

void LlmTritonClient::setTextGenerationResultCallback(
    TextGenerationResultCallback callback) {
    chat_result_callback_ = callback;
}

bool LlmTritonClient::setCurrentModel(const std::string &model_name) {
    LOGI("setCurrentModel modelInfo:{}", model_info_);
    model_name_ = model_name;
    return true;
}

result::AiEngineBaseResult LlmTritonClient::init() {
    result::AiEngineBaseResult error;
    error.success = true;
    if (client_connected_ != clientConnectStatus::SUCCESS) {
        connect();
    } else {
        LOGI("connect() client_connected_ is :{}, "
             "don't try again",
             static_cast<int>(client_connected_));
    }
    if (model_name_.empty()) {
        LOGE("Have not set modelName_, please set modelName_ before loadMoel");
        // 模型未设置
        error.success = false;
        error.error = ai_engine::EngineError(
            (int)AiSdkCommonErrorCode::AISDK_MODEL_NOT_FOUND,
            "[本地模型引擎]: 未设置模型名称");
    }

    modelInfoUpdate();

    nlohmann::json mode_info_json = nlohmann::json::parse(model_info_);
    try {
        for (auto &it_json : mode_info_json["models"]) {
            if (it_json["model"] == model_name_) {
                model_display_name_ = it_json["displayName"].get<std::string>();
                if (it_json.contains("othersInfo") &&
                    it_json["othersInfo"].contains("lora") &&
                    it_json["othersInfo"]["lora"].is_array()) {
                    LOGI("it_json[othersInfo][lora]:{}",
                         it_json["othersInfo"]["lora"].dump());
                    lora_list_.clear();
                    for (auto &it_lora_json : it_json["othersInfo"]["lora"]) {
                        LOGI("lora_name: {}\n",
                             it_lora_json.get<std::string>().c_str());
                        lora_list_.push_back(stringToLoraFunction(
                            it_lora_json.get<std::string>()));
                    }
                }
                break;
            }
        }
    } catch (...) {
        // 捕获解析错误
        LOGE("Json parse error at modelInfo_: {}", model_info_);
        model_display_name_ = "";
    }
    LOGI("init, model_display_name_:{}", model_display_name_);
    LOGI("init modelInfo:{}", model_info_);
    return error;
}

std::string LlmTritonClient::getModelInfo() {
    if (client_connected_ != clientConnectStatus::SUCCESS) {
        connect();
    } else {
        LOGI("connect() clientConnected_ is :{}, "
             "don't try again",
             static_cast<int>(client_connected_));
    }
    modelInfoUpdate();
    LOGI("getModelInfo modelInfo:{}", model_info_);
    return model_info_;
}

bool LlmTritonClient::connect() {
    client_connected_ = clientConnectStatus::FAILED;
    int re_try_time = 10;
    bool live = false;
    for (; re_try_time > 0; re_try_time--) {
        tc::Error err_islive = client_->IsServerLive(&live);
        if (err_islive.IsOk() && live) {
            auto value = std::chrono::system_clock::now().time_since_epoch();
            auto seconds =
                std::chrono::duration_cast<std::chrono::microseconds>(value)
                    .count() /
                1000000.0;
            request_id_ = std::to_string(seconds);
            LOGI("request_id_:{}\n", request_id_);
            client_connected_ = clientConnectStatus::SUCCESS;
            break;
        } else {
            LOGE("live:{} err_islive.Message():{}", live, err_islive.Message());
            sleep(1);
            continue;
        }
    }

    return true;
}

void LlmTritonClient::modelInfoUpdate() {
    std::string model_info_list = getConfigValue("model_info_list", "llm");

    nlohmann::json model_info_json = nlohmann::json::parse(model_info_);
    try {
        model_info_json["models"].clear();
        nlohmann::json new_models =
            nlohmann::json::parse(model_info_list)["models"];
        if (use_local_model_) {
            model_info_json["models"].insert(model_info_json["models"].end(),
                                             new_models.begin(),
                                             new_models.end());
        } else {
            nlohmann::json ready_models = nlohmann::json::array();
            for (auto &model_entry : new_models) {
                bool ready = false;
                client_->IsModelReady(&ready, model_entry["model"]);
                if (ready) {
                    ready_models.push_back(model_entry);
                }
            }
            model_info_json["models"].insert(model_info_json["models"].end(),
                                             ready_models.begin(),
                                             ready_models.end());
        }

        for (auto &it_json : model_info_json["models"]) {
            // 获取模型配置
            inference::ModelConfigResponse config;
            tc::Error err_modelconfig =
                client_->ModelConfig(&config, it_json["model"]);

            if (!err_modelconfig.IsOk()) {
                LOGE("unable to get ModelConfig, err_modelconfig.Message(): "
                     "{}",
                     err_modelconfig.Message());
            } else {
                // 查找名为 'n_ctx' 的参数
                int n_ctx = 0;
                auto it = config.config().parameters().find("n_ctx");
                if (it != config.config().parameters().end()) {
                    LOGI("n_ctx parameter value: {}",
                         it->second.string_value());
                    n_ctx = std::stoi(it->second.string_value());
                } else {
                    LOGI("n_ctx parameter not found.");
                    n_ctx = 0;
                }
                it_json["maxTokens"] = n_ctx;

                auto it_n_predic =
                    config.config().parameters().find("n_predict");
                if (it_n_predic != config.config().parameters().end()) {
                    LOGI("n_predict_input parameter value: {}",
                         it_n_predic->second.string_value());

                    n_predict_input_ =
                        std::stoi(it_n_predic->second.string_value());
                } else {
                    LOGI("n_predict_input parameter not found. use default "
                         "value 1024");
                }
            }
        }
        bool is_local_server = true;

        model_info_json["isLocalServer"] = is_local_server;
        for (auto &it_json : model_info_json["models"]) {
            if (it_json["model"] == model_name_) {
                model_display_name_ = it_json["displayName"].get<std::string>();
                break;
            }
        }
        model_info_ = model_info_json.dump(4);

        // 更新监控路径列表
        LlmTritonClient::updateMonitoredPaths();
    } catch (const nlohmann::json::parse_error &e) {
        // 捕获解析错误
        LOGE("Parse error at byte {}: {}", e.byte, e.what());
    } catch (const nlohmann::json::exception &e) {
        // 捕获所有来自nlohmann::json库的异常
        LOGE("JSON exception: {}", e.what());
    } catch (const std::exception &e) {
        // 捕获所有标准C++异常
        LOGE("Standard exception: {}", e.what());
    }
}

void LlmTritonClient::stopChat() {
    std::unique_lock<std::mutex> lock(chat_client_mutex_);
    LOGI("stopChat");
    if (client_ == nullptr || chat_client_ == nullptr) {
        LOGI("client_ is nullptr or chat_client_ is nullptr in stopChat");
        return;
    }
    try {
        if (is_chatting_.load(std::memory_order_acquire)) {
            struct ChatConfig config;
            std::vector<float> lora_scale_input_data(lora_list_.size(), 0.0f);
            setLoraScale(-1, lora_scale_input_data);
            config.request_type_data_ =
                8; // 7仅支持终止文本输出阶段，8支持终止prefill和文本输出阶段
            config.request_id_ = request_id_;
            config.top_k_input_data_ = 0;
            config.top_p_input_data_ = 0.0;
            config.n_predict_input_data_ = 0;
            config.lora_scale_input_data_ = lora_scale_input_data;
            std::vector<tc::InferInput *> inputs;
            chat_stopped_ = true;

            startInfer(config, inputs);

            TextGenerationResult end_result = createTextGenerationResult(
                std::string{"<|stopChat_8167431|>"}, "stop", true);
            if (chat_result_callback_ != nullptr) {
                chat_result_callback_(end_result);
            }
        }

        if (chat_client_ != nullptr) {
            if (chat_local_client_ != nullptr) {
                LOGI("StopStream destroy chat_local_client_");
                chat_local_client_->StopStream();
                chat_local_client_.reset();
                LOGI("StopStream destroy chat_local_client_ success");
            }
            if (chat_share_client_ != nullptr) {
                LOGI("StopStream destroy chat_share_client_");
                chat_share_client_->StopStream();
                chat_share_client_.reset();
                LOGI("StopStream destroy chat_share_client_ success");
            }
            chat_client_ = nullptr;
        }
        is_chatting_.store(false, std::memory_order_release);

        return;
    } catch (const std::exception &e) {
        LOGE("Caught exception: {}", e.what());
    }
}

void LlmTritonClient::setConfig(const std::string &config) {
    try {
        if (config.empty()) {
            LOGE("config is empty");
            return;
        }
        nlohmann::json config_json = nlohmann::json::parse(config);
        if (!config_json.contains("use_local")) {
            LOGE("use_local not found");
            return;
        }
        use_local_model_ = config_json["use_local"].get<bool>();
        if (use_local_model_) {
            client_ = local_client_.get();
            chat_client_ = chat_local_client_.get();
            client_connected_ = clientConnectStatus::NOTATTEMPTED;
        } else {
            client_ = share_client_.get();
            chat_client_ = chat_share_client_.get();
            client_connected_ = clientConnectStatus::NOTATTEMPTED;
        }
    } catch (const nlohmann::json::parse_error &e) {
        // 捕获解析错误
        LOGE("Parse error at byte {}: {}", e.byte, e.what());
    } catch (const nlohmann::json::exception &e) {
        // 捕获所有来自nlohmann::json库的异常
        LOGE("JSON exception: {}", e.what());
    } catch (const std::exception &e) {
        // 捕获所有标准C++异常
        LOGE("Standard exception: {}", e.what());
    }

    return;
}

std::string LlmTritonClient::getConfigValue(const char *key,
                                            const char *engine) {
    std::string model_name = "model_bank";
    tc::Error err_loadmodel =
        client_->LoadModel(model_name, {}, {}, {}, model_load_timeout_us_);
    if (!err_loadmodel.IsOk()) {
        LOGE("err_loadmodel.Message(): {}", err_loadmodel.Message());
        return "";
    }
    tc::InferOptions options(model_name);
    options.model_version_ = "";
    options.priority_ = 1;

    std::vector<std::string> input_key_data{std::string(key)};
    std::vector<std::string> input_engine_data{std::string(engine)};

    tc::InferInput *input0, *input1;
    std::vector<int64_t> shape{1};
    tc::InferInput::Create(&input0, "key", shape, "BYTES");
    tc::InferInput::Create(&input1, "engine", shape, "BYTES");
    std::shared_ptr<tc::InferInput> input0_ptr, input1_ptr;
    input0_ptr.reset(input0);
    input1_ptr.reset(input1);
    input0_ptr->AppendFromString(input_key_data);
    input1_ptr->AppendFromString(input_engine_data);
    std::vector<tc::InferInput *> inputs = {input0_ptr.get(), input1_ptr.get()};

    tc::InferRequestedOutput *output0;
    tc::InferRequestedOutput::Create(&output0, "value");
    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);
    std::vector<const tc::InferRequestedOutput *> outputs = {output0_ptr.get()};

    tc::InferResult *results;
    tc::Headers http_headers;
    {
        std::unique_lock<std::mutex> lock(infer_mutex_);
        tc::Error err_infer =
            client_->Infer(&results, options, inputs, outputs, http_headers);
        if (!err_infer.IsOk()) {
            LOGE("err_infer.Message(): {}", err_infer.Message());
            return "";
        }
    }

    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    std::vector<std::string> text_output_data;
    tc::Error err_paser = results_ptr->StringData("value", &text_output_data);
    if (!err_paser.IsOk()) {
        LOGE("err_paser.Message(): {}", err_paser.Message());
        return "";
    }
    return text_output_data[0];
}

int LlmTritonClient::setLoraScale(int task_id,
                                  std::vector<float> &lora_scale_input_data) {
    // 确保 lora_scale_input_data 大小与 lora_list_ 一致
    if (lora_scale_input_data.size() != lora_list_.size()) {
        return -1; // 或者 resize，但通常应提前保证一致
    }

    // 填充所有元素为 0
    std::fill(lora_scale_input_data.begin(), lora_scale_input_data.end(), 0.0f);

    // 查找 task_id 在 lora_list_ 中的位置
    auto it = std::find(lora_list_.begin(), lora_list_.end(),
                        static_cast<LoraFunction>(task_id));
    if (it != lora_list_.end()) {
        size_t index = std::distance(lora_list_.begin(), it);
        lora_scale_input_data[index] = 1.0f;
        return 0;
    } else {
        // task_id 不在支持的列表中
        return -1;
    }
}

result::AiEngineBaseResult
LlmTritonClient::startInfer(const struct ChatConfig &config,
                            std::vector<tc::InferInput *> &inputs) {
    result::AiEngineBaseResult result;
    result.success = true;
    bool ready = false;
    tc::Error error = chat_client_->IsModelReady(&ready, model_name_);
    if (!error.IsOk()) {
        LOGE("[LLM]Triton error: unable to get model ready status, error "
             "message: {}",
             error.Message());
        result.success = false;
        result.error = ai_engine::EngineError(
            (int)AiSdkCommonErrorCode::AISDK_SERVICE_ERROR,
            "[LLM]: 模型状态获取失败。");
        return result;
    }
    if (!ready) {
        LOGE("[LLM]Model error: model {} is not ready, we have to LoadModel "
             "model.",
             model_name_);
        error = chat_client_->LoadModel(model_name_);
        if (!error.IsOk()) {
            LOGE("[LLM]Triton error: unable to LoadModel model. {} error "
                 "message: {}",
                 model_name_, error.Message());
            result.success = false;
            result.error = ai_engine::EngineError(
                (int)ai_engine::NlpEngineErrorCode::LocalNlpModelNotLoaded,
                "[LLM]: 模型加载失败。");
            return result;
        }
    }
    auto ptr_request_type =
        createInput("request_type", {1}, "INT32", config.request_type_data_);
    inputs.push_back(ptr_request_type.get());
    auto ptr_request_id = createInput("request_id", {1}, "BYTES",
                                      std::vector{config.request_id_});
    inputs.push_back(ptr_request_id.get());
    auto ptr_text_input = createInput("text_input", {1}, "BYTES",
                                      std::vector{config.chat_prompt_});
    inputs.push_back(ptr_text_input.get());
    auto ptr_temperature_input = createInput("temperature_input", {1}, "FP32",
                                             config.temperature_input_data_);
    inputs.push_back(ptr_temperature_input.get());
    auto ptr_top_k_input =
        createInput("top_k_input", {1}, "INT32", config.top_k_input_data_);
    inputs.push_back(ptr_top_k_input.get());
    auto ptr_top_p_input =
        createInput("top_p_input", {1}, "FP32", config.top_p_input_data_);
    inputs.push_back(ptr_top_p_input.get());
    auto ptr_n_keep_input =
        createInput("n_keep_input", {1}, "INT32", config.n_keep_input_data_);
    inputs.push_back(ptr_n_keep_input.get());
    auto ptr_n_predict_input = createInput("n_predict_input", {1}, "INT32",
                                           config.n_predict_input_data_);
    inputs.push_back(ptr_n_predict_input.get());
    auto ptr_cache_prompt_input = createInput("cache_prompt_input", {1}, "BOOL",
                                              config.cache_prompt_input_data_);
    inputs.push_back(ptr_cache_prompt_input.get());
    auto ptr_stop_input = createInput(
        "stop_input", {static_cast<int64_t>(config.stop_input_data_.size())},
        "BYTES", config.stop_input_data_);
    inputs.push_back(ptr_stop_input.get());
    auto ptr_lora_scale_input = createInput(
        "lora_scale_input",
        {static_cast<int64_t>(config.lora_scale_input_data_.size())}, "FP32",
        config.lora_scale_input_data_);
    for (auto i = 0; i < config.lora_scale_input_data_.size(); i++) {
        LOGI("lora_scale_input_data_{}: {}\n", i,
             config.lora_scale_input_data_[i]);
    }
    inputs.push_back(ptr_lora_scale_input.get());
    auto ptr_stream_input =
        createInput("stream_input", {1}, "BOOL", config.stream_input_data_);
    inputs.push_back(ptr_stream_input.get());

    // Create text output
    tc::InferRequestedOutput *text_output;
    FAIL_IF_ERR(tc::InferRequestedOutput::Create(&text_output, "text_output"),
                "unable to get text_output");
    std::shared_ptr<tc::InferRequestedOutput> text_output_ptr(text_output);

    // Async inference call
    tc::InferOptions options(model_name_);
    options.model_version_ = "";

    std::unique_lock<std::mutex> lock(async_stream_infer_mutex_);
    FAIL_IF_ERR(chat_client_->AsyncStreamInfer(options, inputs,
                                               {text_output_ptr.get()}),
                "unable to AsyncStreamInfer :");
    LOGI("Stream Inference: Start Inference--- {}", config.request_id_);

    return result;
}

std::string LlmTritonClient::asyncStreamLoop(const struct ChatConfig &config) {
    std::string ans;
    bool final_response = false;
    bool has_received_token = false;
    while (!final_response) {
        // Wait until all callbacks are invoked
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() {
            if (result_list_.size() > 0) {
                return true;
            } else {
                return false;
            }
        });
        if (!has_received_token) {
            LOGI("Stream Inference: get first token --- {}",
                 config.request_id_);
            has_received_token = true;
        }
        std::shared_ptr<tc::InferResult> result = result_list_.front();
        result->IsFinalResponse(&final_response);
        std::vector<std::string> text_output_data;
        result->StringData("text_output", &text_output_data);

        if (text_output_data.size() != 1) {
            LOGE("error: received incorrect number of strings for "
                 "text_output:  {}. final_response: {}",
                 text_output_data.size(), final_response);
            // 目前发现后摩和qwen3
            // 0.6b模型会在开始的时候输出一条0长回答，这里直接跳过，暂未发现有其他影响
            continue;
        }
        ans += text_output_data[0];
        printf("prompt:%s\n<----->nlp engine get from triton llm:%s\n",
               config.chat_prompt_.c_str(), ans.c_str());
        if (chat_stopped_) {
            LOGI("聊天结束\n");
            return ans;
        }
        if (config.stream_output_) {
            TextGenerationResult output = createTextGenerationResult(
                text_output_data[0], "normal", false);
            if (chat_result_callback_ != nullptr) {
                chat_result_callback_(output);
            }
        }
        result_list_.erase(result_list_.begin());
    }
    return ans;
}

result::AiEngineBaseResult LlmTritonClient::chat(const std::string &message) {
    is_chatting_.store(true, std::memory_order_release);
    result::AiEngineBaseResult res;
    TextGenerationResult output;
    res.success = true;
    std::string prompt;
    int task_id = -1;
    struct ChatConfig config;
    isThinking_ = false;

    tc::Headers http_headers;
    uint32_t stream_timeout = 0;

    if (chat_client_ == nullptr) {
        auto create_client_and_start_stream =
            [&](std::unique_ptr<tc::InferenceServerGrpcClient> &client,
                bool use_compression) {
                FAIL_IF_ERR(tc::InferenceServerGrpcClient::Create(
                                &client, grpc_server_url_, false, false,
                                tc::SslOptions(), tc::KeepAliveOptions(), true),
                            "unable to create client");

                FAIL_IF_ERR(
                    client->StartStream(
                        [this](tc::InferResult *result) {
                            std::shared_ptr<tc::InferResult> result_ptr(result);
                            std::lock_guard<std::mutex> lock(mutex_);
                            result_list_.push_back(result_ptr);
                            cv_.notify_all();
                        },
                        false /*ship_stats*/, stream_timeout, http_headers),
                    "unable to start stream");
            };

        create_client_and_start_stream(chat_local_client_, false);
        create_client_and_start_stream(chat_share_client_, true);
        chat_client_ = (use_local_model_ ? chat_local_client_.get()
                                         : chat_share_client_.get());
        LOGI("StartStream");
    }

    // 检查模型是否仍然可用
    if (!isModelAvailable(model_name_)) {
        res.success = false;
        res.error = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::LocalNlpModelNotLoaded,
            "[本地模型引擎]: 模型 " + model_name_ + " 已不存在");
        is_chatting_.store(false, std::memory_order_release);
        return res;
    }

    bool is_local_server = true;

    if (is_local_server != use_local_model_) {
        LOGE("推理服务提供者与设定不一致：is_local_server：{} "
             "use_local_model_：{}",
             is_local_server, use_local_model_);
        res.success = false;
        res.error = ai_engine::EngineError(
            (int)AiSdkCommonErrorCode::AISDK_MODEL_NOT_FOUND,
            "[本地模型引擎]: 推理服务提供者与设定不一致，无法提供模型服务");
        return res;
    }
    try {
        nlohmann::json bodys = nlohmann::json::parse(message);

        prompt = "\n<|im_start|>";
        for (auto body : bodys["messages"]) {
            prompt += body["role"].get<std::string>();
            prompt += "\n";
            prompt += body["content"].get<std::string>();
            prompt += "<|im_end|>\n<|im_start|>";
        }
        prompt += "assistant\n";
        if (bodys.contains("task_id")) {
            task_id = bodys["task_id"].get<int>();
        } else {
            std::cout << "bodys task_id is null, set to -1" << std::endl;
        }
        std::cout << prompt << std::endl;
        std::cout << "task_id:" << task_id << std::endl;
        if (bodys.contains("top_k")) {
            config.top_k_input_data_ = bodys["top_k"].get<int>();
        }
        if (bodys.contains("stream")) {
            config.stream_output_ = bodys["stream"].get<bool>();
        }

    } catch (...) {
        res.success = false;
        res.error = ai_engine::EngineError(
            (int)NlpEngineErrorCode::FailedToConnectServer,
            "[本地模型引擎]: NLP对象无效");
        return res;
    }

    result_list_.clear();
    chat_stopped_ = false;

    std::vector<float> lora_scale_input_data(lora_list_.size(), 0.0f);
    setLoraScale(task_id, lora_scale_input_data);

    config.request_id_ = request_id_;
    config.chat_prompt_ = prompt;
    config.n_predict_input_data_ = n_predict_input_;
    config.lora_scale_input_data_ = lora_scale_input_data;
    config.stream_input_data_ = true;
    std::vector<tc::InferInput *> inputs;

    res = startInfer(config, inputs);
    if (!res.success) {
        is_chatting_.store(false, std::memory_order_release);
        return res;
    }
    std::string ans = asyncStreamLoop(config);
    LOGI("Stream Inference: get last token --- {}", config.request_id_);
    LOGI("Inference result: {}", ans);

    if (chat_stopped_) {
        is_chatting_.store(false, std::memory_order_release);
        return res;
    }
    if (!config.stream_output_) {
        output = createTextGenerationResult(ans, "normal", false);
        if (chat_result_callback_ != nullptr) {
            chat_result_callback_(output);
        }
    }
    output = createTextGenerationResult("", "normal", true);
    if (chat_result_callback_ != nullptr) {
        chat_result_callback_(output);
    }
    is_chatting_.store(false, std::memory_order_release);

    return res;
}

bool LlmTritonClient::isThinking() const { return isThinking_; }

// ============================================================================
// 全局单例监控线程方法
// ============================================================================

void LlmTritonClient::registerInstance() {
    std::lock_guard<std::mutex> lock(instance_registry_mutex_);
    registered_instances_.insert(this);
    int count = ++instance_count_;
    LOGI("Instance registered, total count: {}", count);

    // 仅在第一个实例时启动监控线程
    if (count == 1) {
        startMonitorThread();
    }
}

void LlmTritonClient::unregisterInstance() {
    std::lock_guard<std::mutex> lock(instance_registry_mutex_);
    registered_instances_.erase(this);
    // 验证发现经常出现engine数量为0的情况，但此时如果关闭线程会导致线程操作频繁，有风险，
    // 且终止线程受epoll_wait影响导致延迟，此操作暂时注释掉，后续进行分析和讨论
    // int count = --instance_count_;
    // LOGI("Instance unregistered, total count: {}",
    // count);

    // 仅在最后一个实例时停止监控线程
    // if (count == 0) {
    //     stopMonitorThread();
    // }
}

void LlmTritonClient::startMonitorThread() {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    if (monitor_running_) {
        LOGW("Monitor thread already running");
        return;
    }
    FAIL_IF_ERR(tc::InferenceServerGrpcClient::Create(
                    &global_client_, grpc_server_url_, false, false,
                    tc::SslOptions(), tc::KeepAliveOptions(), true),
                "unable to create global_client");

    monitor_running_ = true;
    monitor_thread_ = std::thread(&LlmTritonClient::monitorLoop);
    LOGI("Monitor thread started");
}

void LlmTritonClient::stopMonitorThread() {
    {
        std::lock_guard<std::mutex> lock(monitor_mutex_);
        if (!monitor_running_) {
            LOGW("Monitor thread not running");
            return;
        }
        monitor_running_ = false;
    }

    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    // 清理文件描述符
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    if (inotify_fd_ >= 0) {
        close(inotify_fd_);
        inotify_fd_ = -1;
    }
    if (model_dir_watch_fd_ >= 0) {
        model_dir_watch_fd_ = -1;
    }
    LOGI("Monitor thread stopped");
}

void LlmTritonClient::monitorLoop() {
    // 创建 epoll 实例
    {
        std::lock_guard<std::mutex> lock(monitor_mutex_);
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ < 0) {
            LOGE("Failed to create epoll fd");
            return;
        }

        // 创建 inotify 实例
        inotify_fd_ = inotify_init1(IN_NONBLOCK);
        if (inotify_fd_ < 0) {
            LOGE("Failed to create inotify fd");
            close(epoll_fd_);
            epoll_fd_ = -1;
            return;
        }

        // 将 inotify_fd 添加到 epoll 监控
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = inotify_fd_;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, inotify_fd_, &ev) < 0) {
            LOGE("Failed to add inotify fd to epoll");
            close(epoll_fd_);
            close(inotify_fd_);
            epoll_fd_ = -1;
            inotify_fd_ = -1;
            return;
        }

        // 添加对模型仓库目录的监控
        model_dir_watch_fd_ = inotify_add_watch(
            inotify_fd_, model_repository_path_.c_str(),
            IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE);

        if (model_dir_watch_fd_ < 0) {
            LOGE("Failed to add watch for model repository path: {}",
                 model_repository_path_.c_str());
        }
    }

    LOGI("Monitor thread started, watching: {}",
         model_repository_path_.c_str());

    while (monitor_running_) {
        struct epoll_event events[16];
        int nfds = epoll_wait(epoll_fd_, events, 16, 100); // 1 秒超时

        if (nfds > 0) {
            LOGI("Monitor thread got {} events", nfds);
            // 处理 inotify 事件
            handleInotifyEvents();
            LOGI("handleInotifyEvents done {} events", nfds);
        }
    }

    LOGI("Monitor thread stopping");
}

void LlmTritonClient::handleInotifyEvents() {
    char buffer[4096] __attribute__((aligned(__alignof__(inotify_event))));
    ssize_t len;

    while (true) {
        len = read(inotify_fd_, buffer, sizeof(buffer));
        if (len == -1 && errno == EWOULDBLOCK) {
            break;
        }
        if (len <= 0) {
            break;
        }

        for (char *ptr = buffer; ptr < buffer + len;) {
            struct inotify_event *event = (struct inotify_event *)ptr;
            if (event->len > 0) {
                std::string affected_path =
                    model_repository_path_ + std::string(event->name);
                LOGI("inotify event: mask={} name={}", event->mask,
                     event->name);
                LOGI("inotify event: affected_path={}", affected_path);

                // 检查是否是删除或移动事件
                if (event->mask &
                    (IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_CREATE)) {
                    // 先在锁内复制监控路径列表，然后在锁外处理
                    std::vector<std::string> paths_to_check;
                    {
                        std::lock_guard<std::mutex> lock(monitor_mutex_);
                        paths_to_check = monitored_paths_;
                    }

                    // 在锁外检查和处理
                    for (const auto &monitored : paths_to_check) {
                        LOGI("Monitored path: {}", monitored);
                        if (affected_path == monitored ||
                            affected_path.find(monitored + "/") == 0) {
                            LOGI("Monitored path removed: {}", affected_path);
                            handlePathRemoved(affected_path);
                            break;
                        }
                    }
                }
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
}

void LlmTritonClient::checkMonitoredPaths() {
    std::vector<std::string> removed_paths;
    {
        std::lock_guard<std::mutex> lock(monitor_mutex_);
        for (const auto &path : monitored_paths_) {
            struct stat st;
            if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
                LOGI("Monitored path no longer exists: {}", path.c_str());
                removed_paths.push_back(path);
            }
        }
    }

    for (const auto &path : removed_paths) {
        handlePathRemoved(path);
    }
}

void LlmTritonClient::handlePathRemoved(const std::string &path) {
    // 从 path 提取 model_name
    std::string model_name = path.substr(model_repository_path_.length());
    LOGI("Handling path removal: {}, model_name: {}", path.c_str(),
         model_name.c_str());

    // 从 monitored_paths_ 中移除
    {
        std::lock_guard<std::mutex> lock(monitor_mutex_);
        auto it =
            std::find(monitored_paths_.begin(), monitored_paths_.end(), path);
        if (it != monitored_paths_.end()) {
            monitored_paths_.erase(it);
        }
    }

    // 先复制实例指针列表，避免在持有锁的情况下调用 modelInfoUpdate
    std::vector<LlmTritonClient *> instances_to_update;
    {
        std::lock_guard<std::mutex> lock(instance_registry_mutex_);
        instances_to_update.assign(registered_instances_.begin(),
                                   registered_instances_.end());
    }

    // 在锁外遍历调用，避免死锁
    for (auto *instance : instances_to_update) {
        LOGE("modelInfoUpdate");
        instance->modelInfoUpdate();
        LOGI("Instance updated model_info_: {}", instance->model_info_);
    }

    // 调用 global_client_->UnloadModel 卸载模型
    if (global_client_ != nullptr) {
        tc::Error err = global_client_->UnloadModel(model_name);
        if (!err.IsOk()) {
            LOGE("Failed to unload model {}: {}", model_name.c_str(),
                 err.Message().c_str());
        } else {
            LOGI("Successfully unloaded model {} due to path removal",
                 model_name.c_str());
        }
    }
}

void LlmTritonClient::updateMonitoredPaths() {
    std::vector<std::string> new_paths;

    // 从所有实例中获取 model_info_ 并合并
    std::unordered_set<std::string> all_model_paths;
    {
        std::lock_guard<std::mutex> lock(instance_registry_mutex_);
        for (auto *instance : registered_instances_) {
            try {
                nlohmann::json model_info_json =
                    nlohmann::json::parse(instance->model_info_);
                if (model_info_json.contains("models") &&
                    model_info_json["models"].is_array()) {
                    for (auto &it_json : model_info_json["models"]) {
                        if (it_json.contains("model") &&
                            it_json["model"].is_string()) {
                            std::string model_name =
                                it_json["model"].get<std::string>();
                            std::string full_path =
                                model_repository_path_ + model_name;
                            all_model_paths.insert(full_path);
                        }
                    }
                }
            } catch (const std::exception &e) {
                LOGE("Failed to parse model_info_ from instance: {}", e.what());
            }
        }
    }

    for (const auto &path : all_model_paths) {
        new_paths.push_back(path);
    }

    // 找出需要移除的路径
    std::vector<std::string> to_remove;
    {
        std::lock_guard<std::mutex> lock(monitor_mutex_);
        for (const auto &old_path : monitored_paths_) {
            bool found = false;
            for (const auto &new_path : new_paths) {
                if (old_path == new_path) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                to_remove.push_back(old_path);
            }
        }

        // 移除不再存在的路径
        for (const auto &path : to_remove) {
            auto it = std::find(monitored_paths_.begin(),
                                monitored_paths_.end(), path);
            if (it != monitored_paths_.end()) {
                monitored_paths_.erase(it);
                LOGI("Removed from monitoring: {}", path.c_str());
            }
        }

        // 添加新的路径
        for (const auto &new_path : new_paths) {
            bool found = false;
            for (const auto &old_path : monitored_paths_) {
                if (old_path == new_path) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                monitored_paths_.push_back(new_path);
                LOGI("Added to monitoring: {}", new_path.c_str());
            }
        }
    }
}

bool LlmTritonClient::isModelAvailable(const std::string &model_name) {
    std::lock_guard<std::mutex> lock(instance_registry_mutex_);
    for (auto *instance : registered_instances_) {
        try {
            nlohmann::json model_info_json =
                nlohmann::json::parse(instance->model_info_);
            if (model_info_json.contains("models") &&
                model_info_json["models"].is_array()) {
                for (auto &it_json : model_info_json["models"]) {
                    if (it_json.contains("model") &&
                        it_json["model"].get<std::string>() == model_name) {
                        return true;
                    }
                }
            }
        } catch (const std::exception &e) {
            // 继续检查下一个实例
        }
    }
    return false;
}
} // namespace ai_engine::gen_ai::nlp
