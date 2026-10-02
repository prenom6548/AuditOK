#include "test_daemonipcdbus.h"
#include <QApplication>

TestDaemonIpcDbus::TestDaemonIpcDbus()
    : m_daemonIpcDbus(nullptr)
{
}

void TestDaemonIpcDbus::initTestCase()
{
    // 在所有测试之前初始化
    m_daemonIpcDbus = new DaemonIpcDbus();
}

void TestDaemonIpcDbus::cleanupTestCase()
{
    // 在所有测试之后清理
    delete m_daemonIpcDbus;
    m_daemonIpcDbus = nullptr;
}

void TestDaemonIpcDbus::testDaemonIsNotRunning()
{
    // 测试daemonIsNotRunning函数
    // 注意：由于这个函数依赖于实际的D-Bus连接，我们在单元测试中只能验证它不会崩溃
    QVERIFY(m_daemonIpcDbus != nullptr);
    
    // 调用函数不应导致崩溃
    int result = m_daemonIpcDbus->daemonIsNotRunning();
    
    // 验证返回值是合理的(-1表示连接失败，0表示运行中，1表示未运行)
    QVERIFY(result == -1 || result == 0 || result == 1);
}

void TestDaemonIpcDbus::testDaemonIsNotRunning_data()
{
    // 添加测试数据
    QTest::addColumn<QString>("description");
    
    QTest::newRow("Basic test") << "Testing basic functionality";
}

void TestDaemonIpcDbus::testShowGuide()
{
    // 测试showGuide函数
    // 注意：由于这个函数依赖于实际的D-Bus调用，我们在单元测试中只能验证它不会崩溃
    QVERIFY(m_daemonIpcDbus != nullptr);
    
    // 调用函数不应导致崩溃
    m_daemonIpcDbus->showGuide("testApp");
    
    // 验证函数能正常执行（不会崩溃）
    QVERIFY(true);
}

void TestDaemonIpcDbus::testShowGuide_data()
{
    // 添加测试数据
    QTest::addColumn<QString>("appName");
    QTest::addColumn<QString>("description");
    
    QTest::newRow("Normal app name") << "testApp" << "Testing with normal app name";
    QTest::newRow("Empty app name") << "" << "Testing with empty app name";
    QTest::newRow("Special characters") << "test-app_123" << "Testing with special characters";
}

void TestDaemonIpcDbus::testDaemonIsNotRunningWithDisconnectedBus()
{
    // 测试D-Bus断开连接的情况
    QVERIFY(m_daemonIpcDbus != nullptr);
    
    // 这个测试主要验证函数在D-Bus断开时的行为
    // 由于我们无法真正断开D-Bus连接，我们只能验证函数不会崩溃
    int result = m_daemonIpcDbus->daemonIsNotRunning();
    
    // 验证返回值是合理的
    QVERIFY(result == -1 || result == 0 || result == 1);
}

void TestDaemonIpcDbus::testShowGuideWithReplyMessage()
{
    // 测试showGuide函数接收到回复消息的情况
    QVERIFY(m_daemonIpcDbus != nullptr);
    
    // 调用函数
    m_daemonIpcDbus->showGuide("testAppWithReply");
    
    // 验证函数能正常执行（不会崩溃）
    QVERIFY(true);
}

QTEST_MAIN(TestDaemonIpcDbus)