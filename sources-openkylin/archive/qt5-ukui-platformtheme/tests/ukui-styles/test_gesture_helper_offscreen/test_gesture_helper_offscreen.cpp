/*
 * gesture-helper.cpp / window-manager.cpp — offscreen event injection (no show/exec).
 */

#include <QtTest>
#include <QWidget>
#include <QWheelEvent>
#include <QTouchEvent>
#include <QMouseEvent>
#include <QWindow>

#include "gesture-helper.h"
#include "window-manager.h"

class TestGestureHelperOffscreen : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGestureHelperEvents();
    void testGestureHelperTouch();
    void testWindowManagerBasics();
    void testGestureTouchLifecycle();
    void testWindowManagerMouseEvents();
    void testWindowManagerExtendedEvents();
};

void TestGestureHelperOffscreen::testGestureHelperEvents()
{
    GestureHelper gh;
    QWidget w;
    gh.registerWidget(&w);

    QWheelEvent wheel(QPointF(4, 4), 0, Qt::NoButton, Qt::NoModifier, Qt::Vertical);
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QKeyEvent keyRelease(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier);
    for (QEvent *ev : {static_cast<QEvent *>(&wheel), static_cast<QEvent *>(&keyPress), static_cast<QEvent *>(&keyRelease)}) {
        QVERIFY(!gh.eventFilter(&w, ev));
    }
    gh.unregisterWidget(&w);
}

void TestGestureHelperOffscreen::testGestureHelperTouch()
{
    GestureHelper gh;
    QWidget w;
    QTouchDevice *device = new QTouchDevice();
    device->setType(QTouchDevice::TouchScreen);
    device->setCapabilities(QTouchDevice::Position);

    QList<QTouchEvent::TouchPoint> pts;
    QTouchEvent::TouchPoint p0;
    p0.setId(0);
    p0.setPos(QPointF(10, 10));
    pts.append(p0);

    QTouchEvent begin(QEvent::TouchBegin, device, Qt::NoModifier, Qt::TouchPointPressed, pts);
    QVERIFY(!gh.eventFilter(&w, &begin));

    QTouchEvent::TouchPoint p1;
    p1.setId(1);
    p1.setPos(QPointF(30, 30));
    pts.append(p1);
    QTouchEvent begin2(QEvent::TouchBegin, device, Qt::NoModifier, Qt::TouchPointPressed, pts);
    (void)gh.eventFilter(&w, &begin2);

    QTouchEvent end(QEvent::TouchEnd, device, Qt::NoModifier, Qt::TouchPointReleased, pts);
    (void)gh.eventFilter(&w, &end);
    delete device;
}

void TestGestureHelperOffscreen::testWindowManagerBasics()
{
    WindowManager wm;
    QWidget w;
    w.setWindowFlags(Qt::Window);
    wm.registerWidget(&w);
    wm.unregisterWidget(&w);
}

void TestGestureHelperOffscreen::testGestureTouchLifecycle()
{
    GestureHelper gh;
    QWidget w;
    w.setWindowFlags(Qt::Window);
    w.resize(200, 200);
    gh.registerWidget(&w);

    QTouchDevice *device = new QTouchDevice();
    device->setType(QTouchDevice::TouchScreen);
    device->setCapabilities(QTouchDevice::Position);

    QList<QTouchEvent::TouchPoint> one;
    QTouchEvent::TouchPoint p0;
    p0.setId(0);
    p0.setPos(QPointF(5, 5));
    one.append(p0);

    QTouchEvent touchUpdate(QEvent::TouchUpdate, device, Qt::NoModifier, Qt::TouchPointMoved, one);
    (void)gh.eventFilter(&w, &touchUpdate);

    QTouchEvent touchCancel(QEvent::TouchCancel, device, Qt::NoModifier, Qt::TouchPointReleased, one);
    (void)gh.eventFilter(&w, &touchCancel);

    QEvent activation(QEvent::ActivationChange);
    (void)gh.eventFilter(&w, &activation);

    delete device;
    gh.unregisterWidget(&w);
}

void TestGestureHelperOffscreen::testWindowManagerMouseEvents()
{
    WindowManager wm;
    QWidget w;
    w.setWindowFlags(Qt::Window);
    w.resize(240, 180);
    wm.registerWidget(&w);

    QMouseEvent press(QEvent::MouseButtonPress, QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove, QPoint(40, 40), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPoint(40, 40), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &press);
    (void)wm.eventFilter(&w, &move);
    (void)wm.eventFilter(&w, &release);

    wm.unregisterWidget(&w);
}

void TestGestureHelperOffscreen::testWindowManagerExtendedEvents()
{
    WindowManager wm;
    QWidget w;
    w.setWindowFlags(Qt::Window);
    w.resize(320, 200);
    w.setProperty("useStyleWindowManager", false);
    wm.registerWidget(&w);

    QMouseEvent press(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(!wm.eventFilter(&w, &press));

    w.setProperty("useStyleWindowManager", true);
    wm.registerWidget(&w);

    w.setCursor(Qt::SizeAllCursor);
    QMouseEvent pressTitle(QEvent::MouseButtonPress, QPoint(30, 30), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &pressTitle);

    w.setCursor(Qt::ArrowCursor);
    QMouseEvent pressArrow(QEvent::MouseButtonPress, QPoint(30, 30), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &pressArrow);

    QMouseEvent tinyMove(QEvent::MouseMove, QPoint(31, 31), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &tinyMove);

    QMouseEvent bigMove(QEvent::MouseMove, QPoint(80, 80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &bigMove);

    QMouseEvent release(QEvent::MouseButtonRelease, QPoint(80, 80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    (void)wm.eventFilter(&w, &release);

    QEvent moveEv(QEvent::Move);
    (void)wm.eventFilter(&w, &moveEv);

    wm.unregisterWidget(&w);
}

QTEST_MAIN(TestGestureHelperOffscreen)
#include "test_gesture_helper_offscreen.moc"
