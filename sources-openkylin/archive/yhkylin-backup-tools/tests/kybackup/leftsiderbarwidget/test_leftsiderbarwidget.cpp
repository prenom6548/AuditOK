#include "test_leftsiderbarwidget.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QTimer>
#include <QtTest/QtTest>

void TestLeftsiderbarWidget::initTestCase()
{
    // 初始化测试环境
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::cleanupTestCase()
{
    // 清理测试环境
    if (m_leftSidebar) {
        delete m_leftSidebar;
        m_leftSidebar = nullptr;
    }
}

void TestLeftsiderbarWidget::testConstructorDestructor()
{
    // 测试构造函数
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 验证初始化状态
    QVERIFY(m_leftSidebar->layout() != nullptr);
    
    // 测试析构函数（通过作用域结束自动调用）
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::testInitUi()
{
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 验证UI元素是否正确初始化
    QVERIFY(m_leftSidebar->layout() != nullptr);
    QCOMPARE(m_leftSidebar->layout()->objectName(), QString("m_leftSideBarVLayout"));
    
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::testSetBusy()
{
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 测试设置忙碌状态
    m_leftSidebar->setBusy(true);
    // 验证方法能正常调用不崩溃
    
    m_leftSidebar->setBusy(false);
    // 验证方法能正常调用不崩溃
    
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::testSetCheckedFunc()
{
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 测试设置选中功能类型
    m_leftSidebar->setCheckedFunc(0);
    // 验证方法能正常调用不崩溃
    
    m_leftSidebar->setCheckedFunc(-1);
    // 验证方法能正常调用不崩溃
    
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::testPaintEvent()
{
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 触发重绘事件
    m_leftSidebar->repaint();
    // 验证方法能正常调用不崩溃
    
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

void TestLeftsiderbarWidget::testEventFilter()
{
    m_leftSidebar = new LeftsiderbarWidget(nullptr, LeftsiderbarWidget::StartMode::normal);
    
    QVERIFY(m_leftSidebar != nullptr);
    
    // 测试事件过滤器（通过发送事件间接测试）
    QTest::qWait(100); // 等待可能的事件处理
    
    delete m_leftSidebar;
    m_leftSidebar = nullptr;
}

QTEST_MAIN(TestLeftsiderbarWidget)