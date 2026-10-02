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

#ifndef _TEXTRECOGNITIONSESSION_
#define _TEXTRECOGNITIONSESSION_

#include "_textrecognitionconfig.h"
#include "corevisionserviceproxy.h"
#include "textrecognition.h"
#include "textrecognitionresult.h"

namespace kyai {
namespace core {
namespace vision {

class _TextRecognitionSession {
public:
    _TextRecognitionSession();
    int init();
    void destroyTextRecognitionSession();
    void setRecognitionResultCallback(TextRecognitionResultCallback callback, void *userData);
    void recognizeTextFromImageFileAsync(const char *imageFile);
    void recognizeTextFromImageFileAsync(const char *requestId, const char *imageFile);
    void recognizeTextFromImageDataAsync(const char *imageData, unsigned int imageDataLength);
    void recognizeTextFromImageDataAsync(const char *requestId, const char *imageData, unsigned int imageDataLength);
    void setRecognizeTextModelConfig(const TextRecognitionModelConfig &config);

    void enableInternalEventLoop(bool enable);

private:
    bool initServer();
    int initSession();

private:
    CoreVisionServiceProxy serviceProxy_;
    int sessionId_{0};
    TextRecognitionModelConfig modelConfig_{};
};

}  // namespace vision
}  // namespace core
}  // namespace kyai

#endif