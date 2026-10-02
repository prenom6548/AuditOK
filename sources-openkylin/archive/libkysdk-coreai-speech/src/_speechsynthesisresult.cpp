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

#include "_speechsynthesisresult.h"

#include "result.h"

const char *speech_synthesis_result_get_request_id(SpeechSynthesisResult *result) {
    if (result == nullptr) {
        return "";
    }
    return result->requestId.c_str();
}

SpeechResultReason speech_synthesis_result_get_reason(SpeechSynthesisResult *result) { return result->reason; }

const char *speech_synthesis_result_get_audio_format(SpeechSynthesisResult *result) {
    return result->audioFormat.c_str();
}

int speech_synthesis_result_get_audio_rate(SpeechSynthesisResult *result) { return result->audioRate; }

int speech_synthesis_result_get_audio_channel(SpeechSynthesisResult *result) { return result->audioChannel; }

const uint8_t *speech_synthesis_result_get_data(SpeechSynthesisResult *result, uint32_t *data_length) {
    if (result == nullptr) {
        return nullptr;
    }
    *data_length = result->audioData.size();
    return result->audioData.data();
}

int speech_synthesis_result_get_error_code(SpeechSynthesisResult *result) { return result->errorCode; }

const char *speech_synthesis_result_get_error_message(SpeechSynthesisResult *result) {
    return result->errorMessage.c_str();
}
