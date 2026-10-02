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
#include "../../ukui-volume-control-applet-qt/ukmedia_main_widget.h"
#include <QApplication>
#include "../../ukui-volume-control-applet-qt/QtSingleApplication/QtSingleApplication"
#include <QTest>

class ukuiMediaTrayTest:public testing::Test
{
protected:
    static void SetUpTestCase() {
        int argc = 0;
        char *argv[] = {};
        app = new QtSingleApplication(argc, argv);
        mediaTray = new UkmediaMainWidget();

        // 因ukui-media与pulseaudio建立连接需要3s，因此需要延迟3.5s执行测试用例
        QTest::qWait(3500);
    }

    static void TearDownTestCase() {

        delete mediaTray;
        delete app;
    }

    static UkmediaMainWidget* mediaTray;
    static QtSingleApplication *app;
};

// 静态成员变量初始化
UkmediaMainWidget* ukuiMediaTrayTest::mediaTray = nullptr;
QtSingleApplication* ukuiMediaTrayTest::app = nullptr;

/* -------------------- UkmediaVolumeControl----------------------- */

//  获取默认输出设备idx测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_getDefaultSinkIndex) {
    int index = mediaTray->m_pVolumeControl->getDefaultSinkIndex();
}

//  获取sink-input静音状态测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_getSinkInputMuted) {
    bool muteState = mediaTray->m_pVolumeControl->getSinkInputMuted("kylin-music");
}

//  设置sink-input音量测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSinkInputVolume) {
    mediaTray->m_pVolumeControl->setSinkInputVolume(9, 30000, 2);
}

//  获取sink-input音量测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_getSinkInputVolume) {
    mediaTray->m_pVolumeControl->getSinkInputVolume("kylin-music");
}

//  设置sink-input静音状态测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSinkInputMuted) {
    mediaTray->m_pVolumeControl->setSinkInputMuted("kylin-music", true);
}

//  更改sink-input默认设备测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_moveSinkInput) {
    mediaTray->m_pVolumeControl->moveSinkInput("kylin-music", "alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp__sink");
}

//  更改source-output默认设备测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_moveSoureOutput) {
    mediaTray->m_pVolumeControl->moveSoureOutput("kylin-recorder", "alsa_input.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp_6__source");
}

//  获取source-output音量测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_getSourceOutputVolume) {
    mediaTray->m_pVolumeControl->getSourceOutputVolume("kylin-recorder");
}

//  设置source-output音量测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSourceOutputVolume) {
    mediaTray->m_pVolumeControl->setSourceOutputVolume(9, 30000, 2);
}

//  设置source-output静音状态测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSourceOutputMuted) {
    mediaTray->m_pVolumeControl->setSourceOutputMuted("kylin-recorder", true);
}

//  设置声卡配置文件测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setCardProfile) {
    mediaTray->m_pVolumeControl->setCardProfile(0, "test");
}

//  设置默认输出设备测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setDefaultSink) {
    mediaTray->m_pVolumeControl->setDefaultSink("test");
}

//  设置默认输入设备测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setDefaultSource) {
    mediaTray->m_pVolumeControl->setDefaultSource("test");
}

//  设置默认输出端口测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSinkPort) {
    mediaTray->m_pVolumeControl->setSinkPort("test", "test");
}

//  设置默认输入端口测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_setSourcePort) {
    mediaTray->m_pVolumeControl->setSourcePort("test", "test");
}

//  kill sink-input 测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_killSinkInput) {
    mediaTray->m_pVolumeControl->killSinkInput(1);
}

//  移除输出设备信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeSink) {
    mediaTray->m_pVolumeControl->removeSink(99);
}

//  移除输入设备信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeSource) {
    mediaTray->m_pVolumeControl->removeSource(99);
}

//  移除输出类应用信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeSinkInput) {
    mediaTray->m_pVolumeControl->removeSinkInput(99);
}

//  获取蓝牙电量测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_getBatteryLevel) {
    mediaTray->m_pVolumeControl->getBatteryLevel("test");
}

//  移除输出端口信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeOutputPortMap) {
    mediaTray->m_pVolumeControl->removeOutputPortMap(99);
}

//  移除输入端口信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeInputPortMap) {
    mediaTray->m_pVolumeControl->removeInputPortMap(99);
}

//  移除声卡信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeCardMap) {
    mediaTray->m_pVolumeControl->removeCardMap(99);
}

//  移除声卡配置文件测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeCardProfileMap) {
    mediaTray->m_pVolumeControl->removeCardProfileMap(99);
}

//  移除输出端口信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeSinkPortMap) {
    mediaTray->m_pVolumeControl->removeSinkPortMap(99);
}

//  移除输入端口信息测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeSourcePortMap) {
    mediaTray->m_pVolumeControl->removeSourcePortMap(99);
}

//  移除声卡配置文件测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeProfileMap) {
    mediaTray->m_pVolumeControl->removeProfileMap(99);
}

//  移除输入配置文件测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_removeInputProfile) {
    mediaTray->m_pVolumeControl->removeInputProfile(99);
}

//  查找输出端口测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_findOutputPort) {
    mediaTray->m_pVolumeControl->findOutputPort("Speaker");
}

//  查找输出端口idx测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_findPortSinkIndex) {
    mediaTray->m_pVolumeControl->findPortSinkIndex("Speaker");
}

//  查找输出端口名测试
TEST_F(ukuiMediaTrayTest, Test_volumeControl_findSinkPortName) {
    mediaTray->m_pVolumeControl->findSinkPortName(0);
}

/* -------------------- UkmediaMainWidget----------------------- */

//  获取默认输入设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultInputDevice) {
    QString defaultInputDev =  mediaTray->getDefaultInputDevice();
    qDebug() << "test getDefaultInputDevice" << defaultInputDev;
}

//  获取默认输出设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultOutputDevice) {
    QString defaultOutputDev =  mediaTray->getDefaultOutputDevice();
    qDebug() << "test getDefaultOutputDevice" << defaultOutputDev;
}

//  获取所有输出设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAllOutputDevices) {
    QStringList devicesList = mediaTray->getAllOutputDevices();
    qDebug() << "test getAllOutputDevices" << devicesList;
}

//  获取所有输入设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAllInputDevices) {
    QStringList devicesList = mediaTray->getAllInputDevices();
    qDebug() << "test getAllInputDevices" << devicesList;
}

//  获取默认输出设备音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultOutputVolume) {
    int mediaVolume = mediaTray->getDefaultOutputVolume();
    qDebug() << "Test: getDefaultOutputVolume" << mediaVolume;
}

//  获取默认输入设备音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultInputVolume) {
    int mediaVolume = mediaTray->getDefaultInputVolume();
    qDebug() << "Test: getDefaultInputVolume" << mediaVolume;
}

//  获取默认输出设备静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultOutputMuteState) {
    bool muteState = mediaTray->getDefaultOutputMuteState();
    qDebug() << "Test: getDefaultOutputMuteState" << muteState;
}

//  获取默认输入设备静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getDefaultInputMuteState) {
    bool muteState = mediaTray->getDefaultInputMuteState();
    qDebug() << "Test: getDefaultInputMuteState" << muteState;
}

//  设置默认输出设备音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setDefaultOutputVolume) {
    bool isSucceed = mediaTray->setDefaultOutputVolume(100);
    qDebug() << "Test: setDefaultOutputVolume" << isSucceed;
}

//  设置默认输入设备音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setDefaultInputVolume) {
    bool isSucceed = mediaTray->setDefaultInputVolume(100);
    qDebug() << "Test: setDefaultInputVolume" << isSucceed;
}

//  设置默认输出设备静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setDefaultOutputMuteState) {
    bool isSucceed = mediaTray->setDefaultOutputMuteState(false);
    qDebug() << "Test: setDefaultOutputMuteState" << isSucceed;
}

//  设置默认输入设备静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setDefaultInputMuteState) {
    bool isSucceed = mediaTray->setDefaultInputMuteState(false);
    qDebug() << "Test: setDefaultInputMuteState" << isSucceed;
}

//  获取系统默认输入设备端口名
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getSystemInputDevice) {
    QString portName = mediaTray->getSystemInputDevice();
    qDebug() << "Test: getSystemInputDevice" << portName;
}

//  获取系统默认输出设备端口名
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getSystemOutputDevice) {
    QString portName = mediaTray->getSystemOutputDevice();
    qDebug() << "Test: getSystemOutputDevice" << portName;
}

//  获取系统当前正在使用音频设备的应用名单列表测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAppList) {
    QStringList appList = mediaTray->getAppList();
    qDebug() << "Test: getAppList" << appList;
}

//  初始化静音灯测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_initVolumeLed) {
    mediaTray->initVolumeLed();
}

//  设置耳机端口测试测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setHeadsetPort) {
    mediaTray->setHeadsetPort("test");
}

//  提示音测试测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_handleTimeout) {
    mediaTray->handleTimeout();
}

//  字体gsettings变化slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_fontSizeChangedSlot) {
    mediaTray->fontSizeChangedSlot("systemFont");
}

//  主题gsettings变化slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_ukuiThemeChangedSlot) {
    mediaTray->ukuiThemeChangedSlot("ukui-light");
}

//  音效主题gsettings变化slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_soundThemeChangedSlot) {
    mediaTray->soundThemeChangedSlot("HeYin");
}

//  系统音量滑动条slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_systemVolumeSliderChangedSlot) {
    mediaTray->systemVolumeSliderChangedSlot(100);
}

//  系统音量滑动条slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_advancedSystemSliderChangedSlot) {
    mediaTray->advancedSystemSliderChangedSlot(100);
}

//  系统静音按钮slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_systemVolumeButtonClickedSlot) {
    mediaTray->systemVolumeButtonClickedSlot();
}

//  系统静音按钮slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_appWidgetMuteButtonCLickedSlot) {
    mediaTray->appWidgetMuteButtonCLickedSlot();
}

//  声音托盘鼠标事件slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_activatedSystemTrayIconSlot) {
    mediaTray->activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason::Trigger);
}

//  声音托盘鼠标事件slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_activatedSystemTrayIconSlot1) {
    mediaTray->activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason::MiddleClick);
}

//  声音托盘鼠标事件slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_activatedSystemTrayIconSlot2) {
    mediaTray->activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason::DoubleClick);
}

//  声音设置跳转slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_jumpControlPanelSlot) {
    mediaTray->jumpControlPanelSlot();
}

TEST_F(ukuiMediaTrayTest, Test_mainWidget_verifyAlsaCard) {
    gboolean hsmic = TRUE;
    gboolean hpmic = TRUE;
    mediaTray->verifyAlsaCard(0, &hsmic, &hpmic);
}

//  鼠标滚轮设置静音状态slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_mouseMeddleClickedTraySlot) {
    mediaTray->mouseMeddleClickedTraySlot();
}

//  蓝牙耳机电量变化slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_batteryLevelChanged) {
    mediaTray->batteryLevelChanged("TEST", 100);
}

//  初始化蓝牙设备音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_initBlueDeviceVolume) {
    mediaTray->initBlueDeviceVolume(99, "TEST");
}

//  绘制图标测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_drawLightColoredPixmap) {
    QSize iconSize(24,24);
    mediaTray->drawLightColoredPixmap(QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(iconSize));
}

//  绘制图标测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_drawDarkColoredPixmap) {
    QSize iconSize(24,24);
    mediaTray->drawDarkColoredPixmap(QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(iconSize));
}

//  计算托盘弹出位置测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_caculatePosition) {
    mediaTray->caculatePosition();
}

//  开机音乐是否开启测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_switchStartupPlayMusic) {
    mediaTray->switchStartupPlayMusic();
}

//  系统声音相关配置gsettings变化测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_soundSettingChanged) {
    mediaTray->soundSettingChanged("TEST");
}

//  s3/s4音频设备挂起测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_onPrepareForSleep) {
    mediaTray->onPrepareForSleep(0);
}

//  选择输出设备slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_outputListWidgetCurrentRowChangedSlot) {
    mediaTray->outputListWidgetCurrentRowChangedSlot(1);
}

//  选择输入设备slot测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_inputListWidgetCurrentRowChangedSlot) {
    mediaTray->inputListWidgetCurrentRowChangedSlot(1);
}

//  更新应用音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_updateAppVolume) {
    mediaTray->updateAppVolume("kylin-music", 100, false);
}

//  字符串整理函数测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_stringRemoveUnrecignizedChar) {
    mediaTray->stringRemoveUnrecignizedChar("test");
}

//  获取蓝牙声卡名测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_blueCardName) {
    mediaTray->blueCardName();
}

//  查找文件函数测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_findFile) {
    mediaTray->findFile("/usr/share/sounds/", "test");
}

TEST_F(ukuiMediaTrayTest, Test_mainWidget_getSessionActive) {
    mediaTray->getSessionActive();
}

//  播放提示音测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_playAlertSound) {
    mediaTray->playAlertSound("audio-volume-change");
}

//  获取输出类应用信息测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getPlaybackAppInfo) {
    mediaTray->getPlaybackAppInfo();
}

//  获取输入类应用信息测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getRecordAppInfo) {
    mediaTray->getRecordAppInfo();
}

//  获取应用音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAppVolume) {
    mediaTray->getAppVolume("kylin-music");
}

//  获取应用静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAppMuteState) {
    mediaTray->getAppMuteState("kylin-music");
}

//  获取应用当前默认输出设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAppOutputDevice) {
    mediaTray->getAppOutputDevice("kylin-music");
}

//  获取应用当前默认输入设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_getAppInputDevice) {
    mediaTray->getAppInputDevice("kylin-recorder");
}

//  设置应用静音状态测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setAppMuteState) {
    mediaTray->setAppMuteState("kylin-music", false);
}

//  设置应用音量测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setAppVolume) {
    mediaTray->setAppVolume("kylin-music", 100);
}

//  设置系统输出设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setSystemOutputDevice) {
    mediaTray->setSystemOutputDevice("test");
}

//  设置系统输入设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setSystemInputDevice) {
    mediaTray->setSystemInputDevice("test");
}

//  设置应用输出设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setAppOutputDevice) {
    mediaTray->setAppOutputDevice("kylin-music", 99, "test");
}

//  设置应用输入设备测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_setAppInputDevice) {
    mediaTray->setAppInputDevice("kylin-recorder", 99, "test");
}

//  设置端口是否需要隐藏测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_isPortHidingNeeded) {
    mediaTray->isPortHidingNeeded(0, 0, "HDMI");
}

//  设置端口是否需要隐藏测试
TEST_F(ukuiMediaTrayTest, Test_mainWidget_isPortHidingNeeded2) {
    mediaTray->isPortHidingNeeded(1, 0, "HDMI");
}

/* -------------------- Bluetooth_Dbus ----------------------- */

//  发送蓝牙耳机电量发生变化信号测试
TEST_F(ukuiMediaTrayTest, Test_sendBatteryChangedSignal) {
    mediaTray->m_pBluetoothDbus->sendBatteryChangedSignal("test", 100);
}

//  获取蓝牙耳机电量测试
TEST_F(ukuiMediaTrayTest, Test_getBatteryLevel) {
    mediaTray->m_pBluetoothDbus->getBatteryLevel("test");
}

/* -------------------- MediaAdaptor ----------------------- */

//  获取所有输入设备测试
TEST_F(ukuiMediaTrayTest, dbus_GetAllInputDevicesTest) {
    QStringList list = mediaTray->m_pMediaDbus->getAllInputDevices();
}

//  获取所有输出设备测试
TEST_F(ukuiMediaTrayTest, dbus_GetAllOutputDevicesTest) {
    QStringList list = mediaTray->m_pMediaDbus->getAllOutputDevices();
}

//  获取默认输出设备测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultOutputDeviceTest) {
    QString device = mediaTray->m_pMediaDbus->getDefaultOutputDevice();
}

//  获取默认输入设备测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultInputDeviceTest) {
    QString device = mediaTray->m_pMediaDbus->getDefaultInputDevice();
}

//  获取默认输入设备静音状态测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultInputMuteStateTest) {
    bool state = mediaTray->m_pMediaDbus->getDefaultInputMuteState();
}

//  获取默认输出设备静音状态测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultOutputMuteStateTest) {
    bool state = mediaTray->m_pMediaDbus->getDefaultOutputMuteState();
}

//  获取默认输入设备音量测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultInputVolumeTest) {
    int value = mediaTray->m_pMediaDbus->getDefaultInputVolume();
}

//  获取默认输出设备音量测试
TEST_F(ukuiMediaTrayTest, dbus_GetDefaultOutputVolumeTest) {
    int value = mediaTray->m_pMediaDbus->getDefaultOutputVolume();
}

//  设置默认输入设备测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultInputDeviceTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultInputDevice("test");
}

//  设置默认输出设备测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultOutputDeviceTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultOutputDevice("test");
}

//  设置默认输入设备音量测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultInputVolumeTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultInputVolume(100);
}

//  设置默认输出设备音量测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultOutputVolumeTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultOutputVolume(100);
}

//  设置默认输入设备静音状态测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultInputMuteStateTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultInputMuteState(true);
}

//  设置默认输出设备静音状态测试
TEST_F(ukuiMediaTrayTest, dbus_SetDefaultOutputMuteStateTest) {
    bool state = mediaTray->m_pMediaDbus->setDefaultOutputMuteState(true);
}

//  获取蓝牙耳机电量测试
TEST_F(ukuiMediaTrayTest, dbus_GetBatteryLevelTest) {
    int battery_value = mediaTray->m_pMediaDbus->getBatteryLevel("TEST");
}

/* -------------------- UkmediaOsdDisplayWidget ----------------------- */

//  获取osd弹窗位置测试
TEST_F(ukuiMediaTrayTest, osd_caculatePositionTest) {
    mediaTray->osdWidget->caculatePosition();
}

//  设置osd弹窗图标测试
TEST_F(ukuiMediaTrayTest, osd_setIconTest) {
    mediaTray->osdWidget->setIcon("audio-headphones-symbolic");
}

//  osd弹窗定时器函数测试
TEST_F(ukuiMediaTrayTest, osd_timeoutHandleTest) {
    mediaTray->osdWidget->timeoutHandle();
}

//  osd弹窗重绘函数测试
TEST_F(ukuiMediaTrayTest, osd_repaintWidgetTest) {
    mediaTray->osdWidget->repaintWidget();
}

//  osd弹窗主题gsetting变化测试
TEST_F(ukuiMediaTrayTest, osd_ukuiThemeChangedSlotTest) {
    mediaTray->osdWidget->ukuiThemeChangedSlot("ukui-light");
}

//  osd弹窗获取透明度测试
TEST_F(ukuiMediaTrayTest, osd_getGlobalOpacityTest) {
    mediaTray->osdWidget->getGlobalOpacity();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_headphoneButtonClickedSlot) {
    mediaTray->headsetWidget->headphoneButtonClickedSlot();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_headsetButtonClickedSlot) {
    mediaTray->headsetWidget->headsetButtonClickedSlot();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_microphoneButtonClickedSlot) {
    mediaTray->headsetWidget->microphoneButtonClickedSlot();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_soundSettingButtonClickedSlot) {
    mediaTray->headsetWidget->soundSettingButtonClickedSlot();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_cancelButtonClickedSlot) {
    mediaTray->headsetWidget->cancelButtonClickedSlot();
}

TEST_F(ukuiMediaTrayTest, Test_HeadsetWidget_cset) {
    mediaTray->headsetWidget->cset("name= Capture Switch", HW_CARD, "0", 0, 0);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
