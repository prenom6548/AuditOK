/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef BAIDU_RESPONSE_PROCESSOR_H
#define BAIDU_RESPONSE_PROCESSOR_H

#include <string>
#include <vector>

enum class ProcessResult { SUCCEEDED, FAILED, CONTINUED };

enum class ApproveConclusion { UNKNOWN = -1, PASS, REVIEW, BLOCK };

struct ImageData {
    unsigned int index_ = 0;
    std::vector<unsigned char> data_;
    ApproveConclusion conclusion_ = ApproveConclusion::UNKNOWN;
};

using ImageDataList = std::vector<ImageData>;

class BaiduResponseProcessor {
public:
    static ProcessResult process(const std::string &json, ImageDataList &imageDataList, bool &isBlocked,
                                 std::string &errorMessage);
};

#endif
