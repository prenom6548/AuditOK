/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QGuiApplication>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <KWindowEffects>
#include <KWindowSystem>
#include "schedulestruct.h"
#include <QtCore/QStringList>
#include <QDesktopWidget>
#include <QtCore/QVariant>
#include "lunarcalendarinfo.h"
#include "lunarcalendarwidget.h"
#include "notificationsadaptor.h"
#include "calendardatabase.h"
#include "kysdk/applications/windowmanager/windowmanager.h"
const int WIDGET_GAP = 8;
#define PANEL_CONTROL_IN_CALENDAR "org.ukui.control-center.panel.plugins"

static const QString NotificationsDBusService = "org.ukui.calendar.Notifications";
static const QString NotificationsDBusPath = "/org/ukui/calendar/Notifications";
NotificationsAdaptor::NotificationsAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    // constructor
    //    this->parent() = parent;
    setAutoRelaySignals(true);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(
                NotificationsDBusService,
                QDBusConnectionInterface::ReplaceExistingService,
                QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);
    initGsettings();

}
void NotificationsAdaptor::initGsettings()
{
    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id))
    {
        calendar_gsettings = new QGSettings(calendar_id);
    }
    if(calendar_gsettings != nullptr)
    {
        timemodel = calendar_gsettings->get("hoursystem").toString();
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString &keys)
        {
            timemodel = calendar_gsettings->get("hoursystem").toString();
        });
    }
}
NotificationsAdaptor::~NotificationsAdaptor()
{
    // destructor
    if(calendar_gsettings)
    {
        delete calendar_gsettings;
        calendar_gsettings = nullptr;
    }
}
/**
 * @brief NotificationsAdaptor::ScheduleNotification
 * @param hour 开始小时
 * @param minute 开始分钟
 * @param day 开始天
 * @param month 开始月
 * @param shangwu 是上午/下午
 * @param end_hour 结束小时
 * @param end_minute 结束分钟
 * @param m_descript 内容
 * @param markId id QDateTime::currentDateTime().toString("yyyyMMddhhmmss");获取
 * 增
 */
bool NotificationsAdaptor::ScheduleNotification(int hour, int minute, int day, int month, int  year, int end_minute, int end_hour, const QString m_descript)
{
    if(year <= 1970 || year >= 2099)
    {
        return false;
    }

    if(month < 1 || month > 12)
    {
        return false;
    }

    if(day < 1 || day > 31)
    {
        return false;
    }

    if(minute < 0 || minute > 59)
    {
        return false;
    }
    if(end_minute < 0 || end_minute > 59)
    {
        return false;
    }

    if(hour < 0 || hour > 23)
    {
        return false;
    }
    if(end_hour < 0 || end_hour > 23)
    {
        return false;
    }

    int daysNum = LunarCalendarInfo::Instance()->getMonthDays(year,month);
    if(day > daysNum)
    {
        return false;
    }

    // 新增：检查结束时间是否大于开始时间
    int startTotalMinutes = hour * 60 + minute;
    int endTotalMinutes = end_hour * 60 + end_minute;
    // 排除全天日程的特殊情况（开始和结束时间都为0）
    bool isAllDay = (hour == 0 && minute == 0 && end_hour == 0 && end_minute == 0);
    if (!isAllDay && endTotalMinutes < startTotalMinutes)
    {
        qDebug() << "End time must be greater than or equal to start time";
        return false;
    }

    //2024.7.12日程新需求 31068 如果开始时间结束时间都是空 按照全天处理
    qDebug() << "开始通过Dbus添加到日程";
    //    display = getenv("DISPLAY");
    //    userName = getenv("USER"); //获取当前用户名
    //    xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    MarkInfo info;
    QDate beginDate(year, month, day);
    //    info.m_markId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    info.m_markId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") +
            QString::number(QRandomGenerator::global()->bounded(1000)).rightJustified(3, '0'); //给一个随机三位数，目的，如果助手同时加了三个，不会因为id一样全部删除
    info.m_descript = m_descript;
    info.minute = info.Alarm_end_minute = minute;
    info.hour = info.Alarm_end_hour  = info.displayHour = hour;
    qDebug() << "参数hour:" << hour;
    info.day = day;
    info.week = beginDate.dayOfWeek();
    info.month = month;
    info.m_markStartDate = beginDate;

    //12小时制
    if(timemodel == "12")
    {
        if(hour <= 12)
        {
            info.timePrefix = tr("morning");
        }
        else
        {
            info.timePrefix = tr("afternoon");
            info.displayHour = info.displayHour - 12;
        }
    }
    //24小时制
    else
    {
        info.timePrefix = tr("today");
    }
    //    info.shangwu = shangwu;
    info.end_hour = end_hour;
    info.end_minute = end_minute;
    if(hour == 0 && minute == 0 && end_hour == 0 && end_minute == 0 || hour == 0 && minute == 0 || end_hour == 0 && end_minute == 0 )
    {
        info.m_isAllDay = true;
        //        CalendarDataBase::getInstance().AlarmType = 10;
        CalendarDataBase::getInstance().setAlarmType("At 9am on the same day of the schedule");//10
        info.AlldayRemind = tr("At 9am on the same day of the schedule");
        info.hour = info.Alarm_end_hour = 0;
        info.minute = info.Alarm_end_minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }
    else
    {
        info.m_isAllDay = false;
        //        CalendarDataBase::getInstance().AlarmType = 1;
        CalendarDataBase::getInstance().setAlarmType("Schedule Begin");//1
        info.remind = tr("Schedule Begin");
    }
    info.end_day = 0;
    info.end_month = 0;
    info.timeLong = "";
    info.repeat = tr("No Repeat");
    info.beginrepeat = "";
    info.endrepeat = QDateTime::currentDateTime().addDays(1);
    info.beginDateTime = QDateTime(beginDate);
    info.endDateTime = QDateTime(beginDate);
    info.m_markEndDate= beginDate;
    info.m_isLunar = false;
    if(info.remind == "")
        info.remind = "No prompt";
    else if(info.AlldayRemind == "")
        info.AlldayRemind = "No prompt";
    CalendarDataBase::getInstance().add(info);
    CalendarDataBase::getInstance().insertDataSlot(info);
    qDebug() << "通过Dbus添加成功了";
    return true;
}
//查看日程
bool NotificationsAdaptor::OpenSchedule(int day, int month, int year)
{
    if(year <= 1970 || year >= 2099)
    {
        return false;
    }

    if(month < 1 || month > 12)
    {
        return false;
    }

    if(day < 1 || day > 31)
    {
        return false;
    }

    int daysNum = LunarCalendarInfo::Instance()->getMonthDays(year,month);
    if(day > daysNum)
    {
        return false;
    }

    Q_EMIT dayWantToChange(day, month, year);
    return true;
}
//删除日程
bool NotificationsAdaptor::DeleteSchedule(int hour, int minute, int day, int month, const QString m_descript)
{
    if(month < 1 || month > 12)
    {
        return false;
    }

    if(day < 1 || day > 31)
    {
        return false;
    }

    if(minute < 0 || minute > 59)
    {
        return false;
    }

    if(hour < 0 || hour > 23)
    {
        return false;
    }

    QStringList markIds = getIDFromTimeAndDescript(hour, minute, day, month, m_descript);
    if (markIds.isEmpty()) {
        return false; // 无匹配日程
    }

    bool allDeleted = true;
    for (const QString &id : markIds) {
        int dbResult = CalendarDataBase::getInstance().deleteDataSlotfromMarkID(id);
        int cronResult = CalendarDataBase::getInstance().deleteCrontabfromMarkID(id);

        if (dbResult != DB_OP_SUCC || cronResult != DB_OP_SUCC) {
            allDeleted = false; // 记录删除失败
        }
    }
    return allDeleted;
}
//编辑日程
bool NotificationsAdaptor::EditSchedule(int hour, int minute, int day, int month, const QString m_descript)
{
    if(month < 1 || month > 12)
    {
        return false;
    }

    if(day < 1 || day > 31)
    {
        return false;
    }

    if(minute < 0 || minute > 59)
    {
        return false;
    }

    if(hour < 0 || hour > 23)
    {
        return false;
    }

    QStringList markIds = getIDFromTimeAndDescript(hour, minute, day, month, m_descript);
    if (markIds.isEmpty()) {
        return false;
    }
    QString Markid = markIds.first(); // 取第一个ID编辑
    //当搜到了日程在显示，查找错误的情况下不会弹窗
    if(!Markid.isEmpty())
    {
        if(!dialog)
        {
            qDebug() << "不存在dialog对象";
            dialog = new CSchceduleDlg();
            connect(dialog, &CSchceduleDlg::sigWindowClosed, this, [ = ]()
            {
                if(dialog)
                {
                    dialog->deleteLater();
                    dialog = nullptr;
                }
            });
        }
        if(dialog->isVisible())
        {
            dialog->raise();
            dialog->activateWindow();
            KWindowSystem::forceActiveWindow(dialog->winId());
            KWindowSystem::raiseWindow(dialog->winId());
            return false;
        }
        dialog->setProperty(0, true);
        dialog->setId(Markid);
        dialog->setStatusFromId(Markid);

        // 获取屏幕中心点和尺寸
        QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
        QSize dialogSize = dialog->size();
        // 计算对话框应显示的中心点坐标，并将其移动到屏幕中心
        dialog->move(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2);
        KWindowSystem::forceActiveWindow(dialog->winId());
        KWindowSystem::raiseWindow(dialog->winId());
        KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar);
        dialog->exec();
        return true;
    }
    else
    {
        return false;
    }
}

QStringList NotificationsAdaptor::getIDFromTimeAndDescript(int hour, int minute, int day, int month, const QString m_descript)
{
    QSqlQuery query;
    // 使用start_date确保唯一性（日期包含年份）
    QString queryStr = QString(
                "SELECT id FROM Schedule "
                "WHERE start_hour = %1 AND start_minute = %2 "
                "AND start_day = %3 AND start_month = %4 "
                "AND descript = \"%5\""
                ).arg(hour).arg(minute).arg(day).arg(month).arg(m_descript);

    if (!query.exec(queryStr)) {
        qDebug() << "Query error:" << query.lastError().text();
        return QStringList();
    }

    QStringList markIds;
    while (query.next()) {
        markIds.append(query.value(0).toString());
    }
    return markIds;
}
