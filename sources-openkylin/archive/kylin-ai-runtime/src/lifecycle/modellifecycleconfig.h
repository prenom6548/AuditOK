/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <jsoncpp/json/json.h>

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "lifecycle/modellifecycletypes.h"

namespace lifecycle {

class ModelLifecycleConfig {
public:
    using Seconds = std::chrono::seconds;

    struct ModelLifecycleProfile {
        ModelClass lifecycleClass;
        Seconds idleTimeout{Seconds(0)};
        EngineCallStage callStage{EngineCallStage::kOnInitialization};
        UnloadPolicy unloadPolicy{UnloadPolicy::kUnloadWhenIdle};
        bool unloadOnSleep{true};
    };

    static ModelLifecycleConfig loadFromFile(const std::string &path);
    static std::string defaultConfigPath();

    ModelClass resolveModelClass(const ModelDescriptor &descriptor) const;
    ModelLifecycleProfile profileFor(const ModelDescriptor &descriptor) const;
    Seconds idleTimeout(const ModelDescriptor &descriptor) const;
    EngineCallStage stage(const ModelDescriptor &descriptor) const;
    UnloadPolicy unloadPolicy(const ModelDescriptor &descriptor) const;
    bool unloadOnSleep(const ModelDescriptor &descriptor) const;

    // 基于生命周期标识的配置查询（优先使用 descriptor 版本）
    Seconds idleTimeout(const ModelClass &modelClass) const;
    int sweepIntervalSeconds() const;
    EngineCallStage stage(const ModelClass &modelClass) const;
    UnloadPolicy unloadPolicy(const ModelClass &modelClass) const;
    bool shouldUnloadOnSleep(const ModelClass &modelClass) const;

private:
    void loadFromModelsJson(const Json::Value &root, bool isHighSpec);
    void loadModelNode(const std::string &name, const Json::Value &node, bool isHighSpec);
    void addProfile(const std::string &name, const Json::Value &node, const ModelLifecycleProfile &profile);
    void loadFromJson(const Json::Value &root);
    ModelLifecycleProfile makeProfileFromNode(const Json::Value &node, const ModelLifecycleProfile &base) const;

private:
    ModelLifecycleConfig();

    std::unordered_map<std::string, ModelLifecycleProfile> profiles_;
    ModelLifecycleProfile defaultProfile_;
    std::unordered_map<std::string, ModelClass> capabilityMapping_;
    std::map<ModelClass, Seconds> timeouts_;
    std::map<ModelClass, EngineCallStage> callStages_;
    std::map<ModelClass, UnloadPolicy> unloadPolicies_;
    int sweepIntervalSec_{100};
    std::set<ModelClass> sleepUnloadClasses_;
};

}  // namespace lifecycle
