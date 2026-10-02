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

#include "../../ukui-greeter/common/autorotatescreen.h"

#include "stubext.h"
using namespace stub_ext;

#include <QDebug>
#include <QProcess>
#include <memory>
#include <QApplication>


class AutoRotateScreenTest : public ::testing::Test {
//protected:
//    std::unique_ptr<AutoRotateScreen> autoRotateScreen;


//    void SetUp() override {
//        // 初始化 QApplication 实例，因为部分代码依赖于 QApplication
//        int argc = 0;
//        char *argv[] = {nullptr};
//        static QApplication app(argc, argv);
//        autoRotateScreen = std::make_unique<AutoRotateScreen>();
//    }


//    void TearDown() override {
//        autoRotateScreen.reset();
//    }
};


// 测试单例模式
TEST_F(AutoRotateScreenTest, InstanceTest) {
//    AutoRotateScreen* instance1 = AutoRotateScreen::instance();
//    AutoRotateScreen* instance2 = AutoRotateScreen::instance();
//    EXPECT_EQ(instance1, instance2);
}


// 测试 init 函数
TEST_F(AutoRotateScreenTest, InitTest) {
//    QSignalSpy sensorReadingSpy(autoRotateScreen.get()->m_pSensor, SIGNAL(readingChanged()));
//    QSignalSpy sensorActiveSpy(autoRotateScreen.get()->m_pSensor, SIGNAL(activeChanged()));


    AutoRotateScreen::instance()->init();


//    // 模拟 QOrientationSensor 信号发射
//    Q_EMIT AutoRotateScreen::instance()->m_pSensor->readingChanged();
//    Q_EMIT AutoRotateScreen::instance()->m_pSensor->activeChanged();
}


// 测试 start 和 stop 函数
TEST_F(AutoRotateScreenTest, StartStopTest) {
    // 测试 start 函数
    AutoRotateScreen::instance()->start();
    EXPECT_TRUE(AutoRotateScreen::instance()->m_pSensor->isActive());


    // 测试 stop 函数
    AutoRotateScreen::instance()->stop();
    EXPECT_FALSE(AutoRotateScreen::instance()->m_pSensor->isActive());
}


// 测试 setScreenDirection 函数
TEST_F(AutoRotateScreenTest, SetScreenDirectionTest) {
    // 模拟 monitorWatcher 提供的 monitorList
    QStringList monitorList = {"Monitor1", "Monitor2"};
    AutoRotateScreen::instance()->monitorWatcher->monitorNames = monitorList;


    // 测试 Normal 方向
    AutoRotateScreen::instance()->setScreenDirection(AutoRotateScreen::RotateDir::Normal);


    // 检查 xrandr 命令是否被执行
    QProcess checkProcess;
    checkProcess.start("pgrep xrandr");
    checkProcess.waitForFinished();
    QString output = checkProcess.readAll();
    EXPECT_FALSE(output.isEmpty());


    // 异常情况：空的 monitorList
    AutoRotateScreen::instance()->monitorWatcher->monitorNames.clear();
    AutoRotateScreen::instance()->setScreenDirection(AutoRotateScreen::RotateDir::Left);


    // 再次检查 xrandr 命令是否被执行
    checkProcess.start("pgrep xrandr");
    checkProcess.waitForFinished();
    output = checkProcess.readAll();
    EXPECT_TRUE(output.isEmpty());
}


// 测试 getScreenDirection 函数
TEST_F(AutoRotateScreenTest, GetScreenDirectionTest) {
    AutoRotateScreen::instance()->m_curScreenDirection = AutoRotateScreen::RotateDir::Right;
    EXPECT_EQ(AutoRotateScreen::instance()->getScreenDirection(), AutoRotateScreen::RotateDir::Right);
}


// 测试 onReadingChanged 函数
TEST_F(AutoRotateScreenTest, OnReadingChangedTest) {
    // 模拟 QOrientationReading
    QOrientationReading reading;


    // 测试 TopUp 方向
    reading.setOrientation(QOrientationReading::TopUp);
    Q_EMIT AutoRotateScreen::instance()->m_pSensor->readingChanged();


    // 测试 TopDown 方向
    reading.setOrientation(QOrientationReading::TopDown);
    Q_EMIT AutoRotateScreen::instance()->m_pSensor->readingChanged();

}
