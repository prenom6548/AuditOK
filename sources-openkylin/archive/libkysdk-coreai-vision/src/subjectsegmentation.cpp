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
#include "subjectsegmentation.h"

#include "_subjectsegmentationsession.h"

SubjectSegmentationSession *subject_segmentation_create_session() {
    auto *session = new kyai::core::vision::_SubjectSegmentationSession();
    return (SubjectSegmentationSession *)session;
}
void subject_segmentation_destroy_session(SubjectSegmentationSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)*session)->destroySubjectSegmentationSession();
    delete (kyai::core::vision::_SubjectSegmentationSession *)*session;
    *session = nullptr;
}

void subject_segmentation_init_session_async(SubjectSegmentationSession *session,
                                             SubjectSegmentationInitCallback callback, void *user_data) {
    if (session == nullptr) {
        if (callback) {
            callback(AISDK_INVALID_SESSION, user_data);
        }
        return;
    }

    ((kyai::core::vision::_SubjectSegmentationSession *)session)->init(callback, user_data);
}

void subject_segmentation_result_set_callback(SubjectSegmentationSession *session,
                                              SubjectSegmentationResultCallback callback, void *user_data) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)session)
        ->setSubjectSegmentationResultCallback(callback, user_data);
}

void subject_segmentation_set_config(SubjectSegmentationSession *session, SubjectSegmentationConfig *config) {
    if (session == nullptr || config == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)session)->setSubjectSegmentationModelConfig(*config);
}
void subject_segmentation_run_from_image_file_async(SubjectSegmentationSession *session, const char *request_id,
                                                    const char *image_file) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)session)
        ->subjectSegmentationFromImageFileAsync(request_id, image_file);
}

void subject_segmentation_run_from_image_data_async(SubjectSegmentationSession *session, const char *request_id,
                                                    const char *image_data, unsigned int image_data_length) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)session)
        ->subjectSegmentationFromImageDataAsync(request_id, image_data, image_data_length);
}

void subject_segmentation_enable_internal_event_loop(SubjectSegmentationSession *session, bool enable) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_SubjectSegmentationSession *)session)->enableInternalEventLoop(enable);
}
