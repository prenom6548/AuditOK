#ifndef X11GLOBALEVENTFILTER_H
#define X11GLOBALEVENTFILTER_H

#include <QSharedPointer>
#include <kwin_export.h>

class _XDisplay;
typedef _XDisplay Display;

union _XEvent;
typedef _XEvent XEvent;

namespace KWin
{
/*!
 * \author Yunpeng Zhu.
 * \brief 事件过滤器基类。
 * \details X11EventDispatch 会对继承该类的派生类分发全局事件。
 */
class KWIN_EXPORT X11GlobalEventFilter
{

public:
    X11GlobalEventFilter();
    virtual ~X11GlobalEventFilter();

    virtual bool event(QSharedPointer<XEvent> event) = 0;
};

}

#endif // X11GLOBALEVENTFILTER_H
