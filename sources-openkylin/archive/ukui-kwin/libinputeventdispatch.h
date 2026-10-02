#ifndef LIBINPUTEVENTDISPATCH_H
#define LIBINPUTEVENTDISPATCH_H

#include <ukui-kwin_export.h>
#include "libinput/connection.h"
#include "libinput.h"
#include "logind.h"

#define KWIN_BUILD_TESTING 1

namespace KWin {

class X11GlobalEventFilter;

typedef struct _LDevice
{
    _LDevice() {}
    _LDevice(int t_screen_id): screen_id(t_screen_id){}
    int screen_id;
}LDevice;

typedef struct _LTouchEvent{
    qint32 id;
    QPointF pos;
    quint32 time;
    LDevice device;
}LTouchEvent;

typedef struct _LPointerEvent{
    quint32 time;
}LPointerEvent;

typedef union _UEvent{
    _UEvent(){}
    LTouchEvent touch_event;
    LPointerEvent pointer_event;
}UEvent;

typedef struct _LEvent{
    libinput_event_type type;
    UEvent event;
}LEvent;

/*!
 * \author Yunpeng Zhu.
 * \brief The LibinputEventDispatch class
 */
class  UKUI_KWIN_EXPORT LibinputEventDispatch : public QObject
{
    Q_OBJECT
public:
    static LibinputEventDispatch* self();
    void init();
    void registerGlobalEventFilter(X11GlobalEventFilter *filter);
    void unregisterGlobalEventFilter(X11GlobalEventFilter *filter);
public slots:
    void dispatch(LEvent event);

private:
    LibinputEventDispatch();

    static LibinputEventDispatch *_self;
    LibInput::Connection *m_connection;
    KWin::LogindIntegration *m_logind;
    QList<X11GlobalEventFilter*> m_globalEventFilter;
};

}

#endif // LIBINPUTEVENTDISPATCH_H
