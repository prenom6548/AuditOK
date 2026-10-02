#include <gtest/gtest.h>
#include "dbuscallthread.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QSignalSpy>

class UnitTest_DbusCallThread : public ::testing::Test {
protected:
    DbusCallThread* thread;

    void SetUp() override {
        QVariantList json;
        json << QVariant("test");
        thread = new DbusCallThread(json, "runtime", "appName", "debPath");
    }

    void TearDown() override {
        // delete thread;
    }
};

TEST_F(UnitTest_DbusCallThread, ConstructorTest) {
    QVariantList json;
    json << QVariant("test");
    QString runtime = "test_runtime";
    QString appName = "test_app";
    QString debPath = "test_debPath";

    DbusCallThread thread(json, runtime, appName, debPath);

    EXPECT_EQ(thread.m_runtime.toStdString(), runtime.toStdString());
    EXPECT_EQ(thread.m_appName.toStdString(), appName.toStdString());
    // EXPECT_EQ(thread.m_jsonInfo.toStdString(), json.toStdString());
    EXPECT_EQ(thread.m_debPath.toStdString(), debPath.toStdString());
}

TEST_F(UnitTest_DbusCallThread, RunTest) {
    QSignalSpy successSpy(thread, &DbusCallThread::installSuccess);
    QSignalSpy failSpy(thread, &DbusCallThread::installFail);

    thread->run();

    // 检查是否发出了成功或失败的信号
    EXPECT_TRUE(successSpy.count() > 0 || failSpy.count() > 0);
}

TEST_F(UnitTest_DbusCallThread, RunTestServiceInterfaceInvalid) {
    // 模拟无效的 QDBusInterface
    QDBusInterface* serviceInterface = new QDBusInterface("invalid.service",
                                                          "/invalid/path",
                                                          "invalid.interface",
                                                          QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647);

    ASSERT_FALSE(serviceInterface->isValid());

    delete serviceInterface;
}

TEST_F(UnitTest_DbusCallThread, RunTestInstallSuccess) {
    // 模拟安装成功的情况
    QSignalSpy successSpy(thread, &DbusCallThread::installSuccess);

    thread->run();

    // 检查是否发出了成功的信号
    EXPECT_EQ(successSpy.count(), 1);
}

TEST_F(UnitTest_DbusCallThread, RunTestInstallFail) {
    // 模拟安装失败的情况
    QSignalSpy failSpy(thread, &DbusCallThread::installFail);

    thread->run();

    // 检查是否发出了失败的信号
    EXPECT_EQ(failSpy.count(), 1);
}