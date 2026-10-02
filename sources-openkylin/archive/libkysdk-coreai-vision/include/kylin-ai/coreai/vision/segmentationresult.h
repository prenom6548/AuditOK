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

/**
 * @file segmentationresult.h
 * @brief Result handling interface for interactive image segmentation.
 *
 * This header defines the data structures and accessor functions for retrieving
 * segmentation results. Results include mask data representing segmented regions
 * and error information when operations fail.
 *
 * @note Result objects are owned by the SDK and are only valid during the
 * callback invocation. Do not store pointers or attempt to free them manually.
 *
 * @see segmentation.h for the main segmentation API.
 * @see types.h for SegmentationMaskData structure definition.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_RESULT_H__
#define __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_RESULT_H__

#include <kylin-ai/common/macros.h>
#include <stddef.h>
#include <stdint.h>

#include "types.h"

AISDK_BEGIN_EXTERN_C

/**
 * @brief Opaque handle describing the full segmentation output.
 *
 * Instances are owned by the interface and only valid during the callback that delivers them. Do not store the pointer
 * or attempt to free it manually.
 */
typedef struct _SegmentationResult SegmentationResult;

/**
 * @brief Get the mask data collection for the segmentation result.
 *
 * The returned pointer references an array where each entry contains the shared mask buffer (whole-frame alpha matte)
 * plus the precise pixel points that describe a connected segment contour.
 *
 * @param result      [in]  Segmentation result handle.
 * @return Pointer to the mask entry array, or NULL when data is unavailable.
 *
 * @note The returned memory is owned by the SDK and tied to the lifetime of the enclosing SegmentationResult. Do not
 * free or cache the pointer.
 */
AISDK_EXTERN
const SegmentationMaskData *segmentation_result_get_mask_data(SegmentationResult *result);

/**
 * @brief Get the error code reported by the runtime.
 *
 * A value of 0 indicates success. Non-zero codes map to pipeline failures or validation errors.
 */
AISDK_EXTERN
int segmentation_result_get_error_code(SegmentationResult *result);

/**
 * @brief Get the error message reported by the runtime.
 *
 * Provides human-readable diagnostics when inference fails or validations reject the input.
 */
AISDK_EXTERN
const char *segmentation_result_get_error_message(SegmentationResult *result);

AISDK_END_EXTERN_C

#endif  // __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_RESULT_H__
