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

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <string>
#include <vector>

#include <FreeImage.h>

namespace vision {

namespace image_loader {

struct ImageBaseInfo {
    std::string format;
    int width;
    int height;
};

std::vector<unsigned char> downloadImage(const std::string &imageUrl, std::string &errorMessage);
ImageBaseInfo getImageInfoFromData(unsigned char *data, unsigned int length);
std::string freeImageFormatToString(FREE_IMAGE_FORMAT format);

} // namespace image_loader

} // namespace vision

#endif // IMAGELOADER_H
