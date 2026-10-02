/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef COMMON_H
#define COMMON_H

//Qt
#include <QDebug>
#include <QtDBus>
#include <QtDBus/QDBusConnection>
#include <QProcess>

//gsettings 配置
#define POWER_MANAGER_SETTINGS   "org.ukui.power-manager"
#define SCREEN_SAVER_SETTINGS    "org.ukui.screensaver"
#define SLEEP_DISPLAY_AC_KEY     "sleepDisplayAc"
#define SLEEP_DISPLAY_BAT_KEY    "sleepDisplayBattery"   //空闲关闭显示器
#define SLEEP_COMPUTER_AC        "sleepComputerAc"
#define SLEEP_COMPUTER_BATTERY   "sleepComputerBattery"  //空闲随睡

#define BUTTON_POWER_BATTERY     "buttonPower"     //电源按钮按下执行  "suspend" "shutdown" "interactive"

#define IDLE_BRIGHTNESS          "idleBrightness"        //空闲状态亮度,降低到当前亮度的
#define IDLE_POWER_POLICY_AUTO   "idlePowerPolicyAuto"   //空闲时切换电源策略

constexpr const char* HIBERNATE_COMPUTER_AC_KEY = "hibernateComputerAc";
constexpr const char* HIBERNATE_COMPUTER_BATT_KEY = "hibernateComputerBattery";
constexpr const char* AUTO_BRIGHTNESS_SETTINGS = "org.ukui.SettingsDaemon.plugins.auto-brightness";
constexpr const char* AUTO_BRIGHTNESS_KEY = "autoBrightness"; 


#define SLEEPACTIVATIONENABLED   "sleepActivationEnabled"
#define CLOSEACTIVATIONENABLED   "closeActivationEnabled"

#define BRIGHTNESS_AC                  "brightnessAc"          //亮度值
#define BRIGHTNESS_BATTERY             "brightnessBattery"
#define BRIGHTNESS_ENHANCEMENT_SWITCH  "brightnessEnhancementSwitch"

#define BUTTON_LID_AC            "buttonLidAc"           //盒盖事件触发操作
#define BUTTON_LID_BATTERY       "buttonLidBattery"
#define POWER_POLICY_AC          "powerPolicyAc"         //电源策略(0:性能 1:平衡 2:节能)
#define POWER_POLICY_BATTERY     "powerPolicyBattery"
#define POWER_POLICY_CURRENT     "powerPolicyCurrent"
#define POWER_POLICY_BATTERY_BACKUP     "powerPolicyBatteryBackup"
#define POWER_POLICY_AUTO        "powerPolicyAuto"
#define ON_BATTERY_AUTO_SAVE     "onBatteryAutoSave"     //电池自动开启节能模式
#define LOW_BATTERY_ATUO_SAVE    "lowBatteryAutoSave"    //低电量时自动开启节能模式
#define PERCENTAGE_LOW           "percentageLow"         //低电量百分百通知
#define PERCENTAGE_ACTION        "percentageAction"      //极低电量
#define ACTION_CRITICAL_BATTERY  "actionCriticalBattery" //极低电量时执行
#define DISPLAY_LEFT_TIME        "displayLeftTime"
#define PERCENT_LOW_REDUCE_BRIGHTNESS  "percentageLowReduceBrightness"

#define PRE_POWER_MODE    "preprocessPowerMode"

//dbus 配置
#define GNOME_SESSION_MANAGER               "org.gnome.SessionManager.Presence"
#define SESSION_MANAGER_PATH                "/org/gnome/SessionManager/Presence"
#define POWER_MANAGEMENT_SERVICE            "org.ukui.powermanagement"
#define POWER_MANAGEMENT_PATH               "/"
#define POWER_MANAGEMENT_INTERFACE_NAME     "org.ukui.powermanagement.interface"
#define FREEDESKTOP_UPOWER                  "org.freedesktop.DBus.Properties"

#define UKUI_UPOWER_PATH                    "/upower"
#define UKUI_UPOWER_SERVICE                 "org.ukui.upower"
#define UKUI_UPOWER_INTERFACE               "org.ukui.upower"
#define UKUI_UPOWER_BATTERY_PATH            "/upower/BatteryInfo"
#define UKUI_UPOWER_BATTERY_INTERFACE       "org.ukui.upower.battery"

#define UPOWER_PATH                         "/org/freedesktop/UPower"
#define UPOWER_SERVICE                      "org.freedesktop.UPower"
#define UPOWER_DISPLAY_PATH                 "/org/freedesktop/UPower/devices/DisplayDevice"


//电源管理dbus接口
#define CPU_FREQUENCY_MODULATION     "CpuFrequencyModulation"
#define GET_BRIGHTNESS              "GetBrightness"
#define GET_MAX_BRIGHTNESS          "GetMaxBrightness"
#define GPU_FREQUENCY_MODULATION     "GpuFrequencyModulation"
#define SET_PCIE_ASPM_MODE          "SetPcieAspmMode"
#define SET_DEVICE_POWER_CTL       "SetDevicePowerCtl"
#define SET_AUDIO_MODE              "SetAudioMode"
#define REGULATE_BRIGHTNESS         "RegulateBrightness"
#define IS_BRIGHTNESS_ENHANCEMENT_SUPPORTED  "IsBrightnessEnhancementSupported"

const double BRIGHTNESS_ENHANCEMENT_THRESHOLD_PERCENT = 0.7;
const double BRIGHTNESS_ENHANCEMENT_MAX_PERCENT_150 = 150.0;

#define SYSTEM_IDLE_RESERVE_CPU0    "SystemIdleReserveCpu0"

#define ENTER_TLP                   "enterTlp"
#define EXIT_TLP                    "exitTlp"
#define TLP_TIMER 3500

#define SUSPEND                     "Suspend"
#define TURN_OFF_DISPLAY            "TurnOffDisplay"
#define EXIT_SERVICE                "ExitService"
#define FIRST_RUN_DONOT_SUSPEND     "FirstRunDonotSuspend"

/**
 * UpDeviceState:
 *
 * The device state.
 **/
typedef enum {
    UP_DEVICE_STATE_UNKNOWN,
    UP_DEVICE_STATE_CHARGING,
    UP_DEVICE_STATE_DISCHARGING,
    UP_DEVICE_STATE_EMPTY,
    UP_DEVICE_STATE_FULLY_CHARGED,
    UP_DEVICE_STATE_PENDING_CHARGE,
    UP_DEVICE_STATE_PENDING_DISCHARGE,
    UP_DEVICE_STATE_LAST
} UpDeviceState;


#endif // COMMON_H
