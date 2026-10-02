/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modellifecyclemanager.h"

#include <engineselector/engineselector.h>
#include <modelconfig/modelconfigmanager.h>

#include <chrono>
#include <vector>

#include "enginestopcontroller.h"
#include "utils/logger.h"

namespace lifecycle {

namespace {
std::vector<std::string> parseModelsFromCatalog(const std::string &modelInfo, const std::string &catalogType) {
    std::vector<std::string> models;

    if (modelInfo.empty()) {
        return models;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(modelInfo, root) || !root.isObject()) {
        LOGE("Failed to parse modelInfo JSON");
        return models;
    }

    if (!root.isMember("model_catalog") || !root["model_catalog"].isObject()) {
        LOGE("model_catalog not found in modelInfo");
        return models;
    }

    const Json::Value &catalog = root["model_catalog"];
    if (!catalog.isMember(catalogType) || !catalog[catalogType].isArray()) {
        LOGD("Catalog type '{}' not found", catalogType);
        return models;
    }

    for (const auto &group : catalog[catalogType]) {
        if (group.isObject() && group.isMember("models") && group["models"].isArray()) {
            for (const auto &model : group["models"]) {
                if (model.isString()) {
                    models.push_back(model.asString());
                }
            }
        }
    }

    return models;
}

// 从语音 engine 获取 modelInfo 并解析模型
std::vector<std::string> getSpeechModels(EngineSelector &selector, const std::string &subType) {
    auto engine = selector.selectCoreAiSpeechEngine(DeployType::Local);
    if (!engine) {
        LOGW("Failed to select speech engine");
        return {};
    }

    std::string modelInfo = engine->modelInfo();

    if (subType == "speechToText") {
        // 流式语音识别
        auto streaming = parseModelsFromCatalog(modelInfo, "streaming_asr");
        return streaming;
    }
    if (subType == "textToSpeech") {
        // 语音合成
        return parseModelsFromCatalog(modelInfo, "tts");
    }
    return {};
}

// 从图像 engine 获取 modelInfo 并解析模型
std::vector<std::string> getVisionModels(EngineSelector &selector, const std::string &subType) {
    auto engine = selector.selectCoreAiImageProcessEngine();
    if (!engine) {
        LOGW("Failed to select vision engine");
        return {};
    }

    std::string modelInfo = engine->modelInfo();

    if (subType == "generalSeg") {
        return parseModelsFromCatalog(modelInfo, "seg_sam");
    }
    if (subType == "portraitSubjectSeg") {
        return parseModelsFromCatalog(modelInfo, "matting_portrait");
    }
    if (subType == "generalSubjectSeg") {
        return parseModelsFromCatalog(modelInfo, "matting_universal");
    }
    if (subType == "binarization") {
        return parseModelsFromCatalog(modelInfo, "binarization");
    }
    if (subType == "textRecognition") {
        return parseModelsFromCatalog(modelInfo, "ocr");
    }

    return {};
}

// 从 NLP engine 获取 modelInfo 并解析模型
std::vector<std::string> getNlpModels(EngineSelector &selector, const std::string &subType) {
    auto engine = selector.selectGenAiTextGenerationEngine();
    if (!engine) {
        LOGW("Failed to select NLP engine");
        return {};
    }

    std::string modelInfo = engine->modelInfo();

    if (subType == "llm") {
        return parseModelsFromCatalog(modelInfo, "LLM");
    } else if (subType == "embedding.text") {
        auto embeddingEngine = selector.selectCoreAiEmbeddingEngine("");
        if (embeddingEngine) {
            return parseModelsFromCatalog(embeddingEngine->modelInfo(), "TEXT");
        }
    } else if (subType == "embedding.image") {
        auto embeddingEngine = selector.selectCoreAiEmbeddingEngine("");
        if (embeddingEngine) {
            return parseModelsFromCatalog(embeddingEngine->modelInfo(), "IMAGE");
        }
    }

    return {};
}
}  // namespace

ModelLifecycleManager &ModelLifecycleManager::getInstance() {
    static ModelLifecycleManager instance;
    return instance;
}

void ModelLifecycleManager::loadDefaultConfig() {
    auto loaded = ModelLifecycleConfig::loadFromFile(ModelLifecycleConfig::defaultConfigPath());
    updateConfig(loaded);
}

void ModelLifecycleManager::updateConfig(const ModelLifecycleConfig &config) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    config_ = config;
}

ModelLifecycleConfig ModelLifecycleManager::config() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return config_;
}

ModelUsageHandle ModelLifecycleManager::acquire() { return ModelUsageHandle(this); }

void ModelLifecycleManager::acquireHandle(const ModelDescriptor &descriptor) {
    auto context = findOrCreateContext(descriptor);
    context->onAcquire();
}

void ModelLifecycleManager::markBusy(const ModelDescriptor &descriptor) {
    auto context = findOrCreateContext(descriptor);
    context->markBusy();
}

void ModelLifecycleManager::markIdle(const ModelDescriptor &descriptor) {
    auto context = findOrCreateContext(descriptor);
    context->markIdle();
    context->checkAndUnloadIfDisabled();
}

void ModelLifecycleManager::releaseHandle(const ModelDescriptor &descriptor) {
    auto context = findContextByModelName(descriptor.modelName);
    if (context) {
        context->onRelease();
    }
}

bool ModelLifecycleManager::ensureLoaded(const ModelDescriptor &descriptor, EngineCallStage stage) {
    auto context = findOrCreateContext(descriptor);
    if (!context) {
        return false;
    }
    return context->ensureLoaded(stage);
}

bool ModelLifecycleManager::startAsyncLoad(const ModelDescriptor &descriptor, EngineCallStage stage) {
    auto context = findOrCreateContext(descriptor);
    if (!context) {
        return false;
    }
    return context->startAsyncLoad(stage);
}

bool ModelLifecycleManager::isAsyncLoadComplete(const ModelDescriptor &descriptor) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto iter = contexts_.find(descriptor.modelName);
    if (iter == contexts_.end()) {
        return true;  // 模型上下文不存在，认为加载完成（未加载状态）
    }
    return iter->second->isAsyncLoadComplete();
}

bool ModelLifecycleManager::waitForAsyncLoad(const ModelDescriptor &descriptor, int timeoutMs) {
    auto context = findContextByModelName(descriptor.modelName);
    if (!context) {
        return false;
    }
    return context->waitForAsyncLoad(timeoutMs);
}

void ModelLifecycleManager::sweepIdleContexts() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<ModelRuntimeContext>> contexts;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        for (const auto &item : contexts_) {
            contexts.push_back(item.second);
        }
    }

    for (const auto &context : contexts) {
        auto timeout = config_.idleTimeout(context->descriptor());
        if (context->shouldUnload(now, timeout)) {
            context->forceUnload();
        }
    }
}

std::vector<ModelDescriptor> ModelLifecycleManager::forceUnloadForSleep(const std::string &reason) {
    std::vector<ModelDescriptor> unloaded;
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (const auto &item : contexts_) {
        const auto &ctx = item.second;
        if (!config_.unloadOnSleep(ctx->descriptor())) {
            continue;
        }
        LOGI("Force unload model [{}] reason: {}", ctx->descriptor().fingerprint(), reason);
        ctx->forceUnload();
        unloaded.push_back(ctx->descriptor());
    }
    return unloaded;
}

void ModelLifecycleManager::forceLoadModels(const std::vector<ModelDescriptor> &descriptors) {
    for (const auto &descriptor : descriptors) {
        auto context = findOrCreateContext(descriptor);
        context->forceLoad();
    }
}

void ModelLifecycleManager::forceLoadModelsParallel(const std::vector<ModelDescriptor> &descriptors) {
    if (descriptors.empty()) {
        return;
    }

    LOGI("Loading {} models in parallel", descriptors.size());

    std::vector<std::future<void>> futures;
    auto startTime = std::chrono::high_resolution_clock::now();

    for (const auto &descriptor : descriptors) {
        futures.emplace_back(std::async(std::launch::async, [this, descriptor]() {
            try {
                auto context = findOrCreateContext(descriptor);
                context->forceLoad();
                LOGD("Model loaded: {}", descriptor.fingerprint());
            } catch (const std::exception &e) {
                LOGE("Failed to load model {}: {}", descriptor.fingerprint(), e.what());
            }
        }));
    }

    for (auto &future : futures) {
        future.wait();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    LOGI("Parallel model loading completed in {} ms", duration);
}

void ModelLifecycleManager::loadModelsOnBoot() {
    auto &configManager = ModelConfigManager::getInstance();
    configManager.waitForModelInfosLoaded();

    auto &pluginManager = ai_engine::AiEnginePluginManager::getInstance();
    EngineSelector selector(pluginManager, configManager);

    struct BootLoadEntry {
        ModelFamily family;
        ModelSubType variant;
        AiCapability capability;
        const char *capName;
        const char *subType;
    };

    constexpr BootLoadEntry entries[] = {
        {ModelFamily::kSpeech, ModelSubType::kSpeechToText, AiCapability::Speech, "Speech", "speechToText"},
        {ModelFamily::kSpeech, ModelSubType::kTextToSpeech, AiCapability::Speech, "Speech", "textToSpeech"}};

    std::vector<ModelDescriptor> modelsToLoad;

    for (const auto &entry : entries) {
        if (!configManager.getLoadOnBoot(entry.capName, entry.subType)) {
            continue;
        }

        std::vector<std::string> modelNames;

        if (entry.capability == AiCapability::Speech) {
            modelNames = getSpeechModels(selector, entry.subType);
        } else if (entry.capability == AiCapability::Vision) {
            modelNames = getVisionModels(selector, entry.subType);
        } else if (entry.capability == AiCapability::Nlp) {
            modelNames = getNlpModels(selector, entry.subType);
        }

        for (const auto &modelName : modelNames) {
            ModelDescriptor descriptor;
            descriptor.modelName = modelName;
            descriptor.capability = entry.capability;
            descriptor.modelClass = {entry.family, entry.variant};
            modelsToLoad.push_back(descriptor);
        }
    }

    // 异步加载模型
    if (!modelsToLoad.empty()) {
        LOGI("Loading {} models on boot asynchronously", modelsToLoad.size());

        std::thread([modelsToLoad, this]() {
            auto startTime = std::chrono::high_resolution_clock::now();

            forceLoadModelsParallel(modelsToLoad);

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            LOGI("Async boot model loading completed in {} ms", duration);
        }).detach();

    } else {
        LOGD("No models configured for boot loading");
    }
}

std::shared_ptr<ModelRuntimeContext> ModelLifecycleManager::findOrCreateContext(const ModelDescriptor &descriptor) {
    auto profile = config_.profileFor(descriptor);
    ModelDescriptor resolvedDescriptor = descriptor;
    resolvedDescriptor.modelClass = profile.lifecycleClass;
    const auto key = resolvedDescriptor.modelName;

    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto iter = contexts_.find(key);
        if (iter != contexts_.end()) {
            return iter->second;
        }
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto iter = contexts_.find(key);
    if (iter != contexts_.end()) {
        if (iter->second->descriptor().fingerprint() == resolvedDescriptor.fingerprint()) {
            return iter->second;
        }
        auto context = std::make_shared<ModelRuntimeContext>(resolvedDescriptor, profile.lifecycleClass,
                                                             profile.callStage, profile.unloadPolicy);
        iter->second = context;
        return context;
    }

    auto context = std::make_shared<ModelRuntimeContext>(resolvedDescriptor, profile.lifecycleClass, profile.callStage,
                                                         profile.unloadPolicy);
    contexts_.emplace(key, context);
    return context;
}

std::shared_ptr<ModelRuntimeContext> ModelLifecycleManager::findContextByModelName(const std::string &modelName) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto iter = contexts_.find(modelName);
    if (iter == contexts_.end()) {
        return nullptr;
    }
    return iter->second;
}

bool ModelLifecycleManager::retainModelAndUnloadOthers(const ModelDescriptor &descriptor) {
    protectModel(descriptor);
    std::vector<ModelDescriptor> unloaded;
    std::vector<std::shared_ptr<ModelRuntimeContext>> contextsToCheck;
    bool targetModelFound = false;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        for (const auto &[key, context] : contexts_) {
            if (context->descriptor().capability == descriptor.capability) {
                if (context->descriptor().modelName == descriptor.modelName) {
                    context->forceLoad();
                    LOGI("Retain model [{}]", descriptor.fingerprint());
                    targetModelFound = true;
                    continue;
                }
                contextsToCheck.push_back(context);
            }
        }
    }

    // 如果目标模型上下文不存在，则创建并加载
    if (!targetModelFound) {
        auto targetContext = findOrCreateContext(descriptor);
        if (targetContext) {
            targetContext->forceLoad();
            LOGI("Retain model [{}] (context created)", descriptor.fingerprint());
        }
    }

    // 逐个停止并卸载其他模型
    for (const auto &context : contextsToCheck) {
        EngineStopController::getInstance().stopForModel(context->descriptor(), "retain_model");
        LOGI("Unload model [{}]", context->descriptor().fingerprint());
        context->forceUnload();
        unloaded.push_back(context->descriptor());
    }
    return true;
}

bool ModelLifecycleManager::cancelRetainModel(const ModelDescriptor &descriptor) {
    unprotectModel(descriptor);
    return true;
}

void ModelLifecycleManager::registerModelConfigCallback() {
    if (isCallbackRegistered_) {
        return;
    }

    // 获取模型配置管理器实例
    auto &modelConfigManager = ModelConfigManager::getInstance();

    // 为端侧的每种能力注册回调
    for (int cap = static_cast<int>(AiCapability::Nlp); cap <= static_cast<int>(AiCapability::Speech); cap++) {
        auto capability = static_cast<AiCapability>(cap);

        std::string callbackId = "lifecycle_" + std::to_string(cap) + "_";
        modelConfigManager.updateCallback(capability, DeployType::Local, callbackId,
                                          [this](ModelChangeType type) { this->handleModelConfigChanged(type); });
    }

    isCallbackRegistered_ = true;
    LOGI("ModelLifecycleManager registered model config callbacks");
}

void ModelLifecycleManager::handleModelConfigChanged(ModelChangeType type) const {
    if (type != ModelChangeType::DeployTypeChanged) {
        return;
    }
    auto &modelConfigManager = ModelConfigManager::getInstance();
    bool deployEnabled = modelConfigManager.getModelDeployEnabled(DeployType::Local);

    // 异步遍历所有上下文，处理禁用模型的卸载
    std::thread([this, deployEnabled]() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        for (auto context : contexts_) {
            const auto &descriptor = context.second->descriptor();

            if (deployEnabled) {
                context.second->setDisabled(true);
                LOGI("Model [{}] marked as disabled", descriptor.fingerprint());
                context.second->checkAndUnloadIfDisabled();
            } else {
                context.second->setDisabled(false);
                LOGI("Model [{}] marked as enabled", descriptor.fingerprint());
            }
        }
    }).detach();
}

void ModelLifecycleManager::protectModel(const ModelDescriptor &descriptor) {
    auto context = findContextByModelName(descriptor.modelName);
    if (context) {
        context->setProtected(true);
        LOGI("Model [{}] is now protected from automatic unload", descriptor.fingerprint());
    }
}

void ModelLifecycleManager::unprotectModel(const ModelDescriptor &descriptor) {
    auto context = findContextByModelName(descriptor.modelName);
    if (context) {
        context->setProtected(false);
        LOGI("Model [{}] is no longer protected", descriptor.fingerprint());
    }
}

}  // namespace lifecycle
