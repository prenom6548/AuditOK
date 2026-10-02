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

#include "notification-global-settings.h"
#include <QJsonObject>
#include <QQmlEngine>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QTimer>
#include "settings-manager.h"
#include "settings-properties.h"
#include "settings-properties-info.h"
#include "security-control-config.h"
namespace UkuiNotification{
class NotificationGlobalSettingsPrivate
{
    friend class NotificationGlobalSettings;
public:
    NotificationGlobalSettingsPrivate(NotificationGlobalSettings *q) : q(q) {}
    ~NotificationGlobalSettingsPrivate()
    {
        if (m_usdInterface) {
            delete m_usdInterface;
            m_usdInterface = nullptr;
        }
        if(m_timeChecktimer) {
            delete m_timeChecktimer;
            m_timeChecktimer = nullptr;
        }
    }

    void setAutoDND(bool isAutoDND);
    void setManualDND(bool isDND);
    void overrideDND(bool isDND);
    void setDNDSource(NotificationGlobalSettings::DNDSource source);
    NotificationGlobalSettings::DNDSource dndSource() const;
    void setCachedSetting(SettingsProperty::Property property, const QVariant &value);
private:
    NotificationGlobalSettings *q;
    QJsonObject m_settings;
    QDBusInterface *m_usdInterface = nullptr;
    int m_currentScreenMode = 0; //0-仅主屏；1-镜像；2-扩展；3-仅副屏
    QTimer *m_timeChecktimer = nullptr;
    bool m_isDnD = false;
};

void NotificationGlobalSettingsPrivate::setAutoDND(bool isAutoDND)
{
    setDNDSource(NotificationGlobalSettings::Auto);
    overrideDND(isAutoDND);
}

void NotificationGlobalSettingsPrivate::setManualDND(bool isDND)
{
    //先更新缓存后写入文件
    setCachedSetting(SettingsProperty::ManualDND, isDND);
    setCachedSetting(SettingsProperty::DNDSource, NotificationGlobalSettings::Manual);
    SettingsManager::self()->setGlobalSettings({{SettingsProperty::ManualDND, isDND},
                                                         {SettingsProperty::DNDSource, NotificationGlobalSettings::Manual}});
    overrideDND(isDND);
}

void NotificationGlobalSettingsPrivate::overrideDND(bool isDND)
{
    if (m_isDnD != isDND) {
        m_isDnD = isDND;
        Q_EMIT q->dndChanged(isDND);
    }
}

void NotificationGlobalSettingsPrivate::setDNDSource(NotificationGlobalSettings::DNDSource source)
{
    if (dndSource() != source) {
        setCachedSetting(SettingsProperty::DNDSource, source);
        SettingsManager::self()->setGlobalSettings(SettingsProperty::DNDSource, source);
    }
}

NotificationGlobalSettings::DNDSource NotificationGlobalSettingsPrivate::dndSource() const
{
    return static_cast<NotificationGlobalSettings::DNDSource>(
        m_settings.value(SettingsPropertiesInfo(SettingsProperty::DNDSource).name()).toVariant().toInt());
}

void NotificationGlobalSettingsPrivate::setCachedSetting(SettingsProperty::Property property, const QVariant& value)
{
    SettingsPropertiesInfo info(property);
    m_settings.insert(info.name(), info.valueToString(value));
}
}

using namespace UkuiNotification;// UkuiNotification

/*!
    \class UkuiNotification::NotificationGlobalSettings
    \inmodule UkuiNotification
    \inheaderfile notification-settings/notification-global-settings.h
    \brief 提供通知系统的全局配置管理功能.

    该类负责管理通知系统的全局设置，包括勿扰模式(DND)的配置、时间设置以及屏幕状态相关的设置。
    主要功能包括：
    \list
    \li 勿扰模式的开启/关闭状态管理
    \li 勿扰模式计划开启/关闭时间设置
    \li 多屏幕和全屏状态下的勿扰模式设置
    \li 通知接收和闹钟在勿扰模式下警报设置
    \endlist
 */

/*!
    Constructor
 */
NotificationGlobalSettings::NotificationGlobalSettings(QObject *parent) : QObject(parent), d(new NotificationGlobalSettingsPrivate(this))
{
    qmlRegisterUncreatableType<UkuiNotification::NotificationGlobalSettings>("org.ukui.notification.client", 1, 0, "NotificationGlobalSettings", "");
    connect(SettingsManager::self(), &SettingsManager::settingsDataChanged, this, &NotificationGlobalSettings::settingsDataChanged);
    d->m_settings = SettingsManager::self()->getGlobalSettings();
    d->m_usdInterface = new QDBusInterface("org.ukui.SettingsDaemon",
                                           "/org/ukui/SettingsDaemon/xrandr",
                                           "org.ukui.SettingsDaemon.xrandr");
    if(!d->m_usdInterface->isValid()) {
        qWarning() << "NotificationGlobalSettings: Fail to creat org.ukui.SettingsDaemon dbus interface!";
        return;
    }
    QDBusReply<int> reply = d->m_usdInterface->call("getScreenMode", "ukui-notification-server");
    if (reply.isValid()) {
        d->m_currentScreenMode = reply.value();
    } else {
        qWarning()<<"NotificationGlobalSettings: Call getScreenMode failed!";
    }
    connect(d->m_usdInterface, SIGNAL(screenModeChanged(int)), this, SLOT(screenModeChanged(int)));

    d->m_timeChecktimer = new QTimer;
    d->m_timeChecktimer->setSingleShot(true);
    //设置为精密定时器
    d->m_timeChecktimer->setTimerType(Qt::PreciseTimer);
    connect(d->m_timeChecktimer, &QTimer::timeout, this, &NotificationGlobalSettings::checkTimeForDND);

    d->m_isDnD = isDND();
    updateDNDTime();
}

/*!
    Destructor
 */
NotificationGlobalSettings::~NotificationGlobalSettings()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \property NotificationGlobalSettings::scheduleTurnOnDND
    此属性管理勿扰模式计划开关，开启之后会根据计划时间开启/关闭勿扰模式.

    \sa scheduleTurnOnDNDTime, scheduleTurnOffDNDTime
 */
bool NotificationGlobalSettings::scheduleTurnOnDND()
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::ScheduleTurnOnDND).name()).toVariant().toBool();
}

/*!
    \property NotificationGlobalSettings::scheduleTurnOnDNDTime
    此属性为勿扰模式计划开启时间

    \sa scheduleTurnOffDNDTime
 */
QTime NotificationGlobalSettings::scheduleTurnOnDNDTime()
{
    SettingsPropertiesInfo dndOnTimeInfo(SettingsProperty::ScheduleTurnOnDNDTime);
    QString dndOnTimeStr = d->m_settings.value(dndOnTimeInfo.name()).toString();
    // 字符串为空
    if (dndOnTimeStr.isEmpty()) {
        qWarning() << "ScheduleTurnOnDNDTime is empty, using default value:" << dndOnTimeInfo.defaultValue();
        return QTime::fromString(dndOnTimeInfo.defaultValue(), "HHmm");
    }

    static const QStringList timeFormats = {"HHmm", "HH:mm"};
    //解析时间字符串
    for (const auto &format : timeFormats) {
        QTime dndOnTime = QTime::fromString(dndOnTimeStr, format);
        if (dndOnTime.isValid()) {
            return dndOnTime;
        }
    }
    qWarning() << "Invalid ScheduleTurnOnDNDTime format:" << dndOnTimeStr << ", using default value:" << dndOnTimeInfo.defaultValue();
    return QTime::fromString(dndOnTimeInfo.defaultValue(), "HHmm");
}
/*!
    \property NotificationGlobalSettings::scheduleTurnOffDNDTime
    此属性为勿扰模式计划关闭时间

    \sa scheduleTurnOnDNDTime
 */
QTime NotificationGlobalSettings::scheduleTurnOffDNDTime()
{
    SettingsPropertiesInfo dndOffTimeInfo(SettingsProperty::ScheduleTurnOffDNDTime);
    QString dndOffTimeStr = d->m_settings.value(dndOffTimeInfo.name()).toString();
    if (dndOffTimeStr.isEmpty()) {
        qWarning() << "ScheduleTurnOffDNDTime is empty, using default value:" << dndOffTimeInfo.defaultValue();
        return QTime::fromString(dndOffTimeInfo.defaultValue(), "HHmm");
    }

    static const QStringList timeFormats = {"HHmm", "HH:mm"};

    for (const auto &format : timeFormats) {
        QTime dndOffTime = QTime::fromString(dndOffTimeStr, format);
        if (dndOffTime.isValid()) {
            return dndOffTime;
        }
    }
    qWarning() << "Invalid ScheduleTurnOffDNDTime format:" << dndOffTimeStr << ", using default value:" << dndOffTimeInfo.defaultValue();
    return QTime::fromString(dndOffTimeInfo.defaultValue(), "HHmm");
}

/*!
    \property NotificationGlobalSettings::DNDWhileMultiScreen
    此属性表示多屏幕下是否进入勿扰模式开关

    \sa DNDWhileFullScreen
 */
bool NotificationGlobalSettings::DNDWhileMultiScreen()
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::DNDWhileMultiScreen).name()).toVariant().toBool();
}
/*!
    \property NotificationGlobalSettings::DNDWhileFullScreen
    此属性表示全屏下是否进入勿扰模式开关

    \sa DNDWhileMultiScreen
 */
bool NotificationGlobalSettings::DNDWhileFullScreen()
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::DNDWhileFullScreen).name()).toVariant().toBool();
}

/*!
    \property NotificationGlobalSettings::notifyAlarmWhileDND
    此属性表示勿扰模式下是否允许闹钟提示
 */
bool NotificationGlobalSettings::notifyAlarmWhileDND()
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::NotifyAlarmWhileDND).name()).toVariant().toBool();
}

/*!
    \property NotificationGlobalSettings::receiveNotificationsFromApps
    此属性表示是否允许来自应用的通知
 */
bool NotificationGlobalSettings::receiveNotificationsFromApps()
{
    if (!SecurityControlConfig::instance().isEnabledAllNotification()) {
        return false;
    }
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::ReceiveNotificationsFromApps).name()).toVariant().toBool();
}

void NotificationGlobalSettings::settingsDataChanged()
{
    QJsonObject data = SettingsManager::self()->getGlobalSettings();
    for(SettingsProperty::Property property : GLOBAL_SETTINGS) {
        QString name = SettingsPropertiesInfo(property).name();
        QJsonValue value = data.value(name);
        if(d->m_settings.value(name) != value) {
            d->m_settings.insert(name, value);
            switch (property) {
                case SettingsProperty::ScheduleTurnOnDND:
                    Q_EMIT scheduleTurnOnDNDChanged(value.toVariant().toBool());
                    checkTimeForDND();
                    break;
                case SettingsProperty::ScheduleTurnOnDNDTime:
                    Q_EMIT scheduleTurnOnDNDTimeChanged(QTime::fromString(value.toString(), "HHmm"));
                    checkTimeForDND();
                    break;
                case SettingsProperty::ScheduleTurnOffDNDTime:
                    Q_EMIT scheduleTurnOffDNDTimeChanged(QTime::fromString(value.toString(), "HHmm"));
                    checkTimeForDND();
                    break;
                case SettingsProperty::DNDWhileMultiScreen:
                    Q_EMIT DNDWhileMultiScreenChanged(value.toVariant().toBool());
                    checkDNDMode();
                    break;
                case SettingsProperty::DNDWhileFullScreen:
                    Q_EMIT DNDWhileFullScreenChanged(value.toVariant().toBool());
                    checkDNDMode();
                    break;
                case SettingsProperty::NotifyAlarmWhileDND:
                    Q_EMIT notifyAlarmWhileDNDChanged(value.toVariant().toBool());
                    break;
                case SettingsProperty::ReceiveNotificationsFromApps:
                    Q_EMIT receiveNotificationsFromAppsChanged(value.toVariant().toBool());
                    break;
                case SettingsProperty::ManualDND:
                case SettingsProperty::DNDSource:
                    d->overrideDND(isDND());
                    break;
                default:
                    break;
            }
        }
    }
}

void NotificationGlobalSettings::setScheduleTurnOnDND(bool enable)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::ScheduleTurnOnDND, enable);
}

void NotificationGlobalSettings::setScheduleTurnOnDNDTime(const QTime &time)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::ScheduleTurnOnDNDTime, time);
}

void NotificationGlobalSettings::setScheduleTurnOffDNDTime(const QTime &time)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::ScheduleTurnOffDNDTime, time);
}

void NotificationGlobalSettings::setDNDWhileMultiScreen(bool enable)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::DNDWhileMultiScreen, enable);
}

void NotificationGlobalSettings::setDNDWhileFullScreen(bool enable)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::DNDWhileFullScreen, enable);
}

void NotificationGlobalSettings::setNotifyAlarmWhileDND(bool enable)
{
    SettingsManager::self()->setGlobalSettings(SettingsProperty::NotifyAlarmWhileDND, enable);
}

void NotificationGlobalSettings::setReceiveNotificationsFromApps(bool enable)
{
    if (!SecurityControlConfig::instance().isEnabledAllNotification()) {
        return;
    }
    SettingsManager::self()->setGlobalSettings(SettingsProperty::ReceiveNotificationsFromApps, enable);
}

/*!
    \readonly
    \property NotificationGlobalSettings::DND
    此属性表示当前是否为勿扰模式
 */
bool NotificationGlobalSettings::isDND()
{
    switch (d->dndSource()) {
    case Manual:
        return manualDND();
    case Auto:
    default:
        return autoDND();
    }
}

bool NotificationGlobalSettings::manualDND()
{
    return d->m_settings.value(SettingsPropertiesInfo(SettingsProperty::ManualDND).name()).toVariant().toBool();
}

bool NotificationGlobalSettings::autoDND()
{
    bool dnd = false;
    if(scheduleTurnOnDND()) {
        const QTime currentTime = QTime::currentTime();
        const QTime dndOnTime = scheduleTurnOnDNDTime();
        const QTime dndOffTime = scheduleTurnOffDNDTime();
        if (dndOnTime == dndOffTime) {
            //开始结束时间一致，全天都为勿扰模式
            dnd = true;
        } else if (dndOnTime > dndOffTime) {
            dnd = (currentTime >= dndOnTime) || (currentTime < dndOffTime);
        } else {
            dnd = (currentTime >= dndOnTime) && (currentTime < dndOffTime);
        }
    }
    if(!dnd) {
        if(DNDWhileMultiScreen()) {
            if(d->m_currentScreenMode == 1 || d->m_currentScreenMode == 2) {
                dnd = true;
            }
        }
    }
    if(!dnd) {
        if(DNDWhileFullScreen()) {
            QDBusReply<bool> registeredReply = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("com.kylin.Wlcom"));
            if (registeredReply.isValid() && registeredReply.value()) {
                QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("com.kylin.Wlcom"),
                                                                  QStringLiteral("/com/kylin/Wlcom/View"),
                                                                  QStringLiteral("com.kylin.Wlcom.View"),
                                                                  QStringLiteral("HaveFullScreenActiveWindow"));
                QDBusReply<bool> reply  = QDBusConnection::sessionBus().call(msg);
                if(reply.isValid()) {
                    dnd = reply.value();
                }
            } else {
                QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                  QStringLiteral("/KWin"),
                                                                  QStringLiteral("org.kde.KWin"),
                                                                  QStringLiteral("haveFullScreenActiveWindow"));
                QDBusReply<bool> reply  = QDBusConnection::sessionBus().call(msg);
                if(reply.isValid()) {
                    dnd = reply.value();
                }
            }
        }
    }
    return dnd;
}

void NotificationGlobalSettings::setDND(bool enable)
{
    d->setManualDND(enable);
}

void NotificationGlobalSettings::screenModeChanged(int mode)
{
    qDebug() << "NotificationGlobalSettings::screenModeChanged : " << mode;
    d->m_currentScreenMode = mode;
    checkDNDMode();
}

void NotificationGlobalSettings::checkDNDMode()
{
    d->setAutoDND(autoDND());
}

void NotificationGlobalSettings::checkTimeForDND()
{
    checkDNDMode();
    updateDNDTime();
}

void NotificationGlobalSettings::updateDNDTime()
{
    if (!scheduleTurnOnDND()) {
        // 勿扰模式开关关闭，关闭定时器
        if (d->m_timeChecktimer->isActive()) {
            d->m_timeChecktimer->stop();
        }
        return;
    }

    QTime currentTime = QTime::currentTime();
    QTime dndOnTime = scheduleTurnOnDNDTime();
    QTime dndOffTime = scheduleTurnOffDNDTime();

    int msec = 60000;  // 1分钟
    if (dndOnTime == dndOffTime) {// 开启关闭时间一致，启动定时器无意义
        if (d->m_timeChecktimer->isActive()) {
            d->m_timeChecktimer->stop();
        }
        return;
    } else if (dndOnTime > dndOffTime) {
        if (currentTime >= dndOffTime && currentTime < dndOnTime) {
            msec = currentTime.msecsTo(dndOnTime);
        } else {
            msec = currentTime.msecsTo(dndOffTime);
        }
    } else {
        if (currentTime >= dndOnTime && currentTime < dndOffTime) {
            msec = currentTime.msecsTo(dndOffTime);
        } else {
            msec = currentTime.msecsTo(dndOnTime);
        }
    }
    //跨天处理
    static constexpr int MILLISECONDS_PER_DAY = 24 * 60 * 60 * 1000;
    if (msec < 0) {
        msec += MILLISECONDS_PER_DAY;
    }

    if (d->m_timeChecktimer->isActive()) {
        d->m_timeChecktimer->stop();
    }
    d->m_timeChecktimer->start(msec);
}
