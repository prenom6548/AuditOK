/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/lifecycleawareengine.h"

#include <kylin-ai/common/error.h>

#include <future>

#include "utils/logger.h"

namespace lifecycle {

// 辅助函数：从字符串转换为AiCapability
AiCapability stringToCapability(const std::string &str) {
    if (str == "Nlp") return AiCapability::Nlp;
    if (str == "Vision") return AiCapability::Vision;
    if (str == "Speech") return AiCapability::Speech;
    return AiCapability::Nlp;  // 默认值
}

ModelDescriptor LifecycleAwareTextGenerationEngine::buildDescriptor(const std::string &modelName) const {
    return makeDescriptor(modelName, AiCapability::Nlp, {ModelFamily::kLLM, ModelSubType::kNone});
}

ModelDescriptor LifecycleAwareEmbeddingEngine::buildDescriptor(const std::string &modelName) const {
    return makeDescriptor(modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kNone});
}

ModelDescriptor LifecycleAwareEmbeddingEngine::buildTextEmbeddingDescriptor(const std::string &modelName) const {
    return makeDescriptor(modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kText});
}

ModelDescriptor LifecycleAwareEmbeddingEngine::buildImageEmbeddingDescriptor(const std::string &modelName) const {
    return makeDescriptor(modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
}

ModelGroupDescriptor LifecycleAwareEmbeddingEngine::buildEmbeddingGroupDescriptor(EmbeddingRequestType type) const {
    const std::string capability = mapRequestTypeToGroupName(type);
    ModelGroupDescriptor groupDesc(capability, "Vision", {});

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    auto it = parsedGroupMap_.find(capability);
    if (it != parsedGroupMap_.end() && !it->second.empty()) {
        for (const auto &modelName : it->second) {
            groupDesc.addModel(modelName, engineName());
        }
        LOGD("Using dynamically parsed model group '{}' with {} models", capability.c_str(), it->second.size());
    } else {
        LOGE("Failed to find model group '{}' in the model info", capability.c_str());
    }
    return groupDesc;
}

void LifecycleAwareEmbeddingEngine::parseModelInfo() const {
    auto engine = realEngine();
    if (!engine) return;

    std::string info = engine->modelInfo();
    if (info.empty()) {
        LOGW("Model info is empty");
        return;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(info, root) || !root.isObject()) {
        LOGE("Failed to parse modelInfo JSON or root is not an object");
        return;
    }

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    // 清空现有的模型组信息
    parsedGroupMap_.clear();

    if (!root.isMember("model_catalog") || !root["model_catalog"].isObject()) {
        LOGE("Model info does not contain 'model_catalog' field or it's not an object");
        return;
    }

    const Json::Value &modelCatalog = root["model_catalog"];
    const Json::Value::Members catalogMembers = modelCatalog.getMemberNames();

    for (const auto &catalogType : catalogMembers) {
        const Json::Value &catalogArray = modelCatalog[catalogType];
        if (!catalogArray.isArray()) {
            LOGW("Catalog type '{}' is not an array", catalogType.c_str());
            continue;
        }

        std::vector<std::string> allModels;
        for (const auto &modelGroup : catalogArray) {
            if (!modelGroup.isObject() || !modelGroup.isMember("models") || !modelGroup["models"].isArray()) {
                LOGW("Invalid model group structure for catalog type '{}'", catalogType.c_str());
                continue;
            }

            const Json::Value &modelsArray = modelGroup["models"];
            for (const auto &model : modelsArray) {
                if (model.isString()) {
                    allModels.push_back(model.asString());
                }
            }
        }

        if (!allModels.empty()) {
            std::set<std::string> uniqueModels(parsedGroupMap_[catalogType].begin(),
                                               parsedGroupMap_[catalogType].end());
            for (const auto &model : allModels) {
                uniqueModels.insert(model);
            }
            parsedGroupMap_[catalogType] = std::vector<std::string>(uniqueModels.begin(), uniqueModels.end());
            LOGI("Parsed {} models for catalog type '{}'", allModels.size(), catalogType.c_str());
        }
    }

    LOGI("Successfully parsed model info with {} catalog types", parsedGroupMap_.size());
}

std::string LifecycleAwareEmbeddingEngine::mapRequestTypeToGroupName(EmbeddingRequestType type) const {
    switch (type) {
        case EmbeddingRequestType::TextEmbedding:
            return "TEXT";
        case EmbeddingRequestType::ImageEmbedding:
            return "IMAGE";
        default:
            return "unknown";
    }
}

ModelDescriptor LifecycleAwareImageProcessEngine::buildDescriptor(const std::string &modelName) const {
    return buildDescriptorWithVariant(modelName, ModelSubType::kGeneralSeg);
}

ModelDescriptor LifecycleAwareImageProcessEngine::buildDescriptorWithVariant(const std::string &modelName,
                                                                             ModelSubType variant) const {
    return makeDescriptor(modelName, AiCapability::Vision, {ModelFamily::kImage, variant});
}

ModelGroupDescriptor LifecycleAwareImageProcessEngine::buildImageProcessGroupDescriptor(
    ImageProcessRequestType type) const {
    const std::string capability = mapRequestTypeToGroupName(type);
    ModelGroupDescriptor groupDesc(capability, "Vision", {});

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    auto it = parsedGroupMap_.find(capability);
    if (it != parsedGroupMap_.end() && !it->second.empty()) {
        for (const auto &modelName : it->second) {
            groupDesc.addModel(modelName, engineName());
        }
        LOGD("Using dynamically parsed model group '{}' with {} models", capability.c_str(), it->second.size());
    } else {
        LOGE("Failed to find model group '{}' in the model info", capability.c_str());
    }
    return groupDesc;
}

void LifecycleAwareTextGenerationEngine::registerStopper() {
    auto descriptor = buildDescriptor(currentModel());
    auto alive = aliveFlag();
    updateStopper(descriptor, [this, alive]() {
        if (!alive->load(std::memory_order_acquire) || !realEngine()) {
            LOGD("Stopper: Engine is not ready");
            return;
        }
        LOGI("Stopper: Stop chat");
        realEngine()->stopGenerateContent();
        realEngine()->stopChat();
    });
}

void LifecycleAwareTextGenerationEngine::afterAttach() { registerStopper(); }

bool LifecycleAwareTextGenerationEngine::setCurrentModel(const std::string &model) {
    bool ok = LifecycleAwareEngineProxyBase::setCurrentModel(model);
    if (ok) {
        registerStopper();
    }
    return ok;
}

void LifecycleAwareEmbeddingEngine::afterAttach() {
    parseModelInfo();
    registerStopper();
}

void LifecycleAwareEmbeddingEngine::registerStopper() {
    auto alive = aliveFlag();
    updateStopper(buildTextEmbeddingDescriptor(currentTextEmbeddingModel()), [this, alive]() {
        if (alive->load(std::memory_order_acquire) && realEngine()) {
            LOGI("Stopper: Stop text embedding");
            realEngine()->stopTextEmbedding();
        }
    });
    updateStopper(buildImageEmbeddingDescriptor(currentImageEmbeddingModel()), [this, alive]() {
        if (alive->load(std::memory_order_acquire) && realEngine()) {
            LOGI("Stopper: Stop image embedding");
            realEngine()->stopImageEmbedding();
        }
    });
}

void LifecycleAwareImageProcessEngine::registerStopper() {
    auto alive = aliveFlag();
    updateStopper(buildDescriptorWithVariant(currentPortraitSegmentationModel(), ModelSubType::kPortraitSubjectSeg),
                  [this, alive]() {
                      if (alive->load(std::memory_order_acquire) && realEngine()) {
                          LOGI("Stopper: Stop portrait segmentation");
                          realEngine()->stopMattingProcess();
                      }
                  });
    updateStopper(buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kGeneralSubjectSeg),
                  [this, alive]() {
                      if (alive->load(std::memory_order_acquire) && realEngine()) {
                          LOGI("Stopper: Stop general segmentation");
                          realEngine()->stopMattingProcess();
                      }
                  });
    updateStopper(buildDescriptorWithVariant(activeModelName(), ModelSubType::kBinarization), [this, alive]() {
        if (alive->load(std::memory_order_acquire) && realEngine()) {
            LOGI("Stopper: Stop image binarization");
            realEngine()->stopImageBinarization();
        }
    });
    updateStopper(buildDescriptorWithVariant(currentSegmentationModel(), ModelSubType::kGeneralSeg), [this, alive]() {
        if (alive->load(std::memory_order_acquire) && realEngine()) {
            LOGI("Stopper: Stop segmentation");
            realEngine()->stopSegmentation();
        }
    });
}

void LifecycleAwareImageProcessEngine::afterAttach() {
    parseModelInfo();
    registerStopper();
}

// Text generation
ai_engine::result::AiEngineBaseResult LifecycleAwareTextGenerationEngine::init() {
    ai_engine::result::AiEngineBaseResult result =
        realEngine() ? realEngine()->init() : ai_engine::result::AiEngineBaseResult{};

    if (result.success) {
        auto descriptor = buildDescriptor(currentModel());
        startAsyncLoad(descriptor, EngineCallStage::kOnInitialization);
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextGenerationEngine::destroy() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    return invoke(buildDescriptor(currentModel()), options, [this](auto engine) { return engine->destroy(); });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextGenerationEngine::generateContent(const std::string &input) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(buildDescriptor(currentModel()), options,
                  [this, &input](auto engine) { return engine->generateContent(input); });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextGenerationEngine::chat(const std::string &message) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(buildDescriptor(currentModel()), options,
                  [this, &message](auto engine) { return engine->chat(message); });
}

void LifecycleAwareTextGenerationEngine::stopGenerateContent() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    invoke(buildDescriptor(currentModel()), options, [this](auto engine) {
        LOGI("Stop generate content");
        engine->stopGenerateContent();
        return ai_engine::result::AiEngineBaseResult();
    });
}

void LifecycleAwareTextGenerationEngine::stopChat() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    invoke(buildDescriptor(currentModel()), options, [this](auto engine) {
        LOGI("Stop chat");
        engine->stopChat();
        return ai_engine::result::AiEngineBaseResult();
    });
}

void LifecycleAwareTextGenerationEngine::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    if (realEngine()) {
        realEngine()->setTextGenerationResultCallback(std::move(callback));
    }
}

// Embedding
ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::setCurrentTextEmbeddingModel(
    const std::string &model) {
    auto result =
        realEngine() ? realEngine()->setCurrentTextEmbeddingModel(model) : ai_engine::result::AiEngineBaseResult{};
    registerStopper();
    return result;
}

std::string LifecycleAwareEmbeddingEngine::currentTextEmbeddingModel() {
    return this->realEngine() ? this->realEngine()->currentTextEmbeddingModel() : std::string();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::setCurrentImageEmbeddingModel(
    const std::string &model) {
    auto result =
        realEngine() ? realEngine()->setCurrentImageEmbeddingModel(model) : ai_engine::result::AiEngineBaseResult{};
    registerStopper();
    return result;
}

std::string LifecycleAwareEmbeddingEngine::currentImageEmbeddingModel() {
    return this->realEngine() ? this->realEngine()->currentImageEmbeddingModel() : std::string();
}

void LifecycleAwareEmbeddingEngine::setEmbeddingResultCallback(
    ai_engine::core_ai::embedding::EmbeddingResultCallback callback) {
    if (realEngine()) {
        realEngine()->setEmbeddingResultCallback(std::move(callback));
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::init() {
    ai_engine::result::AiEngineBaseResult result =
        realEngine() ? realEngine()->init() : ai_engine::result::AiEngineBaseResult{};

    if (result.success) {
        auto groupDescTextEmbedding = buildEmbeddingGroupDescriptor(TextEmbedding);
        auto groupDescImageEmbedding = buildEmbeddingGroupDescriptor(ImageEmbedding);

        for (const auto &modelDesc : groupDescTextEmbedding.models) {
            auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                             {ModelFamily::kEmbedding, ModelSubType::kText});
            startAsyncLoad(descriptor, EngineCallStage::kOnInitialization);
        }
        for (const auto &modelDesc : groupDescImageEmbedding.models) {
            auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                             {ModelFamily::kEmbedding, ModelSubType::kImage});
            startAsyncLoad(descriptor, EngineCallStage::kOnInitialization);
        }
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::embeddingText(const std::string &text) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildEmbeddingGroupDescriptor(TextEmbedding);
    ai_engine::result::AiEngineBaseResult result;
    bool embeddingCalled = false;
    auto doEmbedding = [&]() {
        if (!embeddingCalled) {
            result = realEngine() ? realEngine()->embeddingText(text) : ai_engine::result::AiEngineBaseResult{};
            embeddingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kText});
        invoke(descriptor, options, [&](auto engine) { return doEmbedding(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::embeddingImageFromImageFile(
    const std::string &filePath) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildEmbeddingGroupDescriptor(ImageEmbedding);
    ai_engine::result::AiEngineBaseResult result;
    bool embeddingCalled = false;
    auto doEmbedding = [&]() {
        if (!embeddingCalled) {
            result = realEngine() ? realEngine()->embeddingImageFromImageFile(filePath)
                                  : ai_engine::result::AiEngineBaseResult{};
            embeddingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
        invoke(descriptor, options, [&](auto engine) { return doEmbedding(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::embeddingImageFromBase64ImageData(
    const std::string &fileData) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildEmbeddingGroupDescriptor(ImageEmbedding);
    ai_engine::result::AiEngineBaseResult result;
    bool embeddingCalled = false;
    auto doEmbedding = [&]() {
        if (!embeddingCalled) {
            result = realEngine() ? realEngine()->embeddingImageFromBase64ImageData(fileData)
                                  : ai_engine::result::AiEngineBaseResult{};
            embeddingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
        invoke(descriptor, options, [&](auto engine) { return doEmbedding(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareEmbeddingEngine::embeddingTextWithImageModel(
    const std::string &text) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildEmbeddingGroupDescriptor(ImageEmbedding);
    ai_engine::result::AiEngineBaseResult result;
    bool embeddingCalled = false;
    auto doEmbedding = [&]() {
        if (!embeddingCalled) {
            result = realEngine() ? realEngine()->embeddingTextWithImageModel(text)
                                  : ai_engine::result::AiEngineBaseResult{};
            embeddingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
        invoke(descriptor, options, [&](auto engine) { return doEmbedding(); });
    }
    return result;
}

void LifecycleAwareEmbeddingEngine::stopImageEmbedding() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildEmbeddingGroupDescriptor(ImageEmbedding);
    bool stopped = false;
    auto doStop = [&]() {
        if (!stopped) {
            invoke(buildImageEmbeddingDescriptor(currentImageEmbeddingModel()), options, [this](auto engine) {
                LOGI("Stop image embedding");
                return engine->stopImageEmbedding();
            });
            stopped = true;
        }
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kImage});
        invoke(descriptor, options, [&](auto engine) { return doStop(); });
    }
}

void LifecycleAwareEmbeddingEngine::stopTextEmbedding() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildEmbeddingGroupDescriptor(TextEmbedding);
    bool stopped = false;
    auto doStop = [&]() {
        if (!stopped) {
            invoke(buildTextEmbeddingDescriptor(currentTextEmbeddingModel()), options, [this](auto engine) {
                LOGI("Stop text embedding");
                return engine->stopTextEmbedding();
            });
            stopped = true;
        }
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kEmbedding, ModelSubType::kText});
        invoke(descriptor, options, [&](auto engine) { return doStop(); });
    }
}

// Image process
std::string LifecycleAwareImageProcessEngine::currentPortraitSegmentationModel() const {
    return this->realEngine() ? this->realEngine()->currentPortraitSegmentationModel() : std::string();
}

std::string LifecycleAwareImageProcessEngine::currentGeneralSegmentationModel() const {
    return this->realEngine() ? this->realEngine()->currentGeneralSegmentationModel() : std::string();
}

void LifecycleAwareImageProcessEngine::setCurrentPortraitSegmentationModel(const std::string &model) {
    if (realEngine()) {
        realEngine()->setCurrentPortraitSegmentationModel(model);
    }
    registerStopper();
}

void LifecycleAwareImageProcessEngine::setCurrentGeneralSegmentationModel(const std::string &model) {
    if (realEngine()) {
        realEngine()->setCurrentGeneralSegmentationModel(model);
    }
    registerStopper();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::initMattingsModule() {
    LifecycleBorrowOptions borrowOptions;
    borrowOptions.trackUsage = false;
    borrowOptions.callStage = EngineCallStage::kOnInitialization;

    ai_engine::result::AiEngineBaseResult result;
    bool initialized = false;
    if (realEngine()) {
        realEngine()->setCurrentPortraitSegmentationModel("matting_portrait");
        realEngine()->setCurrentGeneralSegmentationModel("matting_universal");
    }
    auto groupDescPortrait = buildImageProcessGroupDescriptor(PortraitSegmentation);
    auto groupDescGeneral = buildImageProcessGroupDescriptor(GeneralSegmentation);

    auto doInit = [this, &initialized, &result]() {
        if (!initialized) {
            result = realEngine() ? realEngine()->initMattingsModule() : ai_engine::result::AiEngineBaseResult{};
            initialized = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDescPortrait.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kPortraitSubjectSeg});
        invoke(descriptor, borrowOptions, [&](auto engine) { return doInit(); });
    }
    for (const auto &modelDesc : groupDescGeneral.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kGeneralSubjectSeg});
        invoke(descriptor, borrowOptions, [&](auto engine) { return doInit(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::destroyMattingsModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDescPortrait = buildImageProcessGroupDescriptor(PortraitSegmentation);
    auto groupDescGeneral = buildImageProcessGroupDescriptor(GeneralSegmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool destroyed = false;
    auto doDestroy = [this, &destroyed, &result]() {
        if (!destroyed) {
            result = realEngine() ? realEngine()->destroyMattingsModule() : ai_engine::result::AiEngineBaseResult{};
            destroyed = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDescPortrait.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kPortraitSubjectSeg});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }
    for (const auto &modelDesc : groupDescGeneral.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kGeneralSubjectSeg});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }
    return result;
}

void LifecycleAwareImageProcessEngine::setMattingResultCallback(
    ai_engine::core_ai::vision::ImageResultCallback callback) {
    if (realEngine()) {
        realEngine()->setMattingResultCallback(std::move(callback));
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::mattingFromBase64ImageData(
    const std::string &imageBase64Data, ai_engine::core_ai::vision::MattingMode mode) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildImageProcessGroupDescriptor(
        mode == ai_engine::core_ai::vision::MattingMode::MATTING_MODE_PORTRAIT ? PortraitSegmentation
                                                                               : GeneralSegmentation);
    auto subType = mode == ai_engine::core_ai::vision::MattingMode::MATTING_MODE_PORTRAIT
                       ? ModelSubType::kPortraitSubjectSeg
                       : ModelSubType::kGeneralSubjectSeg;
    ai_engine::result::AiEngineBaseResult result;
    bool mattingCalled = false;
    auto doMatting = [this, &imageBase64Data, &mode, &result, &mattingCalled]() {
        if (!mattingCalled) {
            result = realEngine() ? realEngine()->mattingFromBase64ImageData(imageBase64Data, mode)
                                  : ai_engine::result::AiEngineBaseResult{};
            mattingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, subType});
        invoke(descriptor, options, [&](auto engine) { return doMatting(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::mattingFromImageFile(
    const std::string &imageFilePath, ai_engine::core_ai::vision::MattingMode mode) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildImageProcessGroupDescriptor(
        mode == ai_engine::core_ai::vision::MattingMode::MATTING_MODE_PORTRAIT ? PortraitSegmentation
                                                                               : GeneralSegmentation);
    auto subType = mode == ai_engine::core_ai::vision::MattingMode::MATTING_MODE_PORTRAIT
                       ? ModelSubType::kPortraitSubjectSeg
                       : ModelSubType::kGeneralSubjectSeg;
    ai_engine::result::AiEngineBaseResult result;
    bool mattingCalled = false;
    auto doMatting = [this, &imageFilePath, &mode, &result, &mattingCalled]() {
        if (!mattingCalled) {
            result = realEngine() ? realEngine()->mattingFromImageFile(imageFilePath, mode)
                                  : ai_engine::result::AiEngineBaseResult{};
            mattingCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, subType});
        invoke(descriptor, options, [&](auto engine) { return doMatting(); });
    }
    return result;
}

void LifecycleAwareImageProcessEngine::stopMattingProcess() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDescPortrait = buildImageProcessGroupDescriptor(PortraitSegmentation);
    auto groupDescGeneral = buildImageProcessGroupDescriptor(GeneralSegmentation);
    bool stopped = false;

    auto doStop = [this, &stopped]() {
        if (!stopped) {
            if (realEngine()) {
                LOGI("Stopping matting process");
                realEngine()->stopMattingProcess();
            }
            stopped = true;
        }
    };

    for (const auto &modelDesc : groupDescPortrait.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kPortraitSubjectSeg});
        invoke(descriptor, options, [&](auto) { doStop(); });
    }
    for (const auto &modelDesc : groupDescGeneral.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Vision,
                                         {ModelFamily::kImage, ModelSubType::kGeneralSubjectSeg});
        invoke(descriptor, options, [&](auto) { doStop(); });
    }
}

void LifecycleAwareImageProcessEngine::parseModelInfo() const {
    auto engine = realEngine();
    if (!engine) return;

    std::string info = engine->modelInfo();
    if (info.empty()) {
        LOGW("Model info is empty");
        return;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(info, root) || !root.isObject()) {
        LOGE("Failed to parse modelInfo JSON or root is not an object");
        return;
    }

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    // 清空现有的模型组信息
    parsedGroupMap_.clear();

    if (!root.isMember("model_catalog") || !root["model_catalog"].isObject()) {
        LOGE("Model info does not contain 'model_catalog' field or it's not an object");
        return;
    }

    const Json::Value &modelCatalog = root["model_catalog"];
    const Json::Value::Members catalogMembers = modelCatalog.getMemberNames();

    for (const auto &catalogType : catalogMembers) {
        const Json::Value &catalogArray = modelCatalog[catalogType];
        if (!catalogArray.isArray()) {
            LOGW("Catalog type '{}' is not an array", catalogType.c_str());
            continue;
        }

        std::vector<std::string> allModels;
        for (const auto &modelGroup : catalogArray) {
            if (!modelGroup.isObject() || !modelGroup.isMember("models") || !modelGroup["models"].isArray()) {
                LOGW("Invalid model group structure for catalog type '{}'", catalogType.c_str());
                continue;
            }

            const Json::Value &modelsArray = modelGroup["models"];
            for (const auto &model : modelsArray) {
                if (model.isString()) {
                    allModels.push_back(model.asString());
                }
            }
        }

        if (!allModels.empty()) {
            std::set<std::string> uniqueModels(parsedGroupMap_[catalogType].begin(),
                                               parsedGroupMap_[catalogType].end());
            for (const auto &model : allModels) {
                uniqueModels.insert(model);
            }
            parsedGroupMap_[catalogType] = std::vector<std::string>(uniqueModels.begin(), uniqueModels.end());
            LOGI("Parsed {} models for catalog type '{}'", allModels.size(), catalogType.c_str());
        }
    }

    LOGI("Successfully parsed model info with {} catalog types", parsedGroupMap_.size());
}

std::string LifecycleAwareImageProcessEngine::mapRequestTypeToGroupName(ImageProcessRequestType type) const {
    switch (type) {
        case ImageProcessRequestType::PortraitSegmentation:
            return "matting_portrait";
        case ImageProcessRequestType::GeneralSegmentation:
            return "matting_universal";
        case ImageProcessRequestType::Segmentation:
            return "seg_sam";
        default:
            return "unknown";
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::initImageBinaryModule() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnInitialization;
    options.trackUsage = false;
    return invoke(buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kBinarization), options,
                  [this](auto engine) { return engine->initImageBinaryModule(); });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::destroyImageBinaryModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    return invoke(buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kBinarization), options,
                  [this](auto engine) { return engine->destroyImageBinaryModule(); });
}

void LifecycleAwareImageProcessEngine::setImageBinaryResultCallback(
    ai_engine::core_ai::vision::ImageResultCallback callback) {
    if (realEngine()) {
        realEngine()->setImageBinaryResultCallback(std::move(callback));
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::binarizeImageFromBase64ImageData(
    const std::string &imageBase64Data) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(
        buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kBinarization), options,
        [this, &imageBase64Data](auto engine) { return engine->binarizeImageFromBase64ImageData(imageBase64Data); });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::binarizeImageFromImageFile(
    const std::string &imageFilePath) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kBinarization), options,
                  [this, &imageFilePath](auto engine) { return engine->binarizeImageFromImageFile(imageFilePath); });
}

void LifecycleAwareImageProcessEngine::stopImageBinarization() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    invoke(buildDescriptorWithVariant(currentGeneralSegmentationModel(), ModelSubType::kBinarization), options,
           [this](auto engine) {
               LOGI("Stopping image binarization");
               engine->stopImageBinarization();
               return ai_engine::result::AiEngineBaseResult();
           });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::initSegmentationModule() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnInitialization;
    options.trackUsage = false;

    if (realEngine()) {
        realEngine()->setCurrentSegmentationModel("seg_sam");
    }

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool initialized = false;

    auto doInit = [this, &initialized, &result]() {
        if (!initialized) {
            result = realEngine() ? realEngine()->initSegmentationModule() : ai_engine::result::AiEngineBaseResult{};
            initialized = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto engine) { return doInit(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::destroySegmentationModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool destroyed = false;

    auto doDestroy = [this, &destroyed, &result]() {
        if (!destroyed) {
            result = realEngine() ? realEngine()->destroySegmentationModule() : ai_engine::result::AiEngineBaseResult{};
            destroyed = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }

    return result;
}

void LifecycleAwareImageProcessEngine::setSegmentationResultCallback(
    const ai_engine::core_ai::vision::SegmentationResultCallback &callback) {
    if (realEngine()) {
        realEngine()->setSegmentationResultCallback(std::move(callback));
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::encodeSegmentationFromBase64ImageData(
    const std::string &base64ImageData) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool encodeCalled = false;
    auto doEncode = [this, &base64ImageData, &result, &encodeCalled]() {
        if (!encodeCalled) {
            result = realEngine() ? realEngine()->encodeSegmentationFromBase64ImageData(base64ImageData)
                                  : ai_engine::result::AiEngineBaseResult{};
            encodeCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto engine) { return doEncode(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::encodeSegmentationFromImageFile(
    const std::string &imageFilePath) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool encodeCalled = false;
    auto doEncode = [this, &imageFilePath, &result, &encodeCalled]() {
        if (!encodeCalled) {
            result = realEngine() ? realEngine()->encodeSegmentationFromImageFile(imageFilePath)
                                  : ai_engine::result::AiEngineBaseResult{};
            encodeCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto engine) { return doEncode(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::decodeSegmentation(
    const std::vector<ai_engine::core_ai::vision::SegmentationPoint> &points, uint32_t minArea) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool decodeCalled = false;
    auto doDecode = [this, &points, &minArea, &result, &decodeCalled]() {
        if (!decodeCalled) {
            result = realEngine() ? realEngine()->decodeSegmentation(points, minArea)
                                  : ai_engine::result::AiEngineBaseResult{};
            decodeCalled = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto engine) { return doDecode(); });
    }
    return result;
}

void LifecycleAwareImageProcessEngine::clearSegmentationCache() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    options.trackUsage = false;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    bool cleared = false;

    auto doClear = [this, &cleared]() {
        if (!cleared) {
            if (realEngine()) {
                LOGI("Clearing segmentation cache");
                realEngine()->clearSegmentationCache();
            }
            cleared = true;
        }
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto) { doClear(); });
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareImageProcessEngine::stopSegmentation() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildImageProcessGroupDescriptor(Segmentation);
    ai_engine::result::AiEngineBaseResult result;
    bool stopped = false;

    auto doStop = [this, &stopped, &result]() {
        if (!stopped) {
            if (realEngine()) {
                LOGI("Stopping segmentation");
                result = realEngine() ? realEngine()->stopSegmentation() : ai_engine::result::AiEngineBaseResult{};
            } else {
                result = ai_engine::result::AiEngineBaseResult{};
            }
            stopped = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor =
            makeDescriptor(modelDesc.modelName, AiCapability::Vision, {ModelFamily::kImage, ModelSubType::kGeneralSeg});
        invoke(descriptor, options, [&](auto) { return doStop(); });
    }
    return result;
}

void LifecycleAwareImageProcessEngine::setCurrentSegmentationModel(const std::string &model) {
    if (realEngine()) {
        realEngine()->setCurrentSegmentationModel(model);
    }
    registerStopper();
}

std::string LifecycleAwareImageProcessEngine::currentSegmentationModel() const {
    return this->realEngine() ? this->realEngine()->currentSegmentationModel() : std::string();
}

std::string LifecycleAwareImageProcessEngine::activeSegmentationModelName() const {
    if (this->realEngine()) {
        auto segmentation = this->realEngine()->currentSegmentationModel();
        return segmentation;
    }
    return std::string();
}

// TextRecognition
ModelDescriptor LifecycleAwareTextRecognitionEngine::buildDescriptor(const std::string &modelName) const {
    return buildDescriptorWithVariant(modelName, ModelSubType::kGeneralSeg);
}

ModelDescriptor LifecycleAwareTextRecognitionEngine::buildDescriptorWithVariant(const std::string &modelName,
                                                                                ModelSubType variant) const {
    return makeDescriptor(modelName, AiCapability::Vision, {ModelFamily::kImage, variant});
}

void LifecycleAwareTextRecognitionEngine::registerStopper() {
    auto descriptor = buildDescriptor(currentModel());
    auto alive = aliveFlag();
    updateStopper(descriptor, [this, alive]() {
        if (!alive->load(std::memory_order_acquire) || !realEngine()) {
            return;
        }
        LOGI("Stopper: Stopping text recognition");
        realEngine()->stopTextRecognition();
    });
}

void LifecycleAwareTextRecognitionEngine::afterAttach() { registerStopper(); }

ai_engine::result::AiEngineBaseResult LifecycleAwareTextRecognitionEngine::init() {
    if (realEngine()) {
        realEngine()->setCurrentTextRecognitionModel("ocr_ppocr");
    }

    ai_engine::result::AiEngineBaseResult result =
        realEngine() ? realEngine()->init() : ai_engine::result::AiEngineBaseResult{};

    if (result.success) {
        auto descriptor = buildDescriptorWithVariant(currentTextRecognitionModel(), ModelSubType::kTextRecognition);
        startAsyncLoad(descriptor, EngineCallStage::kOnInitialization);
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextRecognitionEngine::destroy() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    return invoke(buildDescriptorWithVariant(currentTextRecognitionModel(), ModelSubType::kTextRecognition), options,
                  [this](auto engine) { return engine->destroy(); });
}

void LifecycleAwareTextRecognitionEngine::setTextRecognitionResultCallback(
    const ai_engine::core_ai::vision::TextRecognitionResultCallback callback) {
    if (realEngine()) {
        realEngine()->setTextRecognitionResultCallback(std::move(callback));
    }
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextRecognitionEngine::recognizeTextFromBase64ImageData(
    const std::string &base64ImageData) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(
        buildDescriptorWithVariant(currentTextRecognitionModel(), ModelSubType::kTextRecognition), options,
        [this, &base64ImageData](auto engine) { return engine->recognizeTextFromBase64ImageData(base64ImageData); });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareTextRecognitionEngine::recognizeTextFromFile(
    const std::string &imageFilePath) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    return invoke(buildDescriptorWithVariant(currentTextRecognitionModel(), ModelSubType::kTextRecognition), options,
                  [this, &imageFilePath](auto engine) { return engine->recognizeTextFromFile(imageFilePath); });
}

void LifecycleAwareTextRecognitionEngine::stopTextRecognition() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;
    invoke(buildDescriptorWithVariant(currentTextRecognitionModel(), ModelSubType::kTextRecognition), options,
           [this](auto engine) {
               LOGI("Stopping text recognition");
               engine->stopTextRecognition();
               return ai_engine::result::AiEngineBaseResult();
           });
}

void LifecycleAwareTextRecognitionEngine::setCurrentTextRecognitionModel(const std::string &model) {
    if (realEngine()) {
        realEngine()->setCurrentTextRecognitionModel(model);
    }
    registerStopper();
}
std::string LifecycleAwareTextRecognitionEngine::currentTextRecognitionModel() const {
    return this->realEngine() ? this->realEngine()->currentTextRecognitionModel() : std::string();
}

// Speech
ModelDescriptor LifecycleAwareSpeechEngine::buildDescriptor(const std::string &modelName) const {
    return buildDescriptorWithVariant(modelName, ModelSubType::kNone);
}

ModelDescriptor LifecycleAwareSpeechEngine::buildDescriptorWithVariant(const std::string &modelName,
                                                                       ModelSubType variant) const {
    return makeDescriptor(modelName, AiCapability::Speech, {ModelFamily::kSpeech, variant});
}

// Speech Engine Implementation
ModelGroupDescriptor LifecycleAwareSpeechEngine::buildSpeechGroupDescriptor(SpeechRequestType type) const {
    const std::string capability = mapRequestTypeToGroupName(type);
    ModelGroupDescriptor groupDesc(capability, "Speech", {});

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    auto it = parsedGroupMap_.find(capability);
    if (it != parsedGroupMap_.end() && !it->second.empty()) {
        for (const auto &modelName : it->second) {
            groupDesc.addModel(modelName, engineName());
        }
    } else {
        LOGE("Failed to find model group '{}' in the model info", capability.c_str());
    }
    return groupDesc;
}

LifecycleAwareSpeechEngine::~LifecycleAwareSpeechEngine() {
    for (const auto &entry : stopperDescriptors_) {
        LOGI("Unregistering stopper for model '{}'", entry.first);
        EngineStopController::getInstance().unregisterStopper(entry.second);
    }
    stopperDescriptors_.clear();
    stopperRegistered_.clear();
}

void LifecycleAwareSpeechEngine::registerStopper(std::optional<SpeechRequestType> type) {
    auto registerSingleStopper = [this](const std::string &key, const ModelDescriptor &desc,
                                        std::function<void()> stopFn) {
        if (desc.modelName.empty()) {
            return;
        }

        auto it = stopperDescriptors_.find(key);
        if (it != stopperDescriptors_.end() && it->second.modelName == desc.modelName) {
            return;
        }

        if (it != stopperDescriptors_.end()) {
            EngineStopController::getInstance().unregisterStopper(it->second);
        }

        EngineStopController::getInstance().registerStopper(desc, stopFn);
        stopperDescriptors_[key] = desc;
        stopperRegistered_[key] = true;
    };

    // 注册连续识别的 stopper
    if (!type.has_value() || type.value() == ContinuousRecognition) {
        auto continuousModel = currentContinuousRecognitionModel();
        auto continuousDesc = buildDescriptorWithVariant(continuousModel, ModelSubType::kSpeechToText);
        auto alive = aliveFlag();
        registerSingleStopper("speech_to_text_continuous", continuousDesc, [this, alive]() {
            if (alive->load(std::memory_order_acquire) && realEngine()) {
                LOGI("Stopper: Stopping continuous recognition");
                realEngine()->stopContinuousRecognition();
            }
        });
    }

    // 注册单次识别的 stopper
    if (!type.has_value() || type.value() == OnceRecognition) {
        auto onceModel = currentRecognizeOnceModel();
        auto onceDesc = buildDescriptorWithVariant(onceModel, ModelSubType::kSpeechToText);
        auto alive = aliveFlag();
        registerSingleStopper("speech_to_text_once", onceDesc, [this, alive]() {
            if (alive->load(std::memory_order_acquire) && realEngine()) {
                LOGI("Stopper: Stopping recognize once");
                realEngine()->stopRecognizeOnce();
            }
        });
    }

    // 注册语音合成的 stopper
    if (!type.has_value() || type.value() == Synthesis) {
        auto synthesisModel = currentSynthesizeModel();
        auto synthesisDesc = buildDescriptorWithVariant(synthesisModel, ModelSubType::kTextToSpeech);
        auto alive = aliveFlag();
        registerSingleStopper("text_to_speech", synthesisDesc, [this, alive]() {
            if (alive->load(std::memory_order_acquire) && realEngine()) {
                LOGI("Stopper: Stopping synthesize");
                realEngine()->stopSynthesize();
            }
        });
    }
}

void LifecycleAwareSpeechEngine::afterAttach() {
    parseModelInfo();
    registerStopper();
}

void LifecycleAwareSpeechEngine::parseModelInfo() const {
    auto engine = realEngine();
    if (!engine) return;

    std::string info = engine->modelInfo();
    if (info.empty()) {
        LOGW("Model info is empty");
        return;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(info, root) || !root.isObject()) {
        LOGE("Failed to parse modelInfo JSON or root is not an object");
        return;
    }

    std::lock_guard<std::mutex> lock(groupMapMutex_);

    // 清空现有的模型组信息
    parsedGroupMap_.clear();

    if (!root.isMember("model_catalog") || !root["model_catalog"].isObject()) {
        LOGE("Model info does not contain 'model_catalog' field or it's not an object");
        return;
    }

    const Json::Value &modelCatalog = root["model_catalog"];
    const Json::Value::Members catalogMembers = modelCatalog.getMemberNames();

    for (const auto &catalogType : catalogMembers) {
        const Json::Value &catalogArray = modelCatalog[catalogType];
        if (!catalogArray.isArray()) {
            LOGW("Catalog type '{}' is not an array", catalogType.c_str());
            continue;
        }

        std::vector<std::string> allModels;
        for (const auto &modelGroup : catalogArray) {
            if (!modelGroup.isObject() || !modelGroup.isMember("models") || !modelGroup["models"].isArray()) {
                LOGW("Invalid model group structure for catalog type '{}'", catalogType.c_str());
                continue;
            }

            const Json::Value &modelsArray = modelGroup["models"];
            for (const auto &model : modelsArray) {
                if (model.isString()) {
                    allModels.push_back(model.asString());
                }
            }
        }

        if (!allModels.empty()) {
            std::set<std::string> uniqueModels(parsedGroupMap_[catalogType].begin(),
                                               parsedGroupMap_[catalogType].end());
            for (const auto &model : allModels) {
                uniqueModels.insert(model);
            }
            parsedGroupMap_[catalogType] = std::vector<std::string>(uniqueModels.begin(), uniqueModels.end());
            LOGI("Parsed {} models for catalog type '{}'", allModels.size(), catalogType.c_str());
        }
    }

    LOGI("Successfully parsed model info with {} catalog types", parsedGroupMap_.size());
}

std::string LifecycleAwareSpeechEngine::mapRequestTypeToGroupName(SpeechRequestType type) const {
    switch (type) {
        case SpeechRequestType::OnceRecognition:
            return "non_streaming_asr";
        case SpeechRequestType::ContinuousRecognition:
            return "streaming_asr";
        case SpeechRequestType::Synthesis:
            return "tts";
        default:
            return "unknown";
    }
}

void LifecycleAwareSpeechEngine::setRecognitionResultCallback(
    ai_engine::core_ai::speech::RecognitionResultCallback callback) {
    if (realEngine()) {
        realEngine()->setRecognitionResultCallback(std::move(callback));
    }
}

void LifecycleAwareSpeechEngine::setSynthesisResultCallback(
    ai_engine::core_ai::speech::SynthesisResultCallback callback) {
    if (realEngine()) {
        realEngine()->setSynthesisResultCallback(std::move(callback));
    }
}

std::string LifecycleAwareSpeechEngine::currentContinuousRecognitionModel() const {
    return this->realEngine() ? this->realEngine()->currentContinuousRecognitionModel() : std::string();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::setCurrentContinuousRecognitionModel(
    const std::string &model) {
    auto result = realEngine() ? realEngine()->setCurrentContinuousRecognitionModel(model)
                               : ai_engine::result::AiEngineBaseResult{};
    registerStopper(ContinuousRecognition);
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::initContinuousRecognitionModule() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnInitialization;
    options.trackUsage = false;
    if (realEngine()) {
        LOGI("setCurrentContinuousRecognitionModel: paraformer-online");
        setCurrentContinuousRecognitionModel("paraformer-online");
    }

    auto groupDesc = buildSpeechGroupDescriptor(ContinuousRecognition);
    if (groupDesc.models.empty()) {
        LOGE("initContinuousRecognitionModule failed, no continuous recognition model found");
        return ai_engine::result::AiEngineBaseResult{
            false, {AI_COMMON_SPEECH_UNKNOWN_ERROR, "no continuous recognition model found"}};
    }
    std::promise<ai_engine::result::AiEngineBaseResult> promise;
    std::shared_future sharedFuture = promise.get_future();

    std::once_flag initFlag;

    auto doInit = [this, &promise, &initFlag]() {
        std::call_once(initFlag, [this, &promise]() {
            auto result = realEngine() ? realEngine()->initContinuousRecognitionModule()
                                       : ai_engine::result::AiEngineBaseResult{};
            promise.set_value(result);
        });
    };

    std::vector<std::future<void>> futures;
    auto startTime = std::chrono::high_resolution_clock::now();
    for (const auto &modelDesc : groupDesc.models) {
        auto modelName = modelDesc.modelName;
        auto capability = AiCapability::Speech;
        auto modelClass = ModelClass{ModelFamily::kSpeech, ModelSubType::kSpeechToText};
        auto optionsCopy = options;

        futures.emplace_back(std::async(std::launch::async, [=, this]() {
            auto descriptor = makeDescriptor(modelName, capability, modelClass);
            invoke(descriptor, optionsCopy, [&](auto engine) { return doInit(); });
        }));
    }

    for (auto &future : futures) {
        future.wait();
        future.get();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    LOGI("Finish initContinuousRecognitionModule, time elapsed: {} ms", duration);
    return sharedFuture.get();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::destroyContinuousRecognitionModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(ContinuousRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool destroyed = false;

    auto doDestroy = [this, &destroyed, &result]() {
        if (!destroyed) {
            result = realEngine() ? realEngine()->destroyContinuousRecognitionModule()
                                  : ai_engine::result::AiEngineBaseResult{};
            destroyed = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::startContinuousRecognition(
    const std::string &params) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    auto groupDesc = buildSpeechGroupDescriptor(ContinuousRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool started = false;
    auto doStart = [this, &started, &result, &params]() {
        if (!started) {
            result = realEngine() ? realEngine()->startContinuousRecognition(params)
                                  : ai_engine::result::AiEngineBaseResult{};
            started = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doStart(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::writeContinuousRecognitionAudioData(
    const std::vector<uint8_t> &audioData) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(ContinuousRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool processed = false;

    auto doWriteAudio = [this, &processed, &result, &audioData]() {
        if (!processed) {
            result = realEngine() ? realEngine()->writeContinuousRecognitionAudioData(audioData)
                                  : ai_engine::result::AiEngineBaseResult{};
            processed = true;
        }
        return result;
    };
    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doWriteAudio(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::stopContinuousRecognition() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(ContinuousRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool stopped = false;

    auto doStop = [this, &stopped, &result]() {
        if (!stopped) {
            if (realEngine()) {
                LOGI("Stopping continuous speech recognition");
                result = realEngine()->stopContinuousRecognition();
            } else {
                result = ai_engine::result::AiEngineBaseResult{};
            }
            stopped = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doStop(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::initRecognizeOnceModule() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnInitialization;
    options.trackUsage = false;
    if (realEngine()) {
        LOGI("setCurrentRecognizeOnceModel: paraformer-offline");
        setCurrentRecognizeOnceModel("paraformer-offline");
    }

    auto groupDesc = buildSpeechGroupDescriptor(OnceRecognition);
    if (groupDesc.models.empty()) {
        LOGE("initRecognizeOnceModule failed, no once recognition model found");
        return ai_engine::result::AiEngineBaseResult{
            false, {AI_COMMON_SPEECH_UNKNOWN_ERROR, "no once recognition model found"}};
    }
    std::promise<ai_engine::result::AiEngineBaseResult> promise;
    std::shared_future sharedFuture = promise.get_future();

    std::once_flag initFlag;

    auto doInit = [this, &promise, &initFlag]() {
        std::call_once(initFlag, [this, &promise]() {
            auto result =
                realEngine() ? realEngine()->initRecognizeOnceModule() : ai_engine::result::AiEngineBaseResult{};
            promise.set_value(result);
        });
    };

    std::vector<std::future<void>> futures;
    auto startTime = std::chrono::high_resolution_clock::now();
    for (const auto &modelDesc : groupDesc.models) {
        auto modelName = modelDesc.modelName;
        auto capability = AiCapability::Speech;
        auto modelClass = ModelClass{ModelFamily::kSpeech, ModelSubType::kSpeechToText};
        auto optionsCopy = options;

        futures.emplace_back(std::async(std::launch::async, [=, this]() {
            auto descriptor = makeDescriptor(modelName, capability, modelClass);
            invoke(descriptor, optionsCopy, [&](auto engine) { return doInit(); });
        }));
    }

    for (auto &future : futures) {
        future.wait();
        future.get();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    LOGI("Finish initRecognizeOnceModule, time elapsed: {} ms", duration);
    return sharedFuture.get();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::destroyRecognizeOnceModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(OnceRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool destroyed = false;

    auto doDestroy = [this, &destroyed, &result]() {
        if (!destroyed) {
            result =
                realEngine() ? realEngine()->destroyRecognizeOnceModule() : ai_engine::result::AiEngineBaseResult{};
            destroyed = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::recognizeOnce(const std::string &params,
                                                                                const std::vector<uint8_t> &audioData) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildSpeechGroupDescriptor(OnceRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool processed = false;

    auto doRecognize = [this, &processed, &result, &params, &audioData]() {
        if (!processed) {
            result =
                realEngine() ? realEngine()->recognizeOnce(params, audioData) : ai_engine::result::AiEngineBaseResult{};
            processed = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doRecognize(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::stopRecognizeOnce() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(OnceRecognition);
    ai_engine::result::AiEngineBaseResult result;
    bool stopped = false;

    auto doStop = [this, &stopped, &result]() {
        if (!stopped) {
            if (realEngine()) {
                LOGI("Stopping once speech recognition");
                result = realEngine()->stopRecognizeOnce();
            } else {
                result = ai_engine::result::AiEngineBaseResult{};
            }
            stopped = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kSpeechToText});
        invoke(descriptor, options, [&](auto engine) { return doStop(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::setCurrentRecognizeOnceModel(
    const std::string &model) {
    auto result =
        realEngine() ? realEngine()->setCurrentRecognizeOnceModel(model) : ai_engine::result::AiEngineBaseResult{};
    registerStopper(OnceRecognition);
    return result;
}

std::string LifecycleAwareSpeechEngine::currentRecognizeOnceModel() const {
    return this->realEngine() ? this->realEngine()->currentRecognizeOnceModel() : std::string();
}

// Synthesis methods
ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::initSynthesizeModule() {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnInitialization;
    options.trackUsage = false;
    if (realEngine()) {
        LOGI("setCurrentSynthesizeModel melo-tts");
        setCurrentSynthesizeModel("melo-tts");
    }

    auto groupDesc = buildSpeechGroupDescriptor(Synthesis);
    if (groupDesc.models.empty()) {
        LOGE("initSynthesizeModule failed, no synthesis model found");
        return ai_engine::result::AiEngineBaseResult{false,
                                                     {AI_COMMON_SPEECH_UNKNOWN_ERROR, "no synthesis model found"}};
    }
    std::promise<ai_engine::result::AiEngineBaseResult> promise;
    std::shared_future sharedFuture = promise.get_future();
    std::once_flag initFlag;

    auto doInit = [this, &promise, &initFlag]() {
        std::call_once(initFlag, [this, &promise]() {
            ai_engine::result::AiEngineBaseResult result =
                realEngine() ? realEngine()->initSynthesizeModule() : ai_engine::result::AiEngineBaseResult{};
            promise.set_value(result);
        });
    };

    std::vector<std::future<void>> futures;
    auto startTime = std::chrono::high_resolution_clock::now();
    for (const auto &modelDesc : groupDesc.models) {
        auto modelName = modelDesc.modelName;
        auto capability = AiCapability::Speech;
        auto modelClass = ModelClass{ModelFamily::kSpeech, ModelSubType::kTextToSpeech};
        auto optionsCopy = options;

        futures.emplace_back(std::async(std::launch::async, [=, this]() {
            auto descriptor = makeDescriptor(modelName, capability, modelClass);
            invoke(descriptor, optionsCopy, [&](auto engine) { return doInit(); });
        }));
    }

    for (auto &future : futures) {
        future.wait();
        future.get();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    LOGI("Finish initSynthesizeModule, time elapsed: {} ms", duration);

    return sharedFuture.get();
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::destroySynthesizeModule() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(Synthesis);
    ai_engine::result::AiEngineBaseResult result;
    bool destroyed = false;
    auto doDestroy = [this, &destroyed, &result]() {
        if (!destroyed) {
            result = realEngine() ? realEngine()->destroySynthesizeModule() : ai_engine::result::AiEngineBaseResult{};
            destroyed = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kTextToSpeech});
        invoke(descriptor, options, [&](auto engine) { return doDestroy(); });
    }

    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::synthesizeText(const std::string &params,
                                                                                 const std::string &text) {
    LifecycleBorrowOptions options;
    options.callStage = EngineCallStage::kOnBusinessCall;

    auto groupDesc = buildSpeechGroupDescriptor(Synthesis);
    if (groupDesc.models.empty()) {
        return ai_engine::result::AiEngineBaseResult{};
    }

    ModelGroupDescriptor scopedGroup = groupDesc;
    for (auto &modelDesc : scopedGroup.models) {
        modelDesc = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                   {ModelFamily::kSpeech, ModelSubType::kTextToSpeech});
    }

    return invokeWithGroupScope(scopedGroup, options, [this, &params, &text](auto engine) {
        (void)engine;
        return realEngine() ? realEngine()->synthesizeText(params, text) : ai_engine::result::AiEngineBaseResult{};
    });
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::stopSynthesize() {
    LifecycleBorrowOptions options;
    options.trackUsage = false;

    auto groupDesc = buildSpeechGroupDescriptor(Synthesis);
    ai_engine::result::AiEngineBaseResult result;
    bool stopped = false;

    auto doStop = [this, &stopped, &result]() {
        if (!stopped) {
            if (realEngine()) {
                LOGI("Stopping text to speech synthesis");
                result = realEngine()->stopSynthesize();
            } else {
                result = ai_engine::result::AiEngineBaseResult{};
            }
            stopped = true;
        }
        return result;
    };

    for (const auto &modelDesc : groupDesc.models) {
        auto descriptor = makeDescriptor(modelDesc.modelName, AiCapability::Speech,
                                         {ModelFamily::kSpeech, ModelSubType::kTextToSpeech});
        invoke(descriptor, options, [&](auto engine) { return doStop(); });
    }
    return result;
}

ai_engine::result::AiEngineBaseResult LifecycleAwareSpeechEngine::setCurrentSynthesizeModel(const std::string &model) {
    auto result =
        realEngine() ? realEngine()->setCurrentSynthesizeModel(model) : ai_engine::result::AiEngineBaseResult{};
    registerStopper(Synthesis);
    return result;
}

std::string LifecycleAwareSpeechEngine::currentSynthesizeModel() const {
    return this->realEngine() ? this->realEngine()->currentSynthesizeModel() : std::string();
}

}  // namespace lifecycle
