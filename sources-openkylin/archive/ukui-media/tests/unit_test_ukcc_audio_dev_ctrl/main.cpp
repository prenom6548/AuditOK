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
#include "../../audio/audio-device-control/ukmedia_device_control_widget.h"
#include <QApplication>


class ukccAudioCtrlTest : public testing::Test
{
protected:
    static void SetUpTestCase() {
        int argc = 1;
        char *argv[] = {"ukccAudioCtrlTest"}; //需加参数，否则因SDK Kwidget运行构造函数报错
        app = new QApplication(argc, argv);
        ukccAudioCtrl = new UkmediaDevControlWidget();
    }

    static void TearDownTestCase() {
        delete ukccAudioCtrl;
        delete app;
    }

    static UkmediaDevControlWidget* ukccAudioCtrl;
    static QApplication* app;
};

//  静态成员变量初始化测试
UkmediaDevControlWidget* ukccAudioCtrlTest::ukccAudioCtrl = nullptr;
QApplication* ukccAudioCtrlTest::app = nullptr;


TEST_F(ukccAudioCtrlTest, getPaOutputDevInfoTest) {
    ukccAudioCtrl->outputDevWidget->getPaDevInfo();
}

TEST_F(ukccAudioCtrlTest, getPaInputDevInfoTest) {
    ukccAudioCtrl->inputDevWidget->getPaDevInfo();
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
