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
#ifndef MEDIAKEYMANAGER_H
#define MEDIAKEYMANAGER_H

#include <QDir>
#include <QObject>
#include <KGlobalAccel>
#include <QDBusConnection>
#include <QTime>
#include <QDebug>

#include "media-key-settings.h"
#include "media-key-binding.h"
#include "media-type.h"
#include "media-key-common.h"
#include "media-key-action.h"
#include "media-action-settings.h"
#include "media-key-cancel.h"
#include "media-key-settings-extend.h"

#include "sound.h"
#include "rfkill-state.h"
#include "usd_base_class.h"
#include "xEventMonitor.h"
#include "widget/pop-window-helper.h"

#include "media-key-block-shutcut.h"
#include "usd_global_define.h"
#include "clib-syslog.h"


#undef Bool
typedef struct{
    QString application;
    uint time;
} MediaPlayer;

class MediaKeyManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.ukui.SettingsDaemon.MediaKeys")
public:
    explicit MediaKeyManager(QObject *parent = nullptr);
    bool start();
    void stop();
    void doAction(ActionType type,QString action = "", QString command = "");
    bool getScreenLockState();
    QMap<QString, QStringList> getBlockList();
public Q_SLOTS:
    void externalDoAction(int type, const QString& app);
    void externalDoActionWithName(QString action, const QString& app);
    int getFlightState();
    void setFlightState(int value);

private:
    void registerDbusService();
    bool blockAction(UsdEnumClass::eActionType actionType);
private:
    /**
     * @brief 加载资源
     */
    void initResources();
    /**
     * @brief 声音
     */
    void initSound();
    /**
     * @brief rfkill
     */
    void initRfkill();
    void initXeventMonitor();

    void disconnectSettings();

    void clearShortcutList();

    void initShortcuts();
    void initStaticShortcuts();
    void initCustomShotrcuts();
    void connectSettings();
    /**
     * @brief 注册快捷键与现有冲突时，强行修改需要先注销冲突快捷键
     * @param shortcuts
     */
    void forcedConflictHandling(QString shortcuts);
private Q_SLOTS:
    void onKeysChanged(const QString& key, const QVariant& value);
    void MMhandleRecordEvent(uint);
    void MMhandleRecordEventRelease(uint);
Q_SIGNALS:
    void setToMiniBrightness();
    void airModeStateChanged(bool state);

private:
    MediaKeyBlockShutcut *p_mBlock;

    QMetaEnum m_actionEnum;

    MediaKeySettings* m_mediaKeySettings = nullptr;
    QList<QSharedPointer<MediaKeyBinding>> m_customShortcuts;
    QList<QSharedPointer<MediaKeyBinding>> m_staticShortcuts;
    //截图
    QList<QSharedPointer<MediaKeyBinding>> m_screenshotShortcuts;
    QList<QSharedPointer<MediaKeyExtendSettings>> m_mediaExtendList;

    xEventHandleHadRelase(MUTE_KEY);
    xEventHandleHadRelase(WLAN_KEY);
    xEventHandleHadRelase(MIC_MUTE_KEY);
    xEventHandleHadRelase(RFKILL_KEY);
    xEventHandleHadRelase(TOUCHPAD_KEY);
    xEventHandleHadRelase(TOUCHPAD_ON_KEY);
    xEventHandleHadRelase(TOUCHPAD_OFF_KEY);
    xEventHandleHadRelase(SCREENSAVER_KEY);
    xEventHandleHadRelase(WINDOWSWITCH_KEY);
    xEventHandleHadRelase(CALCULATOR_KEY);
    xEventHandleHadRelase(BLUETOOTH_KEY);
    xEventHandleHadRelase(WEBCAM_KEY);
    xEventHandleHadRelase(PLAY_KEY);
    xEventHandleHadRelase(STOP_KEY);
    xEventHandleHadRelase(PAUSE_KEY);
    xEventHandleHadRelase(RANDOM_KEY);
    xEventHandleHadRelase(REPEAT_KEY);
    xEventHandleHadRelase(SETTINGS_KEY);
    xEventHandleHadRelase(GLOBAL_SEARCH_KEY);
    xEventHandleHadRelase(MEDIA_KEY);
    xEventHandleHadRelase(EJECT_KEY);
    xEventHandleHadRelase(WWW_KEY);
    xEventHandleHadRelase(HELP_KEY);
    xEventHandleHadRelase(HOME_KEY);
    xEventHandleHadRelase(PERFORMANCE_KEY);
    xEventHandleHadRelase(UKUI_SIDEBAR);
    xEventHandleHadRelase(UKUI_ASSISTANT);
};

#endif // MEDIAKEYMANAGER_H
