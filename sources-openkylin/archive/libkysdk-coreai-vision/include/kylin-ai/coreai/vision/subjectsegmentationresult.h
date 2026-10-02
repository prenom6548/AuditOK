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
 * @file subjectsegmentationresult.h
 * @brief Result handling interface for subject segmentation.
 *
 * This header defines the data structures and accessor functions for retrieving
 * subject segmentation results. Results include:
 * - Mask data (alpha channel representing foreground/background)
 * - Foreground image data (PNG format with transparent background)
 * - Edge point groups (contour coordinates of segmented regions)
 * - Error information when operations fail
 *
 * @note Result objects are owned by the SDK and are only valid during the
 * callback invocation. Do not store pointers or attempt to free them manually.
 *
 * @see subjectsegmentation.h for the main segmentation API.
 * @see types.h for common data structures.
 */

#ifndef __SUBJECT_SEGMENTATION_RESULT_H__
#define __SUBJECT_SEGMENTATION_RESULT_H__

#include <stddef.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for subject segmentation result.
 */
typedef struct _SubjectSegmentationResult SubjectSegmentationResult;

/**
 * @brief Structure representing a group of points.
 */
typedef struct {
    PixelPoint *points;
    int point_count;
} SubjectSegmentationPointGroup;

/**
 * @brief Get the request id of the subject segmentation result.
 *
 * @param result [in] Handle to the subject segmentation result.
 * @return Pointer to the request id string, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 *
 * @code
 * const char* request_id = subject_segmentation_result_get_request_id(result);
 * if (request_id) {
 *     // Use request id here
 * }
 * @endcode
 */
const char *subject_segmentation_result_get_request_id(const SubjectSegmentationResult *result);

/**
 * @brief Get the mask data from the subject segmentation result.
 *
 * This function returns the alpha mask of the foreground. The mask represents
 * the transparency level of each pixel (0 for fully transparent background,
 * 255 for fully opaque foreground), and the data is returned as a pointer to
 * the raw binary mask buffer (one byte per pixel).
 *
 * @param result      [in]  Handle to the subject segmentation result.
 * @return Pointer to the mask data, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 *
 * @code
 * const SegmentationMaskData *mask =
 *      subject_segmentation_result_get_mask_data(result);
 * if (mask) {
 *     // Use mask data here
 * }
 * @endcode
 */
const SegmentationMaskData *subject_segmentation_result_get_mask_data(const SubjectSegmentationResult *result);

/**
 * @brief Get the foreground image data from the subject segmentation result.
 *
 * This function returns the foreground image with background removed. The image
 * retains the original color information of the foreground subject, and the
 * data is returned as a pointer to the raw encoded image bytes. The specific
 * format can be retrieved via subject_segmentation_result_get_foreground_image_format().
 *
 * @param result      [in]  Handle to the subject segmentation result.
 * @param width       [out] Pointer to store the width of the image (in pixels).
 * @param height      [out] Pointer to store the height of the image (in
 * pixels).
 * @param data_length [out] Pointer to store the length of the image data in
 * bytes
 * @return Pointer to the foreground image data, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 *
 * @code
 * int width, height;
 * size_t data_length;
 * const unsigned char* fg_image =
 * subject_segmentation_result_get_foreground_image_data(result, &width,
 * &height, &data_length);
 * if (fg_image) {
 *     // Use foreground image data here
 * }
 * @endcode
 */
const unsigned char *subject_segmentation_result_get_foreground_image_data(const SubjectSegmentationResult *result,
                                                                           int *width, int *height,
                                                                           size_t *data_length);

/**
 * @brief Get the format of the foreground image data.
 *
 * @param result [in] Handle to the subject segmentation result.
 * @return Pointer to a null-terminated string describing the foreground image
 * format (e.g., "png"), or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object.
 */
const char *subject_segmentation_result_get_foreground_image_format(const SubjectSegmentationResult *result);

/**
 * @brief Get the edge point groups from the subject segmentation result.
 *
 * Each group contains the edge points of a single connected region,
 * and the points in each group are ordered in a counterclockwise direction.
 *
 * @param result [in] Handle to the subject segmentation result.
 * @param group_counts [out] Pointer to store the number of groups.
 * @return Pointer to the array of SubjectSegmentationPointGroup, or NULL if
 * unavailable.
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 *
 * @code
 * int group_counts;
 * SubjectSegmentationPointGroup *point_groups =
 *     subject_segmentation_result_get_edge_point_groups(result, &group_counts);
 * for (int i = 0; i < group_counts; ++i) {
 *     // Use point_groups[i].points and point_groups[i].point_count here
 *     for (int j = 0; j < point_groups[i].point_count; ++j) {
 *         int x = point_groups[i].points[j].x;
 *         int y = point_groups[i].points[j].y;
 *         // Use (x, y) here
 *     }
 * }
 * @endcode
 */
const SubjectSegmentationPointGroup *subject_segmentation_result_get_edge_point_groups(
    const SubjectSegmentationResult *result, int *group_counts);

/**
 * @brief Get the error code of the subject segmentation result.
 *
 * @param result [in] Handle to the subject segmentation result.
 * @return Error code. 0 means no error, non-zero means failure.
 *
 * @code
 * int err = subject_segmentation_result_get_error_code(result);
 * if (err != 0) {
 *     // Handle error
 * }
 * @endcode
 */
int subject_segmentation_result_get_error_code(const SubjectSegmentationResult *result);

/**
 * @brief Get the error message of the subject segmentation result.
 *
 * @param result [in] Handle to the subject segmentation result.
 * @return Pointer to the error message string, or NULL if no error.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 *
 * @code
 * const char* msg = subject_segmentation_result_get_error_message(result);
 * if (msg) {
 *     // Print or log error message
 * }
 * @endcode
 */
const char *subject_segmentation_result_get_error_message(const SubjectSegmentationResult *result);

#ifdef __cplusplus
}
#endif

#endif  // __SUBJECT_SEGMENTATION_RESULT_H__
