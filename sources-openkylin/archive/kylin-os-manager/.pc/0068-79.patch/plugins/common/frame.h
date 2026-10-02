#ifndef __KYLIN_OS_MANAGER_PLUGIN_COMMON_FRAME_H__
#define __KYLIN_OS_MANAGER_PLUGIN_COMMON_FRAME_H__

#include <string>
#include <QRect>

class Frame {
    typedef void (FrameCallback)(const char *funcName, ...);

public:
    static void setFrameCallback(FrameCallback *frameCallback);
    static QRect geometry();

private:
    static FrameCallback *m_frameCallback;
};

#endif
