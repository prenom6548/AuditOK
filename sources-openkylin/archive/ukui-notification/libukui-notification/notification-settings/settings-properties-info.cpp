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

#include "settings-properties-info.h"
#include <QLocale>
#include <QTime>

using namespace UkuiNotification;

class UkuiNotification::SettingsPropertiesInfoPrivate
{
public:
    SettingsProperty::Property m_property;
    QString m_name;
    QString m_displayName;
    QMetaType::Type m_valueType;
    QString m_defaultValue;
    QMetaType::Type m_settingsPropertyMetaType;
};

/*!
    \class UkuiNotification::SettingsPropertiesInfo
    \inmodule UkuiNotification
    \inheaderfile notification-settings/settings-properties-info.h
    \brief 封装通知设置属性的元数据信息

    该类用于存储单个通知设置属性的完整描述信息，包括属性标识符、内部名称、本地化显示名称、
    值类型、默认值等。提供了属性访问、名称到属性的转换、值字符串化等功能，是通知设置系统中
    属性元数据管理的核心类。
 */

/*!
    Constructor
 */
SettingsPropertiesInfo::SettingsPropertiesInfo(): d(new SettingsPropertiesInfoPrivate)
{
    registerSettingsPropertyMetaTypeOnce();
    d->m_property = SettingsProperty::Invalid;
    d->m_name = QStringLiteral("Invalid");
    d->m_valueType = QMetaType::UnknownType;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    d->m_settingsPropertyMetaType = static_cast<QMetaType::Type>(QMetaType::type("UkuiNotification::SettingsProperty::Property"));
#else
    d->m_settingsPropertyMetaType = static_cast<QMetaType::Type>(QMetaType::fromName("UkuiNotification::SettingsProperty::Property").id());
#endif
}

/*!
    通过委托构造调用默认构造函数完成基础初始化，随后根据传入的属性枚举值设置具体的属性元数据，
 */
SettingsPropertiesInfo::SettingsPropertiesInfo(SettingsProperty::Property property)
    : SettingsPropertiesInfo()
{
    d->m_property = property;

    switch (property) {
        case SettingsProperty::ManualDND:
            d->m_name = QStringLiteral("ManualDND");
            d->m_displayName = tr("Manual do not disturb mode");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::DNDSource:
            d->m_name = QStringLiteral("DNDSource");
            d->m_displayName = tr("Do not disturb mode source");
            d->m_valueType = QMetaType::Int;
            d->m_defaultValue = "0";
            break;

        case SettingsProperty::ScheduleTurnOnDND:
            d->m_name = QStringLiteral("ScheduleTurnOnDND");
            d->m_displayName = tr("Schedule turn on");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::ScheduleTurnOnDNDTime:
            d->m_name = QStringLiteral("ScheduleTurnOnDNDTime");
            d->m_displayName = tr("Schedule turn on time");
            d->m_valueType = QMetaType::QTime;
            d->m_defaultValue = "0000";
            break;

        case SettingsProperty::ScheduleTurnOffDNDTime:
            d->m_name = QStringLiteral("ScheduleTurnOffDNDTime");
            d->m_displayName = tr("Schedule turn off time");
            d->m_valueType = QMetaType::QTime;
            d->m_defaultValue = "0000";
            break;

        case SettingsProperty::DNDWhileMultiScreen:
            d->m_name = QStringLiteral("DNDWhileMultiScreen");
            d->m_displayName = tr("Turn on while multi-screen connected");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::DNDWhileFullScreen:
            d->m_name = QStringLiteral("DNDWhileFullScreen");
            d->m_displayName = tr("Turn on in full screen mode");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::NotifyAlarmWhileDND:
            d->m_name = QStringLiteral("NotifyAlarmWhileDND");
            d->m_displayName = tr("Allow alarm to notify in do not disturb mode");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::ReceiveNotificationsFromApps:
            d->m_name = QStringLiteral("ReceiveNotificationsFromApps");
            d->m_displayName = tr("Get notifications from applications");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "true";
            break;

        case SettingsProperty::AllowNotify:
            d->m_name = QStringLiteral("AllowNotify");
            d->m_displayName = tr("Allow notify");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "true";
            break;

        case SettingsProperty::AllowSound:
            d->m_name = QStringLiteral("AllowSound");
            d->m_displayName = tr("Allow sound for notifications");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "true";
            break;

        case SettingsProperty::ShowContentOnLockScreen:
            d->m_name = QStringLiteral("ShowContentOnLockScreen");
            d->m_displayName = tr("Show notification content on lock screen");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

        case SettingsProperty::ShowNotificationOnLockScreen:
            d->m_name = QStringLiteral("ShowNotificationOnLockScreen");
            d->m_displayName = tr("Show notifications on lock screen");
            d->m_valueType = QMetaType::Bool;
            d->m_defaultValue = "false";
            break;

//        case SettingsProperty::PopupStyle:
//            d->m_name = QStringLiteral("popupStyle");
//            d->m_displayName = tr("Style of popup notification");
//            d->m_valueType = d->m_settingsPropertyMetaType;
//            d->m_defaultValue = "TransientPopup";
//            break;
//
//        case SettingsProperty::TransientPopup:
//            d->m_name = QStringLiteral("TransientPopup");
//            d->m_displayName = tr("Popup: displayed in the upper right corner of the screen, will automatically disappear");
//            d->m_valueType = QMetaType::Int;
//            break;
//
//        case SettingsProperty::ResidentPopup:
//            d->m_name = QStringLiteral("ResidentPopup");
//            d->m_displayName = tr("Notice: will remain on the screen until it is closed");
//            d->m_valueType = QMetaType::Int;
//            break;
//
//        case SettingsProperty::NoPopup:
//            d->m_name = QStringLiteral("NoPopup");
//            d->m_displayName = tr("None: notifications will not be displayed on the screen, only be put into notification center");
//            d->m_valueType = QMetaType::Int;
//            break;


        case SettingsProperty::ConfigurationSource:
            d->m_name = QStringLiteral("ConfigurationSource");
            d->m_displayName = tr("Configuration source");
            d->m_valueType = QMetaType::QString;
            d->m_defaultValue = "Default";
            break;
        case SettingsProperty::Invalid:
            d->m_name = QStringLiteral("Invalid");
            d->m_valueType = QMetaType::UnknownType;
            break;
        // NOTE: new properties must also be added to ::fromName()
    }
}

/*!
    Copy Constructor.
 */
SettingsPropertiesInfo::SettingsPropertiesInfo(const SettingsPropertiesInfo& pi)
    : d(new SettingsPropertiesInfoPrivate(*pi.d))
{
}

/*!
    Destructor.
 */
SettingsPropertiesInfo::~SettingsPropertiesInfo() = default;

/*!
    Copy Assignment Operator.
 */
SettingsPropertiesInfo& SettingsPropertiesInfo::operator=(const SettingsPropertiesInfo& rhs)
{
    *d = *rhs.d;
    return *this;
}

/*!
    Equality Comparison Operator.
 */
bool SettingsPropertiesInfo::operator==(const SettingsPropertiesInfo& rhs) const
{
    return d->m_name == rhs.d->m_name && d->m_displayName == rhs.d->m_displayName &&
           d->m_property == rhs.d->m_property && d->m_valueType == rhs.d->m_valueType;
}

/*!
    返回当前属性用于显示显示的名称.
 */
QString SettingsPropertiesInfo::displayName() const
{
    return d->m_displayName;
}

/*!
    返回当前属性的唯一 name.
 */
QString SettingsPropertiesInfo::name() const
{
    return d->m_name;
}

/*!
    返回当前属性的枚举类型.
    \sa SettingsProperty::Property
 */
SettingsProperty::Property SettingsPropertiesInfo::property() const
{
    return d->m_property;
}

/*!
    返回当前属性的值类型.
    \sa QMetaType::Type
 */
QMetaType::Type SettingsPropertiesInfo::valueType() const
{
    return d->m_valueType;
}

/*!
    根据属性 \a name 获取对应的属性数据.
 */
SettingsPropertiesInfo SettingsPropertiesInfo::fromName(const QString& name)
{
    static QHash<QString, SettingsProperty::Property> propertyHash = {
        { QStringLiteral("ManualDND"), SettingsProperty::ManualDND },
        { QStringLiteral("DNDSource"), SettingsProperty::DNDSource },
        { QStringLiteral("ScheduleTurnOnDND"), SettingsProperty::ScheduleTurnOnDND },
        { QStringLiteral("ScheduleTurnOnDNDTime"), SettingsProperty::ScheduleTurnOnDNDTime },
        { QStringLiteral("ScheduleTurnOffDNDTime"), SettingsProperty::ScheduleTurnOffDNDTime },
        { QStringLiteral("DNDWhileMultiScreen"), SettingsProperty::DNDWhileMultiScreen },
        { QStringLiteral("DNDWhileFullScreen"), SettingsProperty::DNDWhileFullScreen },
        { QStringLiteral("NotifyAlarmWhileDND"), SettingsProperty::NotifyAlarmWhileDND },
        { QStringLiteral("ReceiveNotificationsFromApps"), SettingsProperty::ReceiveNotificationsFromApps },
        { QStringLiteral("AllowNotify"), SettingsProperty::AllowNotify },
        { QStringLiteral("AllowSound"), SettingsProperty::AllowSound },
        { QStringLiteral("ShowContentOnLockScreen"), SettingsProperty::ShowContentOnLockScreen },
        { QStringLiteral("ShowNotificationOnLockScreen"), SettingsProperty::ShowNotificationOnLockScreen },
//        { QStringLiteral("popupStyle"), SettingsProperty::PopupStyle },
//        { QStringLiteral("TransientPopup"), SettingsProperty::TransientPopup },
//        { QStringLiteral("ResidentPopup"), SettingsProperty::ResidentPopup },
//        { QStringLiteral("NoPopup"), SettingsProperty::NoPopup }
        { QStringLiteral("ConfigurationSource"), SettingsProperty::ConfigurationSource },

    };

    return {propertyHash.value(name)};
}

/*!
    返回当前属性的默认值.
 */
QString SettingsPropertiesInfo::defaultValue() const
{
    return d->m_defaultValue;
}

/*!
    将属性 \a value 的值转换为字符串.
 */
QString SettingsPropertiesInfo::valueToString(const QVariant &value) const
{
    if(d->m_valueType == d->m_settingsPropertyMetaType) {
        return SettingsPropertiesInfo(static_cast<SettingsProperty::Property>(value.toInt())).name();
    } else if(d->m_valueType == QMetaType::QTime) {
        return value.toTime().toString("HHmm");
    } else {
        return value.toString();
    }
}
