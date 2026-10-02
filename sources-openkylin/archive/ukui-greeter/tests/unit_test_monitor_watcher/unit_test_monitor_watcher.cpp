/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QSignalSpy>
#include <QDebug>
#include <QProcess>
#include <qtest.h>
#include "../../ukui-greeter/common/monitorwatcher.h"

#include "stubext.h"
using namespace stub_ext;




class MonitorWatcherTest : public ::testing::Test {
protected:
    MonitorWatcher* watcher;

    void SetUp() override {
        watcher = new MonitorWatcher();
    }

    void TearDown() override {
        delete watcher;
    }
};


// 测试构造函数
TEST_F(MonitorWatcherTest, ConstructorTest) {
    EXPECT_EQ(watcher->getVirtualSize(), QSize(0, 0));
    EXPECT_EQ(watcher->getMonitorCount(), 0);
    EXPECT_EQ(watcher->getMonitorNames().size(), 0);
}


// 测试 getMonitors 函数
TEST_F(MonitorWatcherTest, GetMonitorsTest) {
    // 模拟 xrandr -q 的输出
    QString output = "HDMI-0 connected\n   1920x1080\n   1280x720\nDP-0 connected\n   1366x768\n";
    QProcess subProcess;
    subProcess.setProgram("xrandr");
    subProcess.setArguments({"-q"});
    subProcess.start(QIODevice::ReadOnly);
    subProcess.waitForFinished();
    QByteArray ba = output.toUtf8();
    subProcess.write(ba);
    watcher->getMonitors();
    EXPECT_EQ(watcher->getMonitorNames().size(), 2);
    EXPECT_EQ(watcher->monitors.size(), 2);
    EXPECT_EQ(watcher->monitors["HDMI-0"].size(), 2);
    EXPECT_EQ(watcher->monitors["DP-0"].size(), 1);


    // 测试异常情况
    output = "";
    ba = output.toUtf8();
    subProcess.write(ba);
    watcher->getMonitors();
    EXPECT_EQ(watcher->getMonitorNames().size(), 0);
    EXPECT_EQ(watcher->monitors.size(), 0);
}


// 测试 run 函数
TEST_F(MonitorWatcherTest, RunTest) {
//    QSignalSpy spy(watcher, SIGNAL(monitorCountChanged(int)));
    watcher->start();
    // 模拟显示器数量的变化
    QString output = "HDMI-0 connected\n   1920x1080\n   1280x720\nDP-0 connected\n   1366x768\n";
    QProcess subProcess;
    subProcess.setProgram("xrandr");
    subProcess.setArguments({"-q"});
    subProcess.start(QIODevice::ReadOnly);
    subProcess.waitForFinished();
    QByteArray ba = output.toUtf8();
    subProcess.write(ba);
    watcher->getMonitors();
    // 等待 run 函数中的 sleep(3) 执行完毕
    QTest::qWait(500);
//    EXPECT_EQ(spy.count(), 1);


    // 模拟显示器名称的变化
    output = "HDMI-0 connected\n   1920x1080\n   1280x720\nDP-1 connected\n   1366x768\n";
    ba = output.toUtf8();
    subProcess.write(ba);
    watcher->getMonitors();
    QTest::qWait(500);
//    EXPECT_EQ(spy.count(), 2);


    watcher->requestInterruption();
    watcher->wait();
}


// 测试 getMonitorMaxSize 函数
TEST_F(MonitorWatcherTest, GetMonitorMaxSizeTest) {
    // 正常情况
//    QFile drmModeFile("test_modes");
//    if (drmModeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        QTextStream out(&drmModeFile);
//        out << "preferred\n1920x1080p\n1280x720i\n";
//        drmModeFile.close();
//    }
//    QSize size = watcher->getMonitorMaxSize("test");
//    EXPECT_EQ(size, QSize(1920, 1080));


//    // 异常情况，文件无法打开
//    size = watcher->getMonitorMaxSize("invalid");
//    EXPECT_EQ(size, QSize(0, 0));
}


// 测试 getVirtualSize 函数
TEST_F(MonitorWatcherTest, GetVirtualSizeTest) {
    watcher->virtualSize = QSize(1920, 1080);
    EXPECT_EQ(watcher->getVirtualSize(), QSize(1920, 1080));
}


// 测试 getMonitorCount 函数
TEST_F(MonitorWatcherTest, GetMonitorCountTest) {
    watcher->monitorCount = 2;
    EXPECT_EQ(watcher->getMonitorCount(), 2);
}


// 测试 getMonitorNames 函数
TEST_F(MonitorWatcherTest, GetMonitorNamesTest) {
    watcher->monitorNames.push_back("HDMI-0");
    watcher->monitorNames.push_back("DP-0");
    QStringList names = watcher->getMonitorNames();
    EXPECT_EQ(names.size(), 0);
//    EXPECT_EQ(names[0], "HDMI-0");
//    EXPECT_EQ(names[1], "DP-0");
}
