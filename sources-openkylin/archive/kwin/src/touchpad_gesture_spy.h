#pragma once

#include "input_event_spy.h"
#include <QFileSystemWatcher>
#include <QList>
#include <QObject>
#include <kwinglobals.h>

namespace KWin
{

class TouchpadGestureSpy;
class GestureAndActionBase;
class TouchpadGestureRecognizer;
class TouchpadTapGesture;
class TouchpadSwipeGesture;
class GlobalGesture;
class TouchpadSwipeSequenceGesture;

class TouchpadGestureSpy : public QObject, public InputEventSpy
{
    Q_OBJECT
public:
    ~TouchpadGestureSpy();

    void registerTouchpadGesture(GestureAndActionBase *action);
    void updateTouchpadGesture();
    void swipeGestureBegin(int fingerCount, quint32 time) override;
    void swipeGestureUpdate(const QSizeF &delta, quint32 time) override;
    void swipeGestureEnd(quint32 time) override;
    void swipeGestureCancelled(quint32 time) override;

    void tapGestureBegin(int fingerCount, quint32 time) override;

private:
    TouchpadGestureRecognizer *m_touchpadReg;
    QFileSystemWatcher *m_gestureWatcher;
    QList<GestureAndActionBase *> m_touchpadAction;
    GestureAndActionBase *m_gestureTouchpadAndAction;
    GestureAndActionBase *m_newActionKey;
    TouchpadTapGesture *touchpadTap;
    TouchpadSwipeGesture *touchpadSwipe;
    // 测试连续滑动手势
    TouchpadSwipeSequenceGesture *m_touchpadSwipe2;
    QVector<GlobalGesture *> m_gesture;

    KWIN_SINGLETON(TouchpadGestureSpy)
};

}
