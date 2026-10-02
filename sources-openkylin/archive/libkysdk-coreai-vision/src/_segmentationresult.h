/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H
#define KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H

#include <kylin-ai/common/error.h>

#include <string>
#include <vector>

#include "types.h"

struct _SegmentationResult {
    SegmentationMaskData mask_{};
    std::vector<uint8_t> data_{};
    int errorCode{AI_COMMON_SUCCESS};
    std::string errorMessage;
};

#endif  // KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H
