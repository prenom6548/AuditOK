/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../audio/ukmedia_main_widget.h"
#include <QApplication>


class ukccAppCtrlTest : public testing::Test
{
protected:
    static void SetUpTestCase() {
        int argc = 1;
        char *argv[] = {"ukccAppCtrlTest"}; //需加参数，否则因SDK Kwidget运行构造函数报错
        app = new QApplication(argc, argv);
        ukccAppCtrl = new UkmediaAppCtrlWidget();
    }

    static void TearDownTestCase() {
        delete ukccAppCtrl;
        delete app;
    }

    static UkmediaAppCtrlWidget* ukccAppCtrl;
    static QApplication* app;
};

//  静态成员变量初始化测试
UkmediaAppCtrlWidget* ukccAppCtrlTest::ukccAppCtrl = nullptr;
QApplication* ukccAppCtrlTest::app = nullptr;

//  获取系统音量
TEST_F(ukccAppCtrlTest, Test_getSystemVolume) {
    int value = ukccAppCtrl->getSystemVolume();
}

//  设置应用音量测试
TEST_F(ukccAppCtrlTest, Test_setAppVolume) {
    QSlider *slider = ukccAppCtrl->stackWidget->findChild<QSlider*>("kylin-music");

    if (slider)
        slider->setValue(58);
    else
        ukccAppCtrl->setAppVolume(100);
}

//  设置系统音量测试
TEST_F(ukccAppCtrlTest, Test_setSystemVolume) {
    bool state = ukccAppCtrl->setSystemVolume(100);
}

//  设置应用静音状态测试
TEST_F(ukccAppCtrlTest, Test_setAppMuteState) {
    UkuiButtonDrawSvg *btn = ukccAppCtrl->stackWidget->findChild<UkuiButtonDrawSvg *>("kylin-music");

    if (btn)
        btn->click();
    else
        ukccAppCtrl->setAppMuteState();
}

//  设置系统输出设备静音状态测试
TEST_F(ukccAppCtrlTest, Test_setSystemOutputDevice) {
    bool state = ukccAppCtrl->setSystemOutputDevice("Speaker（sof-hda-dsp）");
}

//  设置系统输入设备静音状态测试
TEST_F(ukccAppCtrlTest, Test_setSystemInputDevice) {
    bool state = ukccAppCtrl->setSystemInputDevice("Digital Microphone（sof-hda-dsp）");
}

//  设置应用输出设备测试
TEST_F(ukccAppCtrlTest, Test_setAppOutputDevice) {
    QComboBox *box = ukccAppCtrl->stackWidget->findChild<QComboBox *>("kylin-music-output");

    if (box)
        box->setCurrentIndex(1);
    else
        ukccAppCtrl->setAppOutputDevice("Speaker（sof-hda-dsp）");
}

//  设置应用输入设备测试
TEST_F(ukccAppCtrlTest, Test_setAppInputDevice) {
    QComboBox *box = ukccAppCtrl->stackWidget->findChild<QComboBox *>("kylin-recorder-input");

    if (box)
        box->setCurrentIndex(1);
    else
        ukccAppCtrl->setAppInputDevice("Digital Microphone（sof-hda-dsp）");
}

//  更新端口测试
TEST_F(ukccAppCtrlTest, Test_updatePort) {
    ukccAppCtrl->updatePort();
}

//  更新应用列表测试
TEST_F(ukccAppCtrlTest, Test_updateAppItem) {
    ukccAppCtrl->updateAppItem(SYSTEM_VOLUME_CTRL);
}

//  移除应用列表测试
TEST_F(ukccAppCtrlTest, Test_removeAppItem) {
    ukccAppCtrl->removeAppItem("Test");
}

//  更新系统音量测试
TEST_F(ukccAppCtrlTest, Test_updateSystemVolume) {
    ukccAppCtrl->updateSystemVolume(50);
}

//  更新系统静音状态测试
TEST_F(ukccAppCtrlTest, Test_updateSystemMuteState) {
    ukccAppCtrl->updateSystemMuteState(true);
}

//  应用索引查询测试
TEST_F(ukccAppCtrlTest, Test_indexOfItemCombobox) {
    QComboBox *box = new QComboBox();
    box->addItem(SYSTEM_VOLUME_CTRL);
    int index = ukccAppCtrl->indexOfItemCombobox(SYSTEM_VOLUME_CTRL, box);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
