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

#ifndef SRC_SERVICES_GENAI_VISION_VISION_H
#define SRC_SERVICES_GENAI_VISION_VISION_H

#include <kylin-ai/aiengine.h>

#include <map>

#include "imagegenerationrequest.h"

namespace genai {

static int getVisionImageFormat(const std::string &imageFormat) {
    std::map<std::string, ImageFormat> visionEngineImageFormatMap{
        {"jpg", ImageFormat::IMAGE_FORMAT_JPEG},  {"jpeg", ImageFormat::IMAGE_FORMAT_JPEG},
        {"JPG", ImageFormat::IMAGE_FORMAT_JPEG},  {"JPEG", ImageFormat::IMAGE_FORMAT_JPEG},
        {"png", ImageFormat::IMAGE_FORMAT_PNG},   {"PNG", ImageFormat::IMAGE_FORMAT_PNG},
        {"webp", ImageFormat::IMAGE_FORMAT_WEBP}, {"WEBP", ImageFormat::IMAGE_FORMAT_WEBP},
    };

    if (visionEngineImageFormatMap.find(imageFormat) != visionEngineImageFormatMap.end()) {
        return visionEngineImageFormatMap.at(imageFormat);
    }
    return ImageFormat::IMAGE_FORMAT_UNKNOWN;
}
}  // namespace genai

#endif
