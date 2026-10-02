#ifndef X11GLOBALEVENTFILTER_H
#define X11GLOBALEVENTFILTER_H

#include <QSharedPointer>
#include <ukui-kwin_export.h>

#include "libinputeventdispatch.h"
#include "x11eventdispatch.h"

namespace KWin {
/*!
 * \author Yunpeng Zhu.
 * \brief 事件过滤器基类。
 * \details X11EventDispatch 会对继承该类的派生类分发全局事件。
 */
class UKUI_KWIN_EXPORT X11GlobalEventFilter
{

public:
    X11GlobalEventFilter();
    virtual ~X11GlobalEventFilter();

    virtual bool event(QSharedPointer<XEvent> event) = 0;
    virtual bool event(LEvent event) = 0;
};


}

#endif // X11GLOBALEVENTFILTER_H
