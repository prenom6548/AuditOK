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

#include "modelconfig.h"

#include "configserviceproxy.h"

namespace kyai {
namespace config {
namespace model {
struct ModelConfigPrivate {
    std::unique_ptr<ConfigServiceProxy> configServiceProxy{
        new ConfigServiceProxy()};
};

ModelConfig::ModelConfig()
    : modelConfigPrivate_(std::make_unique<ModelConfigPrivate>()) {}

ModelConfig::~ModelConfig() = default;

std::vector<Model> ModelConfig::getModels(AiCapability capability) const {
    return modelConfigPrivate_->configServiceProxy->getModels(capability);
}

std::vector<Model> ModelConfig::getModels(DeployType deployType) const {
    return modelConfigPrivate_->configServiceProxy->getModels(deployType);
}

std::vector<ExtendedModel> ModelConfig::getExtendedModels(
    AiCapability capability) const {
    return modelConfigPrivate_->configServiceProxy->getExtendedModels(
        capability);
}

std::vector<ExtendedModel> ModelConfig::getExtendedModels(
    DeployType deployType) const {
    return modelConfigPrivate_->configServiceProxy->getExtendedModels(
        deployType);
}

std::string ModelConfig::getCurrentModelName(AiCapability capability,
                                             DeployType deployType) const {
    return modelConfigPrivate_->configServiceProxy->getCurrentModelName(
        capability, deployType);
}

std::vector<MultiAuthentication> ModelConfig::getModelAuthentications(
    const std::string &modelName) const {
    return modelConfigPrivate_->configServiceProxy->getModelAuthentications(
        modelName);
}

int ModelConfig::getModelSchedulingPriority(DeployType deployType) const {
    return modelConfigPrivate_->configServiceProxy->getModelSchedulingPriority(
        deployType);
}

bool ModelConfig::getModelDeployEnabled(DeployType deployType) const {
    return modelConfigPrivate_->configServiceProxy->getModelDeployEnabled(
        deployType);
}

bool ModelConfig::setCurrentModelName(AiCapability capability,
                                      DeployType deployType,
                                      const std::string &modelName) {
    return modelConfigPrivate_->configServiceProxy->setCurrentModelName(
        capability, deployType, modelName);
}

bool ModelConfig::setModelAuthentications(
    const std::string &modelName,
    const std::vector<MultiAuthentication> authentications) {
    return modelConfigPrivate_->configServiceProxy->setModelAuthentications(
        modelName, authentications);
}

bool ModelConfig::setCustomModelApiUrl(const std::string &modelName,
                                       const std::string &apiUrl) {
    return modelConfigPrivate_->configServiceProxy->setCustomModelApiUrl(
        modelName, apiUrl);
}

bool ModelConfig::setCustomModelVersion(const std::string &modelName,
                                        const std::string &modelVersion) {
    return modelConfigPrivate_->configServiceProxy->setCustomModelVersion(
        modelName, modelVersion);
}

bool ModelConfig::setModelSchedulingPriority(DeployType deployType,
                                             int priority) {
    return modelConfigPrivate_->configServiceProxy->setModelSchedulingPriority(
        deployType, priority);
}

bool ModelConfig::setModelDeployEnabled(DeployType deployType, bool enabled) {
    return modelConfigPrivate_->configServiceProxy->setModelDeployEnabled(
        deployType, enabled);
}

bool ModelConfig::clearModelAuthentications(
    const std::string &modelName) const {
    return modelConfigPrivate_->configServiceProxy->clearModelAuthentications(
        modelName);
}

ModelStatus ModelConfig::getModelStatus(const std::string &modelName) const {
    return modelConfigPrivate_->configServiceProxy->getModelStatus(modelName);
}

bool ModelConfig::addCustomModel(const std::string &modelName,
                                 const std::string &model,
                                 const std::string &apiKey,
                                 const std::string &apiUrl) {
    return modelConfigPrivate_->configServiceProxy->addCustomModel(
        modelName, model, apiKey, apiUrl);
}

bool ModelConfig::deleteCustomModel(const std::string &modelName) {
    return modelConfigPrivate_->configServiceProxy->deleteCustomModel(
        modelName);
}

std::vector<Model> ModelConfig::getFreeTrialModels(
    AiCapability capability) const {
    return modelConfigPrivate_->configServiceProxy->getFreeTrialModels(
        capability);
}

}  // namespace model
}  // namespace config
}  // namespace kyai
