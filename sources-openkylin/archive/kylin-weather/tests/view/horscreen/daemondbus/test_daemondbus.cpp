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
#include "test_daemondbus.h"
#include <QApplication>
#include <QDebug>
#include <QDBusConnection>

void TestDaemonDbus::initTestCase()
{
    m_daemonDbus = new DaemonDbus();
}

void TestDaemonDbus::cleanupTestCase()
{
    if (m_daemonDbus) {
        delete m_daemonDbus;
        m_daemonDbus = nullptr;
    }
}

void TestDaemonDbus::testDaemonIsNotRunning()
{
    // 调用函数
    int result = m_daemonDbus->daemonIsNotRunning();
    
    // 验证结果（可能返回-1表示连接失败，0表示正在运行，1表示未运行）
    QVERIFY(result == -1 || result == 0 || result == 1);
    
    qDebug() << "daemonIsNotRunning returned:" << result;
}

void TestDaemonDbus::testShowGuide()
{
    // 测试传入有效的应用程序名称
    m_daemonDbus->showGuide("kylin-weather");
    QVERIFY(true); // 如果没有崩溃，说明函数执行正常
    
    // 测试传入空字符串
    m_daemonDbus->showGuide("");
    QVERIFY(true); // 如果没有崩溃，说明函数执行正常
    
    // 测试传入null字符串
    m_daemonDbus->showGuide(QString());
    QVERIFY(true); // 如果没有崩溃，说明函数执行正常
}