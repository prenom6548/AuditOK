/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QSignalSpy>
#include <QtTest/QtTest>

// 定义private为public以便访问私有成员
#define private public
#define protected public
#include "../core/multimediacontroller.h"
#undef private
#undef protected

// 创建MockDBusInterface用于模拟MPRIS接口
class MockDBusInterface
{
public:
    static bool isDBusConnected;
    static bool isInterfaceValid;
    static int playState;
    static bool pauseCalled;
    static std::string lastServiceName;

    static void reset()
    {
        isDBusConnected = true;
        isInterfaceValid = true;
        playState = 1; // 默认为播放状态
        pauseCalled = false;
        lastServiceName.clear();
    }
};

bool MockDBusInterface::isDBusConnected = true;
bool MockDBusInterface::isInterfaceValid = true;
int MockDBusInterface::playState = 1;
bool MockDBusInterface::pauseCalled = false;
std::string MockDBusInterface::lastServiceName;

// 测试MultimediaController类
class TestMultimediaController : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testIsPause_Playing();
    void testIsPause_Paused();
    void testIsPause_DBusNotConnected();
    void testIsPause_InvalidInterface();

    void testPause_AlreadyPaused();
    void testPause_NeedToPause();

private:
    MultimediaController *m_controller;
};

void TestMultimediaController::initTestCase()
{
    // 测试套件初始化
}

void TestMultimediaController::cleanupTestCase()
{
    // 测试套件清理
}

void TestMultimediaController::init()
{
    // 每个测试前初始化
    m_controller = new MultimediaController();
    MockDBusInterface::reset();
}

void TestMultimediaController::cleanup()
{
    // 每个测试后清理
    delete m_controller;
}

// 重写QDBusConnection::sessionBus().isConnected()的行为
// namespace QDBusConnection {
//    bool isConnected() {
//        return MockDBusInterface::isDBusConnected;
//    }

//    QDBusPendingCall asyncCall(const QDBusMessage& /*message*/) {
//        MockDBusInterface::pauseCalled = true;
//        return QDBusPendingCall();
//    }
//}

// 创建模拟QDBusInterface的行为
namespace QDBus
{
bool isValid()
{
    return MockDBusInterface::isInterfaceValid;
}

QVariant call(const QString & /*method*/)
{
    return QVariant(MockDBusInterface::playState);
}
}

// 测试播放状态下的isPause方法
void TestMultimediaController::testIsPause_Playing()
{
    // 设置模拟播放状态
    MockDBusInterface::playState = 1; // 1表示播放中

    // 测试是否返回false (未暂停)
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    bool result = m_controller->isPause(serviceName);

    QVERIFY(!result);
}

// 测试暂停状态下的isPause方法
void TestMultimediaController::testIsPause_Paused()
{
    // 设置模拟暂停状态
    MockDBusInterface::playState = 0; // 0表示暂停

    // 测试是否返回true (已暂停)
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    bool result = m_controller->isPause(serviceName);

    QVERIFY(result);
}

// 测试DBus未连接时的isPause方法
void TestMultimediaController::testIsPause_DBusNotConnected()
{
    // 设置模拟DBus未连接
    MockDBusInterface::isDBusConnected = false;

    // 测试是否返回true (默认为暂停状态)
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    bool result = m_controller->isPause(serviceName);

    QVERIFY(result);
}

// 测试无效的接口时的isPause方法
void TestMultimediaController::testIsPause_InvalidInterface()
{
    // 设置模拟接口无效
    MockDBusInterface::isInterfaceValid = false;

    // 测试是否返回true (默认为暂停状态)
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    bool result = m_controller->isPause(serviceName);

    QVERIFY(result);
}

// 测试已经处于暂停状态时调用pause方法
void TestMultimediaController::testPause_AlreadyPaused()
{
    // 设置模拟为已暂停状态
    MockDBusInterface::playState = 0; // 0表示暂停

    // 调用pause方法
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    m_controller->pause(serviceName);

    // 验证是否没有调用DBus方法(因为已经暂停)
    QVERIFY(!MockDBusInterface::pauseCalled);
}

// 测试播放状态时调用pause方法
void TestMultimediaController::testPause_NeedToPause()
{
    // 设置模拟为播放状态
    MockDBusInterface::playState = 1; // 1表示播放中

    // 调用pause方法
    std::string serviceName = "org.mpris.MediaPlayer2.TestPlayer";
    m_controller->pause(serviceName);

    // 验证是否调用了DBus方法
    QVERIFY(MockDBusInterface::pauseCalled);
    QCOMPARE(MockDBusInterface::lastServiceName, serviceName);
}

QTEST_MAIN(TestMultimediaController)
#include "tst_multimediacontroller.moc"
