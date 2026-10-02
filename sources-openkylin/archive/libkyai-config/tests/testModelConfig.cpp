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

#include <gio/gio.h>
#include <gio/giotypes.h>

#include <algorithm>
#include <cassert>
#include <cstdio>

#include "modelconfig.h"

void testGetModels() {
    kyai::config::model::ModelConfig modelConfig;
    auto models =
        modelConfig.getModels(kyai::config::model::AiCapability::Vision);
    for (auto &model : models) {
        std::fprintf(stderr, "Model name: %s\n", model.modelName.c_str());
    }
    assert(models.size() >= 2);
}

void tesetSetModleAuthens() {
    kyai::config::model::ModelConfig modelConfig;
    std::vector<kyai::config::model::MultiAuthentication> multiAuthens;
    multiAuthens.push_back(
        {"Item 1",
         {{"APPID", "appId", "123432"},
          {"APIKEY", "apiKey", "2548114fafgfhadfsfdsfasd"},
          {"APISecret", "secretKey", "dsadsdFKIJSKWRJSNFSHFKakfuASfkfja"}}});
    multiAuthens.push_back({"Item 2", {{"APPID", "appId", "123432"}}});
    modelConfig.setModelAuthentications("讯飞-Spark Max", multiAuthens);

    auto authens = modelConfig.getModelAuthentications("讯飞-Spark Max");
    assert(authens.size() == 2);

    multiAuthens.clear();
    multiAuthens.push_back({"Item 1", {{"APPID", "appId", "1234dasdasd222"}}});
    modelConfig.setModelAuthentications("百度-文心AI作画高级版", multiAuthens);
    authens = modelConfig.getModelAuthentications("百度-文心AI作画高级版");
    assert(authens.size() == 1);
}

void testSetModelSchedulingPriority(kyai::config::model::DeployType deployType,
                                    int priority) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setModelSchedulingPriority(deployType, priority);
    int ret = modelConfig.getModelSchedulingPriority(deployType);
    assert(ret == priority);
}

void testSetCurrentModel() {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::PublicCloud, "讯飞-Spark Max");

    auto currentModel = modelConfig.getCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::PublicCloud);

    assert(currentModel == "讯飞-Spark Max");

    modelConfig.setCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::PublicCloud, "百度-ERNIE-Bot-4");

    currentModel = modelConfig.getCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::PublicCloud);
    assert(currentModel == "百度-ERNIE-Bot-4");
}

void testSetModelDeployEnabled() {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setModelDeployEnabled(kyai::config::model::DeployType::Ondevice,
                                      true);
    bool enabled = modelConfig.getModelDeployEnabled(
        kyai::config::model::DeployType::Ondevice);
    assert(enabled == true);

    modelConfig.setModelDeployEnabled(
        kyai::config::model::DeployType::PublicCloud, false);
    enabled = modelConfig.getModelDeployEnabled(
        kyai::config::model::DeployType::PublicCloud);
    assert(enabled == false);
}
void testGetModelStatus() {
    kyai::config::model::ModelConfig modelConfig;
    auto status = modelConfig.getModelStatus("百度-ERNIE-Bot-4");
    std::fprintf(stderr, "Model status: %d\n", (int)status);
}

void testDeleteCustomModel(const std::string &modelName) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.deleteCustomModel(modelName);
    auto models =
        modelConfig.getModels(kyai::config::model::DeployType::Custom);
    auto it =
        std::find_if(models.begin(), models.end(),
                     [modelName](const kyai::config::model::Model &model) {
                         return model.modelName == modelName;
                     });
    assert(it == models.end());
}
void testAddCustomModel(const std::string &modelName,
                        const std::string &modelVersion,
                        const std::string &apiKey, const std::string &apiUrl) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.addCustomModel(modelName, modelVersion, apiKey, apiUrl);

    auto models =
        modelConfig.getExtendedModels(kyai::config::model::DeployType::Custom);
    auto it =
        std::find_if(models.begin(), models.end(),
                     [modelName](const kyai::config::model::Model &model) {
                         return model.modelName == modelName;
                     });
    assert(it != models.end());
    std::fprintf(stderr, "apiUrl name: %s:%s\n", it->apiUrl.c_str(),
                 apiUrl.c_str());
    assert(it->apiUrl == apiUrl);
    assert(it->modelVersion == modelVersion);

    auto authens = modelConfig.getModelAuthentications(modelName);
    std::fprintf(stderr, "authens size: %lu\n", authens.size());
    assert(authens.size() == 1);
    auto authenIt = std::find_if(
        authens.begin(), authens.end(),
        [&apiKey](const kyai::config::model::MultiAuthentication &authen) {
            auto it = std::find_if(
                authen.authentications.begin(), authen.authentications.end(),
                [&apiKey](const kyai::config::model::Authentication &auth) {
                    std::fprintf(stderr, "auth value: %s\n",
                                 auth.value.c_str());
                    return auth.value == apiKey;
                });
            return it != authen.authentications.end();
        });
    assert(authenIt != authens.end());

    bool ret = modelConfig.setCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::Custom, modelName);
    assert(ret);

    const std::string currentModelName = modelConfig.getCurrentModelName(
        kyai::config::model::AiCapability::Nlp,
        kyai::config::model::DeployType::Custom);
    assert(currentModelName == modelName);
}

void testChangeCustomDetails(const std::string &modelName,
                             const std::string &apiUrl,
                             const std::string &apiKey,
                             const std::string &modelVersion) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setCustomModelApiUrl(modelName, apiUrl);
    auto models =
        modelConfig.getExtendedModels(kyai::config::model::DeployType::Custom);

    auto it =
        std::find_if(models.begin(), models.end(),
                     [modelName](const kyai::config::model::Model &model) {
                         return model.modelName == modelName;
                     });
    assert(it != models.end());
    assert(it->apiUrl == apiUrl);

    modelConfig.setCustomModelVersion(modelName, modelVersion);
    models =
        modelConfig.getExtendedModels(kyai::config::model::DeployType::Custom);
    it = std::find_if(models.begin(), models.end(),
                      [modelName](const kyai::config::model::Model &model) {
                          return model.modelName == modelName;
                      });
    assert(it != models.end());
    assert(it->modelVersion == modelVersion);

    modelConfig.setModelAuthentications(modelName,
                                        {{"", {{"APIKey", "apiKey", apiKey}}}});
    auto authens = modelConfig.getModelAuthentications(modelName);
    auto authenIt = std::find_if(
        authens.begin(), authens.end(),
        [&apiKey](const kyai::config::model::MultiAuthentication &authen) {
            auto it = std::find_if(
                authen.authentications.begin(), authen.authentications.end(),
                [&apiKey](const kyai::config::model::Authentication &auth) {
                    return auth.value == apiKey;
                });
            return it != authen.authentications.end();
        });
    assert(authenIt != authens.end());
}

void testSetCurrentModelName(kyai::config::model::AiCapability capability,
                             kyai::config::model::DeployType deployType,
                             const std::string &modelName) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setCurrentModelName(capability, deployType, modelName);
    assert(modelConfig.getCurrentModelName(capability, deployType) ==
           modelName);
}

void testClearModelAuthentications(const std::string &modelName) {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.clearModelAuthentications(modelName);
    auto authens = modelConfig.getModelAuthentications(modelName);
    assert(!authens.empty());
}

void testSetLoadOnBoot() {
    kyai::config::model::ModelConfig modelConfig;
    modelConfig.setLoadOnBoot("speechToText", false);
    assert(modelConfig.getLoadOnBoot("speechToText"));
    modelConfig.setLoadOnBoot("textToSpeech", false);
    assert(!modelConfig.getLoadOnBoot("textToSpeech"));
}

int main() {
    testGetModels();
    tesetSetModleAuthens();
    testSetCurrentModel();
    testSetModelDeployEnabled();
    testGetModelStatus();

    testDeleteCustomModel("DeepSeekV3");
    testDeleteCustomModel("DeepSeek-R1");
    testAddCustomModel("DeepSeekV3", "deepseek-v3",
                       "sk-0aa187ea167a4e78a508a3b4107e4fac",
                       "https://api.deepseek.com/dschat/completions");

    testAddCustomModel("DeepSeek-R1", "deepseek-r1",
                       "sk-dgwggrehasdaefas1234324dfdfdasdqw",
                       "https://api.deepseek.com/dsr1/completions");

    testChangeCustomDetails(
        "DeepSeek-R1", "https://api.deepseek.com/dsr11/completions",
        "sk-dgwggrehasdaefas1234324dfdfdasdqw1", "deepseek-r1");

    testChangeCustomDetails(
        "DeepSeekV3", "https://api.deepseek.com/dschat1/completions",
        "sk-0aa187ea167a4e78a508a3b4107e4fac1", "deepseek-v3");

    testSetCurrentModelName(kyai::config::model::AiCapability::Nlp,
                            kyai::config::model::DeployType::Custom,
                            "DeepSeekV3");
    testSetCurrentModelName(kyai::config::model::AiCapability::Nlp,
                            kyai::config::model::DeployType::Custom,
                            "DeepSeek-R1");

    testSetModelSchedulingPriority(kyai::config::model::DeployType::PublicCloud,
                                   2);
    testSetModelSchedulingPriority(kyai::config::model::DeployType::Ondevice,
                                   3);
    testSetModelSchedulingPriority(
        kyai::config::model::DeployType::Custom, 1);

    testSetCurrentModelName(kyai::config::model::AiCapability::Nlp,
                            kyai::config::model::DeployType::PublicCloud,
                            "讯飞-Spark Max");
    testSetCurrentModelName(kyai::config::model::AiCapability::Nlp,
                            kyai::config::model::DeployType::PublicCloud,
                            "百度-ERNIE-Bot-4");
    testSetLoadOnBoot();

    std::fprintf(stdout, "All tests passed!\n");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);

    return 0;
}