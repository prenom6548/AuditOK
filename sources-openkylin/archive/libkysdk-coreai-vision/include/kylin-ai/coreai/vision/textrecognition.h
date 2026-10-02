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
 * @file textrecognition.h
 * @brief Text recognition (OCR) API.
 *
 * This header provides the interface for optical character recognition (OCR)
 * operations. The API extracts text content from images, returning both the
 * full text and individual text lines with their bounding box coordinates.
 *
 * Typical usage flow:
 * 1. Create a session with text_recognition_create_session()
 * 2. Initialize with text_recognition_init_session()
 * 3. Set result callback with text_recognition_result_set_callback()
 * 4. Optionally configure with text_recognition_set_model_config()
 * 5. Run recognition with text_recognition_recognize_text_from_image_file_async()
 *    or text_recognition_recognize_text_from_image_data_async()
 * 6. Process results in the callback
 * 7. Destroy the session with text_recognition_destroy_session()
 *
 * @see textrecognitionconfig.h for configuration options.
 * @see textrecognitionresult.h for result handling.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_H__
#define __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_H__

#include <kylin-ai/common/macros.h>

#include "config.h"
#include "textrecognitionresult.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for text recognition session.
 */
typedef struct _TextRecognitionSession TextRecognitionSession;

/**
 * @brief Callback function type for text recognition result.
 *
 * This callback is invoked when the text recognition result is available.
 *
 * @param result    [in] The text recognition result handle. The lifecycle
 * of this pointer is managed by the interface; it will be automatically
 * released after the callback returns. Do NOT manually free or use it after the
 * callback.
 * @param user_data [in] User-defined data pointer, passed from the caller.
 *
 * @note The result pointer is only valid during the callback. Do not store or
 * free it manually.
 */
typedef void (*TextRecognitionResultCallback)(TextRecognitionResult *result, void *user_data);

/**
 * @brief Create a new text recognition session.
 *
 * @return The newly created session handle, or NULL on failure.
 *
 * @code
 * TextRecognitionSession* session = text_recognition_create_session();
 * if (!session) {
 *     // Handle error
 * }
 * @endcode
 */
AISDK_EXTERN
TextRecognitionSession *text_recognition_create_session();

/**
 * @brief Destroy a text recognition session.
 *
 * This function releases the memory associated with a session and sets the
 * pointer to NULL.
 *
 * @param session [in,out] Pointer to the session handle pointer. Will be set to
 * NULL after release.
 *
 * @code
 * text_recognition_destroy_session(&session);
 * // session is now NULL
 * @endcode
 */
AISDK_EXTERN
void text_recognition_destroy_session(TextRecognitionSession **session);

/**
 * @brief Initialize a text recognition session.
 *
 * This function initializes the text recognition session.
 *
 * @param session  [in] The session handle.
 * @return 0 on success, non-zero error code on failure.
 *
 * @code
 * int ret = text_recognition_init_session(session);
 * if (ret != 0) {
 *     // Handle error
 * }
 * @endcode
 */
AISDK_EXTERN
int text_recognition_init_session(TextRecognitionSession *session);

/**
 * @brief Set the callback for text recognition result.
 *
 * @param session   [in] The session handle.
 * @param callback  [in] The result callback function.
 * @param user_data [in] User-defined data pointer, will be passed to the
 * callback.
 *
 * @code
 * text_recognition_result_set_callback(session, my_callback, my_data);
 * @endcode
 */
AISDK_EXTERN
void text_recognition_result_set_callback(TextRecognitionSession *session, TextRecognitionResultCallback callback,
                                          void *user_data);

/**
 * @brief Set the model configuration for text recognition.
 *
 * @param session [in] The session handle.
 * @param config  [in] The model configuration handle.
 *
 * @code
 * text_recognition_set_model_config(session, config);
 * @endcode
 */
AISDK_EXTERN
void text_recognition_set_model_config(TextRecognitionSession *session, TextRecognitionModelConfig *config);

/**
 * @brief Perform text recognition from an image file asynchronously.
 *
 * This function starts the text recognition process from the specified
 * image file. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param image_file [in] Path to the image file.
 *
 * @code
 * text_recognition_recognize_text_from_image_file_async(session,
 * "/path/to/image.jpg");
 * @endcode
 */
AISDK_EXTERN
void text_recognition_recognize_text_from_image_file_async(TextRecognitionSession *session, const char *image_file);

/**
 * @brief Perform text recognition from an image file asynchronously with request id.
 *
 * This function starts the text recognition process from the specified
 * image file. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param request_id [in] Optional request id for tracing; pass nullptr to auto-generate.
 * @param image_file [in] Path to the image file.
 *
 * @code
 * text_recognition_recognize_text_from_image_file_async_with_request_id(session,
 * "request-001", "/path/to/image.jpg");
 * @endcode
 */
AISDK_EXTERN
void text_recognition_recognize_text_from_image_file_async_with_request_id(TextRecognitionSession *session,
                                                                           const char *request_id,
                                                                           const char *image_file);

/**
 * @brief Perform text recognition from image data asynchronously.
 *
 * This function starts the text recognition process from the specified
 * image data. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param image_data [in] Image data
 * @param image_data_length [in] Image data length
 *
 * @code
 * text_recognition_recognize_text_from_image_data_async(session, image_data,
 * image_data_length);
 * @endcode
 */
AISDK_EXTERN
void text_recognition_recognize_text_from_image_data_async(TextRecognitionSession *session, const char *image_data,
                                                           unsigned int image_data_length);

/**
 * @brief Perform text recognition from image data asynchronously with request id.
 *
 * This function starts the text recognition process from the specified
 * image data. The result will be delivered via callback.
 *
 * @param session    [in] The session handle.
 * @param request_id [in] Optional request id for tracing; pass nullptr to auto-generate.
 * @param image_data [in] Image data
 * @param image_data_length [in] Image data length
 *
 * @code
 * text_recognition_recognize_text_from_image_data_async_with_request_id(session, "request-001", image_data,
 * image_data_length);
 * @endcode
 */
AISDK_EXTERN
void text_recognition_recognize_text_from_image_data_async_with_request_id(TextRecognitionSession *session,
                                                                           const char *request_id,
                                                                           const char *image_data,
                                                                           unsigned int image_data_length);

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
void text_recognition_enable_internal_event_loop(TextRecognitionSession *session, bool enable);

#ifdef __cplusplus
}
#endif

#endif
