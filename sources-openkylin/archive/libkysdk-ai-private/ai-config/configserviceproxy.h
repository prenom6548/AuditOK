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

#include <mutex>
#include <string>

#include "configserviceglue.h"
#include "modelconfig.h"

namespace kyai {
namespace config {
namespace model {
class ConfigServer {
public:
    static ConfigServer &getInstance();

    ~ConfigServer();
    bool available() const { return connection_ != nullptr; }

    GDBusConnection *getConnection() const { return connection_; }

private:
    ConfigServer();
    void init(const std::string &unixPath);
    void destroy() const;

private:
    GDBusConnection *connection_{nullptr};
    mutable std::mutex mutex_;
};

class ConfigServiceProxy {
public:
    ConfigServiceProxy();

    std::vector<Model> getModels(AiCapability capability) const;
    std::vector<Model> getModels(DeployType deployType) const;
    std::vector<ExtendedModel> getExtendedModels(AiCapability capability) const;
    std::vector<ExtendedModel> getExtendedModels(DeployType deployType) const;
    std::string getCurrentModelName(AiCapability capability,
                                    DeployType deployType) const;
    std::vector<MultiAuthentication> getModelAuthentications(
        const std::string &modelName) const;
    int getModelSchedulingPriority(DeployType deployType) const;
    bool getModelDeployEnabled(DeployType deployType) const;

    bool setCurrentModelName(AiCapability capability, DeployType deployType,
                             const std::string &modelName);

    bool setModelAuthentications(
        const std::string &modelName,
        const std::vector<MultiAuthentication> authentications);

    bool setCustomModelApiUrl(const std::string &modelName,
                              const std::string &apiUrl);
    bool setCustomModelVersion(const std::string &modelName,
                               const std::string &modelVersion);

    bool setModelSchedulingPriority(DeployType deployType, int priority);

    bool setModelDeployEnabled(DeployType deployType, bool enabled);

    bool clearModelAuthentications(const std::string &modelName);

    ModelStatus getModelStatus(const std::string &modelName) const;

    bool addCustomModel(const std::string &modelName, const std::string &model,
                        const std::string &apiKey, const std::string &apiUrl);

    bool deleteCustomModel(const std::string &modelName);

    std::vector<Model> getFreeTrialModels(AiCapability capability) const;

private:
    bool connectToServer();

private:
    AiRuntimeConfigService *proxy_{nullptr};
};
}  // namespace model
}  // namespace config
}  // namespace kyai
