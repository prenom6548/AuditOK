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

#ifndef __KYLIN_AI_GEN_AI_VISION_IMAGE_H__
#define __KYLIN_AI_GEN_AI_VISION_IMAGE_H__

#include <kylin-ai/common/macros.h>

#include "imageconfig.h"
#include "imageresult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ImageResultCallback)(VisionImageResult *image_data, void *user_data);
typedef struct _GenAiImageSession GenAiImageSession;

/**
 * @brief Creates a new GenAI Image session.
 *
 * @return A pointer to the newly created GenAiImageSession, or NULL if creation
 * failed.
 */
AISDK_EXTERN
GenAiImageSession *genai_image_create_session();

/**
 * @brief Destroys a GenAI Image session and frees associated resources.
 *
 * @param session A double pointer to the session to be destroyed. The pointer
 * will be set to NULL after destruction.
 */
AISDK_EXTERN
void genai_image_destroy_session(GenAiImageSession **session);

/**
 * @brief Initializes the GenAI Image session.
 *
 * @param session The session handle.
 * @return AI_COMMON_SUCCESS on success, or an error code on failure.
 */
AISDK_EXTERN
int genai_image_init_session(GenAiImageSession *session);

/**
 * @brief Sets the configuration for the image generation session.
 *
 * @param session The session handle.
 * @param config The configuration object containing generation parameters.
 */
AISDK_EXTERN
void genai_image_set_config(GenAiImageSession *session, ImageConfig *config);

/**
 * @brief Sets the callback function to receive image generation results.
 *
 * @param session The session handle.
 * @param callback The function to call when an image result is ready.
 * @param user_data User-defined data to pass to the callback function.
 */
AISDK_EXTERN
void genai_image_result_set_callback(GenAiImageSession *session, ImageResultCallback callback, void *user_data);

/**
 * @brief Retrieves the list of supported image styles.
 *
 * @param session The session handle.
 * @param number A pointer to an integer where the number of supported styles
 * will be stored.
 * @return An array of strings representing the supported styles.
 */
AISDK_EXTERN
const char **genai_image_get_supported_image_style(GenAiImageSession *session, int *number);

/**
 * @brief Retrieves the list of supported image sizes.
 *
 * @param session The session handle.
 * @param number A pointer to an integer where the number of supported sizes
 * will be stored.
 * @return An array of ImageSize structures representing the supported sizes.
 */
AISDK_EXTERN
const ImageSize *genai_image_get_supported_image_size(GenAiImageSession *session, int *number);

/**
 * @brief Retrieves the maximum supported number of images that can be generated
 * in a single request.
 *
 * @param session The session handle.
 * @return The maximum number of images supported.
 */
AISDK_EXTERN
int genai_image_get_supported_image_number(GenAiImageSession *session);

/**
 * @brief Initiates an asynchronous image generation request.
 *
 * @param session The session handle.
 * @param prompt The text prompt describing the image to generate.
 */
AISDK_EXTERN
void genai_image_generate_image_async(GenAiImageSession *session, const char *prompt);

/**
 * @brief Initiates an asynchronous image generation request with a specific
 * request ID.
 *
 * @param session The session handle.
 * @param prompt The text prompt describing the image to generate.
 * @param request_id A unique identifier for the request.
 * @return AI_COMMON_SUCCESS on success, or an error code on failure
 */
AISDK_EXTERN
int genai_image_generate_image_async_with_request_id(GenAiImageSession *session, const char *prompt,
                                                     const char *request_id);

/**
 * @brief Stops an ongoing image generation request.
 *
 * @param session The session handle.
 * @param request_id The ID of the request to stop. If NULL or an empty string,
 * all requests are stopped.
 * @return AI_COMMON_SUCCESS on success, or an error code on failure.
 */
AISDK_EXTERN
int genai_image_stop_generation(GenAiImageSession *session, const char *request_id);

/**
 * @brief Let the SDK drive its own event loop instead of relying on GLib.
 *
 * By default you must run a GLib loop to dispatch network events. Enabling the
 * internal loop spawns a helper thread so callbacks fire even if the host
 * process does not run GLib.
 *
 * @param session Session instance.
 * @param enable  true enables the internal loop; false reverts to the default.
 */
AISDK_EXTERN
void genai_image_enable_internal_event_loop(GenAiImageSession *session, bool enable);

#ifdef __cplusplus
}
#endif

#endif
