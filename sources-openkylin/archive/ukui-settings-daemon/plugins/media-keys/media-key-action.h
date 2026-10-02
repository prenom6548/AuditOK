/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef MEDIAKEYACTION_H
#define MEDIAKEYACTION_H

#include <QDir>
#include <QTime>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QVariant>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>
#include <algorithm>
#include "sound.h"
#include "media-type.h"
#include "rfkill-state.h"
#include "media-action-settings.h"

#include "QGSettings/qgsettings.h"
#include "widget/pop-window-helper.h"

#include "usd_base_class.h"
extern "C" {
#include "clib-syslog.h"
#include "usd_global_define.h"

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

#ifdef Bool
#undef Bool
#endif
class MediaKeyAction
{
public:
    void doAction(ActionType type,QString action, QString command);
//actions
private:
    /**
     * @brief 触摸板
     */
    void doTouchpadAction(ActionType);
    /**
     * @brief 音量
     */
    void doSoundAction(ActionType);

    /**
     * @brief 麦克风
     */
    void doMicrophonAction();
    /**
     * @brief 亮度
     */
    void doBrightnessAction(ActionType);
    /**
     * @brief powerkey action
     */
//    void doLogoutAction();
    /**
     * @brief session
     */
    void doSessionAction(PowerType);
    /**
     * @brief 电源键
     */
    void doPowerKeyAction();
    /**
     * @brief 文件管理器
     */
    void doOpenFileManagerAction(const QString& path = QString());
    /**
     * @brief 打开家目录
     */
    void doOpenHomeDirAction();
    /**
     * @brief 锁屏
     */
    void doScreensaverAction();
    /**
     * @brief 控制面板
     */
    void doSettingsAction();

    /**
     * @brief 媒体播放器
     */
//    void doMediaAction();
    /**
     * @brief 计算器
     */
    void doOpenCalcAction();
    /**
     * @brief 终端
     */
    void doOpenTerminalAction();
    /**
     * @brief 系统监视器
     */
    void doOpenMonitor();
    /**
     * @brief 网络
     */
    void doOpenNetworkEditor();
    /**
     * @brief 截屏
     */
    void doScreenshotAction(ActionType type);
    /**
     * @brief 浏览器
     */
//    void doUrlAction(const QString);
    /**
     * @brief 多媒体
     */
    void doMultiMediaPlayerAction(const QString&, QVariantList arguments = QVariantList());
    /**
     * @brief 侧边栏
     */
    void doSidebarAction();
    /**
     * @brief 窗口切换
     */
    void doWindowSwitchAction();
    /**
     * @brief 全局搜索
     */
    void doGlobalSearchAction();
    /**
     * @brief 投屏
     */
    void doOpenKdsAction();
    /**
     * @brief wlan
     */
    void doWlanAction();

    /**
     * @brief doAIAssistant 打开AI助手
     */
    void doAIAssistant();

    /**
     * @brief 摄像头
     */
    void doWebcamAction();
    /**
     * @brief 护眼中心
     */
    void doEyeCenterAction();
    /**
     * @brief 飞行模式
     */
    void doFlightModeAction();
    /**
     * @brief 蓝牙
     */
    void doBluetoothAction();
    /**
     * @brief 邮件
     */
    void doOpenEvolutionAction();
    /**
     * @brief 光标提示
     */
    void doLocatePointer();
    /**
     * @brief 语音助手
     */
    void doOpenAsrAssistant();

    /**
     * @brief 电源模式切换
     */
    void doChangePerformanceMode();

    /**
     * @brief doWWW 打开默认浏览器
     */
    void doWWW();

    /**
     * @brief doClipboard 打开剪切板
     */
    void doClipboard();

    /**
     * @brief doAssistant 打开工具箱
     */
    void doAssistant();

    /**
     * @brief doServiceSupport 打开服务与支持
     */
    void doServiceSupport();

    /**
     * @brief doKylinScreencap 唤醒录屏
     */
    void doKylinScreencap();

    /**
     * @brief doExtendMediaKey
     * @param command
     */
    void doExtendMediaKey(QString command);

private:
    void executeCommand(const QString& program, const QStringList &arguments);
    void executeCommand(const QString& program, const QString& argument = QString());
    bool isEnableAction(int action);
    void popWindow(const QString& icon);
};

#endif // MEDIAKEYACTION_H
