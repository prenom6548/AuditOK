/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <triton/client/http_client.h>

#include <memory>
#include <mutex>
#include <string>

#include "lifecycle/modellifecycletypes.h"

namespace lifecycle {

class ModelResourceController {
public:
    explicit ModelResourceController(const ModelDescriptor &descriptor = {});

    void load();
    void unload();
    bool isLoaded() const;

private:
    std::string nameForLog() const;
    bool loadModel(std::string modelName);
    bool unloadModel(const std::string &modelName) const;
    bool isModelLoaded(const std::string &modelName) const;

private:
    ModelDescriptor descriptor_;
    mutable std::mutex mutex_;
    bool loaded_{false};
    std::unique_ptr<triton::client::InferenceServerHttpClient> client_;
};

}  // namespace lifecycle
