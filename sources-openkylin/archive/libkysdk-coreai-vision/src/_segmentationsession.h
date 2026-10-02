/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H
#define KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H

#include "corevisionserviceproxy.h"
#include "segmentation.h"

#include <atomic>

namespace kyai::core::vision {

class _SegmentationSession {
public:
    _SegmentationSession();

    ~_SegmentationSession();

    void initAsync(const SegmentationInitCallback &callback, void *userData);

    void destroy();

    void setSegmentationResultCallback(const SegmentationResultCallback &callback, void *userData);

    void encodeImageFileAsync(const std::string &imageFile, const SegmentationEncodeCallback &callback, void *userData);

    void encodeImageDataAsync(const std::vector<uint8_t> &imageData, const SegmentationEncodeCallback &callback,
                              void *userData);
    int clearCache();

    void runAsync(const std::vector<PixelPoint> &points, uint32_t minArea);

    void enableInternalEventLoop(bool enable);

private:
    bool initServer();

    void initSessionAsync();

    static void onSessionInited(int sessionId, int errorCode, void *userData);

private:
    CoreVisionServiceProxy proxy_;
    std::atomic_int sessionId_{0};
    SegmentationInitCallback initCallback_{nullptr};
    void *initUserData_{nullptr};
};

}  // namespace kyai::core::vision

#endif  // KYSDK_COREAI_VISION_SRC_SEGMENTATION_SESSION_H
