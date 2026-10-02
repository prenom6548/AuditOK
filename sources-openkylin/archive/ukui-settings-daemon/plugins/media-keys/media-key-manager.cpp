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
#include "media-key-manager.h"
#include "transform_key.h"
QMap<QString, QStringList> m_blockList;
QMap<QString, QStringList> m_whiteList;
static xEventMonitor* s_xeventMonitor = nullptr;
MediaKeyManager::MediaKeyManager(QObject *parent) : QObject(parent)
{
    p_mBlock = new MediaKeyBlockShutcut(this);
    m_actionEnum = QMetaEnum::fromType<UsdEnumClass::eActionType>();
    m_blockList = p_mBlock->getBlockList();
    connect(p_mBlock, &MediaKeyBlockShutcut::blockChanged, this, [=](QMap<QString, QStringList> list){
        m_blockList = list;
        USD_LOG(LOG_DEBUG, ".");
        qDebug() << m_blockList;

    });

    connect(p_mBlock, &MediaKeyBlockShutcut::unblockChanged, this, [=](QMap<QString, QStringList> list){
        m_whiteList = list;
        USD_LOG(LOG_DEBUG, ".");
        qDebug() << m_whiteList;
    });

    //注销快捷键
    MediaKeyCancel shortcutCancel;
    shortcutCancel.unRegisterAll(QStringLiteral("ukui-settings-daemon"));

    m_actionEnum = QMetaEnum::fromType<UsdEnumClass::eActionType>();
}

bool MediaKeyManager::start()
{

    m_mediaKeySettings = MediaKeySettings::instance(BINDING_SCHEMA);
    if (!m_mediaKeySettings->resultInitSettings()) {
//        USD_LOG(LOG_WARNING,"media key gsettings init failed .");
        return false;
    }

    QStringList mediaSchemas = QGSettings::getAllSchemaWithFilter(MEDIAKEY_SCHEMA_FILTER);
    Q_FOREACH (QString str, mediaSchemas) {
        m_mediaExtendList.append(QSharedPointer<MediaKeyExtendSettings>(new MediaKeyExtendSettings(str, this)));
    }

    initResources();
    connectSettings();
    return true;
}


void MediaKeyManager::stop()
{
    disconnectSettings();
    clearShortcutList();
    if (s_xeventMonitor) {
       s_xeventMonitor->freeXres();
       s_xeventMonitor->quit();
       s_xeventMonitor->wait(500); //会产生死锁，
       s_xeventMonitor->deleteLater();
    }
}

void MediaKeyManager::externalDoAction(int type, const QString& app)
{
    USD_LOG(LOG_DEBUG,"doaction for external app that is %s", app.toLatin1().data());
    MediaKeyAction().doAction(ActionType(type),"","");
}

void MediaKeyManager::externalDoActionWithName(QString action, const QString &app)
{
    bool isSuccess = false;
    int iAction = m_actionEnum.keyToValue(action.toLatin1().data(), &isSuccess);
    if (isSuccess) {
        MediaKeyAction().doAction(ActionType(iAction), "", "");
    }
}

void MediaKeyManager::registerDbusService()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(sessionBus.registerService("org.ukui.SettingsDaemon")){
        sessionBus.registerObject("/org/ukui/SettingsDaemon/MediaKeys", this, QDBusConnection::ExportAllContents);
    }
}


bool MediaKeyManager::blockAction(UsdEnumClass::eActionType actionType)
{
    int unBlock = -1;
    QMetaEnum actionEnum = QMetaEnum::fromType<UsdEnumClass::eActionType>();
    QString blockKey = actionEnum.valueToKey(actionType);


    if (m_whiteList.count() != 0) {
        for (QMap<QString, QStringList>::const_iterator itor = m_whiteList.constBegin(); itor != m_whiteList.constEnd(); ++itor) {

            USD_LOG_SHOW_PARAMS(blockKey.toLatin1().data());
            if (itor.value().contains(blockKey)) {//all 全部屏蔽
                USD_LOG(LOG_DEBUG,"%s unblock:%s", itor.key().toLatin1().data(),
                        blockKey.toLatin1().data());
                unBlock++;
            }
        }

        if (unBlock > -1) {
            return false;
        } else {
            return true;
        }
    }

    for (QMap<QString, QStringList>::const_iterator itor = m_blockList.constBegin(); itor != m_blockList.constEnd(); ++itor)
    {
        if (itor.value().contains(blockKey) || itor.value().contains("all", Qt::CaseInsensitive)) {
            USD_LOG(LOG_DEBUG,"%s block:%s", itor.key().toLatin1().data(),
                    blockKey.toLatin1().data());
            return true;
        }
    }
    return false;
}

void MediaKeyManager::doAction(ActionType type,QString action, QString command)
{
    if (getScreenLockState() || blockAction(UsdEnumClass::eActionType (type))) {
        USD_LOG(LOG_DEBUG, "get screen lock state is true.");
        //锁屏时不作处理
        return;
    }
    MediaKeyAction().doAction(ActionType(type), action, command);
}

bool MediaKeyManager::getScreenLockState()
{
    bool res = false;
    QDBusMessage mesage = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.ScreenSaver"),
                                                         QStringLiteral("/"),
                                                         QStringLiteral("org.ukui.ScreenSaver"),
                                                         QStringLiteral("GetLockState"));
    QDBusMessage response = QDBusConnection::sessionBus().call(mesage);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if (!response.arguments().isEmpty()) {
            res = response.arguments().takeFirst().toBool();
        }
    } else {
        USD_LOG(LOG_DEBUG, "GetLockState called failed");
    }
    return res;
}

QMap<QString, QStringList> MediaKeyManager::getBlockList()
{
    return m_blockList;
}

void MediaKeyManager::initResources()
{
    MediaActionSettings::instance()->initSettings();
    PopWindowHelper::self()->initWindow();
    initSound();
    initRfkill();
    //注册dbus 服务
    registerDbusService();
    initShortcuts();
}

void MediaKeyManager::initSound()
{
    Sound::self();
}

void MediaKeyManager::initRfkill()
{
    RfkillState::self();
    connect(RfkillState::self(), SIGNAL(airModeStateChanged(bool)), this, SIGNAL(airModeStateChanged(bool)));
}

void MediaKeyManager::initXeventMonitor()
{
    if (!s_xeventMonitor) {
        s_xeventMonitor = new xEventMonitor;
        connect(s_xeventMonitor, SIGNAL(keyPress(uint)), this, SLOT(MMhandleRecordEvent(uint)), Qt::QueuedConnection);
        connect(s_xeventMonitor, SIGNAL(keyRelease(uint)), this, SLOT(MMhandleRecordEventRelease(uint)), Qt::QueuedConnection);
    }
}

void MediaKeyManager::MMhandleRecordEvent(uint eventKeysym)
{
    if (!UsdBaseClass::isWayland()) {
        if (eventKeysym == XKB_KEY_XF86AudioMute) {
            xEventHandle(MUTE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86RFKill) {
            xEventHandle(RFKILL_KEY);
        } else if(eventKeysym == XKB_KEY_XF86WLAN) {
            xEventHandle(WLAN_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TouchpadToggle) {
            xEventHandle(TOUCHPAD_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioMicMute) {
            xEventHandle(MIC_MUTE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TouchpadOn) {
            xEventHandle(TOUCHPAD_ON_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TouchpadOff) {
            xEventHandle(TOUCHPAD_OFF_KEY);
        } else if (eventKeysym == XKB_KEY_XF86ScreenSaver) {
            xEventHandle(SCREENSAVER_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TaskPane) {
            xEventHandle(WINDOWSWITCH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Calculator) {
            xEventHandle(CALCULATOR_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Battery) {
            xEventHandle(PERFORMANCE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Bluetooth) {
            xEventHandle(BLUETOOTH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86WebCam) {
            xEventHandle(WEBCAM_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioMicMute) {
            xEventHandle(MIC_MUTE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioMedia) {
            xEventHandle(MEDIA_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioPlay) {
            xEventHandle(PLAY_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioStop) {
            xEventHandle(STOP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioPause) {
            xEventHandle(PAUSE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRepeat) {
            xEventHandle(PAUSE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRandomPlay) {
            xEventHandle(RANDOM_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Tools) {
            xEventHandle(SETTINGS_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Search) {
            xEventHandle(GLOBAL_SEARCH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Explorer) {
            xEventHandle(HOME_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Eject) {
            xEventHandle(EJECT_KEY);
        } else if (eventKeysym == XKB_KEY_XF86WWW) {
            xEventHandle(WWW_KEY);
        } else if (eventKeysym == XKB_KEY_Help) {
            xEventHandle(HELP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Display) {
            doAction(KDS_KEY);
        } else if (eventKeysym == XKB_KEY_XF86PowerDown) {
            doAction(POWER_DOWN_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioPrev) {
            doAction(PREVIOUS_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioNext) {
            doAction(NEXT_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRewind) {
            doAction(REWIND_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioForward) {
            doAction(FORWARD_KEY);
        } else if (eventKeysym == XKB_KEY_XF86PowerOff) {
            doAction(POWER_OFF_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Messenger) {
            //            doAction(UKUI_SIDEBAR);
        } else if (eventKeysym == XKB_KEY_XF86Mail) {
            doAction(EMAIL_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioLowerVolume) {
            doAction(VOLUME_DOWN_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRaiseVolume) {
            doAction(VOLUME_UP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86MonBrightnessDown) {
            doAction(BRIGHT_DOWN_KEY);
        } else if (eventKeysym == XKB_KEY_XF86MonBrightnessUp) {
            doAction(BRIGHT_UP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Launch1) {
            xEventHandle(UKUI_SIDEBAR);
        } else if (eventKeysym == XKB_KEY_XF86Launch2) {
            xEventHandle(UKUI_ASSISTANT);
        } else if (eventKeysym == XKB_KEY_XF86Launch3) {
            doAction(KYLIN_AI);
        }
        //截图
        int modQt;
        bool ok = symXModXToKeyQt(eventKeysym, s_xeventMonitor->getModifier(), &modQt);
        for (auto& shortcut : m_screenshotShortcuts) {
            for (auto& seq : shortcut->shortcuts()) {
                if (ok && seq[0] == modQt) {
                    doAction(shortcut->actionType());
                }
            }
        }
    }
    if(eventKeysym == XKB_KEY_Control_L || eventKeysym == XKB_KEY_Control_R) {
        doAction(CURSOR_PROMPT_KEY);
    }
}

void MediaKeyManager::MMhandleRecordEventRelease(uint eventKeysym)
{
    if (!UsdBaseClass::isWayland()) {
        if (eventKeysym == XKB_KEY_XF86AudioMute) {
            xEventHandleRelease(MUTE_KEY);
        } else if(eventKeysym == XKB_KEY_XF86RFKill) {
            xEventHandleRelease(RFKILL_KEY);
        } else if (eventKeysym == XKB_KEY_XF86WLAN) {
            xEventHandleRelease(WLAN_KEY);
        }else if (eventKeysym == XKB_KEY_XF86TouchpadToggle) {
            xEventHandleRelease(TOUCHPAD_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioMicMute) {
            xEventHandleRelease(MIC_MUTE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TouchpadOn) {
            xEventHandleRelease(TOUCHPAD_ON_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TouchpadOff) {
            xEventHandleRelease(TOUCHPAD_OFF_KEY);
        } else if (eventKeysym == XKB_KEY_XF86ScreenSaver) {
            xEventHandleRelease(SCREENSAVER_KEY);
        } else if (eventKeysym == XKB_KEY_XF86TaskPane) {
            xEventHandleRelease(WINDOWSWITCH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Calculator) {
            xEventHandleRelease(CALCULATOR_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Battery) {
            xEventHandleRelease(PERFORMANCE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Bluetooth) {
            xEventHandleRelease(BLUETOOTH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86WebCam) {
            xEventHandleRelease(WEBCAM_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioMedia) {
            xEventHandleRelease(MEDIA_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioPlay) {
            xEventHandleRelease(PLAY_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioStop) {
            xEventHandleRelease(STOP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioPause) {
            xEventHandleRelease(PAUSE_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRepeat) {
            xEventHandleRelease(REPEAT_KEY);
        } else if (eventKeysym == XKB_KEY_XF86AudioRandomPlay) {
            xEventHandleRelease(RANDOM_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Tools) {
            xEventHandleRelease(SETTINGS_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Search) {
            xEventHandleRelease(GLOBAL_SEARCH_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Explorer) {
            xEventHandleRelease(HOME_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Eject) {
            xEventHandleRelease(EJECT_KEY);
        } else if (eventKeysym == XKB_KEY_XF86WWW) {
            xEventHandleRelease(WWW_KEY);
        } else if (eventKeysym == XKB_KEY_Help) {
            xEventHandleRelease(HELP_KEY);
        } else if (eventKeysym == XKB_KEY_XF86Launch1) {
            xEventHandleRelease(UKUI_SIDEBAR);
        } else if (eventKeysym == XKB_KEY_XF86Launch2) {
            xEventHandleRelease(UKUI_ASSISTANT);
        }
    }
}

//待考虑
void MediaKeyManager::initShortcuts()
{
    if (UsdBaseClass::isWayland()) {
        initStaticShortcuts();
    }
    if (UsdBaseClass::isXcb()) {
        initXeventMonitor();
    }

    //带字符的组合键
    initCustomShotrcuts();
}

void MediaKeyManager::initStaticShortcuts()
{
    for (int i = 0; i < LENGTH(gs_mediaKeyStatic); ++i) {
        if (!gs_mediaKeyStatic[i].defaultKeys.isEmpty()) {
            const QString& name = gs_mediaKeyStatic[i].actionName;
            ActionType type = gs_mediaKeyStatic[i].type;
            QList<QKeySequence> keys = gs_mediaKeyStatic[i].defaultKeys;
            QSharedPointer<MediaKeyBinding> shortcut = QSharedPointer<MediaKeyBinding>(new MediaKeyBinding(name, type, keys, this));
            shortcut->registerGlobalShortcut();
            m_staticShortcuts.append(shortcut);
        }
    }
}

void MediaKeyManager::initCustomShotrcuts()
{
    const QStringList keys = m_mediaKeySettings->getGsettingsKeys();
    for (int i = 0; i < LENGTH(gs_mediaKeyCustom); ++i) {
        const QString& name = gs_mediaKeyCustom[i].actionName;
        if (keys.contains(name)) {
            QString keyStr = m_mediaKeySettings->getGsettingsValue(name).toString();
            ActionType type = gs_mediaKeyCustom[i].type;
            QSharedPointer<MediaKeyBinding> shortcut = QSharedPointer<MediaKeyBinding>(new MediaKeyBinding(name, type, keyStr));
            if (!UsdBaseClass::isWayland()) {
                switch (shortcut->actionType()) {
                //截图
                case SCREENSHOT_KEY:
                case WINDOW_SCREENSHOT_KEY:
                case AREA_SCREENSHOT_KEY:
                    m_screenshotShortcuts.append(shortcut);
                    break;
                default:
                    shortcut->registerGlobalShortcut();
                    m_customShortcuts.append(shortcut);
                    break;
                }
            } else {
                shortcut->registerGlobalShortcut();
                m_customShortcuts.append(shortcut);
            }
        }
    }

    Q_FOREACH(const QSharedPointer<MediaKeyExtendSettings> &settings, m_mediaExtendList) {
        USD_LOG_SHOW_PARAMS(settings->getShortkeyName().toLatin1().data());
        QSharedPointer<MediaKeyBinding> shortcut =
                QSharedPointer<MediaKeyBinding>(new MediaKeyBinding(settings->getShortkeyName(),
                                                                    EXTEND_MEDIAKEYS,
                                                                    settings->getShortkeys(),
                                                                    settings->getShortkeyCommand(),
                                                                    settings->getShortkeySummary(),
                                                                    this));
        shortcut->registerGlobalShortcut();
        m_customShortcuts.append(shortcut);
    }
}

void MediaKeyManager::connectSettings()
{
    connect(m_mediaKeySettings, &MediaKeySettings::keyChanged, this, &MediaKeyManager::onKeysChanged);

    Q_FOREACH(const QSharedPointer<MediaKeyExtendSettings> &settings, m_mediaExtendList) {
         connect(settings.data(), &MediaKeyExtendSettings::keyChanged, this, &MediaKeyManager::onKeysChanged);
    }
}

void MediaKeyManager::forcedConflictHandling(QString shortcuts)
{
    const KeySeqList& _shortcuts = MediaKeyBinding::listFromString(shortcuts);

    for (auto& shortcut: m_customShortcuts) {
        if (shortcut->shortcuts() == _shortcuts) {
            shortcut->unregisterGlobalShortcut();
            USD_LOG(LOG_DEBUG, "If the mandatory shortcut key conflicts with the current shortcut key," \
                               " unregister %s shortcut key .", shortcut->actionName().toLatin1().data());
        }
    }
    //截图
    if (!UsdBaseClass::isWayland()) {
        for (auto& shortcut: m_screenshotShortcuts) {
            if (shortcut->shortcuts() == _shortcuts) {
                shortcut->unregisterGlobalShortcut();
                USD_LOG(LOG_DEBUG, "If the mandatory shortcut key conflicts with the current shortcut key," \
                                   " unregister %s shortcut key .", shortcut->actionName().toLatin1().data());
            }
        }
    }

}

void MediaKeyManager::disconnectSettings()
{
    disconnect(m_mediaKeySettings, &MediaKeySettings::keyChanged, this, &MediaKeyManager::onKeysChanged);

    Q_FOREACH(const QSharedPointer<MediaKeyExtendSettings> &settings, m_mediaExtendList) {
         disconnect(settings.data(), &MediaKeyExtendSettings::keyChanged, this, &MediaKeyManager::onKeysChanged);
    }
}

void MediaKeyManager::onKeysChanged(const QString &key, const QVariant& value)
{

    forcedConflictHandling(value.toString());
    for (auto& shortcut: m_customShortcuts) {
        if (key == shortcut->actionName()) {
            shortcut->unregisterGlobalShortcut();
            shortcut->setShortcuts(value.toString());
            shortcut->registerGlobalShortcut();
            USD_LOG(LOG_DEBUG, "change key action id is %s", key.toLatin1().data());
        }
    }
    //截图
    if (!UsdBaseClass::isWayland()) {

        for (auto& shortcut: m_screenshotShortcuts) {
            if (key == shortcut->actionName()) {
                shortcut->setShortcuts(value.toString());
                USD_LOG(LOG_DEBUG, "change key action id is %s", key.toLatin1().data());
            }
        }
    }
}

void MediaKeyManager::clearShortcutList()
{
    m_staticShortcuts.clear();
    m_customShortcuts.clear();
    //截图
    m_screenshotShortcuts.clear();
}

int  MediaKeyManager::getFlightState()
{
    return RfkillState::self()->getFlightState();
}
void MediaKeyManager::setFlightState(int value)
{
    RfkillState::self()->setFlightState(value);
}

