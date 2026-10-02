/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modellifecycleconfig.h"

#include <fstream>
#include <sstream>

#include "utils/logger.h"

namespace lifecycle {

namespace {

ModelFamily ModelFamilyFromString(const std::string &value) {
    if (value == "LLM") {
        return ModelFamily::kLLM;
    }
    if (value == "Embedding") {
        return ModelFamily::kEmbedding;
    }
    if (value == "Speech") {
        return ModelFamily::kSpeech;
    }
    if (value == "Image") {
        return ModelFamily::kImage;
    }
    return ModelFamily::kUnknown;
}

ModelSubType ModelSubTypeFromString(const std::string &value) {
    if (value.empty()) {
        return ModelSubType::kNone;
    }
    if (value == "Text" || value == "TextEmbedding") {
        return ModelSubType::kText;
    }
    if (value == "Image" || value == "ImageEmbedding") {
        return ModelSubType::kImage;
    }
    if (value == "SpeechToText") {
        return ModelSubType::kSpeechToText;
    }
    if (value == "TextToSpeech") {
        return ModelSubType::kTextToSpeech;
    }
    if (value == "GeneralSeg") {
        return ModelSubType::kGeneralSeg;
    }
    if (value == "PortraitSubjectSeg") {
        return ModelSubType::kPortraitSubjectSeg;
    }
    if (value == "GeneralSubjectSeg") {
        return ModelSubType::kGeneralSubjectSeg;
    }
    if (value == "Binarization") {
        return ModelSubType::kBinarization;
    }
    if (value == "TextRecognition") {
        return ModelSubType::kTextRecognition;
    }
    return ModelSubType::kUnknown;
}

ModelClass ModelClassFromProfileName(const std::string &name) {
    if (name.empty()) {
        return {};
    }
    auto pos = name.find('.');
    if (pos == std::string::npos) {
        return {ModelFamilyFromString(name), ModelSubType::kNone};
    }
    auto family = name.substr(0, pos);
    auto variant = name.substr(pos + 1);
    return {ModelFamilyFromString(family), ModelSubTypeFromString(variant)};
}

EngineCallStage EngineCallStageFromString(const std::string &value) {
    if (value == "OnBusinessCall") {
        return EngineCallStage::kOnBusinessCall;
    }
    return EngineCallStage::kOnInitialization;
}

UnloadPolicy UnloadPolicyFromString(const std::string &value) {
    if (value == "NeverEvict" || value == "NeverUnload") {
        return UnloadPolicy::kNeverUnload;
    }
    if (value == "EvictWhenIdle" || value == "UnloadWhenIdle") {
        return UnloadPolicy::kUnloadWhenIdle;
    }
    if (value == "EvictWhenNoHandleAndIdle" || value == "UnloadWhenNoHandleAndIdle") {
        return UnloadPolicy::kUnloadWhenNoHandleAndIdle;
    }
    if (value == "UnloadWhenIdleAndDisable") {
        return UnloadPolicy::kUnloadWhenIdleAndDisable;
    }
    // 默认策略：空闲时卸载
    return UnloadPolicy::kUnloadWhenIdle;
}

bool UnloadOnSleepFromNode(const Json::Value &node, bool fallback) {
    if (node.isBool()) {
        return node.asBool();
    }
    if (node.isMember("unloadOnSleep")) {
        return node["unloadOnSleep"].asBool();
    }
    return fallback;
}

long ReadTotalMemoryMB() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) {
        LOGW("Failed to open /proc/meminfo when evaluating machine profile.");
        return -1;
    }

    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemTotal:", 0) != 0) {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        long value{0};
        std::string unit;
        iss >> key >> value >> unit;
        if (key != "MemTotal:" || value <= 0) {
            continue;
        }

        if (unit == "kB" || unit == "KB") {
            return value / 1024;
        }
        if (unit == "mB" || unit == "MB") {
            return value;
        }
        if (unit == "gB" || unit == "GB") {
            return value * 1024;
        }
        return value / 1024;
    }

    LOGW("MemTotal not found in /proc/meminfo.");
    return -1;
}

bool IsHighSpecMachine(int thresholdGb) {
    const auto totalMemMB = ReadTotalMemoryMB();
    if (totalMemMB < 0) {
        return false;
    }
    const long thresholdMB = static_cast<long>(thresholdGb) * 1024;
    return totalMemMB >= thresholdMB;
}

void MergeJsonInPlace(Json::Value &target, const Json::Value &overrides) {
    if (!overrides.isObject()) {
        target = overrides;
        return;
    }
    if (!target.isObject()) {
        target = overrides;
        return;
    }

    for (const auto &name : overrides.getMemberNames()) {
        auto &slot = target[name];
        const auto &src = overrides[name];
        if (slot.isObject() && src.isObject()) {
            MergeJsonInPlace(slot, src);
        } else {
            slot = src;
        }
    }
}

int ResolveHighSpecThresholdGb(const Json::Value &root) {
    if (root.isMember("highSpecMemoryThresholdGB") && root["highSpecMemoryThresholdGB"].isNumeric()) {
        return root["highSpecMemoryThresholdGB"].asInt();
    }

    const Json::Value *profileNode = nullptr;
    if (root.isMember("machineProfiles")) {
        profileNode = &root["machineProfiles"];
    } else if (root.isMember("models") && root["models"].isObject() && root["models"].isMember("machineProfiles")) {
        profileNode = &root["models"]["machineProfiles"];
    }

    if (profileNode != nullptr && profileNode->isObject() && profileNode->isMember("memoryThresholdGB") &&
        (*profileNode)["memoryThresholdGB"].isNumeric()) {
        return (*profileNode)["memoryThresholdGB"].asInt();
    }

    return 32;
}

Json::Value ApplyMachineProfileOverrides(const Json::Value &root, bool isHighSpec) {
    const Json::Value *profileNode = nullptr;
    if (root.isMember("machineProfiles")) {
        profileNode = &root["machineProfiles"];
    } else if (root.isMember("models") && root["models"].isObject() && root["models"].isMember("machineProfiles")) {
        profileNode = &root["models"]["machineProfiles"];
    }

    if (profileNode == nullptr || !profileNode->isObject()) {
        return root;
    }

    const auto &overrides = isHighSpec ? (*profileNode)["highSpecOverrides"] : (*profileNode)["lowSpecOverrides"];
    if (overrides.isNull()) {
        return root;
    }

    Json::Value merged = root;
    MergeJsonInPlace(merged, overrides);

    // 清理配置节点，避免被当成模型配置解析
    merged.removeMember("machineProfiles");
    if (merged.isMember("models") && merged["models"].isObject()) {
        merged["models"].removeMember("machineProfiles");
    }

    return merged;
}

Json::Value ApplySpecOverrides(const Json::Value &node, bool isHighSpec) {
    if (!node.isObject()) {
        return node;
    }

    Json::Value merged = node;
    const std::string overrideKey = isHighSpec ? "highSpecOverrides" : "lowSpecOverrides";
    if (merged.isMember(overrideKey)) {
        MergeJsonInPlace(merged, merged[overrideKey]);
    }

    merged.removeMember("highSpecOverrides");
    merged.removeMember("lowSpecOverrides");
    return merged;
}

template <typename T>
T GetWithFallback(const std::map<ModelClass, T> &store, const ModelClass &cls, const T &defaultValue) {
    auto it = store.find(cls);
    if (it != store.end()) {
        return it->second;
    }
    if (cls.hasVariant()) {
        auto parent = cls.familyOnly();
        auto baseIt = store.find(parent);
        if (baseIt != store.end()) {
            return baseIt->second;
        }
    }
    return defaultValue;
}

}  // namespace

ModelLifecycleConfig::ModelLifecycleConfig() {
    defaultProfile_.idleTimeout = Seconds(300);
    defaultProfile_.callStage = EngineCallStage::kOnInitialization;
    defaultProfile_.unloadPolicy = UnloadPolicy::kNeverUnload;
    defaultProfile_.unloadOnSleep = true;
    defaultProfile_.lifecycleClass = {};

    const ModelClass clsLLM{ModelFamily::kLLM, ModelSubType::kNone};
    const ModelClass clsEmbedding{ModelFamily::kEmbedding, ModelSubType::kNone};
    const ModelClass clsTextEmbedding{ModelFamily::kEmbedding, ModelSubType::kText};
    const ModelClass clsImageEmbedding{ModelFamily::kEmbedding, ModelSubType::kImage};
    const ModelClass clsSpeech{ModelFamily::kSpeech, ModelSubType::kNone};
    const ModelClass clsSpeechToText{ModelFamily::kSpeech, ModelSubType::kSpeechToText};
    const ModelClass clsTextToSpeech{ModelFamily::kSpeech, ModelSubType::kTextToSpeech};
    const ModelClass clsImage{ModelFamily::kImage, ModelSubType::kNone};
    const ModelClass clsImageGeneral{ModelFamily::kImage, ModelSubType::kGeneralSeg};
    const ModelClass clsImagePortrait{ModelFamily::kImage, ModelSubType::kPortraitSubjectSeg};
    const ModelClass clsImageSubject{ModelFamily::kImage, ModelSubType::kGeneralSubjectSeg};
    const ModelClass clsImageBinarization{ModelFamily::kImage, ModelSubType::kBinarization};
    const ModelClass clsTextRecognition{ModelFamily::kImage, ModelSubType::kTextRecognition};

    timeouts_[clsLLM] = Seconds(3600);
    timeouts_[clsEmbedding] = Seconds(900);
    timeouts_[clsTextEmbedding] = Seconds(900);
    timeouts_[clsImageEmbedding] = Seconds(-1);
    timeouts_[clsSpeech] = Seconds(300);
    timeouts_[clsSpeechToText] = Seconds(300);
    timeouts_[clsTextToSpeech] = Seconds(300);
    timeouts_[clsImage] = Seconds(300);
    timeouts_[clsImageGeneral] = Seconds(300);
    timeouts_[clsImagePortrait] = Seconds(300);
    timeouts_[clsImageSubject] = Seconds(300);
    timeouts_[clsImageBinarization] = Seconds(300);
    timeouts_[clsTextRecognition] = Seconds(300);

    callStages_[clsLLM] = EngineCallStage::kOnInitialization;
    callStages_[clsEmbedding] = EngineCallStage::kOnInitialization;
    callStages_[clsTextEmbedding] = EngineCallStage::kOnInitialization;
    callStages_[clsImageEmbedding] = EngineCallStage::kOnInitialization;
    callStages_[clsSpeech] = EngineCallStage::kOnBusinessCall;
    callStages_[clsSpeechToText] = EngineCallStage::kOnInitialization;
    callStages_[clsTextToSpeech] = EngineCallStage::kOnInitialization;
    callStages_[clsImage] = EngineCallStage::kOnBusinessCall;
    callStages_[clsImageGeneral] = EngineCallStage::kOnBusinessCall;
    callStages_[clsImagePortrait] = EngineCallStage::kOnBusinessCall;
    callStages_[clsImageSubject] = EngineCallStage::kOnInitialization;
    callStages_[clsImageBinarization] = EngineCallStage::kOnInitialization;
    callStages_[clsTextRecognition] = EngineCallStage::kOnInitialization;

    unloadPolicies_[clsLLM] = UnloadPolicy::kUnloadWhenNoHandleAndIdle;
    unloadPolicies_[clsEmbedding] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsTextEmbedding] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImageEmbedding] = UnloadPolicy::kNeverUnload;
    unloadPolicies_[clsSpeech] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsSpeechToText] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsTextToSpeech] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImage] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImageGeneral] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImagePortrait] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImageSubject] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsImageBinarization] = UnloadPolicy::kUnloadWhenIdle;
    unloadPolicies_[clsTextRecognition] = UnloadPolicy::kUnloadWhenIdle;

    sleepUnloadClasses_.insert({});

    capabilityMapping_["Nlp"] = clsLLM;
    capabilityMapping_["Embedding"] = clsEmbedding;
    capabilityMapping_["Speech"] = clsSpeech;
    capabilityMapping_["Vision"] = clsImageGeneral;
}

ModelLifecycleConfig ModelLifecycleConfig::loadFromFile(const std::string &path) {
    ModelLifecycleConfig config;
    std::ifstream file(path);
    if (!file.is_open()) {
        LOGW("ModelLifecycleConfig::loadFromFile failed to open file: {}", path);
        return config;
    }

    Json::Value root;
    file >> root;
    config.loadFromJson(root);
    return config;
}

std::string ModelLifecycleConfig::defaultConfigPath() {
    std::ostringstream oss;
    oss << CONFIGS_PATH << "/runtime/model_lifecycle.json";
    return oss.str();
}

ModelClass ModelLifecycleConfig::resolveModelClass(const ModelDescriptor &descriptor) const {
    if (!descriptor.modelClass.isUnknown()) {
        return descriptor.modelClass;
    }
    const auto nameKey = descriptor.modelName;
    if (!nameKey.empty()) {
        const auto it = profiles_.find(nameKey);
        if (it != profiles_.end()) {
            return it->second.lifecycleClass;
        }
        const auto nameMap = capabilityMapping_.find(nameKey);
        if (nameMap != capabilityMapping_.end()) {
            return nameMap->second;
        }
    }

    const auto key = descriptor.capabilityKey();
    const auto it = capabilityMapping_.find(key);
    if (it != capabilityMapping_.end()) {
        return it->second;
    }
    return ModelClassFromProfileName(nameKey);
}

ModelLifecycleConfig::ModelLifecycleProfile ModelLifecycleConfig::profileFor(const ModelDescriptor &descriptor) const {
    const auto nameKey = descriptor.nameKey();
    if (!nameKey.empty()) {
        const auto it = profiles_.find(nameKey);
        if (it != profiles_.end()) {
            return it->second;
        }
    }

    ModelLifecycleProfile profile = defaultProfile_;
    profile.lifecycleClass = resolveModelClass(descriptor);

    if (profile.idleTimeout.count() == 0) {
        profile.idleTimeout = idleTimeout(profile.lifecycleClass);
    }
    if (profile.callStage == EngineCallStage::kOnInitialization) {
        profile.callStage = stage(profile.lifecycleClass);
    }
    profile.unloadPolicy = unloadPolicy(profile.lifecycleClass);
    profile.unloadOnSleep = shouldUnloadOnSleep(profile.lifecycleClass);
    return profile;
}

ModelLifecycleConfig::Seconds ModelLifecycleConfig::idleTimeout(const ModelDescriptor &descriptor) const {
    return profileFor(descriptor).idleTimeout;
}

EngineCallStage ModelLifecycleConfig::stage(const ModelDescriptor &descriptor) const {
    return profileFor(descriptor).callStage;
}

UnloadPolicy ModelLifecycleConfig::unloadPolicy(const ModelDescriptor &descriptor) const {
    return profileFor(descriptor).unloadPolicy;
}

bool ModelLifecycleConfig::unloadOnSleep(const ModelDescriptor &descriptor) const {
    return profileFor(descriptor).unloadOnSleep;
}

ModelLifecycleConfig::Seconds ModelLifecycleConfig::idleTimeout(const ModelClass &modelClass) const {
    return GetWithFallback<Seconds>(timeouts_, modelClass, Seconds(0));
}

int ModelLifecycleConfig::sweepIntervalSeconds() const { return sweepIntervalSec_; }

EngineCallStage ModelLifecycleConfig::stage(const ModelClass &modelClass) const {
    return GetWithFallback<EngineCallStage>(callStages_, modelClass, EngineCallStage::kOnInitialization);
}

UnloadPolicy ModelLifecycleConfig::unloadPolicy(const ModelClass &modelClass) const {
    return GetWithFallback<UnloadPolicy>(unloadPolicies_, modelClass, UnloadPolicy::kUnloadWhenIdle);
}

bool ModelLifecycleConfig::shouldUnloadOnSleep(const ModelClass &modelClass) const {
    if (sleepUnloadClasses_.empty()) {
        return false;
    }
    if (sleepUnloadClasses_.count({}) > 0) {
        return true;
    }
    if (sleepUnloadClasses_.count(modelClass) > 0) {
        return true;
    }
    if (modelClass.hasVariant()) {
        return sleepUnloadClasses_.count(modelClass.familyOnly()) > 0;
    }
    return false;
}

void ModelLifecycleConfig::loadFromJson(const Json::Value &root) {
    int thresholdGb = ResolveHighSpecThresholdGb(root);
    if (thresholdGb <= 0) {
        thresholdGb = 32;
    }
    const bool isHighSpec = IsHighSpecMachine(thresholdGb);
    const auto effectiveRoot = ApplyMachineProfileOverrides(root, isHighSpec);
    loadFromModelsJson(effectiveRoot, isHighSpec);
}

void ModelLifecycleConfig::loadFromModelsJson(const Json::Value &root, bool isHighSpec) {
    profiles_.clear();
    capabilityMapping_.clear();
    sleepUnloadClasses_.clear();

    if (root.isMember("sweepIntervalSec")) {
        sweepIntervalSec_ = root["sweepIntervalSec"].asInt();
    }

    if (root.isMember("defaults")) {
        const auto defaultsNode = ApplySpecOverrides(root["defaults"], isHighSpec);
        defaultProfile_ = makeProfileFromNode(defaultsNode, defaultProfile_);
    }

    if (!root.isMember("models")) {
        return;
    }

    const auto &models = root["models"];
    for (const auto &name : models.getMemberNames()) {
        if (name == "machineProfiles") {
            continue;
        }
        const auto &node = models[name];
        if (node.isArray()) {
            for (const auto &item : node) {
                loadModelNode(name, ApplySpecOverrides(item, isHighSpec), isHighSpec);
            }
        } else {
            loadModelNode(name, ApplySpecOverrides(node, isHighSpec), isHighSpec);
        }
    }

    if (sleepUnloadClasses_.empty() && defaultProfile_.unloadOnSleep) {
        sleepUnloadClasses_.insert({});
    }
}

void ModelLifecycleConfig::loadModelNode(const std::string &name, const Json::Value &node, bool isHighSpec) {
    const auto effectiveNode = ApplySpecOverrides(node, isHighSpec);
    if (effectiveNode.isNull()) {
        return;
    }

    std::string profileName = name;
    if (effectiveNode.isObject() && effectiveNode.isMember("name") && effectiveNode["name"].isString()) {
        profileName = effectiveNode["name"].asString();
    }

    ModelLifecycleProfile profile = makeProfileFromNode(effectiveNode, defaultProfile_);
    if (profile.lifecycleClass.isUnknown() || !profile.lifecycleClass.hasVariant()) {
        profile.lifecycleClass = ModelClassFromProfileName(profileName);
    }
    addProfile(profileName, effectiveNode, profile);

    if (effectiveNode.isMember("variants")) {
        const auto &variants = effectiveNode["variants"];
        for (const auto &variantName : variants.getMemberNames()) {
            const auto variantNode = ApplySpecOverrides(variants[variantName], isHighSpec);
            auto variantProfile = makeProfileFromNode(variantNode, profile);
            if (variantProfile.lifecycleClass.isUnknown() || !variantProfile.lifecycleClass.hasVariant()) {
                variantProfile.lifecycleClass = ModelClassFromProfileName(profileName + "." + variantName);
            }
            const std::string fullName = profileName + "." + variantName;
            addProfile(fullName, variantNode, variantProfile);
        }
    }
}

void ModelLifecycleConfig::addProfile(const std::string &name, const Json::Value &node,
                                      const ModelLifecycleProfile &profile) {
    profiles_[name] = profile;
    if (!profile.lifecycleClass.isUnknown()) {
        capabilityMapping_[name] = profile.lifecycleClass;
    }

    if (profile.unloadOnSleep) {
        sleepUnloadClasses_.insert(profile.lifecycleClass);
    }
}

ModelLifecycleConfig::ModelLifecycleProfile ModelLifecycleConfig::makeProfileFromNode(
    const Json::Value &node, const ModelLifecycleProfile &base) const {
    ModelLifecycleProfile profile = base;
    if (node.isMember("idleTimeoutSec")) {
        profile.idleTimeout = Seconds(node["idleTimeoutSec"].asInt());
    }
    if (node.isMember("loadTrigger")) {
        profile.callStage = EngineCallStageFromString(node["loadTrigger"].asString());
    }
    if (node.isMember("unloadPolicy")) {
        profile.unloadPolicy = UnloadPolicyFromString(node["unloadPolicy"].asString());
    } else if (node.isMember("evictionPolicy")) {
        profile.unloadPolicy = UnloadPolicyFromString(node["evictionPolicy"].asString());
    }

    profile.unloadOnSleep = UnloadOnSleepFromNode(node, profile.unloadOnSleep);
    return profile;
}

}  // namespace lifecycle
