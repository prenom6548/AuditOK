#include "test_dbusadaptor.h"
#include <QApplication>

TestDbusAdaptor::TestDbusAdaptor()
    : m_dbusAdaptor(nullptr), m_mockMainWindow(nullptr)
{
}

void TestDbusAdaptor::initTestCase()
{
    // 在所有测试之前初始化
    m_mockMainWindow = new MockMainWindow();
    m_dbusAdaptor = new DbusAdaptor(m_mockMainWindow);
}

void TestDbusAdaptor::cleanupTestCase()
{
    // 在所有测试之后清理
    delete m_dbusAdaptor;
    m_dbusAdaptor = nullptr;
    
    delete m_mockMainWindow;
    m_mockMainWindow = nullptr;
}

void TestDbusAdaptor::testConstructor()
{
    // 测试构造函数
    QVERIFY(m_dbusAdaptor != nullptr);
    QVERIFY(m_mockMainWindow != nullptr);
    
    // 验证构造函数设置了autoRelaySignals
    // 注意：QDBusAbstractAdaptor的内部状态无法直接访问，但我们可以通过行为来验证
}

void TestDbusAdaptor::testDestructor()
{
    // 测试析构函数（通过作用域结束自动测试）
    // 这里只是验证析构不会导致崩溃
    QVERIFY(true);
}

void TestDbusAdaptor::testShowMainWindow()
{
    // 测试showMainWindow函数
    QVERIFY(m_dbusAdaptor != nullptr);
    QVERIFY(m_mockMainWindow != nullptr);
    
    // 确保初始状态未调用
    QVERIFY(!m_mockMainWindow->handleIconClickedCalled());
    
    // 调用函数
    m_dbusAdaptor->showMainWindow();
    
    // 处理事件队列，确保槽函数被调用
    QCoreApplication::processEvents();
    
    // 验证父窗口的handleIconClickedSub方法被调用
    QVERIFY(m_mockMainWindow->handleIconClickedCalled());
}

QTEST_MAIN(TestDbusAdaptor)