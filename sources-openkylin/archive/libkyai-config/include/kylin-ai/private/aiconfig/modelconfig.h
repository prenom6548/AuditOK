/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include <memory>
#include <string>
#include <vector>

namespace kyai {
namespace config {
namespace model {

enum class AiCapability {
    Nlp,
    Vision,
    Speech,
};

enum class DeployType {
    Ondevice,
    PublicCloud,
    PrivateCloud,
    Custom = PrivateCloud,
    Shared,
};

enum class ModelStatus {
    Idle,
    Running,
    Unknown,
};

struct Authentication {
    std::string friendlyKeyName;
    std::string key;
    std::string value;
};

struct Model {
    std::string modelName;
    std::string vendor;
    AiCapability capability;
    DeployType deployType;
};

struct ExtendedModel : public Model {
    std::string modelVersion;
    std::string apiUrl;
    std::string debName;
    bool is3rdModel{false};
    // 端侧后摩大语言模型返回的模型信息才有这个字段，用于判断后摩卡是否可用
    bool available{true};
};

struct MultiAuthentication {
    std::string itemName;
    std::vector<Authentication> authentications;
};

struct ExtendedMultiAuthentication : public MultiAuthentication {
    bool enabled{true};
};

class ModelConfigPrivate;
class ModelConfig {
public:
    ModelConfig();
    ~ModelConfig();

    std::vector<Model> getModels(AiCapability capability) const;
    std::vector<Model> getModels(DeployType deployType) const;

    std::vector<ExtendedModel> getExtendedModels(AiCapability capability) const;
    std::vector<ExtendedModel> getExtendedModels(DeployType deployType) const;

    std::string getCurrentModelName(AiCapability capability,
                                    DeployType deployType) const;

    std::vector<MultiAuthentication> getModelAuthentications(
        const std::string &modelName) const;

    std::vector<ExtendedMultiAuthentication> getModelExtendedAuthentications(
        const std::string &modelName) const;

    int getModelSchedulingPriority(DeployType deployType) const;
    bool getModelDeployEnabled(DeployType deployType) const;

    bool setCurrentModelName(AiCapability capability, DeployType deployType,
                             const std::string &modelName);

    bool setModelAuthentications(
        const std::string &modelName,
        const std::vector<MultiAuthentication> authentications);

    bool setModelAuthentications(const std::string &modelName,
                                 const std::vector<ExtendedMultiAuthentication>
                                     &extendedMultiAuthentications) const;

    bool setModelSchedulingPriority(DeployType deployType, int priority);

    bool setModelDeployEnabled(DeployType deployType, bool enabled);

    bool clearModelAuthentications(const std::string &modelName) const;

    ModelStatus getModelStatus(const std::string &modelName) const;

    bool setCustomModelApiUrl(const std::string &modelName,
                              const std::string &apiUrl);
    bool setCustomModelVersion(const std::string &modelName,
                               const std::string &modelVersion);

    bool addCustomModel(const std::string &modelName,
                        const std::string &modelVersion,
                        const std::string &apiKey, const std::string &apiUrl);

    bool deleteCustomModel(const std::string &modelName);

    std::vector<Model> getFreeTrialModels(AiCapability capability) const;

    bool setSharedModelName(const std::string &modelName);
    bool stopModelSharing(const std::string &modelName);
    std::string getSharedModelName() const;

    bool getLoadOnBoot(const std::string &modelName) const;
    bool setLoadOnBoot(const std::string &modelName, bool enabled);

private:
    std::unique_ptr<ModelConfigPrivate> modelConfigPrivate_;
};

}  // namespace model
}  // namespace config
}  // namespace kyai
