/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

/**
 * @file types.h
 * @brief Common data types used throughout the Vision SDK.
 *
 * This header defines fundamental data structures shared across multiple
 * vision APIs, including pixel coordinates and segmentation mask data.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_TYPES_H
#define __KYLIN_AI_CORE_AI_VISION_TYPES_H

#include <stddef.h>

/**
 * @brief Structure representing a 2D pixel coordinate.
 *
 * This structure is used to specify pixel positions within an image.
 * Coordinates are zero-indexed with the origin at the top-left corner.
 */
typedef struct {
    int x;  /**< Horizontal pixel coordinate (column index, 0-indexed from left). */
    int y;  /**< Vertical pixel coordinate (row index, 0-indexed from top). */
} PixelPoint;

/**
 * @brief Structure containing segmentation mask data.
 *
 * This structure holds the raw binary mask buffer produced by segmentation operations.
 * The mask data represents an alpha channel where each byte corresponds to
 * a pixel's segmentation value (0 = background, 255 = foreground).
 *
 * @note The mask_data pointer is managed by the SDK and should not be freed
 * manually. It remains valid only during the result callback scope.
 */
typedef struct {
    unsigned char *mask_data;  /**< Pointer to the raw mask buffer (binary alpha values). */
    size_t mask_length;        /**< Total size of the mask buffer in bytes. */
    int mask_width;            /**< Width of the mask in pixels. */
    int mask_height;           /**< Height of the mask in pixels. */
} SegmentationMaskData;

#endif  // __KYLIN_AI_CORE_AI_VISION_TYPES_H
