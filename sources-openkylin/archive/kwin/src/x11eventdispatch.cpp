#include "x11eventdispatch.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
// #include <kwinglobals.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <cstdlib>
#include <unistd.h>

#include "x11globaleventfilter.h"

#define HAVE_XI22 1

namespace KWin
{
X11GetInputEventThread::X11GetInputEventThread()
    : display(nullptr)
{
    init();
    QTimer::singleShot(0, this, [=]() {
        dispatch();
    });
}

X11GetInputEventThread::~X11GetInputEventThread()
{
    if (display)
        XCloseDisplay(display);
}

void X11GetInputEventThread::init()
{
    qRegisterMetaType<XEvent>("XEvent");
    s_thread = new QThread;
    s_thread->start();
    moveToThread(s_thread);
}

static void deleteXEvent(XEvent *ev)
{
    XGenericEventCookie *cookie = (XGenericEventCookie *)(&ev->xcookie);
    XFreeEventData(cookie->display, cookie);
    delete ev;
}

void X11GetInputEventThread::dispatch()
{
    int xi_opcode;
    XIEventMask mask[2];
    XIEventMask *m;
    Window win;
    int use_root = 0;
    int event, error;

    display = XOpenDisplay(0);

    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
        qDebug() << "X Input extension not available.";
        return;
    }

    win = DefaultRootWindow(display);

    /* Select for motion events */
    m = &mask[0];
    // m->deviceid = (deviceid == -1) ? XIAllDevices : deviceid;
    m->deviceid = XIAllDevices;
    m->mask_len = XIMaskLen(XI_LASTEVENT);
    m->mask = static_cast<unsigned char *>(calloc(m->mask_len, sizeof(char)));
    XISetMask(m->mask, XI_ButtonPress);
    XISetMask(m->mask, XI_ButtonRelease);
    XISetMask(m->mask, XI_KeyPress);
    XISetMask(m->mask, XI_KeyRelease);
    XISetMask(m->mask, XI_Motion);
    XISetMask(m->mask, XI_DeviceChanged);
    XISetMask(m->mask, XI_Enter);
    XISetMask(m->mask, XI_Leave);
    XISetMask(m->mask, XI_FocusIn);
    XISetMask(m->mask, XI_FocusOut);
#if HAVE_XI22
    XISetMask(m->mask, XI_TouchBegin);
    XISetMask(m->mask, XI_TouchUpdate);
    XISetMask(m->mask, XI_TouchEnd);
#endif
    if (m->deviceid == XIAllDevices)
        XISetMask(m->mask, XI_HierarchyChanged);
    XISetMask(m->mask, XI_PropertyEvent);

    m = &mask[1];
    // m->deviceid = (deviceid == -1) ? XIAllMasterDevices : deviceid;
    m->deviceid = XIAllMasterDevices;
    m->mask_len = XIMaskLen(XI_LASTEVENT);
    m->mask = static_cast<unsigned char *>(calloc(m->mask_len, sizeof(char)));
    XISetMask(m->mask, XI_RawKeyPress);
    XISetMask(m->mask, XI_RawKeyRelease);
    XISetMask(m->mask, XI_RawButtonPress);
    XISetMask(m->mask, XI_RawButtonRelease);
    XISetMask(m->mask, XI_RawMotion);
#if HAVE_XI22
    XISetMask(m->mask, XI_RawTouchBegin);
    XISetMask(m->mask, XI_RawTouchUpdate);
    XISetMask(m->mask, XI_RawTouchEnd);
#endif

    XISelectEvents(display, win, &mask[0], use_root ? 2 : 1);

    if (!use_root) {
        XISelectEvents(display, DefaultRootWindow(display), &mask[1], 1);
        XMapWindow(display, win);
    }
    XSync(display, False);
    free(mask[0].mask);
    free(mask[1].mask);

    qDebug() << "开始将输入设备的事件(XI2)进行分发...";
    while (true) {
        QSharedPointer<XEvent> ev = QSharedPointer<XEvent>(new XEvent(), deleteXEvent);
        XGenericEventCookie *cookie = (XGenericEventCookie *)&ev->xcookie;
        XNextEvent(display, (XEvent *)ev.data());
        XGetEventData(display, cookie);
        Q_EMIT sendEvent(ev);
        // XFreeEventData(display, cookie);
    }
}

X11EventDispatch *X11EventDispatch::_self = nullptr;

X11EventDispatch *X11EventDispatch::self()
{
    if (!_self)
        _self = new X11EventDispatch();
    return _self;
}

void X11EventDispatch::init()
{
    // 多线程监听x事件, 将事件分发
    if (!m_getEventThread)
        m_getEventThread = new X11GetInputEventThread;
    if (!m_getEventThread) {
        qWarning() << "new X11GetInputEventThread failed! 线程创建失败！";
        return;
    }
    qDebug() << "事件分发线程创建成功..";
    connect(m_getEventThread, &X11GetInputEventThread::sendEvent, this, &X11EventDispatch::dispatch, Qt::QueuedConnection);
}

void X11EventDispatch::dispatch(QSharedPointer<XEvent> event)
{
    for (auto filter : m_globalEventFilter) {
        if (true) /*! \todo 实现事件过滤 */
            filter->event(event);
    }
}

X11EventDispatch::X11EventDispatch()
    : m_getEventThread(nullptr)
{
    qRegisterMetaType<QSharedPointer<XEvent>>("QSharedPointer<XEvent>");
}

void X11EventDispatch::registerGlobalEventFilter(X11GlobalEventFilter *filter)
{
    m_globalEventFilter.append(filter);
}

void X11EventDispatch::unregisterGlobalEventFilter(X11GlobalEventFilter *filter)
{
    m_globalEventFilter.removeOne(filter);
}

}
