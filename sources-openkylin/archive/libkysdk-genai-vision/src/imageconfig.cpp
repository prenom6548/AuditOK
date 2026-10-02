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

#include "imageconfig.h"

#include <kylin-ai/common/error.h>

#include <cstring>

#include "_imageconfig.h"

namespace {
std::string jsonToString(const Json::Value &value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}
}  // namespace

std::string imageConfigToJson(const ImageConfig &config) {
    Json::Value root;

    root["width"] = config.width;
    root["height"] = config.height;
    root["number"] = config.number;
    root["style"] = config.style;
    root["modelName"] = config.modelName;

    // 序列化 JSON 对象
    return jsonToString(root);
}

std::string imageGenerationRequestToString(const ImageGenerationRequest &request) {
    Json::Value root;
    root["prompt"] = request.prompt;
    root["width"] = request.width;
    root["height"] = request.height;
    root["number"] = request.number;
    root["style"] = request.style;
    root["modelName"] = request.modelName;
    return jsonToString(root);
}

ImageConfig *image_config_create() {
    ImageConfig *config = new _ImageConfig();
    return config;
}

void image_config_destroy(ImageConfig **config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

void image_config_set_generation_number(ImageConfig *config, int number) {
    if (config == nullptr) {
        return;
    }
    config->number = number;
}

void image_config_set_style(ImageConfig *config, const char *style) {
    if (config == nullptr || style == nullptr) {
        return;
    }
    config->style = style;
}

void image_config_set_size(ImageConfig *config, ImageSize image_size) {
    if (config == nullptr) {
        return;
    }
    config->height = image_size.height;
    config->width = image_size.width;
}

int image_config_set_model_name(ImageConfig *config, const char *model_name) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (model_name != nullptr) {
        config->modelName = model_name;
    }
    return AI_COMMON_SUCCESS;
}
