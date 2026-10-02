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
#include "_subjectsegmentationconfig.h"

#include <jsoncpp/json/json.h>

SubjectSegmentationConfig *subject_segmentation_config_create() { return new _SubjectSegmentationConfig(); }

void subject_segmentation_config_destroy(SubjectSegmentationConfig **config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

void subject_segmentation_config_set_mode(SubjectSegmentationConfig *config, SubjectSegmentationMode mode) {
    if (config == nullptr) {
        return;
    }
    config->mode = mode;
}

SubjectSegmentationMode subject_segmentation_config_get_mode(const SubjectSegmentationConfig *config) {
    if (config == nullptr) {
        return static_cast<SubjectSegmentationMode>(-1);
    }
    return config->mode;
}

std::string subjectSegmentationConfigToJson(const _SubjectSegmentationConfig &modelConfig) {
    Json::Value root;
    root["mode"] = modelConfig.mode;
    Json::FastWriter writer;
    return writer.write(root);
}
