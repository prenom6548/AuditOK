/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "_speechmodelconfig.h"

#include <jsoncpp/json/json.h>

SpeechModelConfig *speech_model_config_create() { return new _SpeechModelConfig(); }

void speech_model_config_destroy(SpeechModelConfig **config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

void speech_model_config_set_name(SpeechModelConfig *config, const char *name) {
    if (config == nullptr) {
        return;
    }
    config->name = name;
}
void speech_model_config_set_deploy_type(SpeechModelConfig *config, ModelDeployType type) {
    if (config == nullptr) {
        return;
    }
    config->type = type;
}

std::string modelConfigToJson(const _SpeechModelConfig &modelconfig) {
    Json::Value root;
    root["name"] = modelconfig.name;
    switch (modelconfig.type) {
        case ModelDeployType::OnDevice:
            root["type"] = "Local";
            break;
        case ModelDeployType::PrivateCloud:
            root["type"] = "PrivateCloud";
            break;
        case ModelDeployType::PublicCloud:
            root["type"] = "PublicCloud";
            break;
        default:
            root["type"] = "Default";
            break;
    }
    Json::FastWriter writer;
    return writer.write(root);
}
