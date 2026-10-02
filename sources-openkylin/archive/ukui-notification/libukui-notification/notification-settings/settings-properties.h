/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef SETTINGS_PROPERTIES
#define SETTINGS_PROPERTIES
#include <QMap>
#include <QVariant>

namespace UkuiNotification {
namespace SettingsProperty {
Q_NAMESPACE
/**
 * @brief The Property enum contains all settings property types
 *
 */
enum Property {
    Invalid = 0,

    //Global settings about DND mode
    ScheduleTurnOnDND,             //是否在时间段内自动开启勿扰模式，bool类型，默认值false
    ScheduleTurnOnDNDTime,         //勿扰模式开启时间，QDateTime类型，默认值00:00
    ScheduleTurnOffDNDTime,        //勿扰模式结束时间，QDateTime类型，默认值00:00
    DNDWhileMultiScreen,           //多屏启动时是否启动勿扰模式，bool类型，默认值false
    DNDWhileFullScreen,            //全屏状态下是否启动勿扰模式，bool类型，默认值false
    NotifyAlarmWhileDND,           //勿扰模式下是否允许闹钟提示，bool类型，默认值false
    ManualDND,                     //手动勿扰模式开关，bool类型，默认值false
    DNDSource,                     //勿扰模式来源，int类型，0-None 1-Auto 2-Manual，默认值0

    //Show notifications from applications
    ReceiveNotificationsFromApps,       //是否获取来自应用程序的通知，bool类型，默认值true

    //single app notification settings
    AllowNotify = ReceiveNotificationsFromApps + 1,                    //是否接收当前应用通知，bool类型，默认值true
    AllowSound,                     //是否打开通知提示声，bool类型，默认值true
    ShowContentOnLockScreen,        //锁屏界面是否显示通知内容信息，bool类型，默认值false
    ShowNotificationOnLockScreen,   //锁屏界面是否显示通知，bool类型，默认值false
//    PopupStyle,                     //通知样式，appNotificationStyle类型，默认值Mutative
//
//    //single app notification settings中的 popupStyle 类型取值范围
//    TransientPopup = PopupStyle +1,  //横幅模式，显示在右上角，会自动消失
//    ResidentPopup,                   //提示模式，会保留在屏幕上，直到被关闭
//    NoPopup                          //无，通知不会显示在屏幕上，但是会进入通知中心
    ConfigurationSource = 100                    //用于标记配置来源
};
typedef QMap<Property, QVariant> PropertyMap;
}

static const QList<SettingsProperty::Property> GLOBAL_SETTINGS {
            SettingsProperty::ScheduleTurnOnDND,
            SettingsProperty::ScheduleTurnOnDNDTime,
            SettingsProperty::ScheduleTurnOffDNDTime,
            SettingsProperty::DNDWhileMultiScreen,
            SettingsProperty::DNDWhileFullScreen,
            SettingsProperty::NotifyAlarmWhileDND,
            SettingsProperty::ReceiveNotificationsFromApps,
            SettingsProperty::ManualDND,
            SettingsProperty::DNDSource
};
static const QList<SettingsProperty::Property> SINGLE_APPLICATION_SETTINGS {
            SettingsProperty::AllowNotify,
            SettingsProperty::AllowSound,
            SettingsProperty::ShowContentOnLockScreen,
            SettingsProperty::ShowNotificationOnLockScreen,
//            SettingsProperty::PopupStyle,
            SettingsProperty::Property::ConfigurationSource
};
} // namespace UkuiNotification
Q_DECLARE_METATYPE(UkuiNotification::SettingsProperty::Property)

static void registerSettingsPropertyMetaTypeOnce() {
    static bool registered = []() {
        qRegisterMetaType<UkuiNotification::SettingsProperty::Property>("UkuiNotification::SettingsProperty::Property");
        return true;
    }();
}
#endif
