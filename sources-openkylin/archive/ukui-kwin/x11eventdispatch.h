#ifndef X11EVENTDISPATCH_H
#define X11EVENTDISPATCH_H
#include <QObject>
#include <QSharedPointer>
#include <ukui-kwin_export.h>

#include <X11/Xlib.h>
#include <unistd.h>
#include <cstdlib>

namespace KWin {

class X11GlobalEventFilter;

class X11GetInputEventThread : public QObject
{
    Q_OBJECT
public:
    X11GetInputEventThread();
    ~X11GetInputEventThread();
    void init();
    void dispatch();

Q_SIGNALS:
    void sendEvent(QSharedPointer<XEvent> ev);

private:
    QThread *s_thread;
    Display* display;
};

/*!
 * \author Yunpeng Zhu.
 * \brief 负责全局事件的获取和事件的分发。
 * \details 通过Xi2协议获取全局事件。事件的获取由子线程X11GetInputEventThread完成。
 */
class UKUI_KWIN_EXPORT X11EventDispatch : public QObject
{
    Q_OBJECT
public:
    static X11EventDispatch* self();
    void init();
    void registerGlobalEventFilter(X11GlobalEventFilter* filter);
    void unregisterGlobalEventFilter(X11GlobalEventFilter* filter);

public Q_SLOTS:
    void dispatch(QSharedPointer<XEvent> event);

private:
    X11EventDispatch();

    static X11EventDispatch* _self;
    X11GetInputEventThread *m_getEventThread;
    QList<X11GlobalEventFilter*> m_globalEventFilter;
};

}
#endif // X11EVENTDISPATCH_H
