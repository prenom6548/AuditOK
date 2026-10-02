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
#include <QApplication>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include "../../bioauth/include/biodevices.h" // 假设这个头文件存在，并包含了BioAuth类的声明

TEST(BioDevicesTest, init)
{
   BioDevices devices;
   
   EXPECT_EQ(devices.count(),0);

   devices.setUId(1000);
   devices.getAllDevices();
   devices.getUserDevices(1000);
   devices.getDevices(0);
   devices.getDefaultDevice(1000);
   devices.getDefaultDevice(1000,0);

   devices.GetLastDevice("kylin");
   devices.SetLastDevice("kylin",100);
   devices.SetLastDevice("kylin",-1);
   devices.SetLastDevice("testx",100);
   devices.findDevice("testDevice");
   devices.findDevice(100);
   devices.getFirstDevice(1000);
   devices.GetDevList();
   devices.getFeatureCount(1000,0,-1);
   devices.bioTypeToString_tr(0);
   devices.bioTypeToString_tr(1);
   devices.bioTypeToString_tr(2);
   devices.bioTypeToString_tr(3);
   devices.bioTypeToString_tr(4);
   devices.bioTypeToString_tr(5);
   devices.bioTypeToString_tr(6);
   devices.bioTypeToString_tr(7);
   devices.bioTypeToString_tr(8);
   devices.setIsShowHotPlug(true);
   devices.setIsShowHotPlug(false);
   devices.GetUserDevFeatureCount(1000,100);
   devices.GetUserDevCount(1000);
   devices.GetUserFeatures(1000);
   devices.getUseFirstDevice();
   devices.getFailedTimes(); 
   devices.GetHiddenSwitchButton();
   devices.GetQRCodeEnable();
   devices.GetBioAuthEnable();
   devices.getAllDefDevices();
   devices.UpdateStatus(100);
   devices.UpdateStatus(-1);
   devices.GetBioAuthEnable(1000);
   devices.GetDirveIsIdle(100);
   devices.recordAuthDrive("test",100,true);
   devices.recordAuthDrive("test",100,false);
    
   devices.onUSBDeviceHotPlug(10,-1,10);
   devices.onUSBDeviceHotPlug(10,1,10);
   
   devices.m_uniAuthService = nullptr;

   devices.setUId(1000);
   devices.getAllDevices();
   devices.getUserDevices(1000);
   devices.getDevices(0);
   devices.getDefaultDevice(1000);
   devices.getDefaultDevice(1000,0);

   devices.GetLastDevice("kylin");
   devices.SetLastDevice("kylin",100);
   devices.SetLastDevice("kylin",-1);
   devices.findDevice("testDevice");
   devices.findDevice(100);
   devices.getFirstDevice(1000);
   devices.GetDevList();
   devices.getFeatureCount(1000,0,-1);
   devices.bioTypeToString_tr(0);
   devices.bioTypeToString_tr(1);
   devices.bioTypeToString_tr(2);
   devices.bioTypeToString_tr(3);
   devices.bioTypeToString_tr(4);
   devices.bioTypeToString_tr(5);
   devices.bioTypeToString_tr(6);
   devices.bioTypeToString_tr(7);
   devices.bioTypeToString_tr(8);
   devices.setIsShowHotPlug(true);
   devices.setIsShowHotPlug(false);
   devices.GetUserDevFeatureCount(1000,100);
   devices.GetUserDevCount(1000);
   devices.GetUserFeatures(1000);
   devices.getUseFirstDevice();
   devices.getFailedTimes();
   devices.GetHiddenSwitchButton();
   devices.GetQRCodeEnable();
   devices.GetBioAuthEnable();
   devices.getAllDefDevices();
   devices.UpdateStatus(100);
   devices.UpdateStatus(-1);
   devices.GetBioAuthEnable(1000);
   devices.GetDirveIsIdle(100);
   devices.recordAuthDrive("test",100,true);
   devices.recordAuthDrive("test",100,false);

   devices.onUSBDeviceHotPlug(10,-1,10);
   devices.onUSBDeviceHotPlug(10,1,10);
}
  
// 创建一个全局的QApplication对象，因为Qt测试通常需要一个事件循环
int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
