/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include "configserviceproxy.h"

#include <jsoncpp/json/json.h>

#include <optional>

namespace kyai {
namespace config {
namespace model {

const std::string serverUnixPath = "unix:path=/tmp/.kylin-ai-runtime-unix/" +
                                   std::to_string(getuid()) + "/config.sock";

const char *objectPath = "/com/kylin/AiRuntime/Config";
const char *interface = "com.kylin.AiRuntime.Config";

Json::Value parseJsonString(const std::string &jsonString) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(jsonString, root)) {
        std::fprintf(stderr, "Error parsing JSON string\n");
        return {};
    }
    return root;
}

std::string jsonToString(const Json::Value &value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}

std::optional<Json::Value> stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return std::nullopt;
    }
}

std::vector<Model> createModelsFromJsonString(const std::string &jsonString) {
    const auto jsonModels = parseJsonString(jsonString);
    if (jsonModels.empty()) {
        return {};
    }

    std::vector<Model> models;
    for (const auto &model : jsonModels) {
        Model m;
        m.capability = static_cast<AiCapability>(model["capability"].asInt());
        m.deployType = static_cast<DeployType>(model["deployType"].asInt());
        m.modelName = model["modelName"].asString();
        // 为了版本兼容性，使用 vendor 传品牌名称，用于显示图标
        m.vendor = model.isMember("brand") ? model["brand"].asString()
                                           : model["vendor"].asString();
        models.push_back(m);
    }
    return models;
}

std::vector<ExtendedModel> createExtendedModelsFromJsonString(
    const std::string &jsonString) {
    const auto jsonModels = parseJsonString(jsonString);
    if (jsonModels.empty()) {
        return {};
    }

    std::vector<ExtendedModel> models;
    for (const auto &model : jsonModels) {
        ExtendedModel m;
        m.capability = static_cast<AiCapability>(model["capability"].asInt());
        m.deployType = static_cast<DeployType>(model["deployType"].asInt());
        m.modelName = model["modelName"].asString();
        // 为了版本兼容性，使用 vendor 传品牌名称，用于显示图标
        m.vendor = model.isMember("brand") ? model["brand"].asString()
                                           : model["vendor"].asString();
        m.modelVersion = model["modelVersion"].asString();
        m.apiUrl = model["apiUrl"].asString();
        m.debName = model["debName"].asString();
        m.is3rdModel = model["is3rdModel"].asBool();
        // 端侧后摩大语言模型返回的模型信息才有这个字段，未有这个字段默认值设置为true
        m.available =
            !model.isMember("available") || model["available"].asBool();
        models.push_back(m);
    }
    return models;
}

// 样例：
// [
//     {
//         "authentications" :    //         [
//             {
//                 "friendlyKeyName" : "APPID",
//                 "key" : "appIdContinuous",
//                 "value" : "01234567890123456789"
//             },
//             {
//                 "friendlyKeyName" : "APIKey",
//                 "key" : "apiKeyContinuous",
//                 "value" : "01234567890123456789"
//             },
//             {
//                 "friendlyKeyName" : "APISecret",
//                 "key" : "secretKeyContinuous",
//                 "value" : "01234567890123456789"
//             }
//         ],
//         "itemName" : "语音听写&语音合成",
//         "enabled" : true
//     },
//     {
//         "authentications" :
//         [
//             {
//                 "friendlyKeyName" : "APPID",
//                 "key" : "appIdRealTime",
//                 "value" : "01234567890123456789"
//             },
//             {
//                 "friendlyKeyName" : "APIKey",
//                 "key" : "apiKeyRealTime",
//                 "value" : "01234567890123456789"
//             }
//         ],
//         "itemName" : "实时语音转写",
//         "enabled" : true
//     }
// ]
std::string authenticationsToJsonString(
    const std::vector<MultiAuthentication> &authentications) {
    Json::Value root;
    for (const auto &auth : authentications) {
        Json::Value authen;
        authen["itemName"] = auth.itemName;
        authen["enabled"] = true;
        for (const auto &auth : auth.authentications) {
            Json::Value value;
            value["friendlyKeyName"] = auth.friendlyKeyName;
            value["key"] = auth.key;
            value["value"] = auth.value;
            authen["authentications"].append(value);
        }
        root.append(authen);
    }
    return jsonToString(root);
}

std::string extendedMultiAuthenticationToJsonString(
    const std::vector<ExtendedMultiAuthentication>
        &extendedMultiAuthentications) {
    Json::Value root;
    for (const auto &extendedMultiAuthentication :
         extendedMultiAuthentications) {
        Json::Value item;
        item["itemName"] = extendedMultiAuthentication.itemName;
        item["enabled"] = extendedMultiAuthentication.enabled;
        for (const auto &authentication :
             extendedMultiAuthentication.authentications) {
            Json::Value value;
            value["friendlyKeyName"] = authentication.friendlyKeyName;
            value["key"] = authentication.key;
            value["value"] = authentication.value;
            item["authentications"].append(value);
        }
        root.append(item);
    }
    return jsonToString(root);
}

std::vector<MultiAuthentication> jsonStringToAuthentications(
    const std::string &jsonString) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(jsonString, root)) {
        std::fprintf(stderr, "Error parsing JSON string\n");
        return {};
    }
    std::vector<MultiAuthentication> authentications;
    for (const auto &auth : root) {
        MultiAuthentication au;
        if (!auth.isMember("itemName") || !auth.isMember("authentications") ||
            !auth["authentications"].isArray()) {
            std::fprintf(stderr, "Error parsing JSON string\n");
            return {};
        }
        au.itemName = auth["itemName"].asString();
        for (const auto &auth : auth["authentications"]) {
            Authentication a;
            a.friendlyKeyName = auth["friendlyKeyName"].asString();
            a.key = auth["key"].asString();
            a.value = auth["value"].asString();
            au.authentications.push_back(a);
        }
        authentications.push_back(au);
    }
    return authentications;
}

std::vector<ExtendedMultiAuthentication>
jsonStringToExtendedMutilAuthentications(const std::string &jsonString) {
    auto optional = stringToJson(jsonString);
    if (!optional.has_value()) {
        return std::vector<ExtendedMultiAuthentication>{};
    }
    Json::Value root = optional.value();
    std::vector<ExtendedMultiAuthentication> authentications;
    for (const auto &auth : root) {
        ExtendedMultiAuthentication au;
        if (!auth.isMember("itemName") || !auth.isMember("authentications") ||
            !auth["authentications"].isArray()) {
            std::fprintf(stderr, "Error parsing JSON string\n");
            return {};
        }
        au.itemName = auth["itemName"].asString();
        au.enabled = true;
        if (auth.isMember("enabled") && auth["enabled"].isBool()) {
            au.enabled = auth["enabled"].asBool();
        }
        for (const auto &auth : auth["authentications"]) {
            Authentication a;
            a.friendlyKeyName = auth["friendlyKeyName"].asString();
            a.key = auth["key"].asString();
            a.value = auth["value"].asString();
            au.authentications.push_back(a);
        }
        authentications.push_back(au);
    }
    return authentications;
}

ConfigServer &ConfigServer::getInstance() {
    static ConfigServer instance{};
    return instance;
}

ConfigServer::ConfigServer() { init(serverUnixPath); }

ConfigServer::~ConfigServer() { destroy(); }

void ConfigServer::init(const std::string &unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError *error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
        nullptr, /* GDBusAuthObserver */
        nullptr, /* GCancellable */
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n",
                   unixPath.c_str(), error->message);
        g_error_free(error);
        return;
    }
}

void ConfigServer::destroy() const {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

ConfigServiceProxy::ConfigServiceProxy() {
    if (!connectToServer()) {
        std::fprintf(stderr, "Failed to connect to server\n");
    }
}

bool ConfigServiceProxy::connectToServer() {
    if (!ConfigServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }

    GError *error = nullptr;
    auto *connection = ConfigServer::getInstance().getConnection();
    proxy_ = ai_runtime_config_service_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath, nullptr,
        &error);
    if (proxy_ == nullptr) {
        g_printerr("Error creating config service proxy %s: %s\n", objectPath,
                   error->message);
        g_error_free(error);
        return true;
    }

    return true;
}

std::vector<Model> ConfigServiceProxy::getModels(
    AiCapability capability) const {
    gchar *modelInfo{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_models_by_capability_sync(
        proxy_, (int)capability, &modelInfo, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelsByCapability: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelsByCapability\n");
        }
        return {};
    }
    auto models = createModelsFromJsonString(modelInfo);
    g_free(modelInfo);
    return models;
}

std::vector<Model> ConfigServiceProxy::getModels(DeployType deployType) const {
    gchar *modelInfo{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_models_by_deploy_type_sync(
        proxy_, (int)deployType, &modelInfo, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelsByDeployType: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelsByDeployType\n");
        }
        return {};
    }
    auto models = createModelsFromJsonString(modelInfo);
    g_free(modelInfo);
    return models;
}

std::string ConfigServiceProxy::getCurrentModelName(
    AiCapability capability, DeployType deployType) const {
    gchar *modelName{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_current_model_name_sync(
        proxy_, (int)capability, (int)deployType, &modelName, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetCurrentModelName: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetCurrentModelName\n");
        }
        return {};
    }
    std::string result = modelName;
    g_free(modelName);
    return result;
}

std::vector<MultiAuthentication> ConfigServiceProxy::getModelAuthentications(
    const std::string &modelName) const {
    gchar *authenticationsJsonString{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_model_authentications_sync(
        proxy_, modelName.c_str(), &authenticationsJsonString, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelAuthentications: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelAuthentications\n");
        }
        return {};
    }

    auto authentications =
        jsonStringToAuthentications(authenticationsJsonString);
    g_free(authenticationsJsonString);
    return authentications;
}

std::vector<ExtendedMultiAuthentication>
ConfigServiceProxy::getModelExtendedAuthentications(
    const std::string &modelName) const {
    gchar *extendedMultiAuthenticationsJsonString{nullptr};
    GError *error = nullptr;
    bool success =
        ai_runtime_config_service_call_get_model_authentications_sync(
            proxy_, modelName.c_str(), &extendedMultiAuthenticationsJsonString,
            nullptr, &error);
    if (!success) {
        if (error) {
            g_printerr("Error calling GetModelAuthentications: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelAuthentications\n");
        }
        return {};
    }
    auto extendedMultiAuthentications =
        jsonStringToExtendedMutilAuthentications(
            extendedMultiAuthenticationsJsonString);
    printf("extendedMultiAuthenticationsJsonString %s\n",
           extendedMultiAuthenticationsJsonString);
    g_free(extendedMultiAuthenticationsJsonString);
    return extendedMultiAuthentications;
}

int ConfigServiceProxy::getModelSchedulingPriority(
    DeployType deployType) const {
    int priority;
    GError *error = nullptr;
    bool ret =
        ai_runtime_config_service_call_get_model_scheduling_priority_sync(
            proxy_, (int)deployType, &priority, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelSchedulingPriority: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelSchedulingPriority\n");
        }
        return -1;
    }
    return priority;
}

bool ConfigServiceProxy::getModelDeployEnabled(DeployType deployType) const {
    gboolean enabled = false;
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_model_deploy_enabled_sync(
        proxy_, (int)deployType, &enabled, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelDeployEnabled: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelDeployEnabled\n");
        }
        return false;
    }

    return enabled;
}

bool ConfigServiceProxy::setCurrentModelName(AiCapability capability,
                                             DeployType deployType,
                                             const std::string &modelName) {
    gboolean success = false;
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_set_current_model_name_sync(
        proxy_, (int)capability, (int)deployType, modelName.c_str(), &success,
        nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetCurrentModelName: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetCurrentModelName\n");
        }
        return false;
    }
    return success;
}

bool ConfigServiceProxy::setModelAuthentications(
    const std::string &modelName,
    const std::vector<MultiAuthentication> authentications) {
    const std::string authenticationsJsonString =
        authenticationsToJsonString(authentications);
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_set_model_authentication_sync(
        proxy_, modelName.c_str(), authenticationsJsonString.c_str(), nullptr,
        &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetModelAuthentication: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetModelAuthentication\n");
        }
        return false;
    }
    return true;
}

bool ConfigServiceProxy::setModelAuthentications(
    const std::string &modelName, const std::vector<ExtendedMultiAuthentication>
                                      &extendedMultiAuthentications) const {
    std::string authenticationsJsonString =
        extendedMultiAuthenticationToJsonString(extendedMultiAuthentications);
    GError *error = nullptr;
    bool success = ai_runtime_config_service_call_set_model_authentication_sync(
        proxy_, modelName.c_str(), authenticationsJsonString.c_str(), nullptr,
        &error);
    if (!success) {
        if (error) {
            g_printerr("Error calling SetModelAuthentication: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetModelAuthentication\n");
        }
        return false;
    }
    return true;
}

bool ConfigServiceProxy::setCustomModelApiUrl(const std::string &modelName,
                                              const std::string &apiUrl) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_set_custom_model_api_url_sync(
        proxy_, modelName.c_str(), apiUrl.c_str(), &success, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetCustomModelApiUrl: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetCustomModelApiUrl\n");
        }
        return false;
    }
    return success;
}

bool ConfigServiceProxy::setCustomModelVersion(
    const std::string &modelName, const std::string &modelVersion) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_set_custom_model_version_sync(
        proxy_, modelName.c_str(), modelVersion.c_str(), &success, nullptr,
        &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetCustomModelVersion: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetCustomModelVersion\n");
        }
        return false;
    }
    return success;
}

bool ConfigServiceProxy::setModelSchedulingPriority(DeployType deployType,
                                                    int priority) {
    GError *error = nullptr;
    bool ret =
        ai_runtime_config_service_call_set_model_scheduling_priority_sync(
            proxy_, (int)deployType, priority, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetModelSchedulingPriority: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetModelSchedulingPriority\n");
        }
        return false;
    }
    return true;
}

bool ConfigServiceProxy::setModelDeployEnabled(DeployType deployType,
                                               bool enabled) {
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_set_model_deploy_enabled_sync(
        proxy_, (int)deployType, enabled, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetModelDeployEnabled: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetModelDeployEnabled\n");
        }
        return false;
    }
    return true;
}

bool ConfigServiceProxy::clearModelAuthentications(
    const std::string &modelName) {
    GError *error = nullptr;
    bool isOk = ai_runtime_config_service_call_clear_model_authentication_sync(
        proxy_, modelName.c_str(), nullptr, &error);
    if (!isOk) {
        if (error) {
            g_printerr("Error calling ClearModelAuthentications: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling ClearModelAuthentications\n");
        }
        return false;
    }
    return true;
}

ModelStatus ConfigServiceProxy::getModelStatus(
    const std::string &modelName) const {
    gint status = -1;
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_model_status_sync(
        proxy_, modelName.c_str(), &status, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetModelStatus: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetModelStatus\n");
        }
        return ModelStatus::Unknown;
    }
    return static_cast<ModelStatus>(status);
}

bool ConfigServiceProxy::addCustomModel(const std::string &modelName,
                                        const std::string &model,
                                        const std::string &apiKey,
                                        const std::string &apiUrl) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_add_custom_model_sync(
        proxy_, modelName.c_str(), model.c_str(), apiKey.c_str(),
        apiUrl.c_str(), &success, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling AddCustomModel: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling AddCustomModel\n");
        }
        return false;
    }

    return success;
}

bool ConfigServiceProxy::deleteCustomModel(const std::string &modelName) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_delete_custom_model_sync(
        proxy_, modelName.c_str(), &success, nullptr, &error);

    if (!ret) {
        if (error) {
            g_printerr("Error calling DeleteCustomModel: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling DeleteCustomModel\n");
        }
        return false;
    }

    return success;
}

std::vector<ExtendedModel> ConfigServiceProxy::getExtendedModels(
    AiCapability capability) const {
    gchar *modelInfo{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_models_by_capability_sync(
        proxy_, (int)capability, &modelInfo, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetExtendedModels by capability: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetExtendedModels by capability\n");
        }
        return {};
    }
    auto models = createExtendedModelsFromJsonString(modelInfo);
    g_free(modelInfo);
    return models;
}

std::vector<ExtendedModel> ConfigServiceProxy::getExtendedModels(
    DeployType deployType) const {
    gchar *modelInfo{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_models_by_deploy_type_sync(
        proxy_, (int)deployType, &modelInfo, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetExtendedModels by deploy type: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetExtendedModels by deploy type\n");
        }
        return {};
    }
    auto models = createExtendedModelsFromJsonString(modelInfo);
    g_free(modelInfo);
    return models;
}

std::vector<Model> ConfigServiceProxy::getFreeTrialModels(
    AiCapability capability) const {
    gchar *modelInfo{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_free_trial_models_sync(
        proxy_, (int)capability, &modelInfo, nullptr, &error);

    if (!ret) {
        if (error) {
            g_printerr("Error calling getFreeTrialModels: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            std::fprintf(stderr, "Error calling getFreeTrialModels\n");
        }
        return {};
    }
    auto models = createModelsFromJsonString(modelInfo);
    g_free(modelInfo);
    return models;
}

bool ConfigServiceProxy::setSharedModelName(const std::string &modelName) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_set_shared_model_name_sync(
        proxy_, modelName.c_str(), &success, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling setSharedModelName: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling setSharedModelName\n");
        }
        return false;
    }

    return success;
}

bool ConfigServiceProxy::stopModelSharing(const std::string &modelName) {
    GError *error = nullptr;
    gboolean success = false;
    bool ret = ai_runtime_config_service_call_stop_model_sharing_sync(
        proxy_, modelName.c_str(), &success, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling stopModelSharing: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling stopModelSharing\n");
        }
        return false;
    }

    return success;
}

std::string ConfigServiceProxy::getSharedModelName() const {
    gchar *modelName{nullptr};
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_shared_model_name_sync(
        proxy_, &modelName, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling getSharedModelName: %s\n",
                       error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling getSharedModelName\n");
        }
        return {};
    }
    std::string result = modelName;
    g_free(modelName);
    return result;
}

bool ConfigServiceProxy::getLoadOnBoot(const std::string &modelName) const {
    gboolean enabled = false;
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_get_load_on_boot_sync(
        proxy_, modelName.c_str(), &enabled, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling GetLoadOnBoot: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling GetLoadOnBoot\n");
        }
        return false;
    }
    return enabled;
}

bool ConfigServiceProxy::setLoadOnBoot(const std::string &modelName,
                                       bool enabled) {
    GError *error = nullptr;
    bool ret = ai_runtime_config_service_call_set_load_on_boot_sync(
        proxy_, modelName.c_str(), enabled, nullptr, &error);
    if (!ret) {
        if (error) {
            g_printerr("Error calling SetLoadOnBoot: %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Error calling SetLoadOnBoot\n");
        }
        return false;
    }
    return true;
}

}  // namespace model
}  // namespace config
}  // namespace kyai
