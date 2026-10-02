/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef __KYLIN_AI_GEN_AI_VISION_IMAGE_FORMAT_H__
#define __KYLIN_AI_GEN_AI_VISION_IMAGE_FORMAT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enum representing supported image formats.
 */
typedef enum {
    IMAGE_FORMAT_UNKNOWN = -1, /**< Unknown image format */
    IMAGE_FORMAT_JPEG,         /**< JPEG image format */
    IMAGE_FORMAT_PNG,          /**< PNG image format */
    IMAGE_FORMAT_WEBP          /**< WebP image format */
} ImageFormat;

#ifdef __cplusplus
}
#endif

#endif
