/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SHORTCUT_NAVIGATIONS_H__
#define __SHORTCUT_NAVIGATIONS_H__

#include <QObject>
#include <QDBusContext>
#include <QDBusInterface>

//48
class NavigationShortcut: public QObject, QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.ShortcutNavigations")

public:
    explicit NavigationShortcut(QObject *parent = nullptr);
    ~NavigationShortcut();

public Q_SLOTS:

    /**
     * @brief TouchpadToggleState 触摸板状态翻转
     */
    Q_INVOKABLE void TouchpadToggleState();

    /**
     * @brief MuteToggleState 静音状态翻转
     */
    Q_INVOKABLE void MuteToggleState();

    /**
     * @brief VolumeDown 降低音量
     */
    Q_INVOKABLE void VolumeDown();

    /**
     * @brief VolumeUp 增加音量
     */
    Q_INVOKABLE void VolumeUp();

    /**
     * @brief MicMuteToggleState 麦克风状态翻转
     */
    Q_INVOKABLE void MicMuteToggleState();

    /**
     * @brief BrightUp 增大亮度
     */
    Q_INVOKABLE void BrightUp();

    /**
     * @brief BrightDown 减小亮度
     */
    Q_INVOKABLE void BrightDown();

    /**
     * @brief PowerDown 电源按键按下
     */
    Q_INVOKABLE void PowerDown();

    /**
     * @brief PowerOff 关机
     */
    Q_INVOKABLE void PowerOff();

    /**
     * @brief Eject 打开文件管理器
     */
    Q_INVOKABLE void OpenFileManager();

    /**
     * @brief MediaKey 打开默认音频播放器 (usd未实现)
     */
    Q_INVOKABLE void OpenMedia();

    /**
     * @brief OpenCalculator 打开默认计算器
     */
    Q_INVOKABLE void OpenCalculator();

    /**
     * @brief OpenEmail 打开默认邮箱
     */
    Q_INVOKABLE void OpenEmail();

    /**
     * @brief ScreensaverKey 锁屏
     */
    Q_INVOKABLE void OpenScreensaver();

    /**
     * @brief HelpKey
     */
//    Q_INVOKABLE void HelpKey();
    /**
     * @brief WwwKey 打开默认浏览器
     */
    Q_INVOKABLE void OpenWWW();

    /**
     * @brief MediaPlay 多媒体播放
     */
    Q_INVOKABLE void MediaPlay();

    /**
     * @brief MediaPause 多媒体暂停
     */
    Q_INVOKABLE void MediaPause();

    /**
     * @brief MediaPlaypause 多媒体播放、暂停切换
     */
    Q_INVOKABLE void MediaPlaypause();

    /**
     * @brief MediaStop 关闭当前播放内容
     */
    Q_INVOKABLE void MediaStop();

    /**
     * @brief MediaPrevious 前一首
     */
    Q_INVOKABLE void MediaPrevious();

    /**
     * @brief MediaNext 下一首
     */
    Q_INVOKABLE void MediaNext();

    /**
     * @brief MediaRewind 倒带
     */
    Q_INVOKABLE void MediaRewind();

    /**
     * @brief MediaForward 快进
     */
    Q_INVOKABLE void MediaForward();

    /**
     * @brief MediaRepeat 单曲循环
     */
    Q_INVOKABLE void MediaRepeat();

    /**
     * @brief MediaShuffleKey 随机播放
     */
    Q_INVOKABLE void MediaShuffleKey();

    /**
     * @brief CursorPromptKey 强调光标
     */
//    Q_INVOKABLE void CursorPromptKey();

    /**
     * @brief OpenSettings 打开控制面板
     */
    Q_INVOKABLE void OpenSettings();

    /**
     * @brief FileManagerKey 打开文件管理器
     */
    Q_INVOKABLE void FileManagerKey();

    /**
     * @brief ShutdownManagementKey 关机选项
     */
    Q_INVOKABLE void OpenShutdownManagement();
    /**
     * @brief OpenTerminal 打开终端
     */
    Q_INVOKABLE void OpenTerminal();

    /**
     * @brief Screenshot 全屏截图
     */
    Q_INVOKABLE void Screenshot();

    /**
     * @brief WindowScreenshot 窗口截图
     */
    Q_INVOKABLE void WindowScreenshot();

    /**
     * @brief AreaScreenshotKey 区域截图
     */
    Q_INVOKABLE void AreaScreenshot();

    /**
     * @brief WindowSwitchKey 打开多任务视图
     */
    Q_INVOKABLE void OpenWindowSwitch();

    /**
     * @brief SystemMonitorKey 打开系统监视器
     */
    Q_INVOKABLE void SystemMonitorKey();

    /**
     * @brief ConnectionEditorKey 打开网络编辑页面
     */
    Q_INVOKABLE void OpenConnectionEditor();

    /**
     * @brief OpenGlobalSearch 打开全局搜索
     */
    Q_INVOKABLE void OpenGlobalSearch();

    /**
     * @brief OpenDisplaySwitch 打开模式切换器
     */
    Q_INVOKABLE void OpenDisplaySwitch();

    /**
     * @brief ToggleWlanSwitch 翻转网络开关
     */
    Q_INVOKABLE void ToggleWlanSwitch();

    /**
     * @brief CamtoggleState 切换摄像头使能状态
     */
    Q_INVOKABLE void CamToggleState();
//    Q_INVOKABLE void HandledKeys();

    /**
     * @brief OpenUkuiSidebar 打开快捷面板
     */
    Q_INVOKABLE void OpenUkuiSidebar();
//    Q_INVOKABLE void UkuiEyecareCenter();
    /**
     * @brief TouchpadOn 开启触摸板
     */
    Q_INVOKABLE void TouchpadOn();

    /**
     * @brief TouchpadOffKey 关闭触摸板
     */
    Q_INVOKABLE void TouchpadOff();

    /**
     * @brief ToggleFlightmode 切换飞行模式
    */
    Q_INVOKABLE void ToggleFlightModeState();

    /**
     * @brief ToggleFlightmode 开启飞行模式
    */
    Q_INVOKABLE void TurnOnFlightModeState();

    /**
     * @brief ToggleFlightmode 关闭飞行模式
    */
    Q_INVOKABLE void TurnOffFlightModeState();

    /**
     * @brief ToggleBluetoothState 蓝牙状态切换
     */
    Q_INVOKABLE void ToggleBluetoothState();

    /**
     * @brief Asrassistant 打开AI助手
     */
    Q_INVOKABLE void OpenAIassistant();

    /**
     * @brief PerformanceKey 切换能效模式
     */
    Q_INVOKABLE void SwitchPerformanceMode();

    /**
     * @brief UkuiAssistant
     */
//    Q_INVOKABLE void UkuiAssistant();

    /**
     * @brief OpenKylinManager 打开麒麟管家
     */
    Q_INVOKABLE void OpenKylinManager();

    /**
     * @brief OpenUkuiClipboard 打开智能剪切板
     */
    Q_INVOKABLE void OpenClipboard();
    /**
     * @brief KylinScreencap 打开录屏软件
     */
    Q_INVOKABLE void OpenKylinScreencap();
//    Q_INVOKABLE void ExtendMediakeys();

private:
    template <typename...Args>
    QDBusMessage callMediaKeys(const QString &method, Args &&...args);
private:

};

#endif //__SHORTCUT_NAVIGATIONS_H__
