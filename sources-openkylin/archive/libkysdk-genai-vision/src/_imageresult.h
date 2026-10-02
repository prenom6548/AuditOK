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

#ifndef _IMAGERESULT_H_
#define _IMAGERESULT_H_

#include <cstdint>
#include <string>
#include <vector>

#include "imageformat.h"

struct _VisionImageResult {
    ImageFormat format;
    int width{0};
    int height{0};
    int total{0};
    int index{-1};
    std::vector<uint8_t> data;
    int errorCode{0};
    std::string errorMessage;
    std::string requestId;
};

#endif
