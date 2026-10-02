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

#include "modelconfigmanager.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>

#include <algorithm>
#include <chrono>
#include <map>

#include "encryption/encryption.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
const char *publicCloudSchemaId = "com.kylin.aisdk.model.cloud";
const char *customCloudSchemaId = "com.kylin.aisdk.model.custom";
const char *ondeviceSchemaId = "com.kylin.aisdk.model.ondevice";
const char *sharedSchemaId = "com.kylin.aisdk.model.shared";
const char *bootSchemaId = "com.kylin.aisdk.model.boot";

const char *sharedEnabled = "sharedenabled";

const std::string oldModelConfigFile = std::string(getenv("HOME")) + "/.config/model_manager.json";

const std::string baiduNlpModelName = "百度-ERNIE-Bot-4";
const std::string baiduVisionModelName = "百度-文心AI作画高级版";

thread_local bool isModelInfosLoaderThread = false;

std::set<std::string> disabledEngineNames = {"baichuan", "baidu", "deepseek", "qwen", "sensetime", "xunfei"};

const std::string defaultCloudNlpModelName = "kylin-qwen-plus";

std::optional<ModelInfo> createModelInfo(const Json::Value &modelInfoJson, AiCapability capability,
                                         DeployType deployType, const std::string &engineName,
                                         const std::string &vendor) {
    //  样例
    //  {
    //      "vendor": "通义千问",
    //      "models":
    //      [
    //          {
    //              "modelName": "ali-qwen-plus",
    //              "displayName": "通义千问-qwen-plus",
    //              "model": "qwen-plus",
    //              "subConfig":
    //              [
    //                  {
    //                      "name": "qwen-plus",
    //                      "config":
    //                      [
    //                          {
    //                              "friendlyName": "APIKey",
    //                              "configName": "apiKey"
    //                          }
    //                      ]
    //                  }
    //              ],
    //              "maxTokens": 8192,
    //              "feedbackUrl": "https://www.aliyun.com/service",
    //              "supportsWebSearch": true,
    //              "supportsThinking": true,
    //              "supportsThinkingToggle": true
    //          }
    //      ]
    //  }
    if (!modelInfoJson.isMember("modelName")) {
        LOGE("Invalid configs format: {}", util::jsonToString(modelInfoJson));
        return std::nullopt;
    }
    const std::string modelName = modelInfoJson["modelName"].asString();
    const std::string displayName = modelInfoJson["displayName"].asString();
    const std::string modelVersion = modelInfoJson["model"].asString();
    const std::string brand = modelInfoJson["brand"].asString();

    if (!modelInfoJson.isMember("subConfig") || !modelInfoJson["subConfig"].isArray()) {
        LOGE("Invalid configs format: {}", util::jsonToString(modelInfoJson));
        return std::nullopt;
    }

    std::vector<MultiAuthentication> multiAuthentications;
    for (const Json::Value &subConfigItem : modelInfoJson["subConfig"]) {
        if (!subConfigItem.isMember("name") || !subConfigItem.isMember("config") ||
            !subConfigItem["config"].isArray()) {
            LOGE("Invalid configs format: {}", util::jsonToString(modelInfoJson));
            return std::nullopt;
        }

        const std::string itemName = subConfigItem["name"].asString();
        const Json::Value &config = subConfigItem["config"];
        std::vector<Authentication> authentications;
        for (const Json::Value &configItem : config) {
            if (!configItem.isMember("friendlyName") || !configItem["configName"].isString()) {
                LOGE("Invalid configs format: {}", util::jsonToString(modelInfoJson));
                return std::nullopt;
            }
            Authentication authentication;
            authentication.friendlyKeyName = configItem["friendlyName"].asString();
            authentication.key = configItem["configName"].asString();
            authentication.value = std::string{};
            authentications.push_back(authentication);
        }
        multiAuthentications.push_back(MultiAuthentication{itemName, authentications});
    }
    const std::string debName = modelInfoJson["debName"].asString();
    const bool is3rdModel = modelInfoJson["is3rdModel"].asString() == "true";
    const int maxTokens = modelInfoJson["maxTokens"].asInt();

    bool supportsWebSearch{false};
    if (modelInfoJson.isMember("supportsWebSearch") && modelInfoJson["supportsWebSearch"].isBool()) {
        supportsWebSearch = modelInfoJson["supportsWebSearch"].asBool();
    }

    bool supportsThinking{false};
    if (modelInfoJson.isMember("supportsThinking") && modelInfoJson["supportsThinking"].isBool()) {
        supportsThinking = modelInfoJson["supportsThinking"].asBool();
    }

    bool supportsThinkingToggle{false};
    if (modelInfoJson.isMember("supportsThinkingToggle") && modelInfoJson["supportsThinkingToggle"].isBool()) {
        supportsThinkingToggle = modelInfoJson["supportsThinkingToggle"].asBool();
    }

    bool supportsToolChoice{false};
    if (modelInfoJson.isMember("supportsToolChoice") && modelInfoJson["supportsToolChoice"].isBool()) {
        supportsToolChoice = modelInfoJson["supportsToolChoice"].asBool();
    }

    Model model;
    model.modelName = modelName;
    model.displayName = displayName;
    model.modelVersion = modelVersion;
    model.apiUrl = std::string{};
    model.vendor = vendor;
    model.debName = debName;
    model.is3rdModel = is3rdModel;
    model.capability = capability;
    model.deployType = deployType;
    model.engineName = engineName;
    model.brand = brand;
    model.supportsWebSearch = supportsWebSearch;
    model.supportsThinking = supportsThinking;
    model.supportsThinkingToggle = supportsThinkingToggle;
    model.supportsToolChoice = supportsToolChoice;
    model.isFreetrialModel = engineName == "freetrial";
    if (modelInfoJson.isMember("available")) {
        model.available = modelInfoJson["available"].asBool();
    }

    return ModelInfo{model, multiAuthentications};
}

std::vector<ModelInfo> createModelInfosFromJsonString(const std::string &jsonString, AiCapability capability,
                                                      DeployType deployType, const std::string &engineName) {
    std::vector<ModelInfo> modelInfos;

    auto root = util::stringToJson(jsonString);
    if (!root.isObject()) {
        LOGE("Invalid configs format: {}", jsonString);
        return {};
    }

    if (!root.isMember("vendor") || !root.isMember("models") || !root["models"].isArray()) {
        LOGE("Invalid configs format: {}", jsonString);
        return {};
    }

    const std::string vendor = root["vendor"].asString();
    const auto models = root["models"];

    for (const auto &model : models) {
        auto info = createModelInfo(model, capability, deployType, engineName, vendor);
        if (info.has_value()) {
            modelInfos.push_back(info.value());
        }
    }

    return modelInfos;
}

std::vector<ModelInfo> createCustomModelInfosFromJsonString(const std::string &jsonString,
                                                            const std::string &engineName) {
    std::vector<ModelInfo> modelInfos;

    auto root = util::stringToJson(jsonString);
    if (!root.isObject()) {
        LOGE("Invalid configs format: {}", jsonString);
        return {};
    }

    const Json::Value::Members modelNames = root.getMemberNames();
    for (const auto &modelName : modelNames) {
        if (modelName == "currentModel") {
            continue;
        }

        const auto modelConfig = root[modelName];
        if (!modelConfig.isObject()) {
            LOGE("Invalid configs format for: {}", modelName);
            continue;
        }

        if (!modelConfig.isMember("apiUrl")) {
            LOGE("Invalid configs format: {}", modelConfig.toStyledString());
            continue;
        }
        Model model;
        model.modelName = modelName;
        model.displayName = modelConfig["displayName"].asString();
        model.modelVersion = modelConfig["model"].asString();
        model.apiUrl = modelConfig["apiUrl"].asString();
        model.vendor = std::string{};
        model.debName = std::string{};
        model.is3rdModel = false;
        model.capability = AiCapability::Nlp;
        model.deployType = DeployType::Custom;
        model.engineName = engineName;
        model.brand = "custom";
        std::vector<MultiAuthentication> multiAuthentications{
            {"", {{"APIKey", "apiKey", modelConfig["apiKey"].asString()}}},
        };

        modelInfos.push_back({model, multiAuthentications});
    }

    return modelInfos;
}

Json::Value authenticationsToJsonWithEncryption(const std::vector<MultiAuthentication> &authentications) {
    Json::Value root;
    for (const auto &auth : authentications) {
        Json::Value item;
        item["itemName"] = auth.itemName;
        item["enabled"] = auth.enabled;
        for (const auto &auth : auth.authentications) {
            Json::Value authenDetails;
            authenDetails["friendlyKeyName"] = auth.friendlyKeyName;
            authenDetails["key"] = auth.key;
            authenDetails["value"] = Encryption::getInstance().encrypt(auth.value);
            item["authentications"].append(authenDetails);
        }
        root.append(item);
    }
    return root;
}

std::vector<MultiAuthentication> jsonToAuthenticationsWithDecryption(const Json::Value &root) {
    std::vector<MultiAuthentication> authentications;
    for (const auto &item : root) {
        MultiAuthentication multiAuth;
        multiAuth.itemName = item["itemName"].asString();
        bool enabled{true};
        if (item.isMember("enabled") && item["enabled"].isBool()) {
            enabled = item["enabled"].asBool();
        }
        multiAuth.enabled = enabled;
        const auto authens = item["authentications"];
        for (const auto &authDetails : authens) {
            Authentication auth;
            auth.friendlyKeyName = authDetails["friendlyKeyName"].asString();
            auth.key = authDetails["key"].asString();
            auth.value = Encryption::getInstance().decrypt(authDetails["value"].asString());
            multiAuth.authentications.push_back(auth);
        }
        authentications.push_back(multiAuth);
    }
    return authentications;
}

std::vector<MultiAuthentication> customModelJsonToAuthenticationsWithDecryption(const Json::Value &root) {
    if (!root.isMember("apiKey") || !root["apiKey"].isString()) {
        return {};
    }

    const std::string apiKey = root["apiKey"].asString();
    return {{"", {{"APIKey", "apiKey", Encryption::getInstance().decrypt(apiKey)}}}};
}

std::string aiCapabilityString(AiCapability capability) {
    switch (capability) {
        case AiCapability::Nlp:
            return "nlp";
        case AiCapability::Speech:
            return "speech";
        case AiCapability::Vision:
            return "vision";
        default:
            return "";
    }
}

Json::Value authensToEngineConfigJson(const std::vector<MultiAuthentication> &multiAuthens) {
    Json::Value root;
    for (const auto &multiAuth : multiAuthens) {
        for (const auto &auth : multiAuth.authentications) {
            root[auth.key] = auth.value;
        }
    }
    return root;
}

bool isLocalServer(const std::string &jsonString) {
    auto root = util::stringToJson(jsonString);
    if (!root.isObject()) {
        LOGE("Invalid configs format: {}", jsonString);
        return false;
    }

    if (!root.isMember("isLocalServer") || !root["isLocalServer"].isBool()) {
        LOGE("Missing or invalid isLocalServer field: {}", jsonString);
        return false;
    }

    return root["isLocalServer"].asBool();
}

std::map<std::string, std::string> oldModelNameMap{
    {"通义千问（Qwen-Plus） 试用版", "kylin-qwen-plus"},
    {"通义千问（QwQ-32B） 试用版", "kylin-qwq-32b"},
    {"DeepSeek-R1 试用版", "kylin-deepseek-r1"},
    {"DeepSeek-V3 试用版", "kylin-deepseek-v3"},
    {"千问-Plus-试用版", "kylin-qwen-plus"},
    {"QWQ-32B-试用版", "kylin-qwq-32b"},
    {"DeepSeek-R1-试用版", "kylin-deepseek-r1"},
    {"DeepSeek-V3-试用版", "kylin-deepseek-v3"},
};

bool isFreetrialOldModelName(const std::string &oldModelName) {
    return util::mapContains(oldModelNameMap, oldModelName);
}

std::string freetrialOldModelNameToNewModelName(const std::string &oldModelName) {
    return isFreetrialOldModelName(oldModelName) ? oldModelNameMap[oldModelName] : std::string{};
}

}  // namespace

ModelConfigManager &ModelConfigManager::getInstance() {
    static ModelConfigManager instance(ai_engine::AiEnginePluginManager::getInstance());
    return instance;
}

ModelConfigManager::ModelConfigManager(ai_engine::AiEnginePluginManager &aiEnginePluginManager)
    : aiEnginePluginManager_(aiEnginePluginManager),
      cloudSettings_(publicCloudSchemaId),
      customSettings_(customCloudSchemaId),
      ondeviceSettings_(ondeviceSchemaId),
      sharedSettings_(sharedSchemaId),
      bootSettings_(bootSchemaId) {
    modelInfosLoadFuture_ = std::async(std::launch::async, [this]() {
        isModelInfosLoaderThread = true;
        loadModelInfos();
        if (needToMigrateOldModelConfig(baiduNlpModelName)) {
            migrateOldModelConfig(baiduNlpModelName, AiCapability::Nlp);
        }
        if (needToMigrateOldModelConfig(baiduVisionModelName)) {
            migrateOldModelConfig(baiduVisionModelName, AiCapability::Vision);
        }
        setDefaultCloudNlpModelIfNotCurrentSet();
        isModelInfosLoaderThread = false;
    });
    initializeSharedComputingState();
}

void ModelConfigManager::initializeSharedComputingState() {
    isEnableSharedComputingPower_ = sharedSettings_.getBoolValue(sharedEnabled);
    if (!isEnableSharedComputingPower_ && !getSharedModelName().empty()) {
        isEnableRequestComputingPower_ = true;
    }
}

std::vector<Model> ModelConfigManager::getModels(AiCapability capability) {
    if (capability == AiCapability::Nlp && !isNlpModelLoading()) {
        reloadNlpModelInfos();
    }
    waitForModelInfosLoaded();
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    std::vector<Model> models;
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (modelInfo.model.capability != capability) {
            continue;
        }
        // 排除端侧视觉模型
        if (modelInfo.model.capability == AiCapability::Vision && modelInfo.model.deployType == DeployType::Local) {
            continue;
        }
        // 历史遗留，getModels函数返回不包含试用模型
        if (modelInfo.model.isFreetrialModel) {
            continue;
        }
        models.push_back(modelInfo.model);
    }
    return models;
}

std::vector<Model> ModelConfigManager::getModels(DeployType deployType) {
    if (!isNlpModelLoading()) {
        reloadNlpModelInfos();
    }
    waitForModelInfosLoaded();
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    std::vector<Model> models;
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        // 本地NLP三方模型放入自定义模型，并且从本地NLP模型列表中移除
        if (modelInfo.model.capability == AiCapability::Nlp && modelInfo.model.deployType == DeployType::Local &&
            modelInfo.model.is3rdModel) {
            if (deployType == DeployType::Custom) {
                models.push_back(modelInfo.model);
                continue;
            }
            if (deployType == DeployType::Local) {
                continue;
            }
        }
        if (modelInfo.model.deployType != deployType) {
            continue;
        }
        models.push_back(modelInfo.model);
    }
    return models;
}

std::string ModelConfigManager::getCurrentModelName(const Settings &settings, AiCapability capability) const {
    const Json::Value &jsonConfig = util::stringToJson(settings.getJsonValue(aiCapabilityString(capability)));
    if (jsonConfig.isMember("currentModel") && jsonConfig["currentModel"].isString()) {
        std::string current = jsonConfig["currentModel"].asString();  // current可能是modelName或displayName
        {
            // 兼容性代码，兼容旧版本配置，旧版本配置中，modelName为displayName
            std::string modelName;
            if (isDisplayName(current)) {
                modelName = getModelNameByDisplayName(current);
            } else {
                modelName = current;
            }
            return modelName;
        }
    }
    return std::string{};
}

std::string ModelConfigManager::getCurrentModelName(AiCapability capability, DeployType deployType) {
    waitForModelInfosLoaded();
    switch (deployType) {
        case DeployType::Local: {
            return getCurrentModelName(ondeviceSettings_, capability);
        }

        case DeployType::Custom: {
            return getCurrentModelName(customSettings_, capability);
        }
        case DeployType::PublicCloud: {
            std::string currentModelName = getCurrentModelName(cloudSettings_, capability);
            if (capability != AiCapability::Nlp) {
                return currentModelName;
            }
            if (containsModel(currentModelName)) {
                return currentModelName;
            }
            // 云端 NLP 模型不在模型列表的情况，使用统一的回退逻辑
            std::string newCurrentModelName = selectFallbackNlpModel(currentModelName);
            setCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud, newCurrentModelName);
            return newCurrentModelName;
        }
        case DeployType::Shared: {
            std::string modelName{};
            if (isEnableRequestComputingPower_ && !sharedModelInfos_.empty()) {
                modelName = sharedModelInfos_.front().model.modelName;
            }
            return modelName;
        }
        default: {
            return "";
        }
    }
    return "";
}

void ModelConfigManager::updateCloudNlpCurrentModelName() {
    std::vector<ModelInfo> tempNlpModels;
    loadNlpModelInfos("freetrial", tempNlpModels);
    saveFreeTrialNlpModelInfos(tempNlpModels);
    std::string currentModelName = getCurrentModelName(cloudSettings_, AiCapability::Nlp);
    if (containsModel(currentModelName)) {
        return;
    }
    // 云端 NLP 模型不在模型列表的情况，使用统一的回退逻辑
    std::string newCurrentModelName = selectFallbackNlpModel(currentModelName);
    setCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud, newCurrentModelName);
}

std::string ModelConfigManager::selectFallbackNlpModel(const std::string &currentModelName) {
    std::lock_guard locker(modelInfosMutex_);
    // 判断是否为旧名称，如果是旧名称，则转换为新名称
    if (isFreetrialOldModelName(currentModelName)) {
        return freetrialOldModelNameToNewModelName(currentModelName);
    }
    // 如果默认模型在模型列表中，则使用默认模型
    if (util::mapContains(modelInfos_, defaultCloudNlpModelName)) {
        return defaultCloudNlpModelName;
    }
    // 非上述情况，并且默认模型也不在列表中，此时使用第一个试用模型
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (modelInfo.model.capability != AiCapability::Nlp) {
            continue;
        }
        if (modelInfo.model.deployType != DeployType::PublicCloud) {
            continue;
        }
        if (!modelInfo.model.isFreetrialModel) {
            continue;
        }
        return modelName;
    }
    return std::string{};
}

std::vector<MultiAuthentication> ModelConfigManager::getModelAuthentications(const std::string &modelName) const {
    waitForModelInfosLoaded();

    if (!containsModel(modelName)) {
        return {};
    }

    auto modelInfo = getModelInfo(modelName);

    if (modelInfo.model.deployType == DeployType::Local) {
        return {};
    }

    const std::string capabilityKey = aiCapabilityString(modelInfo.model.capability);

    auto &settings = modelInfo.model.deployType == DeployType::Custom ? customSettings_ : cloudSettings_;

    const std::string capabilityJsonString = settings.getJsonValue(capabilityKey);
    Json::Value jsonConfig = util::stringToJson(capabilityJsonString);

    // 兼容性代码，兼容旧版本配置，旧版本配置中，需要用displayName获取鉴权信息
    std::string actualModelName;
    if (jsonConfig.isMember(modelName)) {
        actualModelName = modelName;
    } else if (jsonConfig.isMember(modelInfo.model.displayName)) {
        actualModelName = modelInfo.model.displayName;
    }

    if (actualModelName.empty()) {
        return modelInfo.multiAuthentications;
    }

    Json::Value jsonAuths = jsonConfig[actualModelName];

    // 自选模型中文本模型使用openAI通用apiKey，语音和视觉要使用固定模板
    return (modelInfo.model.deployType == DeployType::Custom && modelInfo.model.capability == AiCapability::Nlp)
               ? customModelJsonToAuthenticationsWithDecryption(jsonAuths)
               : jsonToAuthenticationsWithDecryption(jsonAuths);
}

bool ModelConfigManager::getModelDeployEnabled(DeployType deployType) const {
    switch (deployType) {
        case DeployType::Local: {
            if (isEnableRequestComputingPower()) {
                return false;
            }
            return ondeviceSettings_.getBoolValue("enabled");
        }
        case DeployType::Custom: {
            return customSettings_.getBoolValue("enabled");
        }
        case DeployType::PublicCloud: {
            return cloudSettings_.getBoolValue("enabled");
        }
        case DeployType::Shared: {
            return sharedSettings_.getBoolValue("enabled");
        }
        default: {
            return false;
        }
    }
    return false;
}

bool ModelConfigManager::setCurrentModelName(AiCapability capability, DeployType deployType,
                                             const std::string &targetName) {
    waitForModelInfosLoaded();

    std::string modelName;
    if (isDisplayName(targetName)) {
        modelName = getModelNameByDisplayName(targetName);
    } else {
        modelName = targetName;
    }

    if (!containsModel(modelName) && !modelName.empty()) {
        LOGE("Model not found: {}", modelName);
        return false;
    }

    modelChangedCallbacks_.callAllCallbacks(capability, deployType, ModelChangeType::CurrentModelChanged);

    auto doSetCurrentModelName = [modelName](Settings &settings, AiCapability capability) {
        const std::string jsonString = settings.getJsonValue(aiCapabilityString(capability));
        Json::Value jsonConfig = util::stringToJson(jsonString);
        jsonConfig["currentModel"] = modelName;
        const std::string newJsonString = jsonConfig.toStyledString();
        settings.setJsonValue(aiCapabilityString(capability), newJsonString);
    };

    switch (deployType) {
        case DeployType::Local: {
            doSetCurrentModelName(ondeviceSettings_, capability);
            return true;
        }
        case DeployType::Custom: {
            doSetCurrentModelName(customSettings_, capability);
            return true;
        }
        case DeployType::PublicCloud: {
            doSetCurrentModelName(cloudSettings_, capability);
            return true;
        }
        case DeployType::Shared: {
            doSetCurrentModelName(sharedSettings_, capability);
            return true;
        }

        default:
            return false;
    }
    return false;
}

/// !todo 按照authentications中指定的itemName进行更新
bool ModelConfigManager::setModelAuthentications(const std::string &modelName,
                                                 const std::vector<MultiAuthentication> &authentications) {
    if (authentications.empty()) {
        LOGE("Authentications is empty.");
        return false;
    }

    waitForModelInfosLoaded();

    if (!containsModel(modelName)) {
        LOGE("Model not found: {}.", modelName);
        return false;
    }

    auto modelInfo = getModelInfo(modelName);
    if (modelInfo.model.deployType == DeployType::Local) {
        return false;
    }

    auto &settings = modelInfo.model.deployType == DeployType::Custom ? customSettings_ : cloudSettings_;

    updateModelAuthenticationsSettings(settings, modelInfo, authentications);
    updateModelAuthenticationsInfo(modelName, authentications);

    modelChangedCallbacks_.callAllCallbacks(modelInfo.model.capability, modelInfo.model.deployType,
                                            ModelChangeType::AuthenticationChanged);

    return true;
}

void ModelConfigManager::updateModelAuthenticationsSettings(Settings &settings, const ModelInfo &modelInfo,
                                                            const std::vector<MultiAuthentication> &authentications) {
    const std::string capabilityKey = aiCapabilityString(modelInfo.model.capability);
    Json::Value jsonAuths;
    if (modelInfo.model.deployType == DeployType::Custom && modelInfo.model.capability == AiCapability::Nlp) {
        jsonAuths = Encryption::getInstance().encrypt(
            authentications.empty() ? "" : authentications[0].authentications[0].value);
    } else {
        jsonAuths = authenticationsToJsonWithEncryption(authentications);
    }

    const std::string capabilityJsonString = settings.getJsonValue(capabilityKey);
    Json::Value jsonConfig = util::stringToJson(capabilityJsonString);

    Json::Value *keyObjPtr{nullptr};
    if (modelInfo.model.deployType == DeployType::Custom && modelInfo.model.capability == AiCapability::Nlp) {
        keyObjPtr = &jsonConfig[modelInfo.model.modelName]["apiKey"];
    } else {
        keyObjPtr = &jsonConfig[modelInfo.model.modelName];
    }

    *keyObjPtr = jsonAuths;
    const std::string jsonString = util::jsonToString(jsonConfig);
    settings.setJsonValue(capabilityKey, jsonString);
}

void ModelConfigManager::updateModelAuthenticationsInfo(const std::string &modelName,
                                                        const std::vector<MultiAuthentication> &multiAuthentications) {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    ModelInfo &modelInfo = modelInfos_[modelName];
    modelInfo.multiAuthentications = multiAuthentications;
}

void ModelConfigManager::clearModelAuthenticationsSettings(Settings &settings, const ModelInfo &modelInfo) {
    const std::string capabilityKey = aiCapabilityString(modelInfo.model.capability);
    const std::string capabilityJsonString = settings.getJsonValue(capabilityKey);
    LOGD("{}", capabilityJsonString);
    Json::Value jsonConfig = util::stringToJson(capabilityJsonString);

    if (modelInfo.model.deployType == DeployType::Custom && modelInfo.model.capability == AiCapability::Nlp) {
        jsonConfig[modelInfo.model.modelName]["apiKey"] = "";
        settings.setJsonValue(capabilityKey, jsonConfig.toStyledString());
        return;
    }

    if (jsonConfig.isMember(modelInfo.model.modelName)) {
        auto &authentications = jsonConfig[modelInfo.model.modelName];
        if (authentications.isArray()) {
            for (auto &authentication : authentications) {
                for (auto &auth : authentication["authentications"]) {
                    auth["value"] = "";
                }
            }
        }
        if (jsonConfig["currentModel"].asString() == modelInfo.model.modelName) {
            jsonConfig.removeMember("currentModel");
        }
        std::string jsonString = util::jsonToString(jsonConfig);
        LOGD("jsonString {}", jsonString);
        settings.setJsonValue(capabilityKey, jsonString);
    }
}

void ModelConfigManager::clearModelAuthenticationsInfo(const std::string &modelName) {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    ModelInfo &modelInfo = modelInfos_[modelName];
    modelInfo.multiAuthentications.clear();
}

void ModelConfigManager::setDefaultCloudNlpModelIfNotCurrentSet() {
    // 验证当前选中模型是否为有效模型
    std::string currentModelName = getCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud);
    if (util::mapContains(modelInfos_, currentModelName)) {
        return;
    }
    // 当前选中模型为无效模型时，则需要重新设置，产品要求的默认值：kylin-qwen-plus
    if (util::mapContains(modelInfos_, defaultCloudNlpModelName)) {
        if (setCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud, defaultCloudNlpModelName)) {
            return;
        }
    }
    // 设置"kylin-qwen-plus"为默认模型不成功的场景，选中试用模型中的第一个
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (!modelInfo.model.isFreetrialModel) {
            continue;
        }
        if (setCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud, modelInfo.model.modelName)) {
            break;
        }
    }
}

// 对于云端模型来说是删除所有的模型配置
// 对于自定义模型仅仅是清除apiKey的内容
/// !todo 具体逻辑需要进一步优化
bool ModelConfigManager::clearModelAuthentications(const std::string &modelName) {
    waitForModelInfosLoaded();

    if (!containsModel(modelName)) {
        LOGE("Model not found: {}", modelName);
        return false;
    }

    ModelInfo modelInfo = getModelInfo(modelName);
    auto &settings = modelInfo.model.deployType == DeployType::Custom ? customSettings_ : cloudSettings_;

    clearModelAuthenticationsSettings(settings, modelInfo);
    clearModelAuthenticationsInfo(modelName);

    modelChangedCallbacks_.callAllCallbacks(modelInfo.model.capability, modelInfo.model.deployType,
                                            ModelChangeType::AuthenticationChanged);

    return true;
}

bool ModelConfigManager::setModelSchedulingPriority(DeployType deployType, int priority) {
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Nlp, deployType, ModelChangeType::PriorityChanged);
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Speech, deployType, ModelChangeType::PriorityChanged);
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Vision, deployType, ModelChangeType::PriorityChanged);

    switch (deployType) {
        case DeployType::Local: {
            ondeviceSettings_.setIntValue("priority", priority);
            return true;
        }
        case DeployType::Custom: {
            customSettings_.setIntValue("priority", priority);
            return true;
        }
        case DeployType::PublicCloud: {
            cloudSettings_.setIntValue("priority", priority);
            return true;
        }
        case DeployType::Shared: {
            sharedSettings_.setIntValue("priority", priority);
            return true;
        }
    }
    return false;
}

int ModelConfigManager::getModelSchedulingPriority(DeployType deployType) const {
    switch (deployType) {
        case DeployType::Local: {
            return ondeviceSettings_.getIntValue("priority");
        }
        case DeployType::Custom: {
            return customSettings_.getIntValue("priority");
        }
        case DeployType::PublicCloud: {
            return cloudSettings_.getIntValue("priority");
        }
        case DeployType::Shared: {
            return sharedSettings_.getIntValue("priority");
        }
    }
    return -1;
}

bool ModelConfigManager::setModelDeployEnabled(DeployType deployType, bool enabled) {
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Nlp, deployType, ModelChangeType::DeployTypeChanged);
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Speech, deployType, ModelChangeType::DeployTypeChanged);
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Vision, deployType, ModelChangeType::DeployTypeChanged);

    switch (deployType) {
        case DeployType::Local:
            return ondeviceSettings_.setBoolValue("enabled", enabled);
        case DeployType::Custom:
            return customSettings_.setBoolValue("enabled", enabled);
        case DeployType::PublicCloud:
            return cloudSettings_.setBoolValue("enabled", enabled);
        case DeployType::Shared:
            return sharedSettings_.setBoolValue("enabled", enabled);
    }
    return false;
}

bool ModelConfigManager::needToMigrateOldModelConfig(const std::string &modelName) {
    if (util::readFile(oldModelConfigFile).empty()) {
        return false;
    }

    const auto autehns = getModelAuthentications(modelName);
    return autehns.empty() || autehns.back().authentications.back().value.empty();
}

// 旧版本的配置只有百度的，并且只有nlp的vision的能力
void ModelConfigManager::migrateOldModelConfig(const std::string &modelName, AiCapability capability) {
    std::vector<ModelInfo> odlModelInfos;
    const std::string oldVisionModelName = "百度-文心AI作画高级版";
    const std::string modelInfo = util::readFile(oldModelConfigFile);
    const Json::Value root = util::stringToJson(modelInfo);
    // 百度
    if (!root.isMember("0")) {
        return;
    }
    Json::Value baiduModelInfo = root["0"];
    const std::string capabilityKey = capability == AiCapability::Nlp ? "0" : "1";
    if (!baiduModelInfo.isMember(capabilityKey)) {
        return;
    }

    Json::Value nlpModelInfo = baiduModelInfo[capabilityKey];
    const std::string apiKey = nlpModelInfo["apiKey"].asString();
    const std::string secretKey = nlpModelInfo["secretKey"].asString();
    const std::string currentModelName = getCurrentModelName(capability, DeployType::PublicCloud);
    setModelAuthentications(modelName, {{"",
                                         {{"APIKey", "apiKey", Encryption::getInstance().decrypt(apiKey)},
                                          {"SecretKey", "secretKey", Encryption::getInstance().decrypt(secretKey)}}}});
    if (currentModelName.empty()) {
        setCurrentModelName(capability, DeployType::PublicCloud, modelName);
    }
}

void ModelConfigManager::loadModelInfos() {
    auto allEngineNames = aiEnginePluginManager_.allEngineName();
    std::vector<ModelInfo> nlpModels;
    for (const auto &engineName : allEngineNames) {
        std::vector<ModelInfo> tempNlpModels;
        loadNlpModelInfos(engineName, tempNlpModels);
        if (engineName == "freetrial") {
            saveFreeTrialNlpModelInfos(tempNlpModels);
            continue;
        }
        nlpModels.insert(nlpModels.end(), tempNlpModels.begin(), tempNlpModels.end());

        loadSpeechModelInfos(engineName);
        loadGenaAiVisionModelInfos(engineName);
        loadCoreAiVisionModelInfos(engineName);
    }
    saveNlpModelInfos(nlpModels);
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        LOGI("Model {} loaded, capability:{}, deployType:{}", modelName, static_cast<int>(modelInfo.model.capability),
             static_cast<int>(modelInfo.model.deployType));
    }
}

void ModelConfigManager::loadNlpModelInfos(const std::string &engineName, std::vector<ModelInfo> &outputModels) {
    auto start = std::chrono::high_resolution_clock::now();
    std::unique_ptr engine = aiEnginePluginManager_.createTextGenerationEngine(engineName);
    if (engine == nullptr) {
        return;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    LOGI("Time taken to create engine NLP {} model infos: {} ms", engineName, duration.count());

    std::vector<ModelInfo> modelInfos;
    if (engine->isCloud() && !engine->isCustomModelEngine()) {
        if (engineName == "freetrial") {
            // 删除所有使用模型
            std::lock_guard<std::mutex> lock(modelInfosMutex_);
            for (auto it = modelInfos_.begin(); it != modelInfos_.end();) {
                ModelInfo modelInfo = it->second;
                if (modelInfo.model.isFreetrialModel) {
                    it = modelInfos_.erase(it);
                } else {
                    ++it;
                }
            }
        }
        modelInfos =
            createModelInfosFromJsonString(engine->modelInfo(), AiCapability::Nlp, DeployType::PublicCloud, engineName);
        if (disabledEngineNames.find(engineName) != disabledEngineNames.end()) {
            return;
        }
    } else if (engine->isCustomModelEngine()) {
        checkAndUpdateCustomSettings();  // 兼容旧版本
        modelInfos = createCustomModelInfosFromJsonString(customSettings_.getJsonValue("nlp"), engineName);
    } else if (!engine->isCloud()) {
        auto start = std::chrono::high_resolution_clock::now();
        modelInfos =
            createModelInfosFromJsonString(engine->modelInfo(), AiCapability::Nlp, DeployType::Local, engineName);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        LOGI("Time taken to get modelInfo NLP {} model infos: {} ms", engineName, duration.count());
        std::string jsonString = ondeviceSettings_.getJsonValue(aiCapabilityString(AiCapability::Nlp));
        Json::Value config = util::stringToJson(jsonString);
        removeOndeviceModelConfigIfNotExist(modelInfos, config);

        if (!config.isMember("currentModel")) {
            config["currentModel"] = "";
            modelChangedCallbacks_.callAllCallbacks(AiCapability::Nlp, DeployType::Local,
                                                    ModelChangeType::CurrentModelChanged);
        }
        // 如果为空，且已安装本地模型，则设置默认值
        if (config["currentModel"].asString().empty() && !modelInfos.empty()) {
            std::vector<ModelInfo> buildInModelInfos = filterModelInfos(modelInfos, [](const ModelInfo &info) {
                return info.model.deployType == DeployType::Local && info.model.capability == AiCapability::Nlp &&
                       !info.model.is3rdModel;
            });
            if (!buildInModelInfos.empty()) {
                config["currentModel"] = buildInModelInfos[0].model.modelName;
                modelChangedCallbacks_.callAllCallbacks(AiCapability::Nlp, DeployType::Local,
                                                        ModelChangeType::CurrentModelChanged);
            }
        }
        ondeviceSettings_.setJsonValue(aiCapabilityString(AiCapability::Nlp), util::jsonToString(config));
    }

    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    for (const ModelInfo &modelInfo : modelInfos) {
        modelInfos_[modelInfo.model.modelName] = modelInfo;
        outputModels.push_back(modelInfo);
    }
}

void ModelConfigManager::removeOndeviceModelConfigIfNotExist(const std::vector<ModelInfo> &modelInfos,
                                                             Json::Value &config) {
    if (!config.isMember("currentModel")) {
        return;
    }

    const std::string currentModelName = config["currentModel"].asString();
    bool isCurrentModelExists = false;
    for (const auto &modelInfo : modelInfos) {
        if (modelInfo.model.deployType == DeployType::Local) {
            if (currentModelName == modelInfo.model.modelName) {
                isCurrentModelExists = true;
                break;
            }
        }
    }

    if (!isCurrentModelExists) {
        config.removeMember("currentModel");
        LOGD("Remove ondevice current model config: {}", currentModelName);
    }
}

void ModelConfigManager::loadSpeechModelInfos(const std::string &engineName) {
    std::unique_ptr engine = aiEnginePluginManager_.createSpeechEngine(engineName);
    if (engine != nullptr) {
        DeployType deployType{DeployType::Unknown};
        if (engine->isCustomModelEngine()) {
            deployType = DeployType::Custom;
        } else if (engine->isCloud()) {
            deployType = DeployType::PublicCloud;
        } else {
            deployType = DeployType::Local;
        }
        std::vector<ModelInfo> modelInfos =
            createModelInfosFromJsonString(engine->modelInfo(), AiCapability::Speech, deployType, engineName);

        if (deployType == DeployType::Local) {
            std::string jsonString = ondeviceSettings_.getJsonValue(aiCapabilityString(AiCapability::Speech));
            Json::Value config = util::stringToJson(jsonString);
            if (!config.isMember("currentModel") || config["currentModel"].asString().empty()) {
                if (!modelInfos.empty()) {
                    config["currentModel"] = modelInfos[0].model.modelName;
                }
            }
            ondeviceSettings_.setJsonValue(aiCapabilityString(AiCapability::Speech), util::jsonToString(config));
        }

        std::lock_guard<std::mutex> lock(modelInfosMutex_);
        for (const ModelInfo &modelInfo : modelInfos) {
            modelInfos_[modelInfo.model.modelName] = modelInfo;
        }
    }
}

void ModelConfigManager::loadGenaAiVisionModelInfos(const std::string &engineName) {
    std::unique_ptr engine = aiEnginePluginManager_.createImageGenerationEngine(engineName);
    if (engine == nullptr) {
        return;
    }

    DeployType deployType{DeployType::Unknown};
    if (engine->isCustomModelEngine()) {
        deployType = DeployType::Custom;
    } else if (engine->isCloud()) {
        deployType = DeployType::PublicCloud;
    } else {
        deployType = DeployType::Local;
    }

    std::vector<ModelInfo> modelInfos =
        createModelInfosFromJsonString(engine->modelInfo(), AiCapability::Vision, deployType, engineName);

    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    for (const ModelInfo &modelInfo : modelInfos) {
        modelInfos_[modelInfo.model.modelName] = modelInfo;
    }
}

void ModelConfigManager::loadCoreAiVisionModelInfos(const std::string &engineName) {
    std::unique_ptr engine = aiEnginePluginManager_.createImageProcessEngine(engineName);
    if (engine == nullptr) {
        return;
    }

    std::vector<ModelInfo> modelInfos =
        createModelInfosFromJsonString(engine->modelInfo(), AiCapability::Vision, DeployType::Local, engineName);

    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    for (const ModelInfo &modelInfo : modelInfos) {
        modelInfos_[modelInfo.model.modelName] = modelInfo;
    }
}

bool ModelConfigManager::isNlpModelLoading() {
    std::lock_guard<std::mutex> futureLock(modelInfosLoadFutureMutex_);
    return modelInfosLoadFuture_.valid();
}

void ModelConfigManager::reloadNlpModelInfos() {
    std::lock_guard<std::mutex> lock(modelInfosLoadFutureMutex_);
    modelInfosLoadFuture_ = std::async(std::launch::async, [this]() {
        isModelInfosLoaderThread = true;
        std::vector<ModelInfo> nlpModels;
        auto allEngineNames = aiEnginePluginManager_.allEngineName();
        for (const auto &engineName : allEngineNames) {
            std::vector<ModelInfo> tempModels;
            if (engineName == "freetrial") {
                continue;
            }
            loadNlpModelInfos(engineName, tempModels);
            nlpModels.insert(nlpModels.end(), tempModels.begin(), tempModels.end());
        }

        if (needToMigrateOldModelConfig(baiduNlpModelName)) {
            migrateOldModelConfig(baiduNlpModelName, AiCapability::Nlp);
        }

        saveNlpModelInfos(nlpModels);
        isModelInfosLoaderThread = false;
    });
}

void ModelConfigManager::saveNlpModelInfos(const std::vector<ModelInfo> &modelInfos) {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    // 清理非试用模型信息
    for (auto it = modelInfos_.begin(); it != modelInfos_.end();) {
        ModelInfo modelInfo = it->second;
        if (modelInfo.model.capability == AiCapability::Nlp && (!modelInfo.model.isFreetrialModel)) {
            it = modelInfos_.erase(it);
        } else {
            ++it;
        }
    }

    for (const ModelInfo &modelInfo : modelInfos) {
        modelInfos_[modelInfo.model.modelName] = modelInfo;
    }

    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (!modelInfo.model.isFreetrialModel) {
            LOGI("Model {} loaded", modelInfo.model.modelName);
        }
    }
}

void ModelConfigManager::saveFreeTrialNlpModelInfos(const std::vector<ModelInfo> &modelInfos) {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    // 清理试用模型信息
    for (auto it = modelInfos_.begin(); it != modelInfos_.end();) {
        ModelInfo modelInfo = it->second;
        if (modelInfo.model.isFreetrialModel) {
            it = modelInfos_.erase(it);
        } else {
            ++it;
        }
    }
    for (const ModelInfo &modelInfo : modelInfos) {
        modelInfos_[modelInfo.model.modelName] = modelInfo;
    }
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (modelInfo.model.isFreetrialModel) {
            LOGI("Model {} loaded", modelInfo.model.modelName);
        }
    }
}

bool ModelConfigManager::containsModel(const std::string &modelName) const {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    return modelInfos_.find(modelName) != modelInfos_.end();
}

ModelInfo ModelConfigManager::getModelInfo(const std::string &modelName) const {
    waitForModelInfosLoaded();
    ModelInfo modelInfo{};
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    if (modelInfos_.find(modelName) != modelInfos_.end()) {
        modelInfo = modelInfos_.at(modelName);
    }

    std::lock_guard<std::mutex> sharedLock(sharedComputingMutex_);
    if (isEnableRequestComputingPower_ && !sharedModelInfos_.empty()) {
        auto tmpModelInfo = sharedModelInfos_.front();
        if (tmpModelInfo.model.modelName == modelName) {
            modelInfo = tmpModelInfo;
        }
    }
    return modelInfo;
}

bool ModelConfigManager::addCustomModel(const std::string &displayName, const std::string &modelVersion,
                                        const std::string &apiKey, const std::string &apiUrl) {
    waitForModelInfosLoaded();
    std::string modelName = buildCustomModelName(modelVersion, getCurrentCustomIdSet());
    if (containsModel(modelName)) {
        LOGE("The model has been added: {}", modelName);
        return false;
    }
    if (isDisplayNameExists(displayName)) {
        LOGE("The model has been added: {}", displayName);
        return false;
    }
    Json::Value configJson = util::stringToJson(customSettings_.getJsonValue("nlp"));
    configJson[modelName]["modelName"] = modelName;
    configJson[modelName]["displayName"] = displayName;
    configJson[modelName]["apiKey"] = Encryption::getInstance().encrypt(apiKey);
    configJson[modelName]["apiUrl"] = apiUrl;
    configJson[modelName]["model"] = modelVersion;
    std::string configString = util::jsonToString(configJson);
    LOGI("Add custom model: {}", configString);
    // 当前自定义模型只支持nlp模型
    customSettings_.setJsonValue("nlp", configString);
    Model model;
    model.modelName = modelName;
    model.displayName = displayName;
    model.modelVersion = modelVersion;
    model.apiUrl = apiUrl;
    model.vendor = std::string{};
    model.debName = std::string{};
    model.is3rdModel = false;
    model.capability = AiCapability::Nlp;
    model.deployType = DeployType::Custom;
    model.engineName = "custom";
    model.brand = "custom";
    model.isFreetrialModel = false;
    Authentication authentication;
    authentication.friendlyKeyName = "APIKey";
    authentication.key = "apiKey";
    authentication.value = apiKey;
    MultiAuthentication multiAuthentication;
    multiAuthentication.itemName = std::string{};
    multiAuthentication.authentications = std::vector{authentication};
    ModelInfo modelInfo;
    modelInfo.model = model;
    modelInfo.multiAuthentications = std::vector{multiAuthentication};
    {
        std::lock_guard<std::mutex> lock(modelInfosMutex_);
        modelInfos_[modelInfo.model.modelName] = modelInfo;
    }
    return true;
}

bool ModelConfigManager::deleteCustomModel(const std::string &modelName) {
    Json::Value currentCustomJsonConfig = util::stringToJson(customSettings_.getJsonValue("nlp"));
    if (!currentCustomJsonConfig.isMember(modelName)) {
        return false;
    }

    waitForModelInfosLoaded();

    currentCustomJsonConfig.removeMember(modelName);
    if (currentCustomJsonConfig.isMember("currentModel") && currentCustomJsonConfig["currentModel"].isString() &&
        currentCustomJsonConfig["currentModel"].asString() == modelName) {
        currentCustomJsonConfig["currentModel"] = "";
    }

    customSettings_.setJsonValue("nlp", currentCustomJsonConfig.toStyledString());

    {
        std::lock_guard<std::mutex> lock(modelInfosMutex_);
        modelInfos_.erase(modelName);
    }
    modelChangedCallbacks_.callAllCallbacks(AiCapability::Nlp, DeployType::Custom,
                                            ModelChangeType::CurrentModelChanged);
    return true;
}

bool ModelConfigManager::setCustomModelApiUrl(const std::string &modelName, const std::string &apiUrl) {
    Json::Value currentCustomJsonConfig = util::stringToJson(customSettings_.getJsonValue("nlp"));
    if (!currentCustomJsonConfig.isMember(modelName)) {
        return false;
    }
    currentCustomJsonConfig[modelName]["apiUrl"] = apiUrl;
    LOGI("SetCustomModelApiUrl: {}", currentCustomJsonConfig.toStyledString());
    customSettings_.setJsonValue("nlp", currentCustomJsonConfig.toStyledString());

    waitForModelInfosLoaded();

    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    if (modelInfos_.find(modelName) == modelInfos_.end()) {
        return false;
    }
    modelInfos_[modelName].model.apiUrl = apiUrl;
    return true;
}

bool ModelConfigManager::setCustomModelVersion(const std::string &modelName, const std::string &modelVersion) {
    Json::Value currentCustomJsonConfig = util::stringToJson(customSettings_.getJsonValue("nlp"));
    if (!currentCustomJsonConfig.isMember(modelName)) {
        return false;
    }

    currentCustomJsonConfig[modelName]["model"] = modelVersion;
    customSettings_.setJsonValue("nlp", currentCustomJsonConfig.toStyledString());

    waitForModelInfosLoaded();

    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    if (modelInfos_.find(modelName) == modelInfos_.end()) {
        return false;
    }
    modelInfos_[modelName].model.modelVersion = modelVersion;
    return true;
}

void ModelConfigManager::updateCallback(AiCapability capability, DeployType deployType, const std::string &uuid,
                                        const ModelChangedCallbacks::ModelChangedCallback &callback) {
    return modelChangedCallbacks_.updateCallback(capability, deployType, uuid, callback);
}

void ModelConfigManager::updateAllDeployTypeCallback(AiCapability capability, const std::string &uuid,
                                                     const ModelChangedCallbacks::ModelChangedCallback &callback) {
    updateCallback(capability, DeployType::Local, uuid, callback);
    updateCallback(capability, DeployType::PublicCloud, uuid, callback);
    updateCallback(capability, DeployType::Custom, uuid, callback);
    updateCallback(capability, DeployType::Shared, uuid, callback);
}

void ModelConfigManager::getConfiguredCloudNlpModelList(Json::Value &value, bool isNlpSdk) {
    bool publicCloudEnabled = getModelDeployEnabled(DeployType::PublicCloud);
    if (!publicCloudEnabled) {
        return;
    }
    int priority = getModelSchedulingPriority(DeployType::PublicCloud);
    std::string currentModelName = getCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud);
    // 先获取免费模型
    std::vector<Model> freeTrialModels = getFreeTrialModels();
    for (const Model &model : freeTrialModels) {
        Json::Value subValue;
        if (model.deployType != DeployType::PublicCloud) {
            continue;
        }
        if (isNlpSdk) {
            subValue["modelName"] = model.modelName;
        } else {
            subValue["modelName"] = model.displayName;  // 为了兼容，将modelName赋值为displayName
        }
        subValue["displayName"] = model.displayName;
        subValue["deployType"] = int(model.deployType);
        subValue["supportsWebSearch"] = model.supportsWebSearch;
        subValue["supportsThinking"] = model.supportsThinking;
        subValue["supportsThinkingToggle"] = model.supportsThinkingToggle;
        subValue["priority"] = priority;
        subValue["isModelSelected"] = model.modelName == currentModelName;
        subValue["brand"] = model.brand;
        subValue["supportsToolChoice"] = model.supportsToolChoice;
        value.append(subValue);
    }
    // 非免费模型
    std::vector<Model> models = getModels(AiCapability::Nlp);
    for (const Model &model : models) {
        Json::Value subValue;
        ModelInfo modelInfo = getModelInfo(model.modelName);
        std::vector<MultiAuthentication> multiAuthentications = getModelAuthentications(model.modelName);
        Json::Value jsonMultiAuthentications = authensToEngineConfigJson(multiAuthentications);
        if (model.deployType != DeployType::PublicCloud) {
            continue;
        }
        if (!jsonMultiAuthentications.isMember("apiKey")) {
            continue;
        }
        if (!jsonMultiAuthentications["apiKey"].isString()) {
            continue;
        }
        if (jsonMultiAuthentications["apiKey"].asString().empty()) {
            continue;
        }
        if (model.isFreetrialModel) {
            continue;
        }
        if (isNlpSdk) {
            subValue["modelName"] = model.modelName;
        } else {
            subValue["modelName"] = model.displayName;  // 为了兼容，将modelName赋值为displayName
        }
        subValue["displayName"] = model.displayName;
        subValue["deployType"] = int(model.deployType);
        subValue["supportsWebSearch"] = model.supportsWebSearch;
        subValue["supportsThinking"] = model.supportsThinking;
        subValue["supportsThinkingToggle"] = model.supportsThinkingToggle;
        subValue["priority"] = priority;
        subValue["isModelSelected"] = model.modelName == currentModelName;
        subValue["supportsToolChoice"] = model.supportsToolChoice;
        value.append(subValue);
    }
}

void ModelConfigManager::getConfiguredLocalNlpModelList(Json::Value &value, bool isNlpSdk) {
    if (!isEnableSharedComputingPower_ && !getSharedModelName().empty()) {
        getSharedNlpModel(value);
        return;
    }
    setEnableRequestComputingPower(false);
    bool localEnabled = getModelDeployEnabled(DeployType::Local);
    if (!localEnabled) {
        return;
    }

    int priority = getModelSchedulingPriority(DeployType::Local);
    std::string currentModelName = getCurrentModelName(AiCapability::Nlp, DeployType::Local);
    std::vector<Model> models = getModels(AiCapability::Nlp);
    for (const Model &model : models) {
        Json::Value subValue;
        if (model.deployType != DeployType::Local) {
            continue;
        }
        if (model.deployType == DeployType::Local && model.is3rdModel) {
            continue;
        }
        if (isNlpSdk) {
            subValue["modelName"] = model.modelName;
        } else {
            subValue["modelName"] = model.displayName;  // 为了兼容，将modelName赋值为displayName
        }
        subValue["displayName"] = model.displayName;
        subValue["deployType"] = int(model.deployType);
        subValue["supportsThinking"] = model.supportsThinking;
        subValue["supportsThinkingToggle"] = model.supportsThinkingToggle;
        subValue["priority"] = priority;
        subValue["isModelSelected"] = model.modelName == currentModelName;
        subValue["brand"] = model.brand;
        if (isEnableSharedComputingPower()) {
            subValue["isModelEnabled"] = model.modelName == currentModelName;
        }
        subValue["supportsToolChoice"] = model.supportsToolChoice;
        value.append(subValue);
    }
}

void ModelConfigManager::getConfiguredCustomNlpModelList(Json::Value &value, bool isNlpSdk) {
    bool customEnabled = getModelDeployEnabled(DeployType::Custom);
    if (!customEnabled) {
        return;
    }
    int priority = getModelSchedulingPriority(DeployType::Custom);
    std::string currentModelName = getCurrentModelName(AiCapability::Nlp, DeployType::Custom);
    std::vector<Model> models = getModels(AiCapability::Nlp);
    for (const Model &model : models) {
        if (model.deployType == DeployType::Custom || (model.deployType == DeployType::Local && model.is3rdModel)) {
            Json::Value subValue;
            if (isNlpSdk) {
                subValue["modelName"] = model.modelName;
            } else {
                subValue["modelName"] = model.displayName;  // 为了兼容，将modelName赋值为displayName
            }
            subValue["displayName"] = model.displayName;
            subValue["deployType"] = model.is3rdModel ? int(DeployType::Custom) : int(model.deployType);
            subValue["priority"] = priority;
            subValue["isModelSelected"] = model.modelName == currentModelName;
            subValue["brand"] = model.brand;
            subValue["supportsToolChoice"] = model.supportsToolChoice;
            value.append(subValue);
        }
    }
}

void ModelConfigManager::waitForModelInfosLoaded() const {
    if (isModelInfosLoaderThread) {
        return;
    }

    std::lock_guard<std::mutex> futureLock(modelInfosLoadFutureMutex_);
    if (modelInfosLoadFuture_.valid()) {
        if (modelInfosLoadFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                modelInfosLoadFuture_.get();
            } catch (const std::exception &e) {
                LOGE("Exception during model info loading: {}", e.what());
            }
            modelInfosLoadFuture_ = std::shared_future<void>{};
        } else {
            LOGW("Model infos loading is taking too long, please check the configuration.");
        }
    }
}

bool ModelConfigManager::isLocalModelEnabled() const { return getModelDeployEnabled(DeployType::Local); }

bool ModelConfigManager::setSharedModelName(const std::string &modelName) {
    std::string targetModelName;
    if (isDisplayName(modelName)) {
        targetModelName = getModelNameByDisplayName(modelName);
    } else {
        targetModelName = modelName;
    }
    LOGI("setSharedModelName: {}", targetModelName);

    setEnableSharedComputingPower(true);

#ifdef SUPPORT_LIFE_CYCLE
    lifecycle::ModelDescriptor modelDescriptor;
    modelDescriptor.modelName = targetModelName;
    modelDescriptor.capability = AiCapability::Nlp;
    modelDescriptor.modelClass = {lifecycle::ModelFamily::kLLM, lifecycle::ModelSubType::kNone};
    return lifecycle::ModelLifecycleManager::getInstance().retainModelAndUnloadOthers(modelDescriptor);
#endif
}

bool ModelConfigManager::stopModelSharing(const std::string &modelName) {
    std::string targetModelName;
    if (isDisplayName(modelName)) {
        targetModelName = getModelNameByDisplayName(modelName);
    } else {
        targetModelName = modelName;
    }
    LOGI("stopModelSharing: {}", targetModelName);

    setEnableSharedComputingPower(false);

#ifdef SUPPORT_LIFE_CYCLE
    lifecycle::ModelDescriptor modelDescriptor;
    modelDescriptor.modelName = targetModelName;
    modelDescriptor.capability = AiCapability::Nlp;
    modelDescriptor.modelClass = {lifecycle::ModelFamily::kLLM, lifecycle::ModelSubType::kNone};
    return lifecycle::ModelLifecycleManager::getInstance().cancelRetainModel(modelDescriptor);
#endif
}

bool ModelConfigManager::isEnableSharedComputingPower() const {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    return isEnableSharedComputingPower_;
}

bool ModelConfigManager::isEnableRequestComputingPower() const {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    return isEnableRequestComputingPower_;
}

std::string ModelConfigManager::getSharedModelName() {
    setEnableRequestComputingPower(true);

    std::string engineName = "ondevice";
    auto start = std::chrono::high_resolution_clock::now();
    auto engine = aiEnginePluginManager_.createTextGenerationEngine(engineName);
    if (!engine) {
        LOGE("Get shared model name failed.");
        return "";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    LOGI("Time taken to create engine and get modelInfo NLP {} model infos: {} ms", engineName, duration.count());

    engine->setConfig(R"(
        {
            "use_local": false
        }
    )");

    std::string modelInfo = engine->modelInfo();
    LOGI("getSharedModelName modelInfo: {}", modelInfo);
    if (isLocalServer(modelInfo)) {
        requestComputingPowerDisconnected();
        return "";
    }
    saveSharedModelInfo(modelInfo, engineName);
    std::string sharedModelName;
    if (!sharedModelInfos_.empty()) {
        sharedModelName = sharedModelInfos_.front().model.modelName;
    }
    LOGI("Shared model name: {}", sharedModelName);

    return sharedModelName;
}

void ModelConfigManager::requestComputingPowerDisconnected() {
    isEnableRequestComputingPower_ = false;
    sharedModelInfos_ = {};
    setModelDeployEnabled(DeployType::Shared, false);
}

void ModelConfigManager::saveSharedModelInfo(const std::string &modelInfo, const std::string &engineName) {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    sharedModelInfos_ = createModelInfosFromJsonString(modelInfo, AiCapability::Nlp, DeployType::Shared, engineName);
}

void ModelConfigManager::getSharedNlpModel(Json::Value &value) {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    if (sharedModelInfos_.size() != 1) {
        LOGE("Failed to get shared nlp model");
        return;
    }
    Json::Value subValue;
    subValue["modelName"] = sharedModelInfos_.begin()->model.modelName;
    subValue["displayName"] = sharedModelInfos_.begin()->model.displayName;
    subValue["deployType"] = (int)sharedModelInfos_.begin()->model.deployType;
    subValue["supportsThinking"] = sharedModelInfos_.begin()->model.supportsThinking;
    subValue["supportsThinkingToggle"] = sharedModelInfos_.begin()->model.supportsThinkingToggle;
    subValue["supportsToolChoice"] = sharedModelInfos_.begin()->model.supportsToolChoice;
    subValue["isSharedModel"] = true;
    value.append(subValue);
}

void ModelConfigManager::setEnableSharedComputingPower(bool enable) {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    isEnableSharedComputingPower_ = enable;
    sharedSettings_.setBoolValue(sharedEnabled, enable);
}

void ModelConfigManager::setEnableRequestComputingPower(bool enable) {
    std::lock_guard<std::mutex> lock(sharedComputingMutex_);
    isEnableRequestComputingPower_ = enable;
}

std::string ModelConfigManager::getConfiguredNlpModelList(bool isNlpSdk) {
    Json::Value value = Json::arrayValue;
    // 需求写死的顺序：云端模型、本地模型、自定义模型
    getConfiguredCloudNlpModelList(value, isNlpSdk);
    getConfiguredLocalNlpModelList(value, isNlpSdk);
    getConfiguredCustomNlpModelList(value, isNlpSdk);
    return util::jsonToString(value);
}

std::vector<Model> ModelConfigManager::getFreeTrialModels() {
    std::vector<ModelInfo> tempNlpModels;
    loadNlpModelInfos("freetrial", tempNlpModels);
    saveFreeTrialNlpModelInfos(tempNlpModels);
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    std::vector<Model> modelInfos;
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (!modelInfo.model.isFreetrialModel) {
            continue;
        }
        modelInfos.push_back(modelInfo.model);
    }
    return modelInfos;
}

bool ModelConfigManager::isCloudModelConfigured(const std::vector<ModelInfo> &modelInfos) {
    for (const auto &modelInfo : modelInfos) {
        std::string modelName = modelInfo.model.modelName;

        const std::string capabilityKey = aiCapabilityString(modelInfo.model.capability);

        auto &settings = modelInfo.model.deployType == DeployType::Custom ? customSettings_ : cloudSettings_;

        const std::string capabilityJsonString = settings.getJsonValue(capabilityKey);
        Json::Value jsonConfig = util::stringToJson(capabilityJsonString);
        if (!jsonConfig.isMember(modelName)) {
            return false;
        }
        Json::Value jsonAuths = jsonConfig[modelName];

        for (const auto &multiAuthentication : jsonToAuthenticationsWithDecryption(jsonAuths)) {
            for (const auto &authentication : multiAuthentication.authentications) {
                if (authentication.value.empty()) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ModelConfigManager::isModelEnabled(const std::string &modelName) const {
    // 使能关了的模型认为不存在
    bool publicCloudEnabled = getModelDeployEnabled(DeployType::PublicCloud);
    bool customEnabled = getModelDeployEnabled(DeployType::Custom);
    bool localEnabled = getModelDeployEnabled(DeployType::Local);
    bool sharedEnabled = getModelDeployEnabled(DeployType::Shared);

    ModelInfo modelInfo = getModelInfo(modelName);
    if (modelInfo.model.modelName.empty()) {
        return false;
    }
    if (!publicCloudEnabled && modelInfo.model.deployType == DeployType::PublicCloud) {
        return false;
    }
    if (!customEnabled && modelInfo.model.deployType == DeployType::Custom) {
        return false;
    }
    if (!localEnabled && modelInfo.model.deployType == DeployType::Local) {
        return false;
    }
    if (!sharedEnabled && modelInfo.model.deployType == DeployType::Shared) {
        return false;
    }
    return true;
}

bool ModelConfigManager::isLocalModel(const std::string &modelName) const {
    if (modelName.empty()) {
        return false;
    }
    ModelInfo modelInfo = getModelInfo(modelName);
    return modelInfo.model.deployType == DeployType::Local;
}

bool ModelConfigManager::isNlpModelConfigured(const std::string &modelName) const {
    if (modelName.empty()) {
        return false;
    }
    ModelInfo modelInfo = getModelInfo(modelName);
    if (modelInfo.model.modelName.empty()) {
        return false;
    }
    if (modelInfo.model.capability != AiCapability::Nlp) {
        return true;
    }
    if (modelInfo.model.deployType != DeployType::PublicCloud) {
        return true;
    }
    if (modelInfo.model.engineName == "freetrial") {
        return true;
    }
    std::vector<MultiAuthentication> multiAuthentications = getModelAuthentications(modelName);
    Json::Value jsonMultiAuthentications = authensToEngineConfigJson(multiAuthentications);
    if (!jsonMultiAuthentications.isMember("apiKey")) {
        return false;
    }
    if (!jsonMultiAuthentications["apiKey"].isString()) {
        return false;
    }
    if (jsonMultiAuthentications["apiKey"].asString().empty()) {
        return false;
    }
    return true;
}

bool ModelConfigManager::isDisplayName(const std::string &name) const {
    return !getModelNameByDisplayName(name).empty();
}

std::string ModelConfigManager::getModelNameByDisplayName(const std::string &displayName) const {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (modelInfo.model.displayName == displayName) {
            return modelName;
        }
    }
    return std::string{};
}

std::string ModelConfigManager::getDisplayName(const std::string &modelName) const {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    if (modelInfos_.find(modelName) == modelInfos_.end()) {
        return std::string{};
    }
    return modelInfos_.at(modelName).model.displayName;
}

std::string ModelConfigManager::buildCustomModelName(const std::string &modelVersion,
                                                     const std::set<uint32_t> &currentCustomIdSet) const {
    // 找出其中最小的、未出现的非负整数
    uint32_t newCustomId = 0;
    for (const uint32_t &customId : currentCustomIdSet) {
        if (newCustomId < customId) {
            break;
        }
        if (newCustomId == customId) {
            newCustomId++;
        }
    }
    return fmt::format("{}-{:04d}", modelVersion, newCustomId);
}

std::set<uint32_t> ModelConfigManager::getCurrentCustomIdSet() const {
    std::lock_guard<std::mutex> lock(modelInfosMutex_);
    std::set<uint32_t> customIdSet{};
    for (const auto &[modelName, modelInfo] : modelInfos_) {
        if (modelInfo.model.deployType == DeployType::Custom && modelInfo.model.capability == AiCapability::Nlp) {
            std::vector<std::string> splits = util::stringSplit(modelName, "-");
            if (splits.size() < 2) {
                continue;
            }
            customIdSet.insert(std::stoul(splits.back()));
        }
    }
    return customIdSet;
}

void ModelConfigManager::checkAndUpdateCustomSettings() {
    // 兼容旧版自定义模型设置
    // 旧版格式：
    // {
    //     "currentModel" : "001",
    //     "001" :
    //     {
    //         "name" : "001",
    //         "model" : "glm-4.7",
    //         "apiKey" : "xxx",
    //         "apiUrl" : "https://open.bigmodel.cn/api/paas/v4/chat/completions"
    //     },
    //     "002" :
    //     {
    //         "model" : "glm-4.7",
    //         "name" : "002",
    //         "apiKey" : "xxx",
    //         "apiUrl" : "https://open.bigmodel.cn/api/paas/v4/chat/completions"
    //     }
    // }
    // 新版格式：
    // {
    //     "currentModel" : "glm-4.7-0000",
    //     "glm-4.7-0000" :
    //     {
    //         "modelName" : "glm-4.7-0000",
    //         "displayName" : "003",
    //         "model" : "glm-4.7",
    //         "apiKey" : "xxx",
    //         "apiUrl" : "https://open.bigmodel.cn/api/paas/v4/chat/completions"
    //     },
    //     "glm-4.7-0001" :
    //     {
    //         "modelName" : "glm-4.7-0001",
    //         "displayName" : "004",
    //         "model" : "glm-4.7",
    //         "apiKey" : "xxx",
    //         "apiUrl" : "https://open.bigmodel.cn/api/paas/v4/chat/completions"
    //     }
    // }
    // 以"name"字段作为判断依据，若出现旧版配置，则将旧版配置转换为新版配置，并保存
    // 然后再读取新版配置，将数据存入modelInfos中

    std::string customNlpSettingsString = customSettings_.getJsonValue("nlp");
    if (customNlpSettingsString.empty()) {
        return;
    }
    Json::Value customNlpSettingsJson = util::stringToJson(customNlpSettingsString);
    if (customNlpSettingsJson == Json::objectValue) {
        LOGE("Custom nlp settings invalid: {}", customNlpSettingsString);
        return;
    }
    std::map<std::string, std::string> modelNameMap;
    std::string currentModelName;
    std::set<uint32_t> currentCustomIdSet;
    const std::vector<std::string> &memberNames = customNlpSettingsJson.getMemberNames();
    for (uint32_t i = 0; i < memberNames.size(); ++i) {
        const std::string &modelName = memberNames[i];
        if (modelName == "currentModel") {
            currentModelName = customNlpSettingsJson["currentModel"].asString();
            continue;
        }
        Json::Value &modelConfig = customNlpSettingsJson[modelName];
        if (!modelConfig.isMember("name")) {
            continue;
        }
        // 处理displayName
        modelConfig["displayName"] = modelConfig["name"];
        modelConfig.removeMember("name");
        // 处理modelName
        std::string newModelName = buildCustomModelName(modelConfig["model"].asString(), currentCustomIdSet);
        currentCustomIdSet.insert(i);  // 添加到当前使用的自定义模型id集合中
        modelConfig["modelName"] = newModelName;
        // modelName和newModelName映射
        modelNameMap[modelName] = newModelName;
    }
    // 处理currentModel
    if (!modelNameMap.empty()) {
        // 非空条件说明是旧格式
        customNlpSettingsJson["currentModel"] = modelNameMap[currentModelName];
    }
    // 处理为新版格式
    for (const auto &[modelName, newModelName] : modelNameMap) {
        customNlpSettingsJson[newModelName] = customNlpSettingsJson[modelName];
        customNlpSettingsJson.removeMember(modelName);
    }
    customSettings_.setJsonValue("nlp", util::jsonToString(customNlpSettingsJson));
}

std::vector<ModelInfo> ModelConfigManager::filterModelInfos(const std::vector<ModelInfo> &modelInfos,
                                                            const std::function<bool(const ModelInfo &)> &predicate) {
    std::vector<ModelInfo> result;
    result.reserve(modelInfos.size());
    for (const auto &modelInfo : modelInfos) {
        if (predicate(modelInfo)) {
            result.push_back(modelInfo);
        }
    }
    return result;
}

bool ModelConfigManager::isDisplayNameExists(const std::string &displayName) const {
    return std::any_of(modelInfos_.begin(), modelInfos_.end(), [&displayName](const auto &pair) {
        const auto &[modelName, modelInfo] = pair;
        return modelInfo.model.displayName == displayName;
    });
}

bool ModelConfigManager::getLoadOnBoot(const std::string &capability, const std::string &subType) const {
    std::string jsonStr = bootSettings_.getJsonValue("boot-load-config");
    if (jsonStr.empty() || jsonStr == "{}") {
        return false;
    }

    Json::Value root = util::stringToJson(jsonStr);
    if (root.isNull()) {
        LOGW("Failed to parse boot load config: {}", jsonStr);
        return false;
    }

    if (!root.isMember(capability) || !root[capability].isObject()) {
        return false;
    }

    const Json::Value &capConfig = root[capability];
    if (capConfig.isMember(subType)) {
        return capConfig[subType].asBool();
    }

    // 支持嵌套的子类型，比如 embedding.text
    auto pos = subType.find('.');
    if (pos != std::string::npos) {
        std::string parentType = subType.substr(0, pos);
        std::string childType = subType.substr(pos + 1);

        if (capConfig.isMember(parentType) && capConfig[parentType].isObject()) {
            return capConfig[parentType][childType].asBool();
        }
    }

    return false;
}

void ModelConfigManager::setLoadOnBoot(const std::string &capability, const std::string &subType, bool enabled) {
    std::string jsonStr = bootSettings_.getJsonValue("boot-load-config");
    Json::Value root;

    if (!jsonStr.empty() && jsonStr != "{}") {
        root = util::stringToJson(jsonStr);
    }

    // 处理嵌套的子类型
    auto pos = subType.find('.');
    if (pos != std::string::npos) {
        std::string parentType = subType.substr(0, pos);
        std::string childType = subType.substr(pos + 1);

        if (!root.isMember(capability)) {
            root[capability] = Json::Value(Json::objectValue);
        }
        if (!root[capability].isMember(parentType)) {
            root[capability][parentType] = Json::Value(Json::objectValue);
        }
        root[capability][parentType][childType] = enabled;
    } else {
        if (!root.isMember(capability)) {
            root[capability] = Json::Value(Json::objectValue);
        }
        root[capability][subType] = enabled;
    }

    bootSettings_.setJsonValue("boot-load-config", util::jsonToString(root));
}
