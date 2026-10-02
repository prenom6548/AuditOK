#ifndef __KYLIN_OS_MANAGER_PLUGIN_COMMON_FRAME_H__
#define __KYLIN_OS_MANAGER_PLUGIN_COMMON_FRAME_H__

#include <QRect>
#include "kom_application_interface.h"

class Frame {
    typedef void (FrameCallback)(const char *funcName, ...);

public:
    static void setFrameCallback(FrameCallback *frameCallback);
    static BaseData *GetBaseData();
    static QRect geometry();
    static void SwitchToProblemFeedback();
    static void SwtichToToolBox();
    static void SwitchToFaqSpecificType(char *type);
    static void RunFaultDiagnosis();
    static void RunJunkClean();
    static void ChangeBackButtonState(int state);
    static void ApplyKydoctor(char *mark, bool *ok);
    static void ReleaseKydoctor(char *mark);

private:
    static FrameCallback *m_frameCallback;
};

#endif
