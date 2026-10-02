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

#include "coretextembeddingservice.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>
#include <sodium.h>

#include "utils/logger.h"

namespace {
const std::string objectPath = "/com/kylin/AiRuntime/CoreTextEmbeddingService";
constexpr const char* kSdkVersionAnnotation = "SdkVersion";
constexpr const char* kDefaultSdkVersion = "1.0.0";
std::string createResponse(const std::vector<float>& result, int errorCode, const std::string& errorMessage) {
    Json::Value root;
    root["errorCode"] = errorCode;
    root["errorMessage"] = errorMessage;

    Json::Value resultArray;
    for (int i = 0; i < result.size(); ++i) {
        resultArray.append(result[i]);
    }
    root["vector_result"] = resultArray;
    return root.toStyledString();
}

}  // namespace

CoreTextEmbeddingService::CoreTextEmbeddingService(GDBusConnection& connection, EngineSelector& engineSelector)
    : connection_(connection), engineSelector_(engineSelector) {
    exportSkeleton();
    const std::string protocolVersion = getProtocolVersion();
    if (protocolVersion.empty()) {
        LOGW("Protocol version is empty, use default protocol version: {}", kDefaultSdkVersion);
        setRuntimeProtocolVersion(kDefaultSdkVersion);
        LOGI("CoreTextEmbeddingService protocol version: {}", kDefaultSdkVersion);
    } else {
        setRuntimeProtocolVersion(protocolVersion);
        LOGI("CoreTextEmbeddingService protocol version: {}", protocolVersion);
    }
}

CoreTextEmbeddingService::~CoreTextEmbeddingService() {
    unexportSkeleton();
    LOGD("~CoreTextEmbeddingService");
}

void CoreTextEmbeddingService::stop() {
    for (const auto& handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
}

void CoreTextEmbeddingService::exportSkeleton() {
    delegate_ = ai_runtime_core_text_embedding_service_skeleton_new();

    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init", G_CALLBACK(onHandleInitEngine), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-init-v2", G_CALLBACK(onHandleInitEngineV2), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-init-model", G_CALLBACK(onHandleInitModel), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-embedding-text", G_CALLBACK(onHandleEmbeddingText), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-model-list", G_CALLBACK(onHandleGetModelList), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-model-info", G_CALLBACK(onHandleGetModelInfo), this);
    handlerIds_.push_back(handlerId);

    GError* error = nullptr;

    isExported_ = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_,
                                                   objectPath.c_str(), &error);
    if (!isExported_) {
        LOGE("Error creating server at address {} {}", objectPath, error->message);
        g_error_free(error);
    }
}

void CoreTextEmbeddingService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }

    g_object_unref(delegate_);
}

bool CoreTextEmbeddingService::onHandleInitEngineV2(AiRuntimeCoreTextEmbeddingService* delegate,
                                                    GDBusMethodInvocation* invocation, gpointer userData) {
    // init接口行为变化 新sdk + 新runtime sdk的init会调用init-v2进行通信

    std::shared_ptr<CoreTextEmbeddingService> textEmbeddingService =
        static_cast<CoreTextEmbeddingService*>(userData)->getSharedObject();
    if (!textEmbeddingService) {
        LOGE("[onHandleInitEngineV2] error: textEmbeddingService is nullptr!");
        ai_runtime_core_text_embedding_service_complete_init_v2(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    int sessionId = textEmbeddingService->generateRandomSessionId();
    ModelConfigManager::getInstance().waitForModelInfosLoaded();
    auto textEmbeddingEngine = textEmbeddingService->selectTextEmbeddingEngine("Embedding");
    if (!textEmbeddingEngine) {
        LOGE("[onHandleInitEngineV2] error: engineName: {} does not exist!", "Embedding");
        textEmbeddingService->releaseRandomSessionId(sessionId);
        ai_runtime_core_text_embedding_service_complete_init_v2(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    auto textEmbedding = std::make_shared<TextEmbedding>(std::move(textEmbeddingEngine));
    auto ret = textEmbedding->init();
    if (!ret) {
        LOGE("[onHandleInitEngineV2] error: call init error");
        textEmbeddingService->releaseRandomSessionId(sessionId);
        ai_runtime_core_text_embedding_service_complete_init_v2(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    textEmbeddingService->textEmbeddings_[sessionId] = textEmbedding;
    LOGI("[onHandleInitEngineV2] service: The sessionId of the engine is {}", sessionId);
    ai_runtime_core_text_embedding_service_complete_init_v2(delegate, invocation, sessionId, AI_COMMON_SUCCESS);
    return true;
}

bool CoreTextEmbeddingService::onHandleInitEngine(AiRuntimeCoreTextEmbeddingService* delegate,
                                                  GDBusMethodInvocation* invocation, const gchar* inputJson,
                                                  gpointer userData) {
    // 新sdk init行为变化，所以在新的runtime中要保证兼容 用于旧sdk+新runtime场景 客户端init后
    // runtime handle-init的回调函数 仍然去获取默认模型并加载
    std::shared_ptr<CoreTextEmbeddingService> service =
        static_cast<CoreTextEmbeddingService*>(userData)->getSharedObject();
    if (!service) {
        LOGE("[onHandleInitEngine] error: textEmbeddingService is nullptr!");
        ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    int sessionId = service->generateRandomSessionId();
    ModelConfigManager::getInstance().waitForModelInfosLoaded();
    auto textEmbeddingEngine = service->selectTextEmbeddingEngine("Embedding");
    if (!textEmbeddingEngine) {
        LOGE("[onHandleInitEngine] error: engineName: {} does not exist!", "Embedding");
        service->releaseRandomSessionId(sessionId);
        ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    auto textEmbedding = std::make_shared<TextEmbedding>(std::move(textEmbeddingEngine));
    auto ret = textEmbedding->init();
    if (!ret) {
        LOGE("[onHandleInitEngine] error: call init error");
        service->releaseRandomSessionId(sessionId);
        ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    // 兼容旧sdk + 新runtime 旧的sdk init接口在runtime中的行为会去加载模型
    const std::string modelInfo = textEmbedding->getModelInfo();
    std::string modelName = "";
    if (service->getDefaultModel(modelInfo, modelName) == false) {
        service->releaseRandomSessionId(sessionId);
        LOGE("[onHandleInitEngine] error: get default text embedding model error!");
        ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    ret = textEmbedding->setCurrentTextEmbeddingModel(modelName);
    if (!ret) {
        LOGE("[onHandleInitEngine] error: call init model error");
        service->releaseRandomSessionId(sessionId);
        ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    service->textEmbeddings_[sessionId] = textEmbedding;
    LOGI("[onHandleInitEngine] service: The sessionId of the engine is {}", sessionId);
    ai_runtime_core_text_embedding_service_complete_init(delegate, invocation, sessionId, AI_COMMON_SUCCESS);
    return true;
}

bool CoreTextEmbeddingService::onHandleInitModel(AiRuntimeCoreTextEmbeddingService* delegate,
                                                 GDBusMethodInvocation* invocation, gint sessionId, const gchar* model,
                                                 gpointer userData) {
    std::shared_ptr<CoreTextEmbeddingService> textEmbeddingService =
        static_cast<CoreTextEmbeddingService*>(userData)->getSharedObject();
    if (!textEmbeddingService) {
        LOGE("[onHandleInitModel] error: textEmbeddingService is nullptr!");
        ai_runtime_core_text_embedding_service_complete_init_model(delegate, invocation, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    if (!textEmbeddingService->isTextEmbeddingSessionIdValid(sessionId)) {
        LOGE("[onHandleInitModel] error: sessionId is Invalid!");
        ai_runtime_core_text_embedding_service_complete_init_model(delegate, invocation, AI_COMMON_RUNTIME_ERROR);
        return true;
    }

    auto ret = textEmbeddingService->textEmbeddings_[sessionId]->setCurrentTextEmbeddingModel(model);
    if (!ret) {
        LOGE("[onHandleInitModel] error: call init model error");
        ai_runtime_core_text_embedding_service_complete_init_model(delegate, invocation, AI_COMMON_MODEL_NOT_FOUND);
        return true;
    }
    ai_runtime_core_text_embedding_service_complete_init_model(delegate, invocation, AI_COMMON_SUCCESS);
    return true;
}

int CoreTextEmbeddingService::generateRandomSessionId() {
    uint32_t sessionId;
    randombytes_buf(&sessionId, sizeof(sessionId));
    sessionId = 1 + (sessionId & INT_MAX);
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        randombytes_buf(&sessionId, sizeof(sessionId));
        sessionId = 1 + (sessionId & INT_MAX);
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

std::string CoreTextEmbeddingService::getProtocolVersion() {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, get protocol version failed.");
        return kDefaultSdkVersion;
    }

    GDBusInterfaceInfo* interfaceInfo = ai_runtime_core_text_embedding_service_interface_info();
    if (interfaceInfo == nullptr) {
        LOGW("Interface info is null, use default protocol version.");
        return kDefaultSdkVersion;
    }
    if (interfaceInfo->annotations == nullptr) {
        LOGW("Protocol version annotation not found, use default protocol version.");
        return kDefaultSdkVersion;
    }

    for (GDBusAnnotationInfo** annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation) == 0) {
            return (*annotation)->value;
        }
    }
    return kDefaultSdkVersion;
}
void CoreTextEmbeddingService::setRuntimeProtocolVersion(const std::string& version) {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, set runtime version failed.");
        return;
    }
    ai_runtime_core_text_embedding_service_set_runtime_version(delegate_, version.c_str());
}

bool CoreTextEmbeddingService::onHandleEmbeddingText(AiRuntimeCoreTextEmbeddingService* delegate,
                                                     GDBusMethodInvocation* invocation, const gchar* inputJson,
                                                     gpointer userData) {
    std::shared_ptr<CoreTextEmbeddingService> textEmbeddingService =
        static_cast<CoreTextEmbeddingService*>(userData)->getSharedObject();
    if (!textEmbeddingService) {
        LOGE("[onHandleEmbeddingText] error: textEmbeddingService is nullptr!");

        std::string response = createResponse({}, AI_COMMON_RUNTIME_ERROR,
                                              "[onHandleEmbeddingText] error: textEmbeddingService is nullptr!");
        ai_runtime_core_text_embedding_service_complete_embedding_text(delegate, invocation, response.c_str());
        return true;
    }

    // parse input json
    std::string input(inputJson);
    std::string text;
    int sessionId;
    if (!textEmbeddingService->parseTextEmbeddingInput(input, text, sessionId)) {
        LOGE("[onHandleEmbeddingText] error: parse input json failed!");
        std::string resultJson =
            createResponse({}, AI_COMMON_RUNTIME_ERROR, "[onHandleEmbeddingText] error: parse input json failed!");
        ai_runtime_core_text_embedding_service_complete_embedding_text(delegate, invocation, resultJson.c_str());
        return true;
    }

    if (!textEmbeddingService->isTextEmbeddingSessionIdValid(sessionId)) {
        LOGE("[onHandleEmbeddingText] error: TextEmbeddingEngine is nullptr!");
        std::string resultJson = createResponse({}, AI_COMMON_RUNTIME_ERROR,
                                                "[onHandleEmbeddingText] error: TextEmbeddingEngine is nullptr!");
        ai_runtime_core_text_embedding_service_complete_embedding_text(delegate, invocation, resultJson.c_str());
        return true;
    }
    auto requestState = std::make_shared<std::atomic_bool>(false);
    auto callback = [delegate, invocation, requestState](const TextEmbeddingResult& result) {
        {
            if (requestState->exchange(true)) {
                return;
            }
            if (result.embeddings.empty()) {
                std::string errorMessage = result.errorMessage;
                LOGE("[onHandleEmbeddingText] embedding engine error: errorMessage : {}", errorMessage);
                std::string resultJson = createResponse({}, AI_COMMON_RUNTIME_ERROR,
                                                        "[onHandleEmbeddingText] embedding engine error: "
                                                        "errorMessage : " +
                                                            errorMessage);
                ai_runtime_core_text_embedding_service_complete_embedding_text(delegate, invocation,
                                                                               resultJson.c_str());
                return;
            }
            std::string resultJson = createResponse(result.embeddings, AI_COMMON_SUCCESS, "Success");

            ai_runtime_core_text_embedding_service_complete_embedding_text(delegate, invocation, resultJson.c_str());
        }
    };

    textEmbeddingService->textEmbeddings_[sessionId]->embeddingText(text, callback);
    return true;
}

bool CoreTextEmbeddingService::onHandleGetModelList(AiRuntimeCoreTextEmbeddingService* delegate,
                                                    GDBusMethodInvocation* invocation, gint sessionId,
                                                    gpointer userData) {
    std::shared_ptr<CoreTextEmbeddingService> textEmbeddingService =
        static_cast<CoreTextEmbeddingService*>(userData)->getSharedObject();

    if (!textEmbeddingService->isTextEmbeddingSessionIdValid(sessionId)) {
        LOGE("[onHandleGetModelList] error: sessionId is Invalid!");
        ai_runtime_core_text_embedding_service_complete_get_model_list(delegate, invocation, "{}");
        return true;
    }

    std::string modelInfo = textEmbeddingService->textEmbeddings_[sessionId]->getModelInfo();
    ai_runtime_core_text_embedding_service_complete_get_model_list(delegate, invocation, modelInfo.c_str());
    return true;
}

bool CoreTextEmbeddingService::onHandleGetModelInfo(AiRuntimeCoreTextEmbeddingService* delegate,
                                                    GDBusMethodInvocation* invocation, gint sessionId,
                                                    gpointer userData) {
    // 该接口后续版本会删了，为了保证旧sdk+新runtime兼容性 这里写死了
    Json::Value modelInfo;
    Json::Value textModel;
    textModel["name"] = "ensemble-embd_gte-base_uint8-text";
    textModel["dim"] = 768;
    textModel["ondevice"] = true;
    Json::Value imageModel;
    imageModel["image_text_name"] = "ensemble-embd_cn-clip_512-uint8-text";
    imageModel["image_name"] = "embd_cn-clip_512-uint8-image";
    imageModel["dim"] = 1024;
    imageModel["ondevice"] = true;

    Json::Value models;
    models["text_model"] = textModel;
    models["image_model"] = imageModel;
    modelInfo["models"] = models;

    ai_runtime_core_text_embedding_service_complete_get_model_info(delegate, invocation,
                                                                   modelInfo.toStyledString().c_str());
    return true;
}

bool CoreTextEmbeddingService::parseTextEmbeddingInput(const std::string& jsonInput, std::string& text,
                                                       int& sessionId) const {
    Json::Value root;
    Json::Reader r;
    bool isReader = r.parse(jsonInput, root);
    if (!isReader) {
        LOGE("[parseTextEmbeddingInput] error: Failed to parse JSON input.");
        return false;
    }

    if (!root.isMember("text") || !root["text"].isString()) {
        LOGE("[parseTextEmbeddingInput] error: Key 'text' not found or not a string.");
        return false;
    }

    if (!root.isMember("sessionId") || !root["sessionId"].isInt()) {
        LOGE("[parseTextEmbeddingInput] error: Key 'sessionId' not found or not a int.");
        return false;
    }

    text = root["text"].asString();
    sessionId = root["sessionId"].asInt();
    return true;
}

void CoreTextEmbeddingService::releaseRandomSessionId(int sessionId) { sessionIdSet_.erase(sessionId); }

bool CoreTextEmbeddingService::isTextEmbeddingSessionIdValid(int sessionId) {
    return textEmbeddings_.find(sessionId) != textEmbeddings_.end();
}

bool CoreTextEmbeddingService::getDefaultModel(const std::string& modelInfo, std::string& modelName) {
    // 获取文本向量化的默认模型 认为priority==1 的模型是默认模型
    Json::Value root;
    Json::Reader reader;
    bool isReader = reader.parse(modelInfo, root, false);

    if (!isReader) {
        LOGE("[getDefaultModel] error: Failed to parse JSON. Error: %s", reader.getFormattedErrorMessages().c_str());
        return false;
    }

    if (!root.isMember("models") || !root["models"].isArray()) {
        LOGE("[getDefaultModel] error: 'models' key missing or not an array.");
        return false;
    }

    const Json::Value& models = root["models"];
    if (models.empty()) {
        LOGE("[getDefaultModel] error: 'models' array is empty.");
        return false;
    }

    for (const auto& model : models) {
        if (!model.isMember("model_capability") || !model["model_capability"].isString() ||
            model["model_capability"].asString() != "TEXT") {
            continue;
        }

        if (!model.isMember("priority") || !model["priority"].isInt() || model["priority"].asInt() != 1) {
            // 只拿优先级为1的
            continue;
        }

        if (!model.isMember("model") || !model["model"].isString()) {
            continue;
        }

        modelName = model["model"].asString();
        break;
    }

    if (modelName == "") {
        // 没有找到默认模型，返回false
        return false;
    }

    return true;
}

const std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine>
CoreTextEmbeddingService::selectTextEmbeddingEngine(std::string engineName) const {
    return engineSelector_.selectCoreAiEmbeddingEngine(engineName);
}
