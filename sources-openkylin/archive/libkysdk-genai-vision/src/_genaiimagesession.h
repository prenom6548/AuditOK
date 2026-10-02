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

#ifndef _GENAIIMAGE_
#define _GENAIIMAGE_

#include <memory>

#include "_imageconfig.h"
#include "error.h"
#include "genaivisionserviceproxy.h"
#include "imageresult.h"

namespace kylin {
namespace genai {
namespace vision {

class _GenAiImageSession {
public:
    _GenAiImageSession();
    int init();

    void setImageConfig(const ImageConfig &config);

    void setImageResultCallback(VisionImageResultCallback callback, void *userData);

    const char **getSupportedImageStyle(int *number);
    const ImageSize *getSupportedImageSize(int *number);
    const int getSupportedImageNumber();
    void generateImageAsync(const std::string &prompt);

    int generateImageAsyncWithRequestId(const std::string &prompt, const std::string &requestId);
    int stopGeneration(const char *requestId);

#ifdef UNIT_TEST
    void setServiceProxyForTest(std::unique_ptr<IGenAiVisionServiceProxy> proxy);
#endif

    void enableInternalEventLoop(bool enable);

private:
    void bindReconnectedCallback();
    bool initServer();
    int initSession();

private:
    ImageConfig imageConfig_{};

    std::unique_ptr<IGenAiVisionServiceProxy> serviceProxy_;

    int sessionId_{0};
};

}  // namespace vision
}  // namespace genai
}  // namespace kylin

#endif
