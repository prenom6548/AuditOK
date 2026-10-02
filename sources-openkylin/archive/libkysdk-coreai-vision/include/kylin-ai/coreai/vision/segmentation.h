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
 * @file segmentation.h
 * @brief Interactive image segmentation API.
 *
 * This header provides the interface for interactive image segmentation operations.
 * The API supports asynchronous workflows where users can encode images and perform
 * segmentation with optional pixel point constraints for targeted region extraction.
 *
 * Typical usage flow:
 * 1. Create a session with segmentation_create_session()
 * 2. Initialize asynchronously with segmentation_init_session_async()
 * 3. Set result callback with segmentation_result_set_callback()
 * 4. Encode an image with segmentation_encode_image_file_async() or segmentation_encode_image_data_async()
 * 5. Run segmentation with segmentation_run_async()
 * 6. Process results in the callback
 * 7. Destroy the session with segmentation_destroy_session()
 *
 * @see segmentationresult.h for result handling.
 * @see types.h for common data structures.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_H__
#define __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_H__

#include <kylin-ai/common/macros.h>
#include <stddef.h>

#include "segmentationresult.h"
#include "types.h"

AISDK_BEGIN_EXTERN_C

/**
 * @brief Opaque handle for a segmentation session.
 */
typedef struct _SegmentationSession SegmentationSession;

/**
 * @brief Callback function type for segmentation results.
 *
 * This callback is invoked when the segmentation result is available.
 *
 * @param result    [in] The segmentation result handle. The lifecycle of this pointer is managed by the interface; it
 * will be automatically released after the callback returns. Do NOT manually free or use it after the callback.
 * @param user_data [in] User-defined data pointer, passed from the caller.
 *
 * @note The result pointer is only valid during the callback. Do not store or free it manually.
 *
 * @code
 * void my_callback(SegmentationResult* result, void* user_data) {
 *     // Use result here
 * }
 * @endcode
 */
typedef void (*SegmentationResultCallback)(SegmentationResult *result, void *user_data);

/**
 * @brief Callback for asynchronous session initialization.
 *
 * @param error_code [in] Result of the init call. 0 on success, non-zero on failure.
 * @param user_data  [in] User-defined data pointer, passed from the caller.
 */
typedef void (*SegmentationInitCallback)(int error_code, void *user_data);

/**
 * @brief  Callback for asynchronous image encoding operations.
 *
 * @param error_code [in] Result of the init call. 0 on success, non-zero on failure.
 * @param user_data  [in] User-defined data pointer, passed from the caller.
 */
typedef void (*SegmentationEncodeCallback)(int error_code, void *user_data);

/**
 * @brief Represents a set of focus pixels to constrain segmentation.
 *
 * When unset or empty, the entire image will be segmented.
 */
typedef struct SegmentationPixelSelection {
    const PixelPoint *points;
    size_t count;
} SegmentationPixelSelection;

/**
 * @brief Create a new segmentation session.
 *
 * @return The newly created session handle, or NULL on failure.
 *
 * @code
 * SegmentationSession* session = segmentation_create_session();
 * if (!session) {
 *     // Handle error
 * }
 * @endcode
 */
AISDK_EXTERN
SegmentationSession *segmentation_create_session();

/**
 * @brief Destroy a segmentation session.
 *
 * This function releases the memory associated with a session and sets the pointer to NULL.
 *
 * @param session [in,out] Pointer to the session handle pointer. Will be set to NULL after release.
 *
 * @code
 * segmentation_destroy_session(&session);
 * // session is now NULL
 * @endcode
 */
AISDK_EXTERN
void segmentation_destroy_session(SegmentationSession **session);

/**
 * @brief Initialize a segmentation session.
 *
 * @param session [in] The session handle.
 * @param callback [in] Callback invoked when initialization finishes. Optional.
 * @param user_data[in] User-defined data passed to the callback.
 *
 * @note This function returns immediately and runs initialization on a background thread. The caller must keep the
 * session alive until the callback is invoked.
 *
 * @code
 * segmentation_init_session_async(session, my_init_callback, my_data);
 * @endcode
 */
AISDK_EXTERN
void segmentation_init_session_async(SegmentationSession *session, SegmentationInitCallback callback, void *user_data);

/**
 * @brief Set the callback for segmentation result delivery.
 *
 * @param session   [in] The session handle.
 * @param callback  [in] The result callback function.
 * @param user_data [in] User-defined data pointer, will be passed to the callback.
 *
 * @code
 * segmentation_result_set_callback(session, my_callback, my_data);
 * @endcode
 */
AISDK_EXTERN
void segmentation_result_set_callback(SegmentationSession *session, SegmentationResultCallback callback,
                                      void *user_data);

/**
 * @brief Encode an image file asynchronously.
 *
 * @param session    [in] The session handle, must remain valid while encoding.
 * @param image_file [in] Path to the image file.
 * @param callback   [in] Callback to receive the encode status. Must not be NULL.
 * @param user_data  [in] User-defined data passed to the callback.
 *
 * @note The encoding runs on a background thread. The caller should keep `encoded_image` alive until callback fires.
 */
AISDK_EXTERN
void segmentation_encode_image_file_async(SegmentationSession *session, const char *image_file,
                                          SegmentationEncodeCallback callback, void *user_data);

/**
 * @brief Encode in-memory image bytes asynchronously.
 *
 * @param session           [in] The session handle, must remain valid while encoding.
 * @param image_data        [in] Pointer to raw image bytes.
 * @param image_data_length [in] Length of the raw image buffer.
 * @param callback          [in] Callback to receive the encode status. Must not be NULL.
 * @param user_data         [in] User-defined data passed to the callback.
 *
 */
AISDK_EXTERN
void segmentation_encode_image_data_async(SegmentationSession *session, const char *image_data,
                                          size_t image_data_length, SegmentationEncodeCallback callback,
                                          void *user_data);

/**
 * @brief Clear cached interactive segmentation results.
 *
 * Segmentation operates interactively: each run uses the latest points, while the backend accumulates historical
 * segmentation outputs and returns them together. Call this API to drop all cached masks/images and start a fresh
 * history for subsequent runs.
 *
 * @param session [in] The session handle.
 * @return 0 on success, non-zero on failure.
 */
AISDK_EXTERN
int segmentation_clear_cache(SegmentationSession *session);

/**
 * @brief Perform segmentation asynchronously using the latest encoded image.
 *
 * This call runs segmentation on the most recently encoded image payload. When both segmentation_encode_image_file()
 * and segmentation_encode_image_data() are invoked, the newest encode action wins. Results are delivered via the
 * registered callback.
 *
 * @param session              [in] The session handle.
 * @param pixel_points         [in] Optional pixel selection to restrict segmentation. Pass NULL or an empty collection
 * to process the whole image.
 * @param min_mask_region_area [in] The minimum mask region area (in pixels) to keep.
 */
AISDK_EXTERN
void segmentation_run_async(SegmentationSession *session, const SegmentationPixelSelection *pixel_points,
                            int min_mask_region_area);

/**
 * @brief Enable the SDK-managed GLib event loop for callbacks.
 *
 * By default (enable = false), callbacks are invoked on the caller's current thread, and the caller must start and run
 * the GLib main loop to drive event dispatching. When enable is true, the SDK launches its own GLib event loop
 * internally, so callbacks will be executed on a dedicated SDK thread instead of the caller's thread.
 *
 * @param session [in] The session handle.
 * @param enable  [in] Whether to run the internal event loop.
 */
AISDK_EXTERN
void segmentation_enable_internal_event_loop(SegmentationSession *session, bool enable);

AISDK_END_EXTERN_C

#endif  // __KYLIN_AI_CORE_AI_VISION_SEGMENTATION_H__
