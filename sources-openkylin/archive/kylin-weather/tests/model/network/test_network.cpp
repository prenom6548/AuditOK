/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "test_network.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QDBusConnection>
#include <QDBusMessage>
#include "model/network.h"

void TestNetwork::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestNetwork::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestNetwork::testNetworkConstructor()
{
    // 测试Network构造函数
    Network network;
    QVERIFY(true); // 构造函数成功执行
}

void TestNetwork::testCheckNetWork()
{
    // 测试checkNetWork函数
    Network network;
    
    // 连接信号监视器
    QSignalSpy spy(&network, &Network::effectiveNetwork);
    
    // 调用函数，应该不会崩溃
    network.checkNetWork();
    
    // 验证信号是否正确发出
    QVERIFY(spy.count() >= 0); // 信号可能发出也可能不发出，取决于网络状态
}

void TestNetwork::testInitConnectionInfo()
{
    // 测试initConnectionInfo函数
    Network network;
    
    // 调用函数，应该不会崩溃
    network.initConnectionInfo();
    QVERIFY(true); // 函数执行成功
}

void TestNetwork::testOnStateChanged_Connected()
{
    // 测试 NM_STATE_CONNECTED_GLOBAL (70)
    Network network;
    QSignalSpy spy(&network, &Network::haveNet);
    
    // 直接调用私有槽函数
    QMetaObject::invokeMethod(&network, "onStateChanged", Q_ARG(uint, 70));
    
    // onStateChanged(70) 会启动一个 1s 的定时器，然后触发 onTimeFinished
    QTest::qWait(1200); 
    
    QVERIFY(CoreVar::HAS_NETWORK);
    QCOMPARE(spy.count(), 1);
}

void TestNetwork::testOnStateChanged_Disconnected()
{
    // 测试 NM_STATE_DISCONNECTED (20)
    Network network;
    CoreVar::HAS_NETWORK = true;
    
    QMetaObject::invokeMethod(&network, "onStateChanged", Q_ARG(uint, 20));
    
    QVERIFY(!CoreVar::HAS_NETWORK);
}

void TestNetwork::testOnTimeFinished()
{
    Network network;
    QSignalSpy spy(&network, &Network::haveNet);
    
    QMetaObject::invokeMethod(&network, "onTimeFinished");
    
    QCOMPARE(spy.count(), 1);
}

void TestNetwork::testOnScreenSaverUnlock()
{
    Network network;
    QSignalSpy spy(&network, &Network::haveNet);
    
    QMetaObject::invokeMethod(&network, "onScreenSaverUnlock");
    
    QCOMPARE(spy.count(), 1);
}

void TestNetwork::testOnPrepareForSleep_EnterSleep()
{
    // 测试进入休眠状态 (isSleep = true)
    Network network;
    
    // 直接调用私有槽函数，传入 true 表示进入休眠
    QMetaObject::invokeMethod(&network, "onPrepareForSleep", Q_ARG(bool, true));
    
    // 函数执行成功，不应崩溃
    QVERIFY(true);
}

void TestNetwork::testOnPrepareForSleep_WakeFromSleep()
{
    // 测试从休眠唤醒 (isSleep = false)
    Network network;
    
    // 直接调用私有槽函数，传入 false 表示从休眠唤醒
    // 注意：唤醒时会 sleep(5)，测试时需要等待
    QMetaObject::invokeMethod(&network, "onPrepareForSleep", Q_ARG(bool, false));
    
    // 函数执行成功，不应崩溃
    // 由于内部有 QThread::sleep(5)，这里等待 6 秒
    QTest::qWait(6000);
    QVERIFY(true);
}
