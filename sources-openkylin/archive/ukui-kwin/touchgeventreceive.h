#ifndef TOUCHGEVENTRECEIVE_H
#define TOUCHGEVENTRECEIVE_H

#include <QSharedPointer>
#include <ukui-kwin_export.h>
#include "input.h"
#include <x11globaleventfilter.h>
#include "X11/extensions/XInput2.h"

namespace KWin {

/*!
 * \author Yunpeng Zhu.
 * \brief 获取全局触摸事件，实现触摸动画。
 * \todo 目前触摸动画的支持走的是wayland下的触摸动画流程，目前仅供看效果，这部分流程需要重构。
 *       原因：kwin下input()类的本意是支持wayland，所以会调用大量wayland
 *       下的接口，一旦调用会崩溃。   wayland下对触摸动画和手势的支持不多，
 *       我建议趁着还没有陷进去，应该重新实现一套触摸动画去支持wayland和x
 */
class UKUI_KWIN_EXPORT TouchGEventReceive : public X11GlobalEventFilter
{
public:
    TouchGEventReceive();

    virtual bool event(QSharedPointer<XEvent> event) override;
    virtual bool event(LEvent event) override;
    void getScreenResolution(int device_id);
    void toRealData(XIRawEvent *event, QPoint &pos, int &detail);

private:
    int m_width, m_height;
    double touch_width, touch_height;
    bool m_cursorHidden;
};

}

#endif // TOUCHGEVENTRECEIVE_H
