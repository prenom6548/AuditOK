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

#include "single-application-settings.h"
#include <QJsonObject>
#include <QDebug>
#include "settings-properties-info.h"
#include "applications-settings.h"
#include "security-control-config.h"


namespace UkuiNotification{
class SingleApplicationSettingsPrivate
{
    friend class SingleApplicationSettings;
public:
    SingleApplicationSettingsPrivate();
    SingleApplicationSettingsPrivate(const SingleApplicationSettingsPrivate &other);
    ~SingleApplicationSettingsPrivate();
private:
    QJsonObject m_settings;
    QString m_desktopEntry;
};
}
using namespace UkuiNotification;

SingleApplicationSettingsPrivate::SingleApplicationSettingsPrivate()
{
}

SingleApplicationSettingsPrivate::SingleApplicationSettingsPrivate(const SingleApplicationSettingsPrivate &other)
{
    m_settings = other.m_settings;
    m_desktopEntry = other.m_desktopEntry;
}

SingleApplicationSettingsPrivate::~SingleApplicationSettingsPrivate()
{

}

/*!
    \class UkuiNotification::SingleApplicationSettings
    \inmodule UkuiNotification
    \inheaderfile notification-settings/single-application-settings.h
    \brief 表示单个应用的通知设置
 */

/*!
    Constructor frome desktopEntry.
 */
SingleApplicationSettings::SingleApplicationSettings(const QString &desktopEntry, QObject *parent)
        : QObject(parent)
        , d(new SingleApplicationSettingsPrivate)
{
    d->m_desktopEntry = desktopEntry;
    init();
}

/*!
    Copy Constructor.
 */
SingleApplicationSettings::SingleApplicationSettings(const SingleApplicationSettings &other)
        : d(new SingleApplicationSettingsPrivate(*other.d))
{
    init();
}

/*!
    Copy Assignment Operator.
 */
SingleApplicationSettings &SingleApplicationSettings::operator=(const SingleApplicationSettings &other)
{
    *d = *other.d;
    init();
    return *this;
}

/*!
    Move Assignment Operator.
 */
SingleApplicationSettings &SingleApplicationSettings::operator=(SingleApplicationSettings &&other) Q_DECL_NOEXCEPT
{
    d = other.d;
    other.d = nullptr;
    init();
    return *this;
}

/*!
    Destructor.
 */
SingleApplicationSettings::~SingleApplicationSettings()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

void SingleApplicationSettings::init()
{
    d->m_settings = ApplicationsSettings::self()->getAppSettings(d->m_desktopEntry);
    if(d->m_desktopEntry != QStringLiteral("default") && !d->m_desktopEntry.isEmpty()) {
        connect(ApplicationsSettings::self(), &ApplicationsSettings::applicationUninstalled, this, &SingleApplicationSettings::appUninstalled);
        connect(ApplicationsSettings::self(), &ApplicationsSettings::settingsUpdated, this, &SingleApplicationSettings::settingsDataChanged);
    }
}

/*!
    \property SingleApplicationSettings::allowNotify
    此属性表示应用是否允许发送通知。
 */
bool SingleApplicationSettings::allowNotify() const
{
    if (const auto allowNotify = SecurityControlConfig::instance().isNotificationAllowed(d->m_desktopEntry)) {
        return allowNotify.value();
    }
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::AllowNotify).name()).toVariant().toBool();
}

/*!
    \property SingleApplicationSettings::allowSound
    此属性表示应用是否播放声音。
 */
bool SingleApplicationSettings::allowSound() const
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::AllowSound).name()).toVariant().toBool();
}

/*!
    \property SingleApplicationSettings::showContentOnLockScreen
    此属性表示应用是否在锁屏界面显示通知内容。
 */
bool SingleApplicationSettings::showContentOnLockScreen() const
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::ShowContentOnLockScreen).name()).toVariant().toBool();
}

/*!
    \property SingleApplicationSettings::showNotificationOnLockScreen
    此属性表示应用是否在锁屏界面显示通知。
 */
bool SingleApplicationSettings::showNotificationOnLockScreen() const
{
    if (const auto  showNotificationOnLockScreen = SecurityControlConfig::instance().isShowNotificationOnLockScreenAllowed(d->m_desktopEntry)) {
        return showNotificationOnLockScreen.value();
    }
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::ShowNotificationOnLockScreen).name()).toVariant().toBool();
}

//SettingsProperty::Property UkuiNotification::SingleApplicationSettings::popupStyle() const
//{
//    return SettingsPropertiesInfo::fromName(d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::PopupStyle).name()).toString()).property();
//}

void SingleApplicationSettings::settingsDataChanged(const QString& desktopEntry)
{
    if (desktopEntry != d->m_desktopEntry) {
        return;
    }
    QJsonObject data = ApplicationsSettings::self()->getAppSettings(d->m_desktopEntry);
    if(data.isEmpty()) {
        return;
    }
    for(SettingsProperty::Property property : SINGLE_APPLICATION_SETTINGS) {
        QString name = SettingsPropertiesInfo(property).name();
        QJsonValue value = data.value(name);
        if(d->m_settings.value(name) != value) {
            d->m_settings.insert(name, value);
            switch (property) {
                case SettingsProperty::AllowNotify:
                    Q_EMIT allowNotifyChanged(value.toVariant().toBool());
                    break;
                case SettingsProperty::AllowSound:
                    Q_EMIT allowSoundChanged(value.toVariant().toBool());
                    break;
                case SettingsProperty::ShowContentOnLockScreen:
                    Q_EMIT showContentOnLockScreenChanged(value.toVariant().toBool());
                    break;
                case SettingsProperty::ShowNotificationOnLockScreen:
                    Q_EMIT showNotificationOnLockScreenChanged(value.toVariant().toBool());
                    break;
//                case SettingsProperty::PopupStyle:
//                    Q_EMIT popupStyleChanged(SettingsPropertiesInfo::fromName(value.toString()).property());
//                    break;
                default:
                    break;
            }
        }
    }
}

void SingleApplicationSettings::appUninstalled(const QString &desktopEntry)
{
    if(d->m_desktopEntry == desktopEntry) {
        Q_EMIT uninstalled();
    }
}

void SingleApplicationSettings::setAllowNotify(bool enable)
{
    // 当前配置不允许修改，不写入配置
    if (!isNotificationSwitchEditable()) {
        return;
    }
    qWarning() << d->m_desktopEntry << enable;
    ApplicationsSettings::self()->setAppSetting(d->m_desktopEntry, SettingsProperty::AllowNotify, enable);
}

void SingleApplicationSettings::setAllowSound(bool enable)
{
    ApplicationsSettings::self()->setAppSetting(d->m_desktopEntry, SettingsProperty::AllowSound, enable);
}

void SingleApplicationSettings::setShowContentOnLockScreen(bool enable)
{
    ApplicationsSettings::self()->setAppSetting(d->m_desktopEntry, SettingsProperty::ShowContentOnLockScreen, enable);
}

void SingleApplicationSettings::setShowNotificationOnLockScreen(bool enable)
{
    // 当前配置不允许修改，不写入配置
    if (!isShowNotificationSwitchOnLockScreenEditable()) {
        return;
    }
    ApplicationsSettings::self()->setAppSetting(d->m_desktopEntry, SettingsProperty::ShowNotificationOnLockScreen, enable);
}

bool SingleApplicationSettings::isNotificationSwitchEditable() const
{
    return SecurityControlConfig::instance().isNotificationSwitchEditable(d->m_desktopEntry);
}

bool SingleApplicationSettings::isShowNotificationSwitchOnLockScreenEditable() const
{
    return SecurityControlConfig::instance().isShowNotificationSwitchOnLockScreenEditable(d->m_desktopEntry);
}

//void SingleApplicationSettings::setPopupStyle(SettingsProperty::Property style)
//{
//    ApplicationsSettings::self()->setAppSetting(d->m_desktopEntry, SettingsProperty::PopupStyle, style);
//}

/*!
    返回当前应用的 desktop entry.
 */
QString SingleApplicationSettings::desktopEntry() const
{
    return d->m_desktopEntry;
}
