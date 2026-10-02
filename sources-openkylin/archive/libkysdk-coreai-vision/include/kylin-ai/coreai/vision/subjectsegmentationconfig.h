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
 * @file subjectsegmentationconfig.h
 * @brief Configuration interface for subject segmentation.
 *
 * This header provides the configuration types and functions for subject
 * segmentation operations. It allows users to configure segmentation modes
 * (portrait or general) to optimize the segmentation algorithm for different
 * use cases.
 *
 * @see subjectsegmentation.h for the main segmentation API.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_SUBJECT_SEGMENTATION_CONFIG_H__
#define __KYLIN_AI_CORE_AI_VISION_SUBJECT_SEGMENTATION_CONFIG_H__

#include <kylin-ai/common/enums.h>
#include <kylin-ai/common/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Segmentation mode for subject segmentation.
 *
 * Specifies the algorithmic behavior used during subject segmentation.
 * Different modes are optimized for different types of foreground subjects.
 */
typedef enum {
    SUBJECT_SEGMENTATION_MODE_PORTRAIT = 0,  /**< Portrait segmentation mode, optimized for human subjects. */
    SUBJECT_SEGMENTATION_MODE_GENERAL = 1,   /**< General segmentation mode, suitable for any foreground object. */
} SubjectSegmentationMode;

/**
 * @brief Opaque handle for subject segmentation configuration.
 *
 * This structure holds configuration parameters such as segmentation mode.
 * It must be created via subject_segmentation_config_create() and destroyed
 * via subject_segmentation_config_destroy().
 */
typedef struct _SubjectSegmentationConfig SubjectSegmentationConfig;

/**
 * @brief Create a new subject segmentation configuration instance.
 *
 * Allocates and initializes a default configuration object.
 *
 * @return Pointer to a new SubjectSegmentationConfig instance, or NULL on
 * failure.
 *
 * @code
 * SubjectSegmentationConfig *config = subject_segmentation_config_create();
 * if (!config) {
 *     // Handle allocation failure
 * }
 * @endcode
 */
AISDK_EXTERN
SubjectSegmentationConfig *subject_segmentation_config_create();

/**
 * @brief Destroy a subject segmentation configuration instance.
 *
 * Frees all resources associated with the configuration object and sets the
 * pointer to NULL.
 *
 * @param config [in,out] Address of the configuration handle pointer. After
 * destruction, the pointed-to pointer is set to NULL.
 *
 * @note Passing NULL or a pointer to NULL is safe and has no effect.
 *
 * @code
 * subject_segmentation_config_destroy(&config);
 * // config is now NULL
 * @endcode
 */
AISDK_EXTERN
void subject_segmentation_config_destroy(SubjectSegmentationConfig **config);

/**
 * @brief Set the segmentation mode in the configuration.
 *
 * @param config [in] The configuration handle. Must not be NULL.
 * @param mode   [in] The desired segmentation mode.
 *
 * @note This setting affects model selection and post-processing behavior.
 */
AISDK_EXTERN
void subject_segmentation_config_set_mode(SubjectSegmentationConfig *config, SubjectSegmentationMode mode);

/**
 * @brief Get the current segmentation mode from the configuration.
 *
 * @param config [in] The configuration handle. Must not be NULL.
 * @return The currently configured segmentation mode.
 */
AISDK_EXTERN
SubjectSegmentationMode subject_segmentation_config_get_mode(const SubjectSegmentationConfig *config);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_CORE_AI_VISION_SUBJECT_SEGMENTATION_CONFIG_H__
