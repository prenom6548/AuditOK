#ifndef TEST_LINE_H
#define TEST_LINE_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QLineEdit>
#include <QMouseEvent>
#include <QApplication>
#include "../../../../UIControl/src/line.h"

// 创建一个友元类来访问protected成员
class TestableLine : public line
{
public:
    TestableLine() : line() {}
    
    // 提供公共接口来调用protected方法
    void callMousePressEvent(QMouseEvent *event) {
        mousePressEvent(event);
    }
    
    void callMouseReleaseEvent(QMouseEvent *event) {
        mouseReleaseEvent(event);
    }
    
    void callMouseMoveEvent(QMouseEvent *event) {
        mouseMoveEvent(event);
    }
    
    void callMouseDoubleClickEvent(QMouseEvent *event) {
        mouseDoubleClickEvent(event);
    }
    
    void callLeaveEvent(QEvent *event) {
        leaveEvent(event);
    }
};

class TestLine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testMousePressEvent();
    void testMouseReleaseEvent();
    void testMouseMoveEvent();
    void testMouseDoubleClickEvent();
    void testLeaveEvent();
};

#endif // TEST_LINE_H