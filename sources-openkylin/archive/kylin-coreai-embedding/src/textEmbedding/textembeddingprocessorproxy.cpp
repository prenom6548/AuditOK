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

#include "textembeddingprocessorproxy.h"

#include "coreaiTextEmbeddingDBusFuncs.h"
#include "error.h"
#include "logger.h"
#include "nlohmann/json.hpp"
#include "protocolversion.h"
#include "textembeddingserver.h"

namespace kyai::coreai::embedding {
namespace {

const char *textEmbeddingProcessorObjectPath =
    "/com/kylin/AiRuntime/CoreTextEmbeddingService";

const std::string serverUnixPath = "unix:path=/tmp/.kylin-ai-runtime-unix/" +
                                   std::to_string(getuid()) +
                                   "/core-textembedding.sock";

void setErrorCode(int *pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

}  // namespace

int TextEmbeddingProcessorProxy::connect() {
    // 获取单例连接
    server_ = TextEmbeddingServer::getInstance();
    if (server_ == nullptr) {
        LOGE("Failed to get TextEmbeddingServer instance");
        return COREAI_EMBEDDING_CONNECTION_ERROR;
    }

    // 获取代理
    delegate_ = server_->getTextEmbeddingProxy();
    if (delegate_ == nullptr) {
        LOGE("Failed to get text embedding proxy");
        return COREAI_EMBEDDING_CONNECTION_ERROR;
    }

    /* 在获得连接时，就会去获得runtime的版本
    获取runtime版本,用于init接口兼容, 新sdk
    init接口行为变化,不会去加载模型了,所以要做兼容
    新sdk + 旧runtime 调用原来的init接口
    新sdk + 新runtime 调用init_v2
    */
    runtimeVersion_ = versionToString(getRuntimeProtocolVersion());
    return AI_COMMON_SUCCESS;
}

int TextEmbeddingProcessorProxy::initEngine() {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("init", &errorCode)) {
        return errorCode;
    }

    if (runtimeVersion_ == "1.0.0") {
        // 新sdk + 旧runtime
        LOGI("Runtime version is ", runtimeVersion_,
             " InitEngine call old init_v1");
        return init_v1(errorCode);
    } else {
        // 新sdk + 新runtime
        LOGI("Runtime version is ", runtimeVersion_,
             "InitEngine call new init_v2");
        return init_v2(errorCode);
    }
}

int TextEmbeddingProcessorProxy::initModel(const std::string &model) {
    if (init_ == false) {
        LOGE("Call failed, due to don't call init session");
        return AI_COMMON_INVALID_SESSION;
    }

    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("initModel", &errorCode)) {
        return errorCode;
    }

    GError *gError = nullptr;
    bool success = g_func_coreai_text_init_model(
        delegate_, sessionId_, model.c_str(), &errorCode, nullptr, &gError);
    if (!success) {
        return AI_COMMON_RUNTIME_ERROR;
    }

    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Proxy init model failed,model:", model.c_str(),
             "errorCode:", errorCode, "sessionId:", sessionId_);
        return errorCode;
    }

    LOGI("Proxy init model success,model:", model.c_str(),
         "sessionId:", sessionId_);
    model_ = model;
    return AI_COMMON_SUCCESS;
}

EmbeddingResult *TextEmbeddingProcessorProxy::embeddingText(
    const std::string &text) {
    if (init_ == false) {
        LOGE("Call failed, due to don't call init session");
        return new EmbeddingResult(
            COREAI_EMBEDDING_INIT_ERROR,
            "Call failed, due to don't call init session");
    }

    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("embeddingText", &errorCode)) {
        return new EmbeddingResult(errorCode, "Runtime version is old.");
    }

    errorCode = ensureDefaultModelLoadedIfNeeded();
    if (errorCode != AI_COMMON_SUCCESS) {
        return new EmbeddingResult(errorCode, "Ensure default model failed.");
    }

    GError *gError = nullptr;
    nlohmann::json object;
    object["text"] = text;
    object["sessionId"] = sessionId_;
    char *jsonResult = nullptr;

    bool success = g_func_coreai_text_embedding(
        delegate_, object.dump().c_str(), &jsonResult, nullptr, &gError);

    if (!success) {
        std::string errorMsg = handleErrorOccurred(gError);
        return new EmbeddingResult(COREAI_EMBEDDING_CONNECTION_ERROR, errorMsg);
    }

    auto result = parseResult(jsonResult);
    g_free(jsonResult);
    EmbeddingResult *res = new EmbeddingResult(result);
    return res;
}

void TextEmbeddingProcessorProxy::embeddingTextAsync(
    const std::string &text, TextEmbeddingResultCallback callback,
    void *userData) {
    if (init_ == false) {
        LOGE("Call failed, due to don't call init session");
        EmbeddingResult result(COREAI_EMBEDDING_INIT_ERROR,
                               "Call failed, due to don't call init session");
        callback(&result, userData);
        return;
    }

    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("embeddingText", &errorCode)) {
        EmbeddingResult result(errorCode, "Runtime version is old.");
        callback(&result, userData);
        return;
    }

    errorCode = ensureDefaultModelLoadedIfNeeded();
    if (errorCode != AI_COMMON_SUCCESS) {
        EmbeddingResult result(errorCode, "Ensure default model failed.");
        callback(&result, userData);
        return;
    }

    nlohmann::json object;
    object["text"] = text;
    object["sessionId"] = sessionId_;

    EmbeddingRequestContext *context =
        new EmbeddingRequestContext{callback, userData, shared_from_this()};

    g_func_coreai_text_embedding_async(
        delegate_, object.dump().c_str(), nullptr,
        (GAsyncReadyCallback)embeddingTextAsyncCallback, context);
}

int TextEmbeddingProcessorProxy::init_v1(int errorCode) {
    nlohmann::json config;
    config["engineName"] = "Embedding";
    GError *gError = nullptr;
    bool success =
        g_func_coreai_text_init(delegate_, config.dump().c_str(), &sessionId_,
                                &errorCode, nullptr, &gError);
    if (!success) {
        if (gError) {
            LOGE("ERROR:call kylin-ai-runtime server failed,errCode: ",
                 gError->code, "errMsg: ", gError->message);
            g_error_free(gError);
            gError = nullptr;
        } else {
            LOGE("ERROR:call kylin-ai-runtime server failed.");
        }
        return AI_COMMON_RUNTIME_ERROR;
    }

    if (sessionId_ == -1 || errorCode != AI_COMMON_SUCCESS) {
        LOGE("Proxy init failed, errorCode:", errorCode);
        return errorCode;
    }

    LOGI("Proxy init success,sessionId is:", sessionId_);
    init_ = true;
    return AI_COMMON_SUCCESS;
}

int TextEmbeddingProcessorProxy::init_v2(int errorCode) {
    GError *gError = nullptr;
    bool success = ai_runtime_core_text_embedding_service_call_init_v2_sync(
        delegate_, &sessionId_, &errorCode, nullptr, &gError);

    if (!success) {
        if (gError) {
            LOGE("ERROR:call kylin-ai-runtime server failed,errCode: ",
                 gError->code, "errMsg: ", gError->message);
            g_error_free(gError);
            gError = nullptr;
        } else {
            LOGE("ERROR:call kylin-ai-runtime server failed.");
        }
        return AI_COMMON_RUNTIME_ERROR;
    }

    if (sessionId_ == -1 || errorCode != AI_COMMON_SUCCESS) {
        LOGE("Proxy init failed, errorCode:", errorCode);
        return errorCode;
    }

    LOGI("Proxy init success,sessionId is:", sessionId_);
    init_ = true;
    return AI_COMMON_SUCCESS;
}

EmbeddingResult TextEmbeddingProcessorProxy::parseResult(
    const std::string &jsonResult) {
    if (jsonResult.empty()) {
        LOGE("Embedding result is empty.");
        return EmbeddingResult(COREAI_EMBEDDING_RUNTIME_ERROR,
                               "Embedding result is empty.");
    }

    nlohmann::json root = nlohmann::json::parse(jsonResult, nullptr, false);
    if (root.is_discarded()) {
        LOGE("Embedding result json is discarded.");
        return EmbeddingResult(COREAI_EMBEDDING_RUNTIME_ERROR,
                               "Embedding result json is discarded.");
    }

    if (root.empty() || !root.contains("vector_result") ||
        !root["vector_result"].is_array() || !root.contains("errorMessage") ||
        !root.contains("errorCode")) {
        LOGE("Json format error.");
        return EmbeddingResult(COREAI_EMBEDDING_RUNTIME_ERROR,
                               "Json format error.");
    }

    int errCode = root["errorCode"].get<int>();
    if (errCode != AI_COMMON_SUCCESS) {
        LOGE("Parse embedding result error,errCode: {}, errMsg: {}", errCode,
             root["errorMessage"].get<std::string>().c_str());
        return EmbeddingResult(errCode,
                               root["errorMessage"].get<std::string>());
    }

    return EmbeddingResult(root["vector_result"].get<std::vector<float>>());
}

void TextEmbeddingProcessorProxy::embeddingTextAsyncCallback(GObject *object,
                                                             GAsyncResult *res,
                                                             gpointer data) {
    auto *context = static_cast<EmbeddingRequestContext *>(data);
    auto proxy =
        context->proxy.lock();  // 获得成功 引用计数+1 引用计数的过程是原子的
    if (!proxy) {
        LOGE("Callback operation cancelled: session object destroyed");
        EmbeddingResult result(COREAI_EMBEDDING_CONNECTION_ERROR,
                               "Operation cancelled: session object destroyed");
        context->callback(&result, context->userData);
        delete context;
        context = nullptr;
        return;
    }

    GError *gError = NULL;
    char *jsonResult = nullptr;
    g_func_coreai_text_embedding_async_finish(
        (AiRuntimeCoreTextEmbeddingService *)object, &jsonResult, res, &gError);

    if (gError) {
        std::string errorMsg = proxy->handleErrorOccurred(gError);
        EmbeddingResult result(COREAI_EMBEDDING_CONNECTION_ERROR, errorMsg);
        context->callback(&result, context->userData);
        delete context;
        context = nullptr;
        return;
    }

    if (!jsonResult) {
        LOGE(
            "Te-embedding text async result error:jsonResult is "
            "nullptr!");
        EmbeddingResult result(COREAI_EMBEDDING_RUNTIME_ERROR,
                               "Embedding result is nullptr!");
        context->callback(&result, context->userData);
        delete context;
        context = nullptr;
        return;
    }

    auto result = parseResult(jsonResult);
    g_free(jsonResult);
    context->callback(&result, context->userData);
    delete context;
    context = nullptr;
}

bool TextEmbeddingProcessorProxy::ensureProtocolSupportedForApi(
    const char *apiName, int *errorCode) {
    if (ensureApiSupported(apiName, errorCode)) {
        return true;
    }

    const ProtocolRequirement *requirement = findApiRequirement(apiName);
    ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0},
                                           kProtocolVersionMax};
    const ProtocolRequirement &activeRequirement =
        requirement ? *requirement : defaultRequirement;

    if (apiName) {
        LOGE("Protocol mismatch for API:", apiName,
             "runtime:", versionToString(getRuntimeProtocolVersion()),
             "sdk:", versionToString(getSdkProtocolVersion()),
             "min:", versionToString(activeRequirement.minVersion),
             "max:", versionToString(activeRequirement.maxVersion));
    }

    return false;
}

_EmbeddingModelList *TextEmbeddingProcessorProxy::getModelList(int *errorCode) {
    if (init_ == false) {
        LOGE("Call failed, due to don't call init session");
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }

    if (!ensureProtocolSupportedForApi("getModelList", errorCode)) {
        return nullptr;
    }

    char *output = nullptr;
    GError *gError = nullptr;
    bool success = g_func_coreai_text_get_model_list(delegate_, sessionId_,
                                                     &output, nullptr, &gError);
    if (!success) {
        setErrorCode(errorCode, AI_COMMON_RUNTIME_ERROR);
        return nullptr;
    }

    if (!output) {
        setErrorCode(errorCode, AI_COMMON_RUNTIME_ERROR);
        return nullptr;
    }

    std::string modelList(output);
    g_free(output);
    if (!parseModelList(modelList, modelList_)) {
        setErrorCode(errorCode, AI_COMMON_RUNTIME_ERROR);
        return nullptr;
    }

    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return &modelList_;
}

bool TextEmbeddingProcessorProxy::parseModelList(const std::string &modelList,
                                                 _EmbeddingModelList &list) {
    // 只需要提取文本向量化的模型列表即可，也就是model_capability字段为TEXT所在的json中

    nlohmann::json root;
    root = nlohmann::json::parse(modelList, nullptr, false);
    if (root.is_discarded()) {
        LOGE("getModelList json parse error.");
        return false;
    }

    if (!root.contains("models") || !root["models"].is_array() ||
        root["models"].empty()) {
        LOGE("ModelInfo format error.");
        return false;
    }

    auto models = root["models"].get<std::vector<nlohmann::json>>();
    std::vector<_EmbeddingModelInfo> candidates;
    for (const auto &model : models) {
        if (!model.contains("model_capability") ||
            !model["model_capability"].is_string() ||
            model["model_capability"].get<std::string>() != "TEXT") {
            continue;
        }

        if (!model.contains("model") || !model["model"].is_string() ||
            !model.contains("dim") || !model["dim"].is_number_integer() ||
            !model.contains("priority") ||
            !model["priority"].is_number_integer()) {
            continue;
        }
        candidates.emplace_back(model["model"].get<std::string>(),
                                model["dim"].get<int>(),
                                model["priority"].get<int>());
    }

    if (candidates.empty()) {
        return false;
    }

    // 按 priority 升序排序
    if (!candidates.empty()) {
        std::sort(
            candidates.begin(), candidates.end(),
            [](const _EmbeddingModelInfo &a, const _EmbeddingModelInfo &b) {
                return a.getPriority() < b.getPriority();
            });
    }

    list = _EmbeddingModelList{candidates};
    return true;
}

int TextEmbeddingProcessorProxy::getDefaultModel(std::string &model) {
    int errorCode = AI_COMMON_SUCCESS;
    _EmbeddingModelList *list = getModelList(&errorCode);
    if (list == nullptr) {
        LOGE("Get default model failed: getModelList failed, errorCode:",
             errorCode);
        return errorCode;
    }

    // 获得默认模型
    model = list->getModelInfo(0)->getName();
    return AI_COMMON_SUCCESS;
}

int TextEmbeddingProcessorProxy::ensureDefaultModelLoadedIfNeeded() {
    // 旧 runtime（1.0.0）在 init_v1
    // 阶段会走旧逻辑（自带默认模型行为），不在此强行加载
    if (runtimeVersion_ == "1.0.0") {
        return AI_COMMON_SUCCESS;
    }

    // 新 runtime：如果用户未显式 initModel，则选择并加载默认模型
    if (!model_.empty()) {
        return AI_COMMON_SUCCESS;
    }

    LOGW(
        "Model was not initialized explicitly; SDK is loading the default "
        "model. Consider calling text_embedding_init_model() to load a "
        "specific model,otherwise default model changes may make vector data "
        "incompatible.");

    LOGI("Try to get default model");
    int errorCode = getDefaultModel(model_);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Get default model failed.");
        return errorCode;
    }

    LOGI("Get default model success, model:", model_);
    LOGI("Try to init default model:", model_);
    errorCode = initModel(model_);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Init default model failed, model:", model_);
        return errorCode;
    }

    LOGI("Init default model success, model:", model_);
    return AI_COMMON_SUCCESS;
}

std::string TextEmbeddingProcessorProxy::getModelInfo() {
    if (!init_) {
        return "";
    }

    // 该接口已废弃，后续会删掉，在sdk中先写死返回结果
    nlohmann::json modelInfo = {
        {"models",
         {{"text_model",
           {{"name", "ensemble-embd_gte-base_uint8-text"},
            {"dim", 768},
            {"ondevice", true}}},
          {"image_model",
           {{"image_text_name", "ensemble-embd_cn-clip_512-uint8-text"},
            {"image_name", "embd_cn-clip_512-uint8-image"},
            {"dim", 1024},
            {"ondevice", true}}}}}};

    // 提取 text_model 的部分
    std::string res = modelInfo["models"]["text_model"].dump();

    return res;
}

int TextEmbeddingProcessorProxy::reconnectAndRetry() {
    server_->reconnect();
    delegate_ = server_->getTextEmbeddingProxy();
    if (delegate_ == nullptr) {
        LOGE("Failed to get text embedding proxy after reconnect");
        return COREAI_EMBEDDING_CONNECTION_ERROR;
    }

    // 重连成功 重新拿一个可用的sessionId 并加载模型
    int errorCode = initEngine();
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Failed to init engine after reconnect");
        return errorCode;
    }

    if (runtimeVersion_ == "1.0.0") {
        // 新sdk + 旧runtime 不调用initModel
        return AI_COMMON_SUCCESS;
    }

    // 新sdk + 新runtime
    errorCode = initModel(model_);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Failed to init model after reconnect");
        return errorCode;
    }

    return AI_COMMON_SUCCESS;
}

void TextEmbeddingProcessorProxy::handleConnectionError(GError *gError) {
    if (!gError) {
        return;
    }

    if (gError->code != G_IO_ERROR_CLOSED) {
        return;
    }

    // 处理连接错误
    LOGE("Server closed unexpectedly, start reconnect");
    if (reconnectAndRetry() != AI_COMMON_SUCCESS) {
        LOGE("Reconnect server failed");
    } else {
        LOGI("Reconnect server success");
    }
}

std::string TextEmbeddingProcessorProxy::handleErrorOccurred(GError *gError) {
    std::string errorMsg = "ERROR:call kylin-ai-runtime server failed.";
    if (gError) {
        LOGE("ERROR:call kylin-ai-runtime server failed,errCode:", gError->code,
             "errMsg:", gError->message);

        // 处理连接错误
        handleConnectionError(gError);
        errorMsg = gError->message;
        g_error_free(gError);
        gError = nullptr;
    } else {
        LOGE("ERROR:call kylin-ai-runtime server failed.");
    }

    return errorMsg;
}

}  // namespace kyai::coreai::embedding