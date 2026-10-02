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

#include "../../ukui-greeter/common/touchscreenset.h"

#include "stubext.h"
using namespace stub_ext;

#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QMutex>
#include <memory>
#include <QScreen>


class TouchScreenSetTest : public ::testing::Test {
//protected:
//    std::unique_ptr<TouchScreenSet> touchScreenSet;


//    void SetUp() override {
//        // 初始化 QApplication 实例，因为部分代码依赖于 QApplication
//        int argc = 0;
//        char *argv[] = {nullptr};
//        static QApplication app(argc, argv);
//        touchScreenSet = std::make_unique<TouchScreenSet>();
//    }


//    void TearDown() override {
//        touchScreenSet.reset();
//    }
};


// 测试单例模式
TEST_F(TouchScreenSetTest, InstanceTest) {
    TouchScreenSet* instance1 = TouchScreenSet::instance();
    TouchScreenSet* instance2 = TouchScreenSet::instance();
    EXPECT_EQ(instance1, instance2);
}


// 测试 init 函数
TEST_F(TouchScreenSetTest, InitTest) {
//    QSignalSpy spy(touchScreenSet.get(), SIGNAL(monitorCountChanged(int)));
//    touchScreenSet->init();
    // 模拟 monitorCountChanged 信号发射
    Q_EMIT TouchScreenSet::instance()->m_monitorWatcher->monitorCountChanged(1);
//    EXPECT_EQ(spy.count(), 1);
}


// 测试 getTouchScreenID 函数
TEST_F(TouchScreenSetTest, GetTouchScreenIDTest) {
    // 正常情况
//    Display* dummyDisplay = nullptr;
//    XIDeviceInfo dummyDeviceInfo;
//    dummyDeviceInfo.use = XISlavePointer;
//    dummyDeviceInfo.enabled = true;
//    dummyDeviceInfo.num_classes = 1;
//    dummyDeviceInfo.classes = new XIAnyClassInfo[1];
//    dummyDeviceInfo.classes[0].type = XITouchClass;
//    dummyDeviceInfo.classes[0].mode = XIDirectTouch;


//    EXPECT_TRUE(getTouchScreenID(dummyDisplay, &dummyDeviceInfo));


//    // 异常情况：设备禁用
//    dummyDeviceInfo.enabled = false;
//    EXPECT_FALSE(getTouchScreenID(dummyDisplay, &dummyDeviceInfo));


//    // 异常情况：非触摸设备
//    dummyDeviceInfo.enabled = true;
//    dummyDeviceInfo.classes[0].type = XIButtonClass;
//    EXPECT_FALSE(getTouchScreenID(dummyDisplay, &dummyDeviceInfo));


//    delete[] dummyDeviceInfo.classes;
}


// 测试 setTouchMapScreen 函数
TEST_F(TouchScreenSetTest, SetTouchMapScreenTest) {
    QProcess::execute("touch test_screen_name");
    QProcess::execute("touch test_devicesid");


    // 模拟 QApplication::primaryScreen()->name()
//    QApplication::instance()->screens().first()->setName("test_screen_name");


    // 模拟 setTouchMapScreen 函数的部分操作
    TouchScreenSet::instance()->setTouchMapScreen();


    // 检查 xinput 命令是否被执行
    QProcess checkProcess;
    checkProcess.start("grep 'xinput map-to-output' test_devicesid");
    checkProcess.waitForFinished();
    QString output = checkProcess.readAll();
    EXPECT_FALSE(output.isEmpty());
}
