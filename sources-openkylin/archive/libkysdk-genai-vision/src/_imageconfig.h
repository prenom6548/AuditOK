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

#ifndef _IMAGECONFIG_H
#define _IMAGECONFIG_H

#include <jsoncpp/json/json.h>

#include <cstdint>
#include <string>

#include "imageconfig.h"

struct _ImageConfig {
    int width = 0;
    int height = 0;
    int number = 0;
    std::string style;
    std::string modelName;
};

struct ImageGenerationRequest : public ImageConfig {
    std::string prompt;
};

std::string imageConfigToJson(const ImageConfig &config);

std::string imageGenerationRequestToString(const ImageGenerationRequest &request);

#endif
