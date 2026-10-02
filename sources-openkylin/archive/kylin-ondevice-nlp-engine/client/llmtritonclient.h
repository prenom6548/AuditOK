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

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include <kylin-ai/ai-engine/textgenerationengine.h>
#include <kylin-ai/common/error.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <triton/client/grpc_client.h>
#include "../utils/logger.h"
#include "../utils/utils.h"

namespace tc = triton::client;
namespace ai_engine::gen_ai::nlp {

using CompletionResultCallback =
    std::function<void(const std::string &, bool over)>;
using ResultList = std::vector<std::shared_ptr<tc::InferResult>>;

struct ChatConfig {
    int request_type_data_ = 0;
    std::string request_id_ = "message_1";
    std::string chat_prompt_ = "message";
    float temperature_input_ = 0.0;
    int32_t temperature_input_data_ = 0.0;
    int32_t top_k_input_data_ = 40;
    float top_p_input_data_ = 0.95;
    int32_t n_keep_input_data_ = 0;
    int32_t n_predict_input_data_ = 512;
    bool cache_prompt_input_data_ = true;
    std::vector<std::string> stop_input_data_{"<|im_end|>"};
    std::vector<float> lora_scale_input_data_ = {0, 0};
    bool stream_input_data_ = true;
    bool stream_output_ = true;
};

enum class clientConnectStatus {
    NOTATTEMPTED, // 未尝试
    SUCCESS,      // 成功
    FAILED        // 失败
};

class LlmTritonClient {
public:
    LlmTritonClient();
    ~LlmTritonClient();
    bool setCurrentModel(const std::string &model_name);
    void setTextGenerationResultCallback(TextGenerationResultCallback callback);
    result::AiEngineBaseResult init();
    std::string getModelInfo();
    std::string getCurrentModel() const { return model_name_; };
    std::string getModelName() const { return model_name_; };
    result::AiEngineBaseResult chat(const std::string &message);
    void setConfig(const std::string &config);
    void stopChat();

private:
    bool connect();
    int getLoraCount();
    void modelInfoUpdate();
    std::string getModelDisplayName() const { return model_display_name_; };
    std::string asyncStreamLoop(const struct ChatConfig &config);
    result::AiEngineBaseResult
    startInfer(const struct ChatConfig &config,
               std::vector<tc::InferInput *> &inputs);
    int setLoraScale(int task_id, std::vector<float> &lora_scale_input_data);
    TextGenerationResult
    createTextGenerationResult(const std::string &result,
                               const std::string &finish_reason,
                               const bool &is_end);
    std::string getConfigValue(const char *key, const char *engine);
    bool isThinking() const;
    std::atomic<bool> is_chatting_{false};
    ResultList result_list_;
    bool use_local_model_ = true;
    std::mutex mutex_;
    std::mutex chat_client_mutex_;
    std::mutex infer_mutex_;
    std::mutex async_stream_infer_mutex_;
    std::condition_variable cv_;
    std::string request_id_ = "";
    std::string model_name_ = "";
    std::string model_version_ = "";
    std::string default_model_info_ = "";
    std::string model_display_name_ = "";
    std::string model_info_ = R"(
      {
          "vendor": "",
          "models": []
      })";
    std::vector<LoraFunction> lora_list_ = {};
    clientConnectStatus client_connected_ = clientConnectStatus::NOTATTEMPTED;
    int model_load_timeout_us_ = 60 * 1000 * 1000;
    int n_predict_input_ = 1024;
    bool chat_stopped_ = false;
    bool isThinking_ = false;
    CompletionResultCallback completion_callbacks_;
    TextGenerationResultCallback chat_result_callback_ = nullptr;

    // 用于与triton的事务性连接通信
    tc::InferenceServerGrpcClient *client_ = nullptr;
    std::unique_ptr<tc::InferenceServerGrpcClient> local_client_ = nullptr;
    std::unique_ptr<tc::InferenceServerGrpcClient> share_client_ = nullptr;

    // 用于与triton的大模型对话的连接通信
    // TODO:
    // 后续需要在engine虚基类中增加专门用于休眠和销毁client的接口，避免在chat和stopChat中进行相关操作
    tc::InferenceServerGrpcClient *chat_client_ = nullptr;
    std::unique_ptr<tc::InferenceServerGrpcClient> chat_local_client_ = nullptr;
    std::unique_ptr<tc::InferenceServerGrpcClient> chat_share_client_ = nullptr;

    // 模型目录监控相关 - 全局单例
    static std::string grpc_server_url_;
    static std::string model_repository_path_;
    static std::vector<std::string> monitored_paths_;
    static std::atomic<bool> monitor_running_;
    static std::thread monitor_thread_;
    static int epoll_fd_;
    static int inotify_fd_;
    static int model_dir_watch_fd_;
    static std::mutex monitor_mutex_;
    static std::atomic<int> instance_count_;
    static std::unordered_set<LlmTritonClient *> registered_instances_;
    static std::mutex instance_registry_mutex_;
    static std::unique_ptr<tc::InferenceServerGrpcClient> global_client_;

    // 监控线程相关函数 - 静态全局单例
    static void startMonitorThread();
    static void stopMonitorThread();
    static void monitorLoop();
    static void handlePathRemoved(const std::string &path);
    static void updateMonitoredPaths();
    static bool isModelAvailable(const std::string &model_name);
    static void checkMonitoredPaths();
    static void handleInotifyEvents();

    // 实例注册/注销
    void registerInstance();
    void unregisterInstance();
};

} // namespace ai_engine::gen_ai::nlp