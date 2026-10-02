#include "test_qroundprogressbar.h"
#include <QApplication>
#include <QPaintEvent>

TestQRoundProgressBar::TestQRoundProgressBar()
    : m_progressBar(nullptr)
{
}

void TestQRoundProgressBar::initTestCase()
{
    // 在所有测试之前初始化
    m_progressBar = new QRoundProgressBar();
}

void TestQRoundProgressBar::cleanupTestCase()
{
    // 在所有测试之后清理
    delete m_progressBar;
    m_progressBar = nullptr;
}

void TestQRoundProgressBar::testConstructor()
{
    // 测试构造函数
    QVERIFY(m_progressBar != nullptr);
    
    // 验证初始状态
    QVERIFY(m_progressBar->menu != nullptr);
}

void TestQRoundProgressBar::testSetPersent_data()
{
    QTest::addColumn<int>("percent");
    
    // 测试正常值
    QTest::newRow("zero_percent") << 0;
    QTest::newRow("fifty_percent") << 50;
    QTest::newRow("hundred_percent") << 100;
    
    // 测试边界值
    QTest::newRow("negative_percent") << -10;  // 实现中没有限制范围
    QTest::newRow("over_hundred_percent") << 150;  // 实现中没有限制范围
}

void TestQRoundProgressBar::testSetPersent()
{
    QFETCH(int, percent);
    
    // 调用函数不应导致崩溃
    m_progressBar->setPersent(percent);
    
    // 验证函数能正常执行（不会崩溃）
    QVERIFY(true);
}

QTEST_MAIN(TestQRoundProgressBar)