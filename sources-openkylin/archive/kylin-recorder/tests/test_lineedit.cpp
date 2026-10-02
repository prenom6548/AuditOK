#include "test_lineedit.h"
#include <QMouseEvent>
#include <QApplication>
#include <QTest>

TestLineEdit::TestLineEdit(QObject *parent)
    : QObject(parent)
{
}

void TestLineEdit::test_construct()
{
    // 测试构造函数
    LineEdit* lineEditPtr = new LineEdit(nullptr);
    QVERIFY(lineEditPtr != nullptr);
    delete lineEditPtr;
}

void TestLineEdit::test_mousePressEvent()
{
    // 测试鼠标按下事件
    LineEdit* lineEditPtr = new LineEdit(nullptr);
    QVERIFY(lineEditPtr != nullptr);
    
    // 创建鼠标事件
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(15, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    // 连接信号和槽
    bool clickedEmitted = false;
    connect(lineEditPtr, &LineEdit::clicked, [&clickedEmitted]() {
        clickedEmitted = true;
    });
    
    // 触发鼠标按下事件
    qApp->sendEvent(lineEditPtr, &pressEvent);
    
    // 验证信号是否发出
    QVERIFY(clickedEmitted);
    
    // 测试右键点击，不应该发出信号
    clickedEmitted = false;
    QMouseEvent rightClickEvent(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(15, 15), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    qApp->sendEvent(lineEditPtr, &rightClickEvent);
    
    // 验证信号没有发出
    QVERIFY(!clickedEmitted);
    
    delete lineEditPtr;
}