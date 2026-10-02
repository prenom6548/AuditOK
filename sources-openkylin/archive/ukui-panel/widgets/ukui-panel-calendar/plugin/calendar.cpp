/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "calendar.h"

#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include <QGSettings>
#include <QStringLiteral>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QLocale>
#include <QTranslator>
#include <function-control.h>
#include <QJsonObject>

#include "kysdk/kysdk-system/libkydate.h"

#define HOUR_SYSTEM_CONTROL "org.ukui.control-center.panel.plugins"
#define DATE_FORMAT         "date"          // 日期格式：yyyy/MM/dd、yyyy-MM-dd
#define TIME_FORMAT         "hoursystem"    // 时间格式：12小时制、24小时制
#define EN_FORMAT           "en_US"

#define KYSDK_TIMER_SERVER    "com.kylin.kysdk.TimeServer"
#define KYSDK_TIMER_PATH      "/com/kylin/kysdk/Timer"
#define KYSDK_TIMER_INTERFACE "com.kylin.kysdk.TimeInterface"

#define KYSDK_DATESERVER "com.kylin.kysdk.DateServer"
#define KYSDK_DATEPATH "/com/kylin/kysdk/Date"
#define KYSDK_DATEINTERFACE "com.kylin.kysdk.DateInterface"

#define UKUI_SIDEBAR_SERVER "org.ukui.Sidebar"
#define UKUI_SIDEBAR_PATH "/org/ukui/Sidebar"
#define UKUI_SIDEBAR_INTERFACE "org.ukui.Sidebar"


#define SDK2CPP(func) []()->QString { char* p = func(); QString str(p); free(p); return str; }

Calendar::Calendar(QObject *parent) : QObject(parent)
{
    initGSetting();

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::systemBus().connect(KYSDK_TIMER_SERVER, KYSDK_TIMER_PATH, KYSDK_TIMER_INTERFACE,
                                         "TimeSignal",this, SLOT(timeSignalSlot()));
    QDBusConnection::systemBus().connect(KYSDK_TIMER_SERVER, KYSDK_TIMER_PATH, KYSDK_TIMER_INTERFACE,
                                         "TimeChangeSignal",this, SLOT(timeSignalSlot()));
    QDBusConnection::sessionBus().connect(KYSDK_DATESERVER, KYSDK_DATEPATH, KYSDK_DATEINTERFACE,
                                         "ShortDateSignal",this, SLOT(onShortDateSignal()));

    //get notification state
    QDBusConnection::sessionBus().connect(UKUI_SIDEBAR_SERVER, UKUI_SIDEBAR_PATH, UKUI_SIDEBAR_INTERFACE,
                                          "stateChange", this, SLOT(onStateChanged(QString,QVariantMap)));
    //init shortformat date
    updateShortformatDate();

    initFunctionControl();
}

Calendar::~Calendar()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-calendar-v11"));
}

void Calendar::initGSetting()
{
    const QByteArray id(HOUR_SYSTEM_CONTROL);
    if (QGSettings::isSchemaInstalled(id)) {
        auto gSettings = new QGSettings(id, QByteArray(), this);

        QStringList keys = gSettings->keys();
        // "cn" ...
        if(keys.contains(DATE_FORMAT)) {
            m_dateFormat = gSettings->get(DATE_FORMAT).toString();
        }

        // 12, 24
        if(keys.contains(TIME_FORMAT)) {
            m_hourSystem = gSettings->get(TIME_FORMAT).toString();
        }

        connect(gSettings, &QGSettings::changed, this, [gSettings, this] (const QString &key) {
            if (key == DATE_FORMAT) {
                m_dateFormat = gSettings->get(DATE_FORMAT).toString();
                Q_EMIT timeUpdated();

            } else if(key == TIME_FORMAT) {
                m_hourSystem = gSettings->get(TIME_FORMAT).toString();
                Q_EMIT timeUpdated();
            }
        });
    }
}

void Calendar::updateShortformatDate()
{
    m_shortDate = SDK2CPP(kdk_system_get_shortformat_date)();

    if (m_shortDate.contains(QStringLiteral("/"))) {
        m_separator = QStringLiteral("/");
    } else if (m_shortDate.contains(QStringLiteral("-"))) {
        m_separator = QStringLiteral("-");
    } else if (m_shortDate.contains(QStringLiteral("."))) {
        m_separator = QStringLiteral(".");
    } else {
        m_separator = m_dateFormat == QStringLiteral("cn") ? QStringLiteral("/") : QStringLiteral("-");
    }
}

void Calendar::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-calendar-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-calendar-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-calendar-v11"), true);
    });
}

void Calendar::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-calendar-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "Calendar function control config:" << controlConfig;
    //是否禁用左键点击
    if (controlConfig.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (disable != m_disableMouseLeftButton) {
            m_disableMouseLeftButton = disable;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        m_disableMouseLeftButton = false;
        Q_EMIT disableMouseLeftButtonChanged();
    }
    //是否禁用右键点击
    if (controlConfig.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (disable != m_disableMouseRightButton) {
            m_disableMouseRightButton = disable;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        m_disableMouseRightButton = false;
        Q_EMIT disableMouseRightButtonChanged();
    }
}

bool Calendar::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool Calendar::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

QString Calendar::time() const
{
    return SDK2CPP(kdk_system_nowtime)();
#if 0
    if (m_hourSystem == QStringLiteral("24")) {
        return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm"));
    }

    return QDateTime::currentDateTime().toString(QStringLiteral("hh:mm ap")).split(" ").first();
#endif
}

QString Calendar::ap() const
{
    if (m_hourSystem == QStringLiteral("24")) {
        return {};
    }

    return QDateTime::currentDateTime().toString("ap");
}

QString Calendar::week() const
{
    if (QLocale::system().name() == EN_FORMAT) {
        return SDK2CPP(kdk_system_shortweek)();
    } else {
        return SDK2CPP(kdk_system_longweek)();
    }
    //return QDateTime::currentDateTime().toString("ddd");
}

QString Calendar::year() const
{
    QStringList dateParts = m_shortDate.split(separator());
    if (dateParts.count() > 0 && !dateParts[0].isEmpty()) {
        return dateParts[0];
    } else {
        qWarning() << "[ukui-panel] calendar short format date from sdk invalid, return format 'yyyy'.";
        return QDateTime::currentDateTime().toString("yyyy");
    }
}

QString Calendar::month() const
{
    QStringList dateParts = m_shortDate.split(separator());
    if (m_shortDate.split(separator()).count() > 2 && !dateParts[1].isEmpty() && !dateParts[2].isEmpty()) {
        return dateParts[1] + m_separator + dateParts[2];
    } else {
        qWarning() << "[ukui-panel] calendar short format date from sdk invalid, return format 'MM dd'.";
        QString fmt = "MM" + separator() + "dd";
        return QDateTime::currentDateTime().toString(fmt);
    }
}

QString Calendar::separator() const
{
    return m_separator;
}

QString Calendar::tooltipText() const
{
    return SDK2CPP(kdk_system_get_longformat_date)();
}

QStringList Calendar::allData() const
{
    QStringList list;
    list << time();
    list << ap();
    list << week();
    list << year();
    list << month();

    return list;
}

int Calendar::unreadMsgCount() const
{
    return m_unreadmsgCount;
}

QString Calendar::notifyIconTooltipText()
{
    if (unreadMsgCount() == 1) {
        return QString::number(1) + QString(tr(" new notification"));
    } else if (unreadMsgCount() > 1) {
        return QString::number(unreadMsgCount()) + QString(tr(" new notifications"));
    }

    return QString(tr("Notification center"));
}

void Calendar::openSidebar()
{
    QDBusInterface sidebarIFace(UKUI_SIDEBAR_SERVER, UKUI_SIDEBAR_PATH, UKUI_SIDEBAR_INTERFACE, QDBusConnection::sessionBus(), this);
    sidebarIFace.asyncCallWithArgumentList("active", (QVariantList() << "sidebar"));
}

void Calendar::timeSignalSlot()
{
    m_shortDate = SDK2CPP(kdk_system_get_shortformat_date)();
    Q_EMIT timeUpdated();
}

void Calendar::onStateChanged(const QString &module, const QVariantMap &data)
{
    if (module == QStringLiteral("sidebar")) {
        if (data.contains(QStringLiteral("unreadMsg"))) {
            m_unreadmsgCount = data.value(QStringLiteral("unreadMsg")).toInt();
            Q_EMIT unreadMsgChanged();
        }
    }
}

void Calendar::onShortDateSignal()
{
    updateShortformatDate();
    Q_EMIT timeUpdated();
}

void Calendar::activeCalendar()
{
    QDBusInterface calendar(QStringLiteral("org.ukui.kylin_calendar"),
                            QStringLiteral("/org/ukui/kylin_calendar"),
                            QStringLiteral("org.ukui.kylin_calendar"));
    calendar.asyncCall("command_Control", QVariant(""));
}
