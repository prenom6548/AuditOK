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

#include "_speechrecognitionresult.h"

#include <map>

#include "result.h"

const char *speech_recognition_result_get_request_id(SpeechRecognitionResult *result) {
    if (result == nullptr) {
        return "";
    }
    return result->requestId.c_str();
}

SpeechResultReason speech_recognition_result_get_reason(SpeechRecognitionResult *result) { return result->reason; }

const char *speech_recognition_result_get_reason_str(SpeechRecognitionResult *result) {
    std::map<SpeechResultReason, const char *> map{
        // {NO_MATCH, "NO_MATCH"},
        {SPEECH_ERROR_OCCURRED, "SPEECH_ERROR_OCCURRED"},
        {SPEECH_RECOGNITION_STARTED, "SPEECH_RECOGNITION_STARTED"},
        {SPEECH_RECOGNIZING, "SPEECH_RECOGNIZING"},
        {SPEECH_RECOGNIZED, "SPEECH_RECOGNIZED"},
        {SPEECH_RECOGNITION_COMPLETED, "SPEECH_RECOGNITION_COMPLETED"},
        {SPEECH_SYNTHESIS_STARTED, "SPEECH_SYNTHESIS_STARTED"},
        {SPEECH_SYNTHESIZING, "SPEECH_SYNTHESIZING"},
        {SPEECH_SYNTHESIS_COMPLETED, "SPEECH_SYNTHESIS_COMPLETED"},
    };
    if (map.find(result->reason) != map.end()) {
        return map[result->reason];
    } else {
        return nullptr;
    }
}

const char *speech_recognition_result_get_text(SpeechRecognitionResult *result) { return result->text.c_str(); }

int speech_recognition_result_get_speaker_id(SpeechRecognitionResult *result) { return result->speakerId; }

int speech_recognition_result_get_error_code(SpeechRecognitionResult *result) { return result->errorCode; }

const char *speech_recognition_result_get_error_message(SpeechRecognitionResult *result) {
    return result->errorMessage.c_str();
}
