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
#ifndef _SUBJECTSEGMENTATIONSESSION_
#define _SUBJECTSEGMENTATIONSESSION_

#include "_subjectsegmentationconfig.h"
#include "corevisionserviceproxy.h"
#include "subjectsegmentation.h"
#include "subjectsegmentationresult.h"

namespace kyai {
namespace core {
namespace vision {

class _SubjectSegmentationSession {
public:
    _SubjectSegmentationSession();
    void init(SubjectSegmentationInitCallback callback, void *userData);
    void destroySubjectSegmentationSession();
    void setSubjectSegmentationResultCallback(SubjectSegmentationResultCallback callback, void *userData);
    void subjectSegmentationFromImageFileAsync(const char *requestId, const char *imageFile);
    void subjectSegmentationFromImageDataAsync(const char *requestId, const char *data, unsigned int dataLength);
    void setSubjectSegmentationModelConfig(const SubjectSegmentationConfig &config);
    void enableInternalEventLoop(bool enable);

private:
    bool initServer();
    void initSession();
    static void onSessionInitialized(int sessionId, int errorCode, void *userData);

private:
    CoreVisionServiceProxy serviceProxy_;
    int sessionId_{0};
    SubjectSegmentationConfig modelConfig_{};
    SubjectSegmentationInitCallback initCallback_{nullptr};
    void *initUserData_{nullptr};
};

}  // namespace vision
}  // namespace core
}  // namespace kyai

#endif
