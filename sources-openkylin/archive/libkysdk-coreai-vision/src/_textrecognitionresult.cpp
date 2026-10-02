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

#include "_textrecognitionresult.h"

#include "textrecognitionresult.h"

// 获取一行中的文本内容
const char *text_line_get_value(TextLine *text_line) {
    if (text_line == nullptr) {
        return nullptr;
    }
    return text_line->text.c_str();
}

// 获取一行文本的角点位置信息（四个角的位置信息），point_number固定输出为4
PixelPoint *text_line_get_corner_points(TextLine *text_line, int *point_number) {
    if (text_line == nullptr) {
        return nullptr;
    }
    if (text_line->points.empty()) {
        return nullptr;
    }
    if (point_number == nullptr) {
        return nullptr;
    }
    *point_number = 4;
    return text_line->points.begin().base();
}

// 获取识别结果的整体文本信息，不带格式
const char *text_recognition_result_get_value(TextRecognitionResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->allText.c_str();
}
TextLine **text_recognition_result_get_text_lines(TextRecognitionResult *result, int *line_count) {
    if (result == nullptr) {
        return nullptr;
    }

    if (line_count == nullptr) {
        return nullptr;
    }
    *line_count = result->lines.size();
    return result->lines.data();
}
int text_recognition_result_get_error_code(TextRecognitionResult *result) {
    if (result == nullptr) {
        return -1;
    }
    return result->errorCode;
}

const char *text_recognition_result_get_error_message(TextRecognitionResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->errorMessage.c_str();
}

const char *text_recognition_result_get_request_id(TextRecognitionResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->requestId.c_str();
}
