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

#include "configservice.h"

#include <jsoncpp/json/json.h>

#include "modelconfig/modelstatustracker.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
const char *objectPath = "/com/kylin/AiRuntime/Config";
const char *interface = "com.kylin.AiRuntime.Config";

std::string modelsToJsonString(const std::vector<Model> &models) {
    Json::Value modelsArray;
    for (auto &model : models) {
        Json::Value modelJson;
        modelJson["capability"] = (int)model.capability;
        modelJson["deployType"] = (int)model.deployType;
        modelJson["debName"] = model.debName;
        modelJson["brand"] = model.brand;
        modelJson["is3rdModel"] = model.is3rdModel;
        modelJson["modelName"] = model.displayName;  // 历史原因，最开始是displayName作为modelName
        modelJson["vendor"] = model.vendor;
        modelJson["modelVersion"] = model.modelVersion;
        modelJson["apiUrl"] = model.apiUrl;
        if (model.available.has_value()) {
            modelJson["available"] = model.available.value();
        }
        modelsArray.append(modelJson);
    }
    return util::jsonToString(modelsArray);
}

std::string authensToJsonString(const std::vector<MultiAuthentication> &auths) {
    Json::Value authsArray;
    for (auto &auth : auths) {
        Json::Value authJson;
        authJson["itemName"] = auth.itemName;
        authJson["enabled"] = auth.enabled;
        for (auto &authen : auth.authentications) {
            Json::Value authenJson;
            authenJson["friendlyKeyName"] = authen.friendlyKeyName;
            authenJson["key"] = authen.key;
            authenJson["value"] = authen.value;
            authJson["authentications"].append(authenJson);
        }
        authsArray.append(authJson);
    }
    return util::jsonToString(authsArray);
}

std::vector<MultiAuthentication> jsonStringToAuthentications(const std::string &jsonString) {
    const Json::Value jsonAuthens = util::stringToJson(jsonString);

    std::vector<MultiAuthentication> multiAuthentications;
    for (const auto &authen : jsonAuthens) {
        const std::string itemName = authen["itemName"].asString();
        const Json::Value jsonAuthentications = authen["authentications"];
        bool enabled{true};
        if (authen.isMember("enabled") && authen["enabled"].isBool()) {
            enabled = authen["enabled"].asBool();
        }
        std::vector<Authentication> authentications;
        for (const auto &item : jsonAuthentications) {
            std::string keyValue = item["value"].asString();
            util::removeZeroWidthChar(keyValue);
            authentications.push_back({item["friendlyKeyName"].asString(), item["key"].asString(), keyValue});
        }

        multiAuthentications.push_back({itemName, authentications, enabled});
    }
    return multiAuthentications;
}

}  // namespace

ConfigService::ConfigService(GDBusConnection &connection, ModelConfigManager &modelConfigManager)
    : connection_(connection), modelManager_(modelConfigManager) {
    exportSkeleton();
}

ConfigService::~ConfigService() { unexportSkeleton(); }

void ConfigService::stop() {
    for (const auto &handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
}

void ConfigService::exportSkeleton() {
    delegate_ = ai_runtime_config_service_skeleton_new();
    connectSignals();

    GError *error = nullptr;

    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

void ConfigService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    g_object_unref(delegate_);
}

void ConfigService::connectSignals() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-get-model-status", G_CALLBACK(onHandleGetModelStatus), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-get-models-by-capability", G_CALLBACK(onHandleGetModelsByCapability), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-models-by-deploy-type",
                                 G_CALLBACK(onHandleGetModelsByDeployType), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-get-current-model-name", G_CALLBACK(onHandleGetCurrentModelName), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-get-model-deploy-enabled", G_CALLBACK(onHandleGetModelDeployEnabled), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-set-current-model-name", G_CALLBACK(onHandleSetCurrentModelName), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-set-model-authentication",
                                 G_CALLBACK(onHandleSetModelAuthentications), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-set-model-scheduling-priority",
                                 G_CALLBACK(onHandleSetModelSchedulingPriority), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-model-scheduling-priority",
                                 G_CALLBACK(onHandleGetModelSchedulingPriority), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-model-authentications",
                                 G_CALLBACK(onHandleGetModelAuthentications), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-set-model-deploy-enabled", G_CALLBACK(onHandleSetModelDeployEnabled), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-clear-model-authentication",
                                 G_CALLBACK(onHandleClearModelAuthentications), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-add-custom-model", G_CALLBACK(onHandleAddCustomModel), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-delete-custom-model", G_CALLBACK(onHandleDeleteCustomModel), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-set-custom-model-version", G_CALLBACK(onHandleSetCustomModelVersion), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-set-custom-model-api-url", G_CALLBACK(onHandleSetCustomModelApiUrl), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-get-free-trial-models", G_CALLBACK(onHandleGetFreeTrialModels), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle_set_shared_model_name", G_CALLBACK(onHandleSetSharedModelName), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle_stop_model_sharing", G_CALLBACK(onHandleStopModelSharing), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle_get_shared_model_name", G_CALLBACK(onHandleGetSharedModelName), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-load-on-boot", G_CALLBACK(onHandleGetLoadOnBoot), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-set-load-on-boot", G_CALLBACK(onHandleSetLoadOnBoot), this);
    handlerIds_.push_back(handlerId);
}

bool ConfigService::onHandleGetModelStatus(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                           gchar *displayName, gpointer userData) {
    if (displayName == nullptr || strlen(displayName) == 0) {
        LOGW("Invalid model name provided.");
        ai_runtime_config_service_complete_get_model_status(service, invocation, gint(ModelStatus::Unknown));
        return true;
    }
    ConfigService *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    if (modelName.empty()) {
        LOGW("Model name not found: {}.", displayName);
        ai_runtime_config_service_complete_get_model_status(service, invocation, gint(ModelStatus::Unknown));
        return true;
    }
    ModelStatus status = ModelStatusTracker::modelStatus(modelName);
    LOGI("ModelName: {} DisplayName: {} Status:{}.", modelName, displayName, int(status));
    ai_runtime_config_service_complete_get_model_status(service, invocation, gint(status));
    return true;
}

bool ConfigService::onHandleGetModelsByCapability(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  int capability, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    auto models = sharedConfigService->modelManager_.getModels(AiCapability(capability));
    const std::string modelsJsonString = modelsToJsonString(models);
    ai_runtime_config_service_complete_get_models_by_capability(service, invocation, modelsJsonString.c_str());
    return true;
}

bool ConfigService::onHandleGetModelsByDeployType(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  int deployType, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    auto models = sharedConfigService->modelManager_.getModels(DeployType(deployType));
    const std::string modelsJsonString = modelsToJsonString(models);
    ai_runtime_config_service_complete_get_models_by_deploy_type(service, invocation, modelsJsonString.c_str());
    return true;
}

bool ConfigService::onHandleGetCurrentModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                gint capability, gint deployType, gpointer userData) {
    ConfigService *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName =
        sharedConfigService->modelManager_.getCurrentModelName(AiCapability(capability), DeployType(deployType));
    std::string displayName = sharedConfigService->modelManager_.getDisplayName(modelName);
    LOGI("GetCurrentModelName: {}, Capability: {}, DeployType: {}.", displayName, capability, deployType);
    ai_runtime_config_service_complete_get_current_model_name(service, invocation, displayName.c_str());
    return true;
}

bool ConfigService::onHandleGetModelAuthentications(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                    gchar *displayName, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    std::vector<MultiAuthentication> multiAuthentications =
        sharedConfigService->modelManager_.getModelAuthentications(modelName);
    std::string multiAuthenticationsString = authensToJsonString(multiAuthentications);
    ai_runtime_config_service_complete_get_model_authentications(service, invocation,
                                                                 multiAuthenticationsString.c_str());
    return true;
}

bool ConfigService::onHandleSetCurrentModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                gint capability, gint deployType, gchar *displayName,
                                                gpointer userData) {
    ConfigService *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    bool success = sharedConfigService->modelManager_.setCurrentModelName(AiCapability(capability),
                                                                          DeployType(deployType), displayName);
    LOGI("SetCurrentModelName: {}, Capability: {}, DeployType: {}.", displayName, capability, deployType);
    ai_runtime_config_service_complete_set_current_model_name(service, invocation, success);
    return true;
}

bool ConfigService::onHandleGetModelDeployEnabled(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  gint deployType, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    bool enabled = sharedConfigService->modelManager_.getModelDeployEnabled((DeployType)deployType);
    ai_runtime_config_service_complete_get_model_deploy_enabled(service, invocation, enabled);
    return true;
}

bool ConfigService::onHandleSetModelAuthentications(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                    gchar *displayName, gchar *authentications, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    auto modelAuthentications = jsonStringToAuthentications(authentications);
    sharedConfigService->modelManager_.setModelAuthentications(modelName, modelAuthentications);
    ai_runtime_config_service_complete_set_model_authentication(service, invocation);
    return true;
}

bool ConfigService::onHandleSetCustomModelApiUrl(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                 gchar *displayName, gchar *apiUrl, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    bool success = sharedConfigService->modelManager_.setCustomModelApiUrl(modelName, apiUrl);
    ai_runtime_config_service_complete_set_custom_model_api_url(service, invocation, success);
    return true;
}

bool ConfigService::onHandleSetCustomModelVersion(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  gchar *displayName, gchar *modelVersion, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    bool success = sharedConfigService->modelManager_.setCustomModelVersion(modelName, modelVersion);
    ai_runtime_config_service_complete_set_custom_model_version(service, invocation, success);
    return true;
}

bool ConfigService::onHandleClearModelAuthentications(AiRuntimeConfigService *service,
                                                      GDBusMethodInvocation *invocation, gchar *displayName,
                                                      gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    bool success = sharedConfigService->modelManager_.clearModelAuthentications(modelName);
    if (not success) {
        LOGE("clearModelAuthentications error");
    }
    ai_runtime_config_service_complete_clear_model_authentication(service, invocation);
    return true;
}

bool ConfigService::onHandleSetModelSchedulingPriority(AiRuntimeConfigService *service,
                                                       GDBusMethodInvocation *invocation, gint deployType,
                                                       gint priority, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    sharedConfigService->modelManager_.setModelSchedulingPriority((DeployType)deployType, priority);
    ai_runtime_config_service_complete_set_model_authentication(service, invocation);
    return true;
}

bool ConfigService::onHandleGetModelSchedulingPriority(AiRuntimeConfigService *service,
                                                       GDBusMethodInvocation *invocation, gint deployType,
                                                       gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    int priority = sharedConfigService->modelManager_.getModelSchedulingPriority((DeployType)deployType);
    ai_runtime_config_service_complete_get_model_scheduling_priority(service, invocation, priority);
    return true;
}

bool ConfigService::onHandleSetModelDeployEnabled(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  gint deployType, gboolean enabled, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    sharedConfigService->modelManager_.setModelDeployEnabled((DeployType)deployType, enabled);
    ai_runtime_config_service_complete_set_model_deploy_enabled(service, invocation);
    return true;
}

bool ConfigService::onHandleAddCustomModel(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                           gchar *modelName, gchar *modelVersion, gchar *apiKey, gchar *apiUrl,
                                           gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    bool success = sharedConfigService->modelManager_.addCustomModel(modelName, modelVersion, apiKey, apiUrl);
    ai_runtime_config_service_complete_add_custom_model(service, invocation, success);
    return true;
}

bool ConfigService::onHandleDeleteCustomModel(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              gchar *displayName, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelName = sharedConfigService->modelManager_.getModelNameByDisplayName(displayName);
    bool success = sharedConfigService->modelManager_.deleteCustomModel(modelName);
    ai_runtime_config_service_complete_delete_custom_model(service, invocation, success);
    return true;
}

bool ConfigService::onHandleGetFreeTrialModels(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                               gint capability, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    auto models = configService->modelManager_.getFreeTrialModels();
    const std::string modelsJsonString = modelsToJsonString(models);

    LOGI("GetFreeTrialModels: {}, Capability: {}.", modelsJsonString, capability);

    ai_runtime_config_service_complete_get_free_trial_models(service, invocation, modelsJsonString.c_str());
    return true;
}

bool ConfigService::onHandleSetSharedModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                               gchar *modelName, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    bool success = configService->modelManager_.setSharedModelName(modelName);

    ai_runtime_config_service_complete_set_shared_model_name(service, invocation, success);
    return true;
}

bool ConfigService::onHandleStopModelSharing(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                             gchar *modelName, gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    bool success = configService->modelManager_.stopModelSharing(modelName);

    ai_runtime_config_service_complete_stop_model_sharing(service, invocation, success);
    return true;
}

bool ConfigService::onHandleGetSharedModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                               gpointer userData) {
    auto *configService = static_cast<ConfigService *>(userData);
    const std::string modelName = configService->modelManager_.getSharedModelName();

    ai_runtime_config_service_complete_get_shared_model_name(service, invocation, modelName.c_str());
    return true;
}

bool ConfigService::onHandleGetLoadOnBoot(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                          gchar *model, gpointer userData) {
    if (model == nullptr || strlen(model) == 0) {
        LOGW("Invalid capability parameter.");
        ai_runtime_config_service_complete_get_load_on_boot(service, invocation, FALSE);
        return true;
    }

    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();
    std::string modelStr = model;
    std::string capStr, subTypeStr;
    if (modelStr == "speechToText") {
        capStr = "Speech";
        subTypeStr = "speechToText";
    } else if (modelStr == "textToSpeech") {
        capStr = "Speech";
        subTypeStr = "textToSpeech";
    } else {
        LOGW("Invalid capability parameter{}", modelStr);
        ai_runtime_config_service_complete_get_load_on_boot(service, invocation, FALSE);
        return true;
    }
    bool enabled = sharedConfigService->modelManager_.getLoadOnBoot(capStr, subTypeStr);

    LOGI("GetLoadOnBoot - Capability: {}, SubType: {}, Enabled: {}", capStr, subTypeStr, enabled);

    ai_runtime_config_service_complete_get_load_on_boot(service, invocation, enabled ? TRUE : FALSE);
    return true;
}

bool ConfigService::onHandleSetLoadOnBoot(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                          gchar *model, gboolean enabled, gpointer userData) {
    if (model == nullptr || strlen(model) == 0) {
        LOGW("Invalid capability parameter.");
        ai_runtime_config_service_complete_set_load_on_boot(service, invocation);
        return true;
    }

    auto *configService = static_cast<ConfigService *>(userData);
    std::shared_ptr sharedConfigService = configService->getSharedObject();

    std::string modelStr = model;
    std::string capStr, subTypeStr;
    if (modelStr == "speechToText") {
        capStr = "Speech";
        subTypeStr = "speechToText";
    } else if (modelStr == "textToSpeech") {
        capStr = "Speech";
        subTypeStr = "textToSpeech";
    } else {
        LOGW("Invalid capability parameter:{}", modelStr);
        ai_runtime_config_service_complete_set_load_on_boot(service, invocation);
        return true;
    }
    bool isEnabled = enabled ? true : false;

    sharedConfigService->modelManager_.setLoadOnBoot(capStr, subTypeStr, isEnabled);

    LOGI("SetLoadOnBoot - Capability: {}, SubType: {}, Enabled: {}", capStr, subTypeStr, isEnabled);

    ai_runtime_config_service_complete_set_load_on_boot(service, invocation);
    return true;
}
