/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/modelresourcecontroller.h"

#include "utils/logger.h"

namespace lifecycle {

ModelResourceController::ModelResourceController(const ModelDescriptor &descriptor) : descriptor_(descriptor) {
    auto error = triton::client::InferenceServerHttpClient::Create(&client_, "localhost:8000");
    if (!error.IsOk()) {
        LOGE("ModelResourceController failed to create client: {}", error.Message());
        client_ = nullptr;
    }
}

void ModelResourceController::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (loaded_) {
        return;
    }
    if (!loadModel(descriptor_.modelName)) {
        loaded_ = false;
        return;
    }
    loaded_ = true;
    LOGI("ModelResourceController marked model [{}] as loaded.", nameForLog());
}

void ModelResourceController::unload() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return;
    }
    if (!unloadModel(descriptor_.modelName)) {
        return;
    }
    loaded_ = false;
    LOGI("ModelResourceController marked model [{}] as unloaded.", nameForLog());
}

bool ModelResourceController::isLoaded() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return loaded_;
}

std::string ModelResourceController::nameForLog() const {
    if (!descriptor_.modelName.empty()) {
        return descriptor_.fingerprint();
    }
    return "unknown";
}

bool ModelResourceController::loadModel(std::string modelName) {
    if (!client_) {
        LOGE("ModelResourceController failed to load model [{}], client is null.", nameForLog());
        return false;
    }
    if (modelName.empty()) {
        LOGE("ModelResourceController failed to load model [{}], model name is empty.", nameForLog());
        return true;
    }
    if (isModelLoaded(modelName)) {
        LOGI("ModelResourceController model [{}] is already loaded.", nameForLog());
        return true;
    }
    LOGI("ModelResourceController loading model [{}]...", nameForLog());
    auto error = client_->LoadModel(modelName);
    if (!error.IsOk()) {
        loaded_ = false;
        LOGE("ModelResourceController failed to load model [{}]: {}", nameForLog(), error.Message());
        return false;
    }
    LOGI("ModelResourceController loaded model [{}].", nameForLog());
    return true;
}

bool ModelResourceController::unloadModel(const std::string &modelName) const {
    if (!client_) {
        LOGE("ModelResourceController failed to unload model [{}], client is null.", nameForLog());
        return false;
    }
    if (modelName.empty()) {
        LOGE("ModelResourceController failed to unload model [{}], model name is empty.", nameForLog());
        return true;
    }
    if (!isModelLoaded(modelName)) {
        LOGI("ModelResourceController model [{}] is not loaded.", nameForLog());
        return true;
    }
    LOGI("ModelResourceController unloading model [{}]...", nameForLog());
    auto error = client_->UnloadModel(modelName);
    if (!error.IsOk()) {
        LOGE("ModelResourceController failed to unload model [{}]: {}", nameForLog(), error.Message());
        return false;
    }
    LOGI("ModelResourceController unloaded model [{}].", nameForLog());
    return true;
}

bool ModelResourceController::isModelLoaded(const std::string &modelName) const {
    bool ready = false;
    auto error = client_->IsModelReady(&ready, modelName);
    if (!error.IsOk()) {
        LOGE("ModelResourceController failed to check model [{}] ready: {}", nameForLog(), error.Message());
        return false;
    }
    return ready;
}

}  // namespace lifecycle
