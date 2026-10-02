/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <cstdint>
#include <string>

#include "services/modelconfig/modelconfigenums.h"

namespace lifecycle {

enum class ModelFamily { kUnknown = 0, kLLM, kEmbedding, kSpeech, kImage };

enum class ModelSubType {
    kUnknown = 0,
    kNone,
    kText,
    kImage,
    kSpeechToText,
    kTextToSpeech,
    kGeneralSeg,
    kPortraitSubjectSeg,
    kGeneralSubjectSeg,
    kBinarization,
    kTextRecognition,
};

struct ModelClass {
    ModelFamily family{ModelFamily::kUnknown};
    ModelSubType variant{ModelSubType::kUnknown};

    bool operator==(const ModelClass &other) const noexcept {
        return family == other.family && variant == other.variant;
    }
    bool operator<(const ModelClass &other) const noexcept {
        if (family != other.family) {
            return static_cast<int>(family) < static_cast<int>(other.family);
        }
        return static_cast<int>(variant) < static_cast<int>(other.variant);
    }
    bool operator!=(const ModelClass &other) const noexcept { return !(*this == other); }
    bool hasVariant() const noexcept { return variant != ModelSubType::kNone; }
    bool isUnknown() const noexcept { return family == ModelFamily::kUnknown; }
    ModelClass familyOnly() const noexcept { return {family, ModelSubType::kNone}; }
};

enum class LifecycleState { kUnloaded = 0, kLoading, kLoaded, kActive, kIdle, kUnloading };

enum class EngineCallStage { kOnInitialization = 0, kOnBusinessCall };

enum class UnloadPolicy {
    kNeverUnload = 0,            // 永不自动卸载
    kUnloadWhenIdle,             // 空闲时卸载（activeTaskCount==0 即可）
    kUnloadWhenNoHandleAndIdle,  // 无handle持有且空闲时卸载（handleRefCount==0 && activeTaskCount==0）
    kUnloadWhenIdleAndDisable  // 空闲且禁用时卸载（handleRefCount==0 && activeTaskCount==0 && disable==true）
};

struct ModelDescriptor {
    std::string modelName;
    AiCapability capability{AiCapability::Nlp};
    ModelClass modelClass;
    std::string engineName;

    std::string fingerprint() const;
    std::string capabilityKey() const;
    std::string nameKey() const;
};

std::string toString(ModelFamily family);
std::string toString(ModelSubType variant);
std::string toString(AiCapability capability);
std::string toString(DeployType deployType);

inline std::string toString(const ModelClass &cls) {
    if (cls.family == ModelFamily::kUnknown) {
        return "Unknown";
    }
    std::string base = toString(cls.family);
    if (cls.variant == ModelSubType::kUnknown || cls.variant == ModelSubType::kNone) {
        return base;
    }
    return base + "." + toString(cls.variant);
}

inline std::string ModelDescriptor::fingerprint() const {
    return toString(capability) + ":" + toString(modelClass) + ":" + modelName;
}

inline std::string ModelDescriptor::capabilityKey() const { return toString(capability); }

inline std::string ModelDescriptor::nameKey() const { return toString(modelClass); }

inline std::string toString(ModelFamily family) {
    switch (family) {
        case ModelFamily::kLLM:
            return "LLM";
        case ModelFamily::kEmbedding:
            return "Embedding";
        case ModelFamily::kSpeech:
            return "Speech";
        case ModelFamily::kImage:
            return "Image";
        case ModelFamily::kUnknown:
        default:
            return "Unknown";
    }
}

inline std::string toString(ModelSubType variant) {
    switch (variant) {
        case ModelSubType::kNone:
            return "None";
        case ModelSubType::kText:
            return "Text";
        case ModelSubType::kImage:
            return "Image";
        case ModelSubType::kSpeechToText:
            return "SpeechToText";
        case ModelSubType::kTextToSpeech:
            return "TextToSpeech";
        case ModelSubType::kGeneralSeg:
            return "GeneralSeg";
        case ModelSubType::kGeneralSubjectSeg:
            return "GeneralSubjectSeg";
        case ModelSubType::kPortraitSubjectSeg:
            return "PortraitSubjectSeg";
        case ModelSubType::kBinarization:
            return "Binarization";
        case ModelSubType::kTextRecognition:
            return "TextRecognition";
        case ModelSubType::kUnknown:
        default:
            return "Unknown";
    }
}

inline std::string toString(AiCapability capability) {
    switch (capability) {
        case AiCapability::Nlp:
            return "Nlp";
        case AiCapability::Vision:
            return "Vision";
        case AiCapability::Speech:
            return "Speech";
        default:
            return "Unknown";
    }
}

inline std::string toString(DeployType deployType) {
    switch (deployType) {
        case DeployType::Local:
            return "Local";
        case DeployType::PublicCloud:
            return "PublicCloud";
        case DeployType::Custom:
            return "Custom";
        default:
            return "Unknown";
    }
}

}  // namespace lifecycle
