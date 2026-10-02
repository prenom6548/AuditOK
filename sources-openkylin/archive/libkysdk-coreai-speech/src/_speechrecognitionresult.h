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

#ifndef _SPEECHRCOGNITIONRESULT_H_
#define _SPEECHRCOGNITIONRESULT_H_

#include <string>

#include "result.h"

struct _SpeechRecognitionResult {
    std::string requestId;
    SpeechResultReason reason;
    std::string text;
    int speakerId;
    int errorCode;
    std::string errorMessage;
};

const char *speech_recognition_result_get_reason_str(SpeechRecognitionResult *result);

#endif