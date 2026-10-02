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
 * @file subjectsegmentation.h
 * @brief Subject segmentation API for foreground extraction.
 *
 * This header provides the interface for automatic subject segmentation,
 * which extracts foreground subjects (people, objects) from images. The API
 * supports both portrait-specific and general-purpose segmentation modes.
 *
 * Typical usage flow:
 * 1. Create a session with subject_segmentation_create_session()
 * 2. Initialize asynchronously with subject_segmentation_init_session_async()
 * 3. Set result callback with subject_segmentation_result_set_callback()
 * 4. Optionally configure with subject_segmentation_set_config()
 * 5. Run segmentation with subject_segmentation_run_from_image_file_async()
 *    or subject_segmentation_run_from_image_data_async()
 * 6. Process results in the callback
 * 7. Destroy the session with subject_segmentation_destroy_session()
 *
 * @see subjectsegmentationconfig.h for configuration options.
 * @see subjectsegmentationresult.h for result handling.
 */

#ifndef __SUBJECT_SEGMENTATION_H__
#define __SUBJECT_SEGMENTATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "subjectsegmentationresult.h"

/**
 * @brief Opaque handle for subject segmentation session.
 */
typedef struct _SubjectSegmentationSession SubjectSegmentationSession;

/**
 * @brief Callback function type for subject segmentation result.
 *
 * This callback is invoked when the subject segmentation result is available.
 *
 * @param result    [in] The subject segmentation result handle. The lifecycle
 * of this pointer is managed by the interface; it will be automatically
 * released after the callback returns. Do NOT manually free or use it after the
 * callback.
 * @param user_data [in] User-defined data pointer, passed from the caller.
 *
 * @note The result pointer is only valid during the callback. Do not store or
 * free it manually.
 *
 * @code
 * void my_callback(SubjectSegmentationResult* result, void* user_data) {
 *     // Use result here
 * }
 * @endcode
 */
typedef void (*SubjectSegmentationResultCallback)(SubjectSegmentationResult *result, void *user_data);

/**
 * @brief Completion callback for @ref subject_segmentation_init_session_async.
 *
 * The callback runs after the background session initialization finishes. Use
 * it to begin issuing requests without blocking the main thread.
 *
 * @param error_code Matches the synchronous return codes; 0 means success.
 * @param user_data  User data from registration.
 */
typedef void (*SubjectSegmentationInitCallback)(int error_code, void *user_data);

/**
 * @brief Create a new subject segmentation session.
 *
 * @return The newly created session handle, or NULL on failure.
 *
 * @code
 * SubjectSegmentationSession* session = subject_segmentation_create_session();
 * if (!session) {
 *     // Handle error
 * }
 * @endcode
 */
SubjectSegmentationSession *subject_segmentation_create_session();

/**
 * @brief Destroy a subject segmentation session.
 *
 * This function releases the memory associated with a session and sets the
 * pointer to NULL.
 *
 * @param session [in,out] Pointer to the session handle pointer. Will be set to
 * NULL after release.
 *
 * @code
 * subject_segmentation_destroy_session(&session);
 * // session is now NULL
 * @endcode
 */
void subject_segmentation_destroy_session(SubjectSegmentationSession **session);

/**
 * @brief Initialize a subject segmentation session asynchronously.
 *
 * This function initializes the subject segmentation session in the background.
 * The result will be delivered via callback.
 *
 * @param session  [in] The session handle.
 * @param callback [in] The completion callback function.
 * @param user_data [in] User-defined data pointer, will be passed to the
 * callback.
 *
 * @code
 * void init_callback(int error_code, void* user_data) {
 *     if (error_code == 0) {
 *         // Initialization succeeded
 *     } else {
 *         // Handle error
 *     }
 * }
 *
 * subject_segmentation_init_session_async(session, init_callback, my_data);
 * @endcode
 */

void subject_segmentation_init_session_async(SubjectSegmentationSession *session,
                                             SubjectSegmentationInitCallback callback, void *user_data);

/**
 * @brief Set the callback for subject_segmentation result.
 *
 * @param session   [in] The session handle.
 * @param callback  [in] The result callback function.
 * @param user_data [in] User-defined data pointer, will be passed to the
 * callback.
 *
 * @code
 * subject_segmentation_result_set_callback(session, my_callback, my_data);
 * @endcode
 */
void subject_segmentation_result_set_callback(SubjectSegmentationSession *session,
                                              SubjectSegmentationResultCallback callback, void *user_data);

/**
 * @brief Set the model configuration for subject segmentation.
 *
 * @param session [in] The session handle.
 * @param config  [in] The model configuration handle.
 *
 * @code
 * subject_segmentation_set_config(session, config);
 * @endcode
 */
void subject_segmentation_set_config(SubjectSegmentationSession *session, SubjectSegmentationConfig *config);

/**
 * @brief Perform subject segmentation from an image file asynchronously.
 *
 * This function starts the subject segmentation process from the specified
 * image file. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param request_id [in] Optional request id for tracing; pass nullptr to auto-generate.
 * @param image_file [in] Path to the image file.
 *
 * @code
 * subject_segmentation_run_from_image_file_async(session,
 * "request-001", "/path/to/image.jpg");
 * @endcode
 */
void subject_segmentation_run_from_image_file_async(SubjectSegmentationSession *session, const char *request_id,
                                                    const char *image_file);

/**
 * @brief Perform subject segmentation from image data asynchronously.
 *
 * This function starts the subject segmentation process from the specified
 * image data. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param request_id [in] Optional request id for tracing; pass nullptr to auto-generate.
 * @param image_data [in] Image data
 * @param image_data_length [in] Image data length
 *
 * @code
 * subject_segmentation_run_from_image_data_async(session, "request-001", image_data,
 * image_data_length);
 * @endcode
 */
void subject_segmentation_run_from_image_data_async(SubjectSegmentationSession *session, const char *request_id,
                                                    const char *image_data, unsigned int image_data_length);

/**
 * @brief Let the SDK drive its own event loop instead of relying on GLib.
 *
 * By default, you must run a GLib loop to dispatch network events. Enabling the
 * internal loop spawns a helper thread so callbacks fire even if the host
 * process does not run GLib.
 *
 * @param session Session instance.
 * @param enable  true enables the internal loop; false reverts to the default.
 */
AISDK_EXTERN
void subject_segmentation_enable_internal_event_loop(SubjectSegmentationSession *session, bool enable);
#ifdef __cplusplus
}
#endif

#endif  // __SUBJECT_SEGMENTATION_H__
