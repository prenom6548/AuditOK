#include "test_clipbutton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QTest>

TestClipButton::TestClipButton(QObject *parent)
    : QObject(parent)
    , clipButton(nullptr)
{
}

void TestClipButton::test_construct()
{
    // 测试构造函数
    clipButton = new ClipButton(":/img/标记.png", nullptr);
    QVERIFY(clipButton != nullptr);
    delete clipButton;
    clipButton = nullptr;
}

void TestClipButton::test_paintEvent()
{
    // 测试绘制事件
    clipButton = new ClipButton(":/img/标记.png", nullptr);
    QVERIFY(clipButton != nullptr);
    
    // 触发绘制事件
    clipButton->show();
    QTest::qWait(100); // 等待绘制完成
    
    delete clipButton;
    clipButton = nullptr;
}

void TestClipButton::test_mousePressEvent()
{
    // 测试鼠标按下事件
    clipButton = new ClipButton(":/img/标记.png", nullptr);
    QVERIFY(clipButton != nullptr);
    
    // 设置对象名称以便测试不同的分支
    clipButton->setObjectName("leftBtn");
    
    // 创建父窗口
    QWidget parentWidget;
    clipButton->setParent(&parentWidget);
    parentWidget.resize(100, 100);
    clipButton->move(10, 10);
    
    // 创建鼠标事件
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(15, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    // 触发鼠标按下事件
    qApp->sendEvent(clipButton, &pressEvent);
    
    delete clipButton;
    clipButton = nullptr;
}

void TestClipButton::test_mouseReleaseEvent()
{
    // 测试鼠标释放事件
    clipButton = new ClipButton(":/img/标记.png", nullptr);
    QVERIFY(clipButton != nullptr);
    
    // 创建父窗口
    QWidget parentWidget;
    clipButton->setParent(&parentWidget);
    parentWidget.resize(100, 100);
    clipButton->move(10, 10);
    
    // 设置对象名称以便测试不同的分支
    clipButton->setObjectName("leftBtn");
    
    // 创建鼠标事件
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(15, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    // 触发鼠标释放事件
    qApp->sendEvent(clipButton, &releaseEvent);
    
    // 测试右按钮
    clipButton->setObjectName("rightBtn");
    qApp->sendEvent(clipButton, &releaseEvent);
    
    delete clipButton;
    clipButton = nullptr;
}

void TestClipButton::test_mouseMoveEvent()
{
    // 测试鼠标移动事件
    clipButton = new ClipButton(":/img/标记.png", nullptr);
    QVERIFY(clipButton != nullptr);
    
    // 创建父窗口
    QWidget parentWidget;
    clipButton->setParent(&parentWidget);
    parentWidget.resize(100, 100);
    clipButton->move(10, 10);
    
    // 设置对象名称以便测试不同的分支
    clipButton->setObjectName("leftBtn");
    
    // 创建鼠标事件
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(15, 15), QPointF(25, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    // 触发鼠标移动事件
    qApp->sendEvent(clipButton, &moveEvent);
    
    // 测试右按钮
    clipButton->setObjectName("rightBtn");
    qApp->sendEvent(clipButton, &moveEvent);
    
    // 测试边界情况
    clipButton->move(-5, 10);
    qApp->sendEvent(clipButton, &moveEvent);
    
    clipButton->move(90, 10);
    qApp->sendEvent(clipButton, &moveEvent);
    
    delete clipButton;
    clipButton = nullptr;
}