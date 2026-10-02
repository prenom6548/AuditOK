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
#include <QIcon>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include "../../bioauth/include/loginoptionswidget.h" // 假设这个头文件存在，并包含了BioAuth类的声明

TEST(LoginOptionsWidgetTest, init)
{
    LoginOptionsWidget loginWidget;
    
    EXPECT_EQ(loginWidget.getLoginOptCount(),0);
    EXPECT_EQ(loginWidget.convertDeviceType(0),0);

    int nLoginOptType,nDrvId;
    loginWidget.getCurLoginOpt(nLoginOptType,nDrvId);
    loginWidget.getFirstDevInfo();
    loginWidget.updateUIStatus(false);
    loginWidget.updateUIStatus(true);
    loginWidget.updateUkeyUIStatus(0);
    loginWidget.updateUkeyUIStatus(1);
    loginWidget.updateUkeyUIStatus(8);
    loginWidget.setUser(1000);
    loginWidget.setCurrentDevice(-1);
    loginWidget.setCurrentDevice("test");
    loginWidget.setDeviceDisable(100,false);
    loginWidget.setDeviceDisable(100,true);
    loginWidget.isDeviceDisable(false);
    loginWidget.lockStatusChanged(false);  
    loginWidget.lockStatusChanged(true);  
   
    loginWidget.readDevicesInfo();
    loginWidget.onIdentifyComplete(1000,true,0);
    loginWidget.onIdentifyComplete(1000,false,0);
    loginWidget.onIdentifyComplete(-1,true,0);
    loginWidget.onAuthFinished();
    loginWidget.onStatusChanged(1000,"test");
    loginWidget.onFrameWritten(1000);
    loginWidget.onUSBDeviceCountChange(0);
    loginWidget.onUSBDeviceCountChange(1);
    loginWidget.onOptionSelected(-1);
    loginWidget.onOptionSelected(0);
    loginWidget.onOptionSelected(1);
    loginWidget.setQRCodeMsg("");
    loginWidget.setQRCodeMsg("test");
 
    EXPECT_EQ(loginWidget.GetDefaultDevice(1000),""); 
    EXPECT_EQ(loginWidget.GetDefaultDevice(1000,0),"");

    loginWidget.startAuth(nullptr,1000);
    loginWidget.stopAuth();
    loginWidget.SetExtraInfo("test1","test2");
    EXPECT_EQ(loginWidget.getHasUkeyOptions(),false);
    loginWidget.setSelectedPassword();
    loginWidget.setAllDeviceDisable(true);
    loginWidget.setAllDeviceDisable(false);

    loginWidget.notifyOptionsChange(-1);
    loginWidget.notifyOptionsChange(0);
    loginWidget.notifyOptionsChange(1);
    loginWidget.optionSelected(LOGINOPT_TYPE_GENERAL_UKEY,nullptr);
    QImage image;
    loginWidget.updateImage(image);
    loginWidget.authComplete(1000,-1,-1);
    loginWidget.updateAuthMsg("test");
    loginWidget.updateWndSize(0,100);
    loginWidget.initUI();
    loginWidget.initConnections();
    loginWidget.addOptionButton(LOGINOPT_TYPE_PASSWORD,0,"LOGINOPT_TYPE_PASSWORD");
    loginWidget.addOptionButton(LOGINOPT_TYPE_GENERAL_UKEY,1,"LOGINOPT_TYPE_GENERAL_UKEY");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FACE,2,"LOGINOPT_TYPE_FACE");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FINGERPRINT,3,"LOGINOPT_TYPE_FINGERPRINT");
    loginWidget.addOptionButton(LOGINOPT_TYPE_IRIS,4,"LOGINOPT_TYPE_IRIS");
    loginWidget.addOptionButton(LOGINOPT_TYPE_VOICEPRINT,5,"LOGINOPT_TYPE_VOICEPRINT");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FINGERVEIN,6,"LOGINOPT_TYPE_FINGERVEIN");
    loginWidget.addOptionButton(LOGINOPT_TYPE_QRCODE,7,"LOGINOPT_TYPE_QRCODE");
    loginWidget.onOptionSelected(-1);
    loginWidget.onOptionSelected(0);
    loginWidget.onOptionSelected(1);
    loginWidget.clearOptionButtons();
    loginWidget.updateOptionButtons();
    loginWidget.startAuth_();
    loginWidget.stopAuth();
    QPixmap pix = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    loginWidget.PixmapToRound(pix,32);
    QPixmap pix1;
    loginWidget.PixmapToRound(pix1,32);
    loginWidget.scaledPixmap(1024,768,"/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg"); 
    loginWidget.scaledPixmap(1024,768,"/usr/share/backgrounds/test-warty-final-ubuntukylin.jpg"); 
    loginWidget.scaledPixmap(1024,768,"/usr/share/ukui-biometric/images/ukui-loginopt-face.svg"); 
    loginWidget.scaledPixmap(1024,768,"/usr/share/ukui-biometric/images/FingerPrint.gif"); 
    loginWidget.scaledPixmap(1024,768,"/usr/share/ukui-biometric/ukui-biometric.conf"); 
    loginWidget.updatePixmap();

    loginWidget.drawSymbolicColoredPixmap(pix,"white"); 
    loginWidget.drawSymbolicColoredPixmap(pix,"blue"); 
    loginWidget.drawSymbolicColoredPixmap(pix,"gray"); 
    loginWidget.drawSymbolicColoredPixmap(pix,"black"); 
    loginWidget.drawSymbolicColoredPixmap(pix,"red"); 
    
    QImage img;   
    QImage img1 = pix1.toImage();
    loginWidget.setFaceImg(img,1);
    loginWidget.setFaceImg(img,2);
    loginWidget.setFaceImg(img,3);
    loginWidget.setFaceImg(img1,1);

    loginWidget.convertDeviceType(BIOTYPE_FINGERPRINT);
    loginWidget.convertDeviceType(BIOTYPE_FINGERVEIN);
    loginWidget.convertDeviceType(BIOTYPE_IRIS);
    loginWidget.convertDeviceType(BIOTYPE_FACE);
    loginWidget.convertDeviceType(BIOTYPE_VOICEPRINT);
    loginWidget.convertDeviceType(UniT_General_Ukey);
    loginWidget.convertDeviceType(UniT_Advanced_Ukey);
    loginWidget.convertDeviceType(REMOTE_QRCODE_TYPE);
    loginWidget.convertDeviceType(-1);
}
 
TEST(LoginOptionsWidgetTest, noProxy) {
    LoginOptionsWidget loginWidget;
    loginWidget.m_biomericProxy = nullptr;
    loginWidget.SetExtraInfo("test1","test2");
}

// 模拟startAuth的调用
TEST(LoginOptionsWidgetTest, startAuth) {
    DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
    pDeviceInfo->device_id = 100;
    pDeviceInfo->device_shortname = "TestDevice";
    pDeviceInfo->device_fullname = "Test Device Full Name";
    pDeviceInfo->driver_enable = 1;
    pDeviceInfo->device_available = 1;
    pDeviceInfo->biotype = BIOTYPE_FINGERPRINT;
    pDeviceInfo->stotype = 0;
    pDeviceInfo->eigtype = 0;
    pDeviceInfo->vertype = 0;
    pDeviceInfo->idtype = 0;
    pDeviceInfo->bustype = 0;
    pDeviceInfo->dev_status = 0;
    pDeviceInfo->ops_status = 0;

    LoginOptionsWidget loginWidget;
    loginWidget.onOptionSelected(-1);
    loginWidget.onOptionSelected(0);
    loginWidget.onOptionSelected(1);
    loginWidget.m_mapDevices[LOGINOPT_TYPE_FINGERPRINT].push_back(pDeviceInfo);
    loginWidget.setCurrentDevice("TestDevice"); 
    loginWidget.setCurrentDevice(100); 
    loginWidget.setCurrentDevice(pDeviceInfo); 
    loginWidget.startAuth(pDeviceInfo,1000);

    loginWidget.m_isInAuth = true;
    loginWidget.stopAuth();

    loginWidget.onStatusChanged(100,"this is test");
}

// 模拟认证完成
TEST(LoginOptionsWidgetTest, onIdentifyComplete) {
    LoginOptionsWidget loginWidget;
    loginWidget.m_isStopped = false;

    loginWidget.onIdentifyComplete(-1,-1,-1);
    loginWidget.onIdentifyComplete(0,-1,-1);
    loginWidget.onIdentifyComplete(0,-1,1000);
    loginWidget.onIdentifyComplete(0,-1,200);
    loginWidget.onIdentifyComplete(-1,-1,-3);
}

//模拟device管理
TEST(LoginOptionsWidgetTest, testAllDevice) {
    DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
    pDeviceInfo->device_id = 100;
    pDeviceInfo->device_shortname = "TestDevice";
    pDeviceInfo->device_fullname = "Test Device Full Name";
    pDeviceInfo->driver_enable = 1;
    pDeviceInfo->device_available = 1;
    pDeviceInfo->biotype = BIOTYPE_FINGERPRINT;
    pDeviceInfo->stotype = 0;
    pDeviceInfo->eigtype = 0;
    pDeviceInfo->vertype = 0;
    pDeviceInfo->idtype = 0;
    pDeviceInfo->bustype = 0;
    pDeviceInfo->dev_status = 0;
    pDeviceInfo->ops_status = 0;

    DeviceInfoPtr pDeviceInfo1 = std::make_shared<DeviceInfo>();
    pDeviceInfo1->device_id = 101;
    pDeviceInfo1->device_shortname = "TestDevice1";
    pDeviceInfo1->device_fullname = "Test Device Full Name";
    pDeviceInfo1->driver_enable = 1;
    pDeviceInfo1->device_available = 1;
    pDeviceInfo1->biotype = BIOTYPE_FINGERVEIN;
    pDeviceInfo1->stotype = 0;
    pDeviceInfo1->eigtype = 0;
    pDeviceInfo1->vertype = 0;
    pDeviceInfo1->idtype = 0;
    pDeviceInfo1->bustype = 0;
    pDeviceInfo1->dev_status = 0;
    pDeviceInfo1->ops_status = 0;

    DeviceInfoPtr pDeviceInfo2 = std::make_shared<DeviceInfo>();
    pDeviceInfo2->device_id = 102;
    pDeviceInfo2->device_shortname = "TestDevice2";
    pDeviceInfo2->device_fullname = "Test Device Full Name";
    pDeviceInfo2->driver_enable = 1;
    pDeviceInfo2->device_available = 1;
    pDeviceInfo2->biotype = BIOTYPE_IRIS;
    pDeviceInfo2->stotype = 0;
    pDeviceInfo2->eigtype = 0;
    pDeviceInfo2->vertype = 0;
    pDeviceInfo2->idtype = 0;
    pDeviceInfo2->bustype = 0;
    pDeviceInfo2->dev_status = 0;
    pDeviceInfo2->ops_status = 0;

    DeviceInfoPtr pDeviceInfo3 = std::make_shared<DeviceInfo>();
    pDeviceInfo3->device_id = 103;
    pDeviceInfo3->device_shortname = "TestDevice3";
    pDeviceInfo3->device_fullname = "Test Device Full Name";
    pDeviceInfo3->driver_enable = 1;
    pDeviceInfo3->device_available = 1;
    pDeviceInfo3->biotype = BIOTYPE_FACE;
    pDeviceInfo3->stotype = 0;
    pDeviceInfo3->eigtype = 0;
    pDeviceInfo3->vertype = 0;
    pDeviceInfo3->idtype = 0;   
    pDeviceInfo3->bustype = 0;
    pDeviceInfo3->dev_status = 0;
    pDeviceInfo3->ops_status = 0;

    DeviceInfoPtr pDeviceInfo4 = std::make_shared<DeviceInfo>();
    pDeviceInfo4->device_id = 104;
    pDeviceInfo4->device_shortname = "TestDevice4";
    pDeviceInfo4->device_fullname = "Test Device Full Name";
    pDeviceInfo4->driver_enable = 1;
    pDeviceInfo4->device_available = 1;
    pDeviceInfo4->biotype = BIOTYPE_VOICEPRINT;
    pDeviceInfo4->stotype = 0;
    pDeviceInfo4->eigtype = 0;
    pDeviceInfo4->vertype = 0;
    pDeviceInfo4->idtype = 0;
    pDeviceInfo4->bustype = 0;
    pDeviceInfo4->dev_status = 0;
    pDeviceInfo4->ops_status = 0;

    DeviceInfoPtr pDeviceInfo5 = std::make_shared<DeviceInfo>();
    pDeviceInfo5->device_id = 105;
    pDeviceInfo5->device_shortname = "TestDevice5";
    pDeviceInfo5->device_fullname = "Test Device Full Name";
    pDeviceInfo5->driver_enable = 1;
    pDeviceInfo5->device_available = 1;
    pDeviceInfo5->biotype = 6;
    pDeviceInfo5->stotype = 0;
    pDeviceInfo5->eigtype = 0;
    pDeviceInfo5->vertype = 0;
    pDeviceInfo5->idtype = 0;
    pDeviceInfo5->bustype = 0;
    pDeviceInfo5->dev_status = 0;
    pDeviceInfo5->ops_status = 0;
 
    DeviceInfoPtr pDeviceInfo6 = std::make_shared<DeviceInfo>();
    pDeviceInfo6->device_id = 106;
    pDeviceInfo6->device_shortname = "TestDevice6";
    pDeviceInfo6->device_fullname = "Test Device Full Name";
    pDeviceInfo6->driver_enable = 1;
    pDeviceInfo6->device_available = 1;
    pDeviceInfo6->biotype = 6;
    pDeviceInfo6->stotype = 0;
    pDeviceInfo6->eigtype = 0;
    pDeviceInfo6->vertype = 0;
    pDeviceInfo6->idtype = 0;
    pDeviceInfo6->bustype = 0;
    pDeviceInfo6->dev_status = 0;
    pDeviceInfo6->ops_status = 0;

    DeviceInfoPtr pDeviceInfo7 = std::make_shared<DeviceInfo>();
    pDeviceInfo7->device_id = 107;
    pDeviceInfo7->device_shortname = "TestDevice7";
    pDeviceInfo7->device_fullname = "Test Device Full Name";
    pDeviceInfo7->driver_enable = 1;
    pDeviceInfo7->device_available = 1;
    pDeviceInfo7->biotype = 7;
    pDeviceInfo7->stotype = 0;
    pDeviceInfo7->eigtype = 0;
    pDeviceInfo7->vertype = 0;
    pDeviceInfo7->idtype = 0;
    pDeviceInfo7->bustype = 0;
    pDeviceInfo7->dev_status = 0;
    pDeviceInfo7->ops_status = 0;

    DeviceInfoPtr pDeviceInfo8 = std::make_shared<DeviceInfo>();
    pDeviceInfo8->device_id = 108;
    pDeviceInfo8->device_shortname = "TestDevice8";
    pDeviceInfo8->device_fullname = "Test Device Full Name";
    pDeviceInfo8->driver_enable = 1;
    pDeviceInfo8->device_available = 1;
    pDeviceInfo8->biotype = 8;
    pDeviceInfo8->stotype = 0;
    pDeviceInfo8->eigtype = 0;
    pDeviceInfo8->vertype = 0;
    pDeviceInfo8->idtype = 0;
    pDeviceInfo8->bustype = 0;
    pDeviceInfo8->dev_status = 0;
    pDeviceInfo8->ops_status = 0;

    LoginOptionsWidget loginWidget;
    loginWidget.m_mapDevices[0].push_back(pDeviceInfo); 
    loginWidget.m_mapDevices[1].push_back(pDeviceInfo1); 
    loginWidget.m_mapDevices[2].push_back(pDeviceInfo2); 
    loginWidget.m_mapDevices[3].push_back(pDeviceInfo2); 
    loginWidget.m_mapDevices[4].push_back(pDeviceInfo4); 
    loginWidget.m_mapDevices[5].push_back(pDeviceInfo5); 
    loginWidget.m_mapDevices[6].push_back(pDeviceInfo6); 
    loginWidget.m_mapDevices[7].push_back(pDeviceInfo7); 
    loginWidget.m_mapDevices[8].push_back(pDeviceInfo8);

    loginWidget.addOptionButton(LOGINOPT_TYPE_PASSWORD,0,"LOGINOPT_TYPE_PASSWORD");
    loginWidget.addOptionButton(LOGINOPT_TYPE_PASSWORD,0,"LOGINOPT_TYPE_PASSWORD");
    loginWidget.addOptionButton(LOGINOPT_TYPE_GENERAL_UKEY,1,"LOGINOPT_TYPE_GENERAL_UKEY");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FACE,2,"LOGINOPT_TYPE_FACE");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FINGERPRINT,3,"LOGINOPT_TYPE_FINGERPRINT");
    loginWidget.addOptionButton(LOGINOPT_TYPE_IRIS,4,"LOGINOPT_TYPE_IRIS");
    loginWidget.addOptionButton(LOGINOPT_TYPE_VOICEPRINT,5,"LOGINOPT_TYPE_VOICEPRINT");
    loginWidget.addOptionButton(LOGINOPT_TYPE_FINGERVEIN,6,"LOGINOPT_TYPE_FINGERVEIN");
    loginWidget.addOptionButton(LOGINOPT_TYPE_QRCODE,7,"LOGINOPT_TYPE_QRCODE");

    loginWidget.setCurrentDevice(100);
    int nLoginOptType,nDrvId;
    loginWidget.getCurLoginOpt(nLoginOptType,nDrvId);
 
    loginWidget.getFirstDevInfo();
    loginWidget.updateOptionButtons();
    loginWidget.setSelectedPassword();
    
    loginWidget.startAuth(pDeviceInfo,1000);
    loginWidget.startAuth(pDeviceInfo1,1000);
    loginWidget.startAuth(pDeviceInfo2,1000);
    loginWidget.startAuth(pDeviceInfo3,1000);
    loginWidget.startAuth(pDeviceInfo4,1000);
    loginWidget.startAuth(pDeviceInfo5,1000);
    loginWidget.startAuth(pDeviceInfo6,1000);
    loginWidget.startAuth(pDeviceInfo7,1000);
    loginWidget.startAuth(pDeviceInfo8,1000);

}
 
// 创建一个全局的QApplication对象，因为Qt测试通常需要一个事件循环
int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
