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
#include <QCoreApplication>
#include <QString>
#include <memory>
#include <iostream>
#include "../../ukui-greeter/common/machinemodel.h"

// 测试 MachineModel 类
class MachineModelTest : public ::testing::Test {
protected:
    std::shared_ptr<MachineModel> machineModel;


    void SetUp() override {
        machineModel = MachineModel::getMachineModelInstance();
    }


    void TearDown() override {
        machineModel.reset();
    }
};


// 测试 getSysVendor 函数
TEST_F(MachineModelTest, getSysVendor) {
    QString vendor = machineModel->getSysVendor();
    EXPECT_EQ(vendor, "Vendor");
}


// 测试 getProductName 函数
TEST_F(MachineModelTest, getProductName) {
    QString productName = machineModel->getProductName();
    EXPECT_EQ(productName, "Product");
}


// 测试 getProductFamily 函数
TEST_F(MachineModelTest, getProductFamily) {
    QString productFamily = machineModel->getProductFamily();
    EXPECT_EQ(productFamily, "Family");
}


// 测试 initMachineType 函数
TEST_F(MachineModelTest, initMachineType) {
    machineModel->initMachineType();
    QString machineType = machineModel->getTheMachineType();
    EXPECT_EQ(machineType, "");  // 这里根据实际的配置文件设置进行调整
}


//// 测试 LibinputSwitchEvent 类
//class LibinputSwitchEventTest : public ::testing::Test {
//protected:
//    std::unique_ptr<LibinputSwitchEvent> libinputSwitchEvent;


//    void SetUp() override {
//        // 使用模拟的 QProcess
//        qputenv("QT_QPA_PLATFORM", "minimal");
//        qRegisterMetaType<Event *>("Event*");
//        libinputSwitchEvent = std::make_unique<LibinputSwitchEvent>(nullptr);
//    }


//    void TearDown() override {
//        libinputSwitchEvent.reset();
//    }
//};


//// 测试 geInitDevicesStatus 函数
//TEST_F(LibinputSwitchEventTest, geInitDevicesStatus) {
//    // 测试不同的机器类型
//    libinputSwitchEvent->m_machineType = "SLATE";
//    EXPECT_TRUE(libinputSwitchEvent->geInitDevicesStatus());


//    libinputSwitchEvent->m_machineType = "LAPTOP";
//    // EXPECT_TRUE(libinputSwitchEvent->geInitDevicesStatus());


//    libinputSwitchEvent->m_machineType = "ALLINONE";
//    EXPECT_FALSE(libinputSwitchEvent->geInitDevicesStatus());


//    libinputSwitchEvent->m_machineType = "OTHER";
//    EXPECT_FALSE(libinputSwitchEvent->geInitDevicesStatus());
//}


//// 测试 dealEvent 函数
//TEST_F(LibinputSwitchEventTest, dealEvent) {
//    Event event;
//    event.type = LIBINPUT_EVENT_SWITCH_TOGGLE;
//    event.event.switchEventDate.switchType = LIBINPUT_SWITCH_TABLET_MODE;
//}
