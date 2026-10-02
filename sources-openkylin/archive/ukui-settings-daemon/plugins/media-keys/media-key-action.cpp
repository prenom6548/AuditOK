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
#include "media-key-action.h"
#include "media-key-manager.h"

static const int AUDIO_SEEK_TIME = 5*1000*1000; //5s

#define TIME_LIMIT(time) \
    static QTime startTime = QTime::currentTime();  \
    static int elapsed = -1;    \
    elapsed = startTime.msecsTo(QTime::currentTime());  \
    if(elapsed > 0 && elapsed <= time){  \
        /*USD_LOG(LOG_ERR,"time in TIME_LIMIT");*/    \
        return; \
    }   \
    startTime = QTime::currentTime();

void MediaKeyAction::doAction(ActionType type,QString action, QString command)
{
    switch(type){
    case TOUCHPAD_KEY:
    case TOUCHPAD_ON_KEY:
    case TOUCHPAD_OFF_KEY:
        doTouchpadAction(type);
        break;
    case MUTE_KEY:
    case VOLUME_DOWN_KEY:
    case VOLUME_UP_KEY:
        doSoundAction(type);
        break;
    case MIC_MUTE_KEY:
        doMicrophonAction();
        break;
    case BRIGHT_UP_KEY:
    case BRIGHT_DOWN_KEY:
        doBrightnessAction(type);
        break;
    case POWER_DOWN_KEY:
        doSessionAction(POWER_SHUTDOWN);
        break;
    case POWER_OFF_KEY:
        doPowerKeyAction();
        break;
    case SHUTDOWN_MANAGEMENT_KEY:
        doSessionAction(POWER_INTER_ACTIVE);
        break;
    case EJECT_KEY:
        break;
    case HOME_KEY:
        doOpenHomeDirAction();
        break;
    case EMAIL_KEY:
        doOpenEvolutionAction();
        break;
    case SCREENSAVER_KEY:
        USD_LOG(LOG_DEBUG,"ready check...%d",type);
        if (!isEnableAction(type)) {
            return;
        }
        doScreensaverAction();
        break;
    case SETTINGS_KEY:
        doSettingsAction();
        break;
    case WINDOWSWITCH_KEY:
        doWindowSwitchAction();
        break;
    case FILE_MANAGER_KEY:
        doOpenFileManagerAction();
        break;
    case PLAY_KEY:
    case PLAYPAUSE_KEY:
        doMultiMediaPlayerAction("playPause");
        break;
    case PAUSE_KEY:
        doMultiMediaPlayerAction("pause");
        break;
    case STOP_KEY:
        doMultiMediaPlayerAction("stop");
        break;
    case PREVIOUS_KEY:
        doMultiMediaPlayerAction("previous");
        break;
    case NEXT_KEY:
        doMultiMediaPlayerAction("next");
        break;
    case REWIND_KEY:
        doMultiMediaPlayerAction("seek", QVariantList({qlonglong(-AUDIO_SEEK_TIME)}));
        break;
    case FORWARD_KEY:
        doMultiMediaPlayerAction("seek", QVariantList({qlonglong(AUDIO_SEEK_TIME)}));
        break;
    case REPEAT_KEY:
        doMultiMediaPlayerAction("repeat");
        break;
    case RANDOM_KEY:
        doMultiMediaPlayerAction("shuffle");
        break;
    case TERMINAL_KEY:
        doOpenTerminalAction();
        break;
    case SCREENSHOT_KEY:
    case AREA_SCREENSHOT_KEY:
    case WINDOW_SCREENSHOT_KEY:
        doScreenshotAction(type);
        break;
    case SYSTEM_MONITOR_KEY:
        doOpenMonitor();
        break;
    case CONNECTION_EDITOR_KEY:
        doOpenNetworkEditor();
        break;
    case GLOBAL_SEARCH_KEY:
        doGlobalSearchAction();
        break;
    case KDS_KEY:
        doOpenKdsAction();
        break;
    case WLAN_KEY:
        doWlanAction();
        break;
    case WEBCAM_KEY:
        doWebcamAction();
        break;
    case UKUI_SIDEBAR:
        doSidebarAction();
        break;
    case UKUI_EYECARE_CENTER:
        doEyeCenterAction();
        break;
    case RFKILL_KEY:
        doFlightModeAction();
        break;
    case CALCULATOR_KEY:
        doOpenCalcAction();
        break;
    case BLUETOOTH_KEY:
        doBluetoothAction();
        break;
    case CURSOR_PROMPT_KEY:
        doLocatePointer();
        break;
    case ASRASSISTANT:
        doOpenAsrAssistant();
        break;
    case PERFORMANCE_KEY:
        doChangePerformanceMode();
        break;
    case WWW_KEY:
        doWWW();
        break;
    case UKUI_ASSISTANT:
        doAssistant();
        break;
    case UKUI_CLIPBOARD:
        doClipboard();
        break;
    case KYLIN_SERVICE_SUPPORT:
        doServiceSupport();
        break;
    case KYLIN_SCREENCAP:
        doKylinScreencap();
        break;
    case KYLIN_AI:
        doAIAssistant();
        break;
    case EXTEND_MEDIAKEYS:
        doExtendMediaKey(command);
        break;
    default:
        break;
    }
}

void MediaKeyAction::doTouchpadAction(ActionType type)
{
    QVariant value = MediaActionSettings::instance()->getTouchpadState();
    if (value.isValid()) {
        bool state  = value.toBool();
        switch (type) {
        case TOUCHPAD_KEY:
            state = !state;
            break;
        case TOUCHPAD_ON_KEY:
            state = true;
            break;
        case TOUCHPAD_OFF_KEY:
            state = false;
            break;
        default:
            break;
        }
        MediaActionSettings::instance()->setTouchpadState(state);
        PopWindowHelper::self()->showCustomOsdByDbus(state ? QStringLiteral("ukui-touchpad-on-symbolic") : QStringLiteral("ukui-touchpad-off-symbolic"));
    }
}

#define MIN_PROGRESSBAR 0
#define MAX_PROGRESSBAR 100
#define VOLUME_STEP 6

void MediaKeyAction::doSoundAction(ActionType type)
{
    int maxVolume = PopWindowHelper::self()->getMaxVolume();
    if (!Sound::self()->isExistSink()) {
        USD_LOG(LOG_NOTICE, "There is no Sink exist!");
        PopWindowHelper::self()->showVolumeBarByDbus(0, maxVolume, true);
        return;
    }

    int volume  = Sound::self()->getSinkVolume();
    bool mute = Sound::self()->getSinkMute();
    USD_LOG(LOG_DEBUG,"get sink volume  : %d" , volume);
    switch(type){
    case MUTE_KEY:
            mute = !mute;
        break;
    case VOLUME_DOWN_KEY:
        volume -= VOLUME_STEP;
        if(volume <= MIN_PROGRESSBAR){
            volume = MIN_PROGRESSBAR;
            mute = true;
        }else{
            mute = false;
        }
        break;
    case VOLUME_UP_KEY:
        if(mute){
            mute = false;
        }
        volume += VOLUME_STEP;
        if (volume >= maxVolume){
            volume = maxVolume;
        }
        break;
    default:break;
    }
    Sound::self()->setSinkVolume(volume);
    Sound::self()->setSinkMute(mute);
    PopWindowHelper::self()->showVolumeBarByDbus(volume, maxVolume, mute);
}

void MediaKeyAction::doMicrophonAction()
{
    if (!Sound::self()->isExistSource()) {
        USD_LOG(LOG_NOTICE, "There is no Mic exist!");
        PopWindowHelper::self()->showCustomOsdByDbus("ukui-microphone-off-symbolic");
        return;
    }

    bool mute = Sound::self()->getSourceMute();
    Sound::self()->setSourceMute(!mute);
    PopWindowHelper::self()->showCustomOsdByDbus(!mute ? "ukui-microphone-off-symbolic" : "ukui-microphone-on-symbolic");
}

void MediaKeyAction::doBrightnessAction(ActionType type)
{
    uint brightness = MediaActionSettings::instance()->updateBrightnessWithAction(type);
    uint maxBrightness = MediaActionSettings::instance()->getMaxBrightness();
    PopWindowHelper::self()->showBrightnessBarByDbus(brightness, maxBrightness);
}

void MediaKeyAction::doSessionAction(PowerType buttonType)
{
    QStringList arguments;

     if (!isEnableAction(buttonType)) {
         return;
     }

    switch (buttonType) {
    case POWER_HIBERNATE:
        arguments << "--hibernate";
        break;
    case POWER_INTER_ACTIVE:
        if (MediaActionSettings::instance()->getSessionState()) {
            USD_LOG(LOG_DEBUG, "session win-key-release is true");
            return;
        }
        break;
    case POWER_SHUTDOWN:
        arguments << "--shutdown";
        break;
    case POWER_SUSPEND:
        arguments << "--suspend";
        break;
    }
    executeCommand("ukui-session-tools", arguments);
}

void MediaKeyAction::doPowerKeyAction()
{
    //间隔 500 ms 内响应一次
    TIME_LIMIT(500);
    QVariant value = MediaActionSettings::instance()->getPowerKeyState();
    PowerType buttonType = POWER_INTER_ACTIVE;
    if (value.isValid()) {
        buttonType = (PowerType)value.toInt();
    } else {
        USD_LOG(LOG_WARNING, "get power key state is null");
    }

    doSessionAction(buttonType);
}

void MediaKeyAction::doOpenFileManagerAction(const QString& path)
{
    QStringList arguments;
    if (!path.isEmpty()) {
        arguments << "--show-folders" << path;
    }
    executeCommand("peony", arguments);
}

void MediaKeyAction::doOpenHomeDirAction()
{
    const QString& homePath = QDir::homePath();
    doOpenFileManagerAction(homePath);
}

void MediaKeyAction::doScreensaverAction()
{
    executeCommand("ukui-screensaver-command", "--lock");
}

void MediaKeyAction::doSettingsAction()
{
    executeCommand("/usr/bin/ukui-control-center");
}

void MediaKeyAction::doOpenCalcAction()
{
    executeCommand("kylin-calculator");
}

void MediaKeyAction::doOpenTerminalAction()
{
    executeCommand("x-terminal-emulator");
}

void MediaKeyAction::doOpenMonitor()
{
    executeCommand("ukui-system-monitor");
}

void MediaKeyAction::doOpenNetworkEditor()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.kylin.network",
                                                          "/com/kylin/network",
                                                          "com.kylin.network",
                                                          "showKylinNM");
    message << 2;
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if(response.type() != QDBusMessage::ReplyMessage){
        executeCommand("kylin-nm", "--sw");
    }
}

void MediaKeyAction::doScreenshotAction(ActionType type)
{
    QStringList arguments;;
    switch (type) {
    case SCREENSHOT_KEY:
        arguments << "full";
        break;
    case AREA_SCREENSHOT_KEY:
        arguments << "gui";
        break;
    case WINDOW_SCREENSHOT_KEY:
        arguments << "screen";
        break;
    default:
        arguments << "gui";
        break;
    }
    executeCommand("kylin-screenshot", arguments);
}

void MediaKeyAction::doMultiMediaPlayerAction(const QString &operation, QVariantList arguments)
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.SettingsDaemon"),
                                                          QStringLiteral("/mpris/controller"),
                                                          QStringLiteral("org.ukui.mpris.controller"),
                                                          QStringLiteral("operation"));
    message << operation << arguments;
    QDBusConnection::sessionBus().asyncCall(message);
}

void MediaKeyAction::doSidebarAction()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.Sidebar",
                                                          "/org/ukui/Sidebar",
                                                          "org.ukui.Sidebar",
                                                          "shortcutsActive");
    QDBusPendingCall response = QDBusConnection::sessionBus().asyncCall(message);
     USD_LOG(LOG_DEBUG,"...");
    if (response.reply().ReplyMessage != QDBusMessage::ReplyMessage){
        executeCommand("ukui-sidebar", QStringList({"-s", "control"}));
      USD_LOG(LOG_DEBUG,"...");
    }
}

void MediaKeyAction::doWindowSwitchAction()
{
    executeCommand("ukui-window-switch", "--show-workspace");
}

void MediaKeyAction::doGlobalSearchAction()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.search.service",
                                                         "/",
                                                         "org.ukui.search.service",
                                                         "mainWindowSwitch");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() != QDBusMessage::ReplyMessage){
        executeCommand("ukui-search", "-s");
    }
}

void MediaKeyAction::doOpenKdsAction()
{
    executeCommand("ukydisplayswitch");
}

void MediaKeyAction::doWlanAction()
{
    if (!RfkillState::self()->isSupportWlan()) {
        USD_LOG(LOG_NOTICE, "Don't support wlan!");
        return;
    }

    bool state = RfkillState::self()->getWlanState();
    QString icon;
    if (state && RfkillState::self()->setWlanState(state)) {
        icon = QStringLiteral("network-wireless-connected-symbolic");
    } else {
        icon = QStringLiteral("network-wireless-offline-symbolic");
    }
    PopWindowHelper::self()->showCustomOsdByDbus(icon);
}

void MediaKeyAction::doAIAssistant()
{
    executeCommand("kylin-aiassistant");
}


void MediaKeyAction::doWebcamAction()
{
    QDBusInterface camera(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                          "/camera", \
                          SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                          QDBusConnection::systemBus());

    QDBusReply<int> reply2 = camera.call("getCameraEnable");

    if (reply2.isValid()) {
        int result = reply2.value();
        if (result < 0) {
            return;
        }
        camera.call("enableCamera", !result);
        PopWindowHelper::self()->showCustomOsdByDbus(!result ? QStringLiteral("camera-switch-symbolic") : QStringLiteral("camera-switch-off-symbolic"));
    } else {
        USD_LOG(LOG_ERR,"camera dbus interface failed .");
    }
}

void MediaKeyAction::doEyeCenterAction()
{
    executeCommand("eye-protection-center");
}

void MediaKeyAction::doFlightModeAction()
{
    int flightState = RfkillState::self()->getFlightState();

    if(flightState == -1) {
        USD_LOG(LOG_ERR,"get flight mode error");
        return;
    }
    flightState = !flightState;

    PopWindowHelper::self()->showCustomOsdByDbus(flightState ? "ukui-airplane-mode-on-symbolic" : "ukui-airplane-mode-off-symbolic");
    RfkillState::self()->setFlightState(flightState);
}

void MediaKeyAction::doBluetoothAction()
{

}

void MediaKeyAction::doOpenEvolutionAction()
{
    executeCommand("evolution");
}

void MediaKeyAction::doLocatePointer()
{
    MediaActionSettings::instance()->setLocatePointer();
}

void MediaKeyAction::doOpenAsrAssistant()
{
    executeCommand("kylin-asrassistant");
}

void MediaKeyAction::doChangePerformanceMode()
{
    int powerMode;
    QString svgName;
    if (!UsdBaseClass::powerModeControlByHardware(powerMode)) {
        powerMode = MediaActionSettings::instance()->getPowerSettings(QStringLiteral("power-policy-battery")).toInt();
        QDBusInterface upowerInterface("org.freedesktop.UPower",
                                       "/org/freedesktop/UPower",
                                       "org.freedesktop.UPower",
                                       QDBusConnection::systemBus());

        if (upowerInterface.isValid()) {
            QString policyKey;
            bool result = upowerInterface.property("OnBattery").toBool();
            if (result == UsdBaseClass::BatteryState::Connected) {  //connected
                policyKey = "power-policy-battery";
            } else {
                policyKey = "power-policy-ac";
            }

            powerMode = MediaActionSettings::instance()->getPowerSettings(policyKey).toInt();
            powerMode = powerMode >= UsdBaseClass::PowerMode::EcoMode ? UsdBaseClass::PowerMode::PerformanceMode : powerMode + 1;
            MediaActionSettings::instance()->setPowerSettings(policyKey, powerMode);
        } else{
            USD_LOG(LOG_DEBUG,"can't get battery state so that we can't changed the power mode");
            return;
        }
    }

    switch (powerMode) {
    case UsdBaseClass::PowerMode::PerformanceMode:
        svgName = "ukui-performance-symbolic";
        break;
    case UsdBaseClass::PowerMode::AutoMode:
        svgName = "ukui-auto-symbolic";
        break;
    case UsdBaseClass::PowerMode::EcoMode:
        svgName = "ukui-eco-symbolic";
        break;
    default:
        return;
        break;
    }

    PopWindowHelper::self()->showCustomOsdByDbus(svgName);
}

void MediaKeyAction::doWWW()
{
    g_app_info_launch_default_for_uri_async("http://", NULL,  NULL,  NULL,  NULL);
}

void MediaKeyAction::doClipboard()
{
    executeCommand("ukui-clipboard");
}

void MediaKeyAction::doServiceSupport()
{
    executeCommand("/usr/bin/kylin-os-manager", "-jumpTab=ServiceSupport");
}

void MediaKeyAction::doKylinScreencap()
{
    executeCommand("/usr/bin/kylin-screencap", "gui");
}

void MediaKeyAction::doAssistant()
{
    executeCommand("kylin-assistant");
}


void MediaKeyAction::doExtendMediaKey(QString command)
{
    USD_LOG_SHOW_PARAMS(command.toLatin1().data());
    QString cmd;
    QStringList args;
    if (command.contains(" ")) {
        QStringList cmdList = command.split(" ");
        cmd = cmdList.takeFirst();
        args.append(cmdList);
    } else {
        cmd = command;
    }

    executeCommand(cmd, args);
}

void MediaKeyAction::executeCommand(const QString& program, const QStringList &arguments)
{
    QProcess process;
    USD_LOG(LOG_DEBUG,"exec:%s, arguments:%s", program.toLatin1().data(), QString(arguments.join(" ")).toLatin1().data());
    process.setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    process.setProgram(program);
    if (!arguments.isEmpty()) {
        process.setArguments(arguments);
    }
    if (!process.startDetached()) {
        USD_LOG(LOG_DEBUG, "%s had a error", program.toLatin1().data());
    }
}

void MediaKeyAction::executeCommand(const QString &program, const QString &argument)
{
    QStringList args;
    if (!argument.isEmpty()) {
        args.append(argument);
    }
    executeCommand(program, args);
}

bool MediaKeyAction::isEnableAction(int action)
{
    bool ret = false;
    QString key = "";

    switch (action) {
    case POWER_SHUTDOWN:
        key = "shutdown";
        break;
    case POWER_HIBERNATE:
        key = "hibernate";
        break;
     case POWER_SUSPEND:
        key = "suspend";
        break;
     case SCREENSAVER_KEY:
        key = "lockscreen";
        break;
    case POWER_INTER_ACTIVE:
        key = "POWER_INTER_ACTIVE";
        break;
    default:
        USD_LOG(LOG_DEBUG,"check %d",action);
        return true;
    }

    QDBusInterface gmInter("org.ukui.SettingsDaemon",
                                  "/GlobalSignal",
                                  "org.ukui.SettingsDaemon.GlobalSignal",
                                  QDBusConnection::sessionBus());
    if(!gmInter.isValid()) {
        USD_LOG(LOG_DEBUG,"interface is unvalid");
        return true;
    }

    QDBusReply<bool> boolRep = gmInter.call("isHidePoweroffUi");
    ret = boolRep.value();
    if (ret) {
        USD_LOG(LOG_DEBUG,"disbale :%d", action);
        return false;
    }

    QDBusReply<QStringList> strListRep = gmInter.call("getShutdownDisableOptions");
    if (strListRep.value().contains(key)) {
        USD_LOG(LOG_DEBUG,"disable key:%s",key.toLatin1().data());
        return false;
    }

    return true;
}
