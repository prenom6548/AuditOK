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

#include "exceptiongroupprocesswatcher.h"
#include <QCoreApplication>
#include <QGSettings/qgsettings.h>
#include <QtTest>

// add necessary includes here

class TestExceptionGroupProcessWatcher : public QObject
{
    Q_OBJECT

public:
    TestExceptionGroupProcessWatcher();
    ~TestExceptionGroupProcessWatcher();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
};

TestExceptionGroupProcessWatcher::TestExceptionGroupProcessWatcher()
{
}

TestExceptionGroupProcessWatcher::~TestExceptionGroupProcessWatcher()
{
}

void TestExceptionGroupProcessWatcher::initTestCase()
{
}

void TestExceptionGroupProcessWatcher::cleanupTestCase()
{
}

void TestExceptionGroupProcessWatcher::test_case1()
{
    QEventLoop eventLoop;
    auto pim = std::make_shared<ProcessInfoManager>(std::make_shared<ConfigManager>());
    ExceptionGroupProcessWatcher watcher(pim);
    connect(&watcher, &ExceptionGroupProcessWatcher::exceptionSessionAppCaught, this, [&watcher](const std::string &desktopFile, int pid) {
        qDebug() << "exceptionSessionAppCaught: " << QString::fromStdString(desktopFile) << pid;
        watcher.stopWatcher();
    });

    connect(&watcher, &ExceptionGroupProcessWatcher::exceptionSessionAppCaught, &eventLoop, &QEventLoop::quit);

    watcher.addPath("/sys/fs/cgroup/systemd/user.slice/user-1000.slice/session-1.scope", sched_policy::GroupType::SessionScopeGroup);
    watcher.startWatcher();
    QTimer::singleShot(5000, this, [&eventLoop, &watcher] {
        qDebug() << "Time out";
        watcher.stopWatcher();
        eventLoop.quit();
    });
    eventLoop.exec();
}

QTEST_MAIN(TestExceptionGroupProcessWatcher)

#include "tst_exceptiongroupprocesswatcher.moc"
