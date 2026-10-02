#include "test_global.h"
#include "../../../../../UIControl/src/include/global.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QtTest/QtTest>

void TestGlobal::initTestCase()
{
    // 初始化测试环境
}

void TestGlobal::cleanupTestCase()
{
    // 清理测试环境
}

void TestGlobal::testGlobalInitWithWayland()
{
    // 保存原始环境变量
    QByteArray originalSessionType = qgetenv("XDG_SESSION_TYPE");
    
    // 设置环境变量为wayland
    qputenv("XDG_SESSION_TYPE", "wayland");
    
    // 调用函数
    Global::global_init();
    
    // 验证结果
    QVERIFY(Global::isWayland);
    QCOMPARE(qgetenv("QT_QPA_PLATFORM"), QByteArray("wayland"));
    
    // 恢复原始环境变量
    if (!originalSessionType.isEmpty()) {
        qputenv("XDG_SESSION_TYPE", originalSessionType);
    } else {
        qunsetenv("XDG_SESSION_TYPE");
    }
}

void TestGlobal::testGlobalInitWithoutWayland()
{
    // 保存原始环境变量
    QByteArray originalSessionType = qgetenv("XDG_SESSION_TYPE");
    
    // 设置环境变量为非wayland
    qputenv("XDG_SESSION_TYPE", "x11");
    
    // 调用函数
    Global::global_init();
    
    // 验证结果
    QVERIFY(!Global::isWayland);
    
    // 恢复原始环境变量
    if (!originalSessionType.isEmpty()) {
        qputenv("XDG_SESSION_TYPE", originalSessionType);
    } else {
        qunsetenv("XDG_SESSION_TYPE");
    }
}

void TestGlobal::testGlobalEnd()
{
    // 调用函数
    Global::global_end();
    
    // 目前该函数为空，只需确保能正常调用
    QVERIFY(true);
}

QTEST_MAIN(TestGlobal)