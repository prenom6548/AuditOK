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

#include "imageloader.h"
#include <FreeImage.h>
#include <curl/curl.h>
#include "cpr/api.h"
#include "cpr/callback.h"

namespace {

const std::map<FREE_IMAGE_FORMAT, std::string> formatToString = {
    {FIF_UNKNOWN, "unknown"}, {FIF_JPEG, "jpeg"}, {FIF_PNG, "png"}, {FIF_WEBP, "webp"}};

bool writeImageData(std::string data, intptr_t userdata) {
    std::vector<unsigned char> *buffer = (std::vector<unsigned char> *)(userdata);
    for (const auto &ch : data) {
        buffer->push_back(ch);
    }

    return true;
}

} // namespace

std::vector<unsigned char> vision::image_loader::downloadImage(const std::string &imageUrl, std::string &errorMessage) {
    std::vector<unsigned char> imageData;
    imageData.reserve(1024 * 1024);
    cpr::Url url{imageUrl};
    cpr::Session session;
    session.SetUrl(url);
    cpr::Response response = session.Download(cpr::WriteCallback{writeImageData, (intptr_t)&imageData});

    if (response.error.code != cpr::ErrorCode::OK) {
        errorMessage = response.error.message;
    }

    return imageData;
}

vision::image_loader::ImageBaseInfo vision::image_loader::getImageInfoFromData(unsigned char *data,
                                                                               unsigned int length) {
    FIMEMORY *memory = FreeImage_OpenMemory(data, length);
    FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memory);
    FIBITMAP *bits = FreeImage_LoadFromMemory(format, memory);
    int width = FreeImage_GetWidth(bits);
    int height = FreeImage_GetHeight(bits);
    FreeImage_Unload(bits);
    FreeImage_CloseMemory(memory);

    return image_loader::ImageBaseInfo{freeImageFormatToString(format), width, height};
}

std::string vision::image_loader::freeImageFormatToString(FREE_IMAGE_FORMAT format) {
    auto it = formatToString.find(format);
    if (it != formatToString.end()) {
        return it->second;
    } else {
        return "unknown";
    }
}