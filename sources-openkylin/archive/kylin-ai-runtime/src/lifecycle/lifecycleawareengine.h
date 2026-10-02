/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <kylin-ai/ai-engine/embeddingengine.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>
#include <kylin-ai/ai-engine/speechengine.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>
#include <kylin-ai/ai-engine/textrecognitionengine.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lifecycle/enginestopcontroller.h"
#include "lifecycle/lifecyclerequestscope.h"
#include "lifecycle/modellifecyclemanager.h"

namespace lifecycle {

// 模型组描述符，用于表示一个功能对应多个模型的情况
struct ModelGroupDescriptor {
    std::string groupName;                // 组名，比如"speech_recognition"
    std::string capabilityKey;            // 能力键
    ModelClass modelClass;                // 模型类
    std::vector<ModelDescriptor> models;  // 包含的实际模型列表

    // 构造函数
    ModelGroupDescriptor(const std::string &group, const std::string &cap, const ModelClass &cls)
        : groupName(group), capabilityKey(cap), modelClass(cls) {}
    // 添加子模型
    void addModel(const std::string &modelName, const std::string &engineName = "") {
        ModelDescriptor desc;
        desc.modelName = modelName;
        desc.capability = stringToCapability(capabilityKey);
        desc.modelClass = modelClass;
        desc.engineName = engineName;
        models.push_back(desc);
    }

private:
    AiCapability stringToCapability(const std::string &str) {
        if (str == "Nlp") return AiCapability::Nlp;
        if (str == "Vision") return AiCapability::Vision;
        if (str == "Speech") return AiCapability::Speech;
        return AiCapability::Nlp;  // 默认值
    }
};

template <typename TInterface>
class LifecycleAwareEngineProxyBase : public TInterface {
public:
    LifecycleAwareEngineProxyBase() : instanceId_(generateInstanceId()) {}

    virtual ~LifecycleAwareEngineProxyBase() {
        alive_->store(false, std::memory_order_release);
        if (stopRegistered_) {
            EngineStopController::getInstance().unregisterStopper(stopDescriptor_);
        }
    }

    bool attach(std::unique_ptr<TInterface> engine) {
        if (!engine) {
            return false;
        }
        realEngine_ = std::move(engine);
        handle_ = ModelLifecycleManager::getInstance().acquire();
        afterAttach();
        return true;
    }

protected:
    virtual std::string activeModelName() const { return currentModel(); }
    virtual ModelDescriptor buildDescriptor(const std::string &modelName) const = 0;
    virtual void afterAttach() {}

    // 构建模型组描述符
    virtual ModelGroupDescriptor buildGroupDescriptor(const std::string &groupName) const {
        ModelGroupDescriptor groupDesc(groupName, getCapabilityKey(), {});
        return groupDesc;
    }
    virtual std::string getCapabilityKey() const = 0;

    template <typename Fn>
    auto invoke(const ModelDescriptor &descriptor, const LifecycleBorrowOptions &borrowOptions, Fn &&fn)
        -> decltype(fn(std::declval<TInterface *>())) {
        LifecycleRequestScope scope(handle_, descriptor, borrowOptions);
        return fn(this->realEngine_.get());
    }

    // 对模型组执行操作
    template <typename Fn>
    void invokeGroup(const ModelGroupDescriptor &groupDesc, const LifecycleBorrowOptions &borrowOptions, Fn &&fn) {
        for (const auto &modelDesc : groupDesc.models) {
            LifecycleRequestScope scope(handle_, modelDesc, borrowOptions);
            fn(this->realEngine_.get(), modelDesc);
        }
    }

    // Execute one business call under a unified lifecycle window for the whole model group.
    template <typename Fn>
    auto invokeWithGroupScope(const ModelGroupDescriptor &groupDesc, const LifecycleBorrowOptions &borrowOptions,
                              Fn &&fn) -> decltype(fn(std::declval<TInterface *>())) {
        std::vector<std::unique_ptr<LifecycleRequestScope>> scopes;
        scopes.reserve(groupDesc.models.size());
        for (const auto &modelDesc : groupDesc.models) {
            scopes.emplace_back(std::make_unique<LifecycleRequestScope>(handle_, modelDesc, borrowOptions));
        }
        return fn(this->realEngine_.get());
    }

    // 异步加载模型
    bool startAsyncLoad(const ModelDescriptor &descriptor, EngineCallStage stage) {
        return handle_.startAsyncLoad(descriptor, stage);
    }

    void updateStopper(const ModelDescriptor &descriptor, std::function<void()> stopFn) {
        ModelDescriptor stopperDescriptor = makeStopperDescriptor(descriptor);

        // 如果是同一个 stopper descriptor 且已经注册，直接更新 stopFunction 而不是重新注册
        if (stopRegistered_ && stopDescriptor_.fingerprint() == stopperDescriptor.fingerprint()) {
            // 只需要更新 stopFunction，不需要改变 refCount
            EngineStopController::getInstance().registerStopper(stopperDescriptor, std::move(stopFn), false);
            return;
        }

        // 不同的 descriptor，需要先取消注册旧的，再注册新的
        if (stopRegistered_) {
            EngineStopController::getInstance().unregisterStopper(stopDescriptor_);
            stopRegistered_ = false;
        }
        if (!stopFn) {
            return;
        }
        stopDescriptor_ = stopperDescriptor;
        EngineStopController::getInstance().registerStopper(stopDescriptor_, std::move(stopFn));
        stopRegistered_ = true;
    }

private:
    static std::string generateInstanceId() {
        static std::atomic<uint64_t> counter{0};
        return std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
    }

    // 为 stopper 注册创建带有唯一标识的 descriptor
    ModelDescriptor makeStopperDescriptor(const ModelDescriptor &baseDescriptor) const {
        ModelDescriptor stopperDescriptor = baseDescriptor;
        // 在 modelName 后附加实例ID，确保每个 Engine 实例有唯一的 key
        stopperDescriptor.modelName = baseDescriptor.modelName + "_instance_" + instanceId_;
        return stopperDescriptor;
    }

public:
    ModelDescriptor makeDescriptor(const std::string &modelName, AiCapability capability,
                                   const ModelClass &modelClass = {}) const {
        ModelDescriptor descriptor;
        descriptor.modelName = modelName;
        descriptor.capability = capability;
        descriptor.modelClass = modelClass;
        descriptor.engineName = this->realEngine() ? this->realEngine()->engineName() : std::string();
        return descriptor;
    }

    // AbstractAiEngine overrides
    std::string engineName() const override {
        return this->realEngine_ ? this->realEngine_->engineName() : std::string();
    }
    bool isCloud() const override { return this->realEngine_ && this->realEngine_->isCloud(); }
    bool isBuiltInEngine() const override { return this->realEngine_ && this->realEngine_->isBuiltInEngine(); }
    bool isCustomModelEngine() const override { return this->realEngine_ && this->realEngine_->isCustomModelEngine(); }
    std::string modelInfo() const override {
        return this->realEngine_ ? this->realEngine_->modelInfo() : std::string();
    }
    bool setCurrentModel(const std::string &model) override {
        return this->realEngine_ && this->realEngine_->setCurrentModel(model);
    }
    std::string currentModel() const override {
        return this->realEngine_ ? this->realEngine_->currentModel() : std::string();
    }
    std::string currentModelName() const override {
        return this->realEngine_ ? this->realEngine_->currentModelName() : std::string();
    }
    void setConfig(const std::string &config) override {
        if (this->realEngine_) {
            this->realEngine_->setConfig(config);
        }
    }

    TInterface *realEngine() const { return this->realEngine_.get(); }

    // 存活标志，供 stopper lambda 捕获以安全检测对象是否已析构
    std::shared_ptr<std::atomic<bool>> aliveFlag() const { return alive_; }

private:
    std::string instanceId_;
    std::unique_ptr<TInterface> realEngine_;
    ModelUsageHandle handle_;
    ModelDescriptor stopDescriptor_;
    bool stopRegistered_{false};
    std::shared_ptr<std::atomic<bool>> alive_{std::make_shared<std::atomic<bool>>(true)};
};

class LifecycleAwareTextGenerationEngine
    : public LifecycleAwareEngineProxyBase<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> {
public:
    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;
    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input) override;
    ai_engine::result::AiEngineBaseResult chat(const std::string &message) override;
    void stopGenerateContent() override;
    void stopChat() override;
    void setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) override;
    bool setCurrentModel(const std::string &model) override;

protected:
    ModelDescriptor buildDescriptor(const std::string &modelName) const override;
    void afterAttach() override;
    void registerStopper();
    std::string getCapabilityKey() const override { return "Nlp"; }
};

class LifecycleAwareEmbeddingEngine
    : public LifecycleAwareEngineProxyBase<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> {
public:
    enum EmbeddingRequestType {
        TextEmbedding,
        ImageEmbedding,
        Unknown,
    };
    ai_engine::result::AiEngineBaseResult setCurrentTextEmbeddingModel(const std::string &model) override;
    std::string currentTextEmbeddingModel() override;
    void setEmbeddingResultCallback(ai_engine::core_ai::embedding::EmbeddingResultCallback callback) override;

    ai_engine::result::AiEngineBaseResult setCurrentImageEmbeddingModel(const std::string &model) override;
    std::string currentImageEmbeddingModel() override;
    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult embeddingText(const std::string &text) override;
    ai_engine::result::AiEngineBaseResult embeddingImageFromImageFile(const std::string &filePath) override;
    ai_engine::result::AiEngineBaseResult embeddingImageFromBase64ImageData(const std::string &fileData) override;
    ai_engine::result::AiEngineBaseResult embeddingTextWithImageModel(const std::string &text) override;
    void stopImageEmbedding() override;
    void stopTextEmbedding() override;

protected:
    ModelDescriptor buildDescriptor(const std::string &modelName) const override;
    ModelDescriptor buildTextEmbeddingDescriptor(const std::string &modelName) const;
    ModelDescriptor buildImageEmbeddingDescriptor(const std::string &modelName) const;
    ModelGroupDescriptor buildEmbeddingGroupDescriptor(EmbeddingRequestType type) const;
    void afterAttach() override;
    void registerStopper();
    std::string getCapabilityKey() const override { return "Vision"; }

private:
    void parseModelInfo() const;
    std::string mapRequestTypeToGroupName(EmbeddingRequestType type) const;

    mutable std::map<std::string, std::vector<std::string>> parsedGroupMap_;
    mutable std::mutex groupMapMutex_;
};

class LifecycleAwareImageProcessEngine
    : public LifecycleAwareEngineProxyBase<ai_engine::core_ai::vision::AbstractImageProcessEngine> {
public:
    enum ImageProcessRequestType {
        PortraitSegmentation,
        GeneralSegmentation,
        Segmentation,
        Unknown,
    };
    std::string currentPortraitSegmentationModel() const override;
    std::string currentGeneralSegmentationModel() const override;
    void setCurrentPortraitSegmentationModel(const std::string &model) override;
    void setCurrentGeneralSegmentationModel(const std::string &model) override;
    ai_engine::result::AiEngineBaseResult initMattingsModule() override;
    ai_engine::result::AiEngineBaseResult destroyMattingsModule() override;
    void setMattingResultCallback(ai_engine::core_ai::vision::ImageResultCallback callback) override;
    ai_engine::result::AiEngineBaseResult mattingFromBase64ImageData(
        const std::string &imageBase64Data, ai_engine::core_ai::vision::MattingMode mode) override;
    ai_engine::result::AiEngineBaseResult mattingFromImageFile(const std::string &imageFilePath,
                                                               ai_engine::core_ai::vision::MattingMode mode) override;
    void stopMattingProcess() override;

    ai_engine::result::AiEngineBaseResult initImageBinaryModule() override;
    ai_engine::result::AiEngineBaseResult destroyImageBinaryModule() override;
    void setImageBinaryResultCallback(ai_engine::core_ai::vision::ImageResultCallback callback) override;
    ai_engine::result::AiEngineBaseResult binarizeImageFromBase64ImageData(const std::string &imageBase64Data) override;
    ai_engine::result::AiEngineBaseResult binarizeImageFromImageFile(const std::string &imageFilePath) override;
    void stopImageBinarization() override;

    ai_engine::result::AiEngineBaseResult initSegmentationModule() override;
    ai_engine::result::AiEngineBaseResult destroySegmentationModule() override;
    void setSegmentationResultCallback(const ai_engine::core_ai::vision::SegmentationResultCallback &callback) override;
    ai_engine::result::AiEngineBaseResult encodeSegmentationFromBase64ImageData(
        const std::string &base64ImageData) override;
    ai_engine::result::AiEngineBaseResult encodeSegmentationFromImageFile(const std::string &imageFilePath) override;
    ai_engine::result::AiEngineBaseResult decodeSegmentation(
        const std::vector<ai_engine::core_ai::vision::SegmentationPoint> &points, uint32_t minArea) override;
    void clearSegmentationCache() override;
    ai_engine::result::AiEngineBaseResult stopSegmentation() override;
    void setCurrentSegmentationModel(const std::string &model) override;
    std::string currentSegmentationModel() const override;
    std::string activeSegmentationModelName() const;

protected:
    ModelDescriptor buildDescriptor(const std::string &modelName) const override;
    ModelDescriptor buildDescriptorWithVariant(const std::string &modelName, ModelSubType variant) const;
    ModelGroupDescriptor buildImageProcessGroupDescriptor(ImageProcessRequestType type) const;
    void afterAttach() override;
    void registerStopper();
    std::string getCapabilityKey() const override { return "Vision"; }

private:
    void parseModelInfo() const;
    std::string mapRequestTypeToGroupName(ImageProcessRequestType type) const;

    mutable std::map<std::string, std::vector<std::string>> parsedGroupMap_;
    mutable std::mutex groupMapMutex_;
};

class LifecycleAwareTextRecognitionEngine
    : public LifecycleAwareEngineProxyBase<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> {
public:
    std::string currentTextRecognitionModel() const override;
    void setCurrentTextRecognitionModel(const std::string &model) override;
    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;
    void setTextRecognitionResultCallback(ai_engine::core_ai::vision::TextRecognitionResultCallback callback) override;
    ai_engine::result::AiEngineBaseResult recognizeTextFromBase64ImageData(const std::string &imageBase64Data) override;
    ai_engine::result::AiEngineBaseResult recognizeTextFromFile(const std::string &imageFilePath) override;
    void stopTextRecognition() override;

protected:
    ModelDescriptor buildDescriptor(const std::string &modelName) const override;
    ModelDescriptor buildDescriptorWithVariant(const std::string &modelName, ModelSubType variant) const;
    void afterAttach() override;
    void registerStopper();
    std::string getCapabilityKey() const override { return "Vision"; }
};

class LifecycleAwareSpeechEngine
    : public LifecycleAwareEngineProxyBase<ai_engine::core_ai::speech::AbstractSpeechEngine> {
public:
    ~LifecycleAwareSpeechEngine() override;
    enum SpeechRequestType {
        OnceRecognition,
        ContinuousRecognition,
        Synthesis,
        Unknown,
    };
    void setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback) override;
    void setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback) override;

    std::string currentContinuousRecognitionModel() const override;
    ai_engine::result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string &model) override;
    ai_engine::result::AiEngineBaseResult initContinuousRecognitionModule() override;
    ai_engine::result::AiEngineBaseResult destroyContinuousRecognitionModule() override;
    ai_engine::result::AiEngineBaseResult startContinuousRecognition(const std::string &params) override;
    ai_engine::result::AiEngineBaseResult writeContinuousRecognitionAudioData(
        const std::vector<uint8_t> &audioData) override;
    ai_engine::result::AiEngineBaseResult stopContinuousRecognition() override;

    ai_engine::result::AiEngineBaseResult initRecognizeOnceModule() override;
    ai_engine::result::AiEngineBaseResult destroyRecognizeOnceModule() override;
    ai_engine::result::AiEngineBaseResult recognizeOnce(const std::string &params,
                                                        const std::vector<uint8_t> &audioData) override;
    ai_engine::result::AiEngineBaseResult stopRecognizeOnce() override;
    ai_engine::result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string &model) override;
    std::string currentRecognizeOnceModel() const override;

    ai_engine::result::AiEngineBaseResult initSynthesizeModule() override;
    ai_engine::result::AiEngineBaseResult destroySynthesizeModule() override;
    ai_engine::result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text) override;
    ai_engine::result::AiEngineBaseResult stopSynthesize() override;
    ai_engine::result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string &model) override;
    std::string currentSynthesizeModel() const override;

protected:
    ModelDescriptor buildDescriptor(const std::string &modelName) const override;
    ModelDescriptor buildDescriptorWithVariant(const std::string &modelName, ModelSubType variant) const;
    ModelGroupDescriptor buildSpeechGroupDescriptor(SpeechRequestType type) const;
    void afterAttach() override;
    void registerStopper(std::optional<SpeechRequestType> type = std::nullopt);
    std::string getCapabilityKey() const override { return "Speech"; }

private:
    void parseModelInfo() const;
    std::string mapRequestTypeToGroupName(SpeechRequestType type) const;

    mutable std::map<std::string, std::vector<std::string>> parsedGroupMap_;
    mutable std::mutex groupMapMutex_;
    std::map<std::string, ModelDescriptor> stopperDescriptors_;
    std::map<std::string, bool> stopperRegistered_;
};

}  // namespace lifecycle
