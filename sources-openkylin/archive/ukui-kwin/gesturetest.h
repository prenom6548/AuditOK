#ifndef GESTURETEST_H
#define GESTURETEST_H

#include <QObject>

#include <gestures.h>

namespace KWin {

/*!
 * \author Yunpeng Zhu.
 * \brief 测试手势识别器的识别效果
 */

class GestureTest : public QObject
{
    Q_OBJECT
public:
    static GestureTest* self(QObject *parent = nullptr);

    GestureRecognizer* gestureRecognizer(){
        return m_gestureRecognizer;
    }

    void startGesture(int id, QPointF pos);
    void updateGesture(int id, QPointF pos);
    void endGesture(int id, QPointF pos);

private:
    GestureTest(QObject *parent = nullptr);
    void init();
    void printSwipeGesture(SwipeGesture *gesture);
    void printPinchGesture(PinchGesture *gesture);
    void printLongPressGesture(LongPressGesture *gesture);
    QString dirToStr(Gesture::GestureDirection direction);

    static GestureTest *_self;
    GestureRecognizer *m_gestureRecognizer;

    SwipeGesture *m_swipe[5 + 1][4 + 1];
    PinchGesture *m_pinch[4 + 2][2 + 1];
    LongPressGesture *m_longPress[5 + 1];
};

}

#endif // GESTURETEST_H
