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

#ifndef _TEXTRCOGNITIONRESULT_H_
#define _TEXTRCOGNITIONRESULT_H_

#include <string>
#include <vector>

#include "textrecognitionresult.h"

struct _TextLine {
    std::string text;
    std::vector<PixelPoint> points;
};

struct _TextRecognitionResult {
    std::string allText;
    std::vector<_TextLine *> lines;
    int errorCode;
    std::string errorMessage;
    std::string requestId;

    ~_TextRecognitionResult() {
        for (auto line : lines) {
            delete line;
        }
    }
};

#endif