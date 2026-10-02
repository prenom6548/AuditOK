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

#include <jsoncpp/json/json.h>

#include <future>
#include <optional>

#include "engine/aienginepluginmanager.h"
#include "modelchangedcallbacks.h"
#include "modelconfigenums.h"
#include "modelsettings.h"
#ifdef SUPPORT_LIFE_CYCLE
#include "lifecycle/modellifecyclemanager.h"
#endif

struct Authentication {
    std::string friendlyKeyName;
    std::string key;
    std::string value;
};

struct MultiAuthentication {
    std::string itemName;
    std::vector<Authentication> authentications;
    bool enabled{true};  // 为了兼容以前版本，这里必须是true
};

struct Model {
    std::string modelName;
    std::string displayName;
    std::string modelVersion;
    std::string apiUrl;
    std::string vendor;
    std::string debName;
    std::string engineName;
    std::string brand;
    AiCapability capability;
    DeployType deployType;
    bool is3rdModel{false};
    bool supportsWebSearch{false};
    bool supportsThinking{false};
    bool supportsThinkingToggle{false};
    bool isFreetrialModel{false};  // 是否为试用模型
    bool isModelEnabled{true};
    bool isSharedModel{false};
    bool supportsToolChoice{false};
    std::optional<bool> available;  // 端侧后摩大语言模型返回的模型信息才有这个字段，用于判断后摩卡是否可用
};

struct ModelInfo {
    Model model;
    std::vector<MultiAuthentication> multiAuthentications;
};

class ModelConfigManager {
public:
    using ModelName = std::string;

public:
    static ModelConfigManager &getInstance();

    ModelConfigManager(const ModelConfigManager &) = delete;

    ModelConfigManager &operator=(const ModelConfigManager &) = delete;

    std::vector<Model> getModels(AiCapability capability);

    std::vector<Model> getModels(DeployType deployType);

    std::string getCurrentModelName(AiCapability capability, DeployType deployType);

    // 用于及时更新云端NLP当前模型，因为模型有可能会被卸载
    void updateCloudNlpCurrentModelName();

    std::vector<MultiAuthentication> getModelAuthentications(const std::string &modelName) const;

    bool getModelDeployEnabled(DeployType deployType) const;

    bool setCurrentModelName(AiCapability capability, DeployType deployType, const std::string &modelName);

    bool setModelAuthentications(const std::string &modelName, const std::vector<MultiAuthentication> &authentications);

    bool clearModelAuthentications(const std::string &modelName);

    bool setModelSchedulingPriority(DeployType deployType, int priority);

    int getModelSchedulingPriority(DeployType deployType) const;

    bool setModelDeployEnabled(DeployType deployType, bool enabled);

    ModelInfo getModelInfo(const std::string &modelName) const;

    bool addCustomModel(const std::string &displayName, const std::string &modelVersion, const std::string &apiKey,
                        const std::string &apiUrl);

    bool deleteCustomModel(const std::string &modelName);

    bool setCustomModelApiUrl(const std::string &modelName, const std::string &apiUrl);

    bool setCustomModelVersion(const std::string &modelName, const std::string &modelVersion);

    void updateCallback(AiCapability capability, DeployType deployType, const std::string &uuid,
                        const ModelChangedCallbacks::ModelChangedCallback &callback);

    void updateAllDeployTypeCallback(AiCapability capability, const std::string &uuid,
                                     const ModelChangedCallbacks::ModelChangedCallback &callback);

    std::string getConfiguredNlpModelList(bool isNlpSdk);

    std::vector<Model> getFreeTrialModels();

    bool isCloudModelConfigured(const std::vector<ModelInfo> &modelInfos);

    bool isModelEnabled(const std::string &modelName) const;

    bool isLocalModel(const std::string &modelName) const;

    bool isNlpModelConfigured(const std::string &modelName) const;

    void waitForModelInfosLoaded() const;

    bool isLocalModelEnabled() const;

    bool isDisplayName(const std::string &name) const;

    std::string getModelNameByDisplayName(const std::string &displayName) const;

    std::string getDisplayName(const std::string &modelName) const;

    bool setSharedModelName(const std::string &modelName);
    bool stopModelSharing(const std::string &modelName);
    bool isEnableSharedComputingPower() const;
    bool isEnableRequestComputingPower() const;
    std::string getSharedModelName();
    void requestComputingPowerDisconnected();

    bool getLoadOnBoot(const std::string &capability, const std::string &subType) const;
    void setLoadOnBoot(const std::string &capability, const std::string &subType, bool enabled);

private:
    explicit ModelConfigManager(ai_engine::AiEnginePluginManager &aiEnginePluginManager);
    void initializeSharedComputingState();
    std::string getCurrentModelName(const Settings &settings, AiCapability capability) const;

    bool needToMigrateOldModelConfig(const std::string &modelName);
    void migrateOldModelConfig(const std::string &modelName, AiCapability capability);
    void loadModelInfos();
    void loadNlpModelInfos(const std::string &engineName, std::vector<ModelInfo> &outputModels);
    void loadSpeechModelInfos(const std::string &engineName);
    void loadGenaAiVisionModelInfos(const std::string &engineName);
    void loadCoreAiVisionModelInfos(const std::string &engineName);

    bool isNlpModelLoading();
    void reloadNlpModelInfos();
    void saveNlpModelInfos(const std::vector<ModelInfo> &modelInfos);
    void saveFreeTrialNlpModelInfos(const std::vector<ModelInfo> &modelInfos);

    void removeOndeviceModelConfigIfNotExist(const std::vector<ModelInfo> &modelInfos, Json::Value &config);

    bool containsModel(const std::string &modelName) const;

    void updateModelAuthenticationsSettings(Settings &settings, const ModelInfo &modelInfo,
                                            const std::vector<MultiAuthentication> &authentications);
    void updateModelAuthenticationsInfo(const std::string &modelName,
                                        const std::vector<MultiAuthentication> &authentications);

    void clearModelAuthenticationsSettings(Settings &settings, const ModelInfo &modelInfo);
    void clearModelAuthenticationsInfo(const std::string &modelName);
    void setDefaultCloudNlpModelIfNotCurrentSet();

    void getConfiguredCloudNlpModelList(Json::Value &value, bool isNlpSdk);

    void getConfiguredLocalNlpModelList(Json::Value &value, bool isNlpSdk);

    void getConfiguredCustomNlpModelList(Json::Value &value, bool isNlpSdk);

    std::string buildCustomModelName(const std::string &modelVersion,
                                     const std::set<uint32_t> &currentCustomIdSet) const;

    std::set<uint32_t> getCurrentCustomIdSet() const;

    void checkAndUpdateCustomSettings();
    void saveSharedModelInfo(const std::string &modelInfo, const std::string &engineName);
    void getSharedNlpModel(Json::Value &value);
    void setEnableSharedComputingPower(bool enable);
    void setEnableRequestComputingPower(bool enable);

    static std::vector<ModelInfo> filterModelInfos(const std::vector<ModelInfo> &modelInfos,
                                                   const std::function<bool(const ModelInfo &)> &predicate);

    bool isDisplayNameExists(const std::string &displayName) const;

    std::string selectFallbackNlpModel(const std::string &currentModelName);

private:
    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    Settings cloudSettings_;
    Settings ondeviceSettings_;
    Settings customSettings_;
    Settings sharedSettings_;
    Settings bootSettings_;
    std::map<ModelName, ModelInfo> modelInfos_;
    ModelChangedCallbacks modelChangedCallbacks_;
    mutable std::mutex modelInfosMutex_;
    mutable std::shared_future<void> modelInfosLoadFuture_;
    mutable std::mutex modelInfosLoadFutureMutex_;
    mutable std::mutex sharedComputingMutex_;
    bool isEnableSharedComputingPower_ = false;
    bool isEnableRequestComputingPower_ = false;
    std::vector<ModelInfo> sharedModelInfos_;
};
