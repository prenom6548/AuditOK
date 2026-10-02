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

#ifndef _SUBJECTSEGMENTATIONRESULT_H_
#define _SUBJECTSEGMENTATIONRESULT_H_

#include <memory>
#include <string>
#include <vector>

#include "subjectsegmentationresult.h"

struct _SubjectSegmentationResult {
    std::unique_ptr<SegmentationMaskData> maskDataPtr;
    std::vector<unsigned char> imageData;
    int width;
    int height;
    std::string format;
    std::string requestId;
    std::vector<SubjectSegmentationPointGroup> pointGroups;
    int errorCode;
    std::string errorMessage;

    ~_SubjectSegmentationResult() {
        for (auto &pointGroup : pointGroups) {
            delete[] pointGroup.points;
        }
        if (maskDataPtr && maskDataPtr->mask_data) {
            delete[] maskDataPtr->mask_data;
            maskDataPtr->mask_data = nullptr;
        }
    }
};

#endif
