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
#include <QTest>
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../audio/ukmedia_main_widget.h"
#include <QApplication>


class ukccAudioTest:public testing::Test
{
protected:
    static void SetUpTestCase() {
        int argc = 0;
        char *argv[] = {};
        app = new QApplication(argc, argv);
        ukccAudioPlugin = new UkmediaMainWidget();

        // 因ukui-media与pulseaudio建立连接需要3s，因此需要延迟3.5s执行测试用例
        QTest::qWait(3500);
    }

    static void TearDownTestCase() {
        delete ukccAudioPlugin;
        delete app;
    }

    static UkmediaMainWidget* ukccAudioPlugin;
    static QApplication* app;
};

//  静态成员变量初始化测试
UkmediaMainWidget* ukccAudioTest::ukccAudioPlugin = nullptr;
QApplication* ukccAudioTest::app = nullptr;

/* -------------------- UkmediaVolumeControl ----------------------- */

//  获取默认输出设备静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_getSinkMute) {
    bool muteState = ukccAudioPlugin->m_pVolumeControl->getSinkMute();
    qDebug() << "Test: getSinkMute" << muteState;
}

//  获取默认输出设备音量测试
TEST_F(ukccAudioTest, Test_volumeControl_getSinkVolume) {
    int paVolume = ukccAudioPlugin->m_pVolumeControl->getSinkVolume();
    qDebug() << "Test: getSinkVolume" << paVolume;
}

//  获取默认输出设备音量左右声道平衡值测试
TEST_F(ukccAudioTest, Test_volumeControl_getBalanceVolume) {
    float balanceValue = ukccAudioPlugin->m_pVolumeControl->getBalanceVolume();
    qDebug() << "Test: getBalanceVolume" << balanceValue;
}

//  获取默认输出设备索引测试
TEST_F(ukccAudioTest, Test_volumeControl_getDefaultSinkIndex) {
    int index = ukccAudioPlugin->m_pVolumeControl->getDefaultSinkIndex();
    qDebug() << "Test: getDefaultSinkIndex" << index;
}

//  设置默认输出设备静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_setSinkMute) {
    ukccAudioPlugin->m_pVolumeControl->setSinkMute(true);
    qDebug() << "Test: setSinkMute" << ukccAudioPlugin->m_pVolumeControl->getSinkMute();
}

//  设置默认输出设备音量测试
TEST_F(ukccAudioTest, Test_volumeControl_setSinkVolume) {
    int index = ukccAudioPlugin->m_pVolumeControl->getDefaultSinkIndex();
    ukccAudioPlugin->m_pVolumeControl->setSinkVolume(index, 18350);
    qDebug() << "Test: setSinkVolume" << ukccAudioPlugin->m_pVolumeControl->getSinkVolume();
}

//  设置默认输出设备音量左右声道平衡值测试
TEST_F(ukccAudioTest, Test_volumeControl_setBalanceVolume) {
    int index = ukccAudioPlugin->m_pVolumeControl->getDefaultSinkIndex();
    ukccAudioPlugin->m_pVolumeControl->setBalanceVolume(index, 65536, 1.0);
    qDebug() << "Test: setBalanceVolume" << ukccAudioPlugin->m_pVolumeControl->getBalanceVolume();
}

//  获取默认输入设备静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_getSourceMute) {
    bool muteState = ukccAudioPlugin->m_pVolumeControl->getSourceMute();
    qDebug() << "Test: getSourceMute" << muteState;
}

//  获取默认输入设备音量测试
TEST_F(ukccAudioTest, Test_volumeControl_getSourceVolume) {
    int paVolume = ukccAudioPlugin->m_pVolumeControl->getSourceVolume();
    qDebug() << "test getSourceVolume" << paVolume;
}

//  获取默认输入设备索引测试
TEST_F(ukccAudioTest, Test_volumeControl_getDefaultSourceIndex) {
    int index = ukccAudioPlugin->m_pVolumeControl->getDefaultSourceIndex();
    qDebug() << "Test: getDefaultSourceIndex" << index;
}

//  设置默认输入设备静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_setSourceMute) {
    ukccAudioPlugin->m_pVolumeControl->setSourceMute(true);
    qDebug() << "Test: setSourceMute" << ukccAudioPlugin->m_pVolumeControl->getSourceMute();
}

//  设置默认输入设备音量测试
TEST_F(ukccAudioTest, Test_volumeControl_setSourceVolume) {
    int index = ukccAudioPlugin->m_pVolumeControl->getDefaultSourceIndex();
    ukccAudioPlugin->m_pVolumeControl->setSourceVolume(index, 65536);
    qDebug() << "Test: setSourceVolume" << index << ukccAudioPlugin->m_pVolumeControl->getSourceVolume();
}

//  设置sink-input音量测试
TEST_F(ukccAudioTest, Test_volumeControl_setSinkInputVolume) {
    ukccAudioPlugin->m_pVolumeControl->setSinkInputVolume(1, 100);
}

//  设置sink-input静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_setSinkInputMuted) {
    ukccAudioPlugin->m_pVolumeControl->setSinkInputMuted(1, true);
}

//  设置source-output音量测试
TEST_F(ukccAudioTest, Test_volumeControl_setSourceOutputVolume) {
    ukccAudioPlugin->m_pVolumeControl->setSourceOutputVolume(1, 100);
}

//  设置source-output静音状态测试
TEST_F(ukccAudioTest, Test_volumeControl_setSourceOutputMuted) {
    ukccAudioPlugin->m_pVolumeControl->setSourceOutputMuted(1, true);
}

//  设置默认输出设备测试
TEST_F(ukccAudioTest, Test_volumeControl_setDefaultSink) {
    ukccAudioPlugin->m_pVolumeControl->setDefaultSink("TEST");
}

//  设置默认输入设备测试
TEST_F(ukccAudioTest, Test_volumeControl_setDefaultSource) {
    ukccAudioPlugin->m_pVolumeControl->setDefaultSource("TEST");
}

//  设置默认输出设备端口测试
TEST_F(ukccAudioTest, Test_volumeControl_setSinkPort) {
    ukccAudioPlugin->m_pVolumeControl->setSinkPort("TEST", "TEST");
}

//  设置默认输入设备端口测试
TEST_F(ukccAudioTest, Test_volumeControl_setSourcePort) {
    ukccAudioPlugin->m_pVolumeControl->setSourcePort("TEST", "TEST");
}

//  寻找sink活跃端口测试
TEST_F(ukccAudioTest, Test_volumeControl_findSinkActivePortName) {
    ukccAudioPlugin->m_pVolumeControl->findSinkActivePortName("TEST");
}

//  移除输出声卡idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeCardSink) {
    ukccAudioPlugin->m_pVolumeControl->removeCardSink(99, "TEST");
}

//  移除声卡idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeCard) {
    ukccAudioPlugin->m_pVolumeControl->removeCard(99);
}

//  移除输出idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSink) {
    ukccAudioPlugin->m_pVolumeControl->removeSink(99);
}

//  移除输入声卡idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeCardSource) {
    ukccAudioPlugin->m_pVolumeControl->removeCardSource(99, "TEST");
}

//  移除输入idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSource) {
    ukccAudioPlugin->m_pVolumeControl->removeSource(99);
}

//  移除sink-input idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSinkInput) {
    ukccAudioPlugin->m_pVolumeControl->removeSinkInput(99);
}

//  移除source-output idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSourceOutput) {
    ukccAudioPlugin->m_pVolumeControl->removeSourceOutput(99);
}

//  移除客户端idx测试
TEST_F(ukccAudioTest, Test_volumeControl_removeClient) {
    ukccAudioPlugin->m_pVolumeControl->removeClient(99);
}

//  设置连接信息提示测试
TEST_F(ukccAudioTest, Test_volumeControl_setConnectingMessage) {
    ukccAudioPlugin->m_pVolumeControl->setConnectingMessage("TEST");
}

//  获取sink-input音量测试
TEST_F(ukccAudioTest, Test_volumeControl_getSinkInputVolume) {
    ukccAudioPlugin->m_pVolumeControl->getSinkInputVolume("kylin-music");
}

//  获取source-output音量测试
TEST_F(ukccAudioTest, Test_volumeControl_getSourceOutputVolume) {
    ukccAudioPlugin->m_pVolumeControl->getSourceOutputVolume("kylin-recorder");
}

//  移除输入端口信息测试
TEST_F(ukccAudioTest, Test_volumeControl_removeInputPortMap) {
    ukccAudioPlugin->m_pVolumeControl->removeInputPortMap(99);
}

//  移除声卡信息测试
TEST_F(ukccAudioTest, Test_volumeControl_removeCardMap) {
    ukccAudioPlugin->m_pVolumeControl->removeCardMap(99);
}

//  移除声卡配置文件测试
TEST_F(ukccAudioTest, Test_volumeControl_removeCardProfileMap) {
    ukccAudioPlugin->m_pVolumeControl->removeCardProfileMap(99);
}

//  移除输出端口测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSinkPortMap) {
    ukccAudioPlugin->m_pVolumeControl->removeSinkPortMap(99);
}

//  移除输入端口测试
TEST_F(ukccAudioTest, Test_volumeControl_removeSourcePortMap) {
    ukccAudioPlugin->m_pVolumeControl->removeSourcePortMap(99);
}

//  移除声卡配置文件测试
TEST_F(ukccAudioTest, Test_volumeControl_removeProfileMap) {
    ukccAudioPlugin->m_pVolumeControl->removeProfileMap(99);
}

//  移除声卡输入配置文件测试
TEST_F(ukccAudioTest, Test_volumeControl_removeInputProfile) {
    ukccAudioPlugin->m_pVolumeControl->removeInputProfile(99);
}

//  通过输出设备idx获取输出结构体信息测试
TEST_F(ukccAudioTest, Test_volumeControl_getSinkInfoByIndex) {
    ukccAudioPlugin->m_pVolumeControl->getSinkInfoByIndex(0);
}

//  通过输出设备名获取输出结构体信息测试
TEST_F(ukccAudioTest, Test_volumeControl_getSinkInfoByName) {
    ukccAudioPlugin->m_pVolumeControl->getSinkInfoByName("TEST");
}

//  通过输入设备idx获取输入结构体信息测试
TEST_F(ukccAudioTest, Test_volumeControl_getSourceInfoByIndex) {
    ukccAudioPlugin->m_pVolumeControl->getSourceInfoByIndex(0);
}

//  通过输入设备名获取输入结构体信息测试
TEST_F(ukccAudioTest, Test_volumeControl_getSourceInfoByName) {
    ukccAudioPlugin->m_pVolumeControl->getSourceInfoByName("TEST");
}

// -------------------- UkmediaMainWidget-----------------------

//   gsettings值变化测试
TEST_F(ukccAudioTest, Test_mainWidget_onKeyChanged) {
    ukccAudioPlugin->onKeyChanged("TEST");
}

//  开机音乐开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_startupButtonSwitchChangedSlot) {
    ukccAudioPlugin->startupButtonSwitchChangedSlot(true);
}

//  关机音乐开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_poweroffButtonSwitchChangedSlot) {
    ukccAudioPlugin->poweroffButtonSwitchChangedSlot(true);
}

//  注销音乐开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_logoutMusicButtonSwitchChangedSlot) {
    ukccAudioPlugin->logoutMusicButtonSwitchChangedSlot(true);
}

//  唤醒音乐开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_wakeButtonSwitchChangedSlot) {
    ukccAudioPlugin->wakeButtonSwitchChangedSlot(true);
}

//  提示音开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_alertSoundButtonSwitchChangedSlot) {
    ukccAudioPlugin->alertSoundButtonSwitchChangedSlot(true);
}

//  降噪开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_noiseReductionButtonSwitchChangedSlot) {
    ukccAudioPlugin->noiseReductionButtonSwitchChangedSlot(false);
}

//  侦听开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_loopbackButtonSwitchChangedSlot) {
    ukccAudioPlugin->loopbackButtonSwitchChangedSlot(false);
}

//  音量增强开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_volumeIncreaseBtuuonSwitchChangedSlot) {
    ukccAudioPlugin->volumeIncreaseBtuuonSwitchChangedSlot(false);
}

//  开机音乐开关slot测试
TEST_F(ukccAudioTest, Test_mainWidget_bootMusicSettingsChanged) {
    ukccAudioPlugin->bootMusicSettingsChanged("TEST");
}

//  系统主题gsettings变化slot测试
TEST_F(ukccAudioTest, Test_mainWidget_ukuiThemeChangedSlot) {
    ukccAudioPlugin->ukuiThemeChangedSlot("ukui-light");
}

//  获取系统默认输入音量测试
TEST_F(ukccAudioTest, Test_mainWidget_getInputVolume) {
    ukccAudioPlugin->getInputVolume();
}

//  获取系统默认输出音量测试
TEST_F(ukccAudioTest, Test_mainWidget_getOutputVolume) {
    ukccAudioPlugin->getOutputVolume();
}

//  系统全局主题变化slot测试
TEST_F(ukccAudioTest, Test_mainWidget_globalThemeChangedSlot) {
    ukccAudioPlugin->globalThemeChangedSlot("HeYin");
}

//  更新主题测试
TEST_F(ukccAudioTest, Test_mainWidget_updateTheme) {
    ukccAudioPlugin->updateTheme(ukccAudioPlugin);
}

//  查找输出端口测试
TEST_F(ukccAudioTest, Test_mainWidget_findPortSink) {
    ukccAudioPlugin->findPortSink(99, "TEST");
}

//  查找输入端口测试
TEST_F(ukccAudioTest, Test_mainWidget_findPortSource) {
    ukccAudioPlugin->findPortSource(99, "TEST");
}

//  增加‘无’端口测试
TEST_F(ukccAudioTest, Test_mainWidget_addNoneItem) {
    ukccAudioPlugin->addNoneItem(SoundType::SINK);
}

//  增加‘无’端口测试
TEST_F(ukccAudioTest, Test_mainWidget_addNoneItem2) {
    ukccAudioPlugin->addNoneItem(SoundType::SOURCE);
}

//  输入端口索引查询测试
TEST_F(ukccAudioTest, Test_mainWidget_indexOfInputPortInInputCombobox) {
    ukccAudioPlugin->indexOfInputPortInInputCombobox("TEST");
}

//  输入静音按钮slot测试
TEST_F(ukccAudioTest, Test_mainWidget_inputMuteButtonSlot) {
    ukccAudioPlugin->inputMuteButtonSlot();
}

//  输出静音按钮slot测试
TEST_F(ukccAudioTest, Test_mainWidget_outputMuteButtonSlot) {
    ukccAudioPlugin->outputMuteButtonSlot();
}

//  音效主题下拉框slot测试
TEST_F(ukccAudioTest, Test_mainWidget_themeComboxIndexChangedSlot) {
    ukccAudioPlugin->themeComboxIndexChangedSlot(1);
}

//  输出音量滑动条slot测试
TEST_F(ukccAudioTest, Test_mainWidget_outputWidgetSliderChangedSlot) {
    ukccAudioPlugin->outputWidgetSliderChangedSlot(100);
}

//  输出音量滑动条slot测试
TEST_F(ukccAudioTest, Test_mainWidget_outputWidgetSliderChangedSlotInBlue) {
    ukccAudioPlugin->outputWidgetSliderChangedSlotInBlue(100);
}

//  定时器slot测试
TEST_F(ukccAudioTest, Test_mainWidget_timeSliderSlot) {
    ukccAudioPlugin->timeSliderSlot();
}

//  输入音量滑动条slot测试
TEST_F(ukccAudioTest, Test_mainWidget_inputWidgetSliderChangedSlot) {
    ukccAudioPlugin->inputWidgetSliderChangedSlot(100);
}

//  声道平衡滑动条slot测试
TEST_F(ukccAudioTest, Test_mainWidget_balanceSliderChangedMono) {
    ukccAudioPlugin->balanceSliderChangedMono(0.0, 65536, 0);
}

//  声道平衡滑动条slot测试
TEST_F(ukccAudioTest, Test_mainWidget_balanceSliderChangedSlot) {
    ukccAudioPlugin->balanceSliderChangedSlot(0.0);
}

//  寻找输出端口测试
TEST_F(ukccAudioTest, Test_mainWidget_findOutputPortName) {
    ukccAudioPlugin->findOutputPortName(0, "TEST");
}

//  寻找输入端口测试
TEST_F(ukccAudioTest, Test_mainWidget_findInputPortName) {
    ukccAudioPlugin->findInputPortName(0, "TEST");
}

//  寻找高优先级配置文件测试
TEST_F(ukccAudioTest, Test_mainWidget_findHighPriorityProfile) {
    ukccAudioPlugin->findHighPriorityProfile(0, "TEST");
}

//  可用输入设备是否包含蓝牙设备测试
TEST_F(ukccAudioTest, Test_mainWidget_inputComboboxDeviceContainBluetooth) {
    ukccAudioPlugin->inputComboboxDeviceContainBluetooth();
}

//  蓝牙声卡名获取测试
TEST_F(ukccAudioTest, Test_mainWidget_blueCardNameInCombobox) {
    ukccAudioPlugin->blueCardNameInCombobox();
}

//  设置声卡配置文件测试
TEST_F(ukccAudioTest, Test_mainWidget_setCardProfile) {
    ukccAudioPlugin->setCardProfile("TEST", "TEST");
}

//  更新设备端口测试
TEST_F(ukccAudioTest, Test_mainWidget_updateCboxDevicePort) {
    ukccAudioPlugin->updateCboxDevicePort();
}

//  输出设备下拉框slot测试
TEST_F(ukccAudioTest, Test_mainWidget_cboxoutputListWidgetCurrentRowChangedSlot) {
    ukccAudioPlugin->cboxoutputListWidgetCurrentRowChangedSlot();
}

//  输入设备下拉框slot测试
TEST_F(ukccAudioTest, Test_mainWidget_cboxinputListWidgetCurrentRowChangedSlot) {
    ukccAudioPlugin->cboxinputListWidgetCurrentRowChangedSlot();
}

//  自定义音效下拉框slot测试
TEST_F(ukccAudioTest, Test_mainWidget_customSoundEffectsSlot) {
    ukccAudioPlugin->customSoundEffectsSlot(1);
}

//  重置自定义音效测试
TEST_F(ukccAudioTest, Test_mainWidget_resetCustomSoundEffects) {
    ukccAudioPlugin->resetCustomSoundEffects("HeYin", "audio-volume-change");
}

TEST_F(ukccAudioTest, Test_mainWidget_addValue) {
    ukccAudioPlugin->addValue("TEST", "TEST");
}

TEST_F(ukccAudioTest, Test_mainWidget_inputStreamMapCardName) {
    ukccAudioPlugin->inputStreamMapCardName("TEST", "TEST");
}

TEST_F(ukccAudioTest, Test_mainWidget_outputStreamMapCardName) {
    ukccAudioPlugin->outputStreamMapCardName("TEST", "TEST");
}

TEST_F(ukccAudioTest, Test_mainWidget_findInputStreamCardName) {
    ukccAudioPlugin->findInputStreamCardName("TEST");
}

TEST_F(ukccAudioTest, Test_mainWidget_findOutputStreamCardName) {
    ukccAudioPlugin->findOutputStreamCardName("TEST");
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
