/*
 * UKUI Framework
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DATETIME_H
#define DATETIME_H

#include <QDebug>
#include <QObject>
#include <QDBusContext>
#include <QDBusReply>
#include <QDBusInterface>
class DateTime : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.DateTime")
public:
    explicit DateTime(QObject *parent = nullptr);
    ~DateTime();

public slots:
    /**
     * @brief GetTimeZone
     * 获取时区
     * @return QStringList
     */
    Q_INVOKABLE const QString GetTimeZone() const;
    /**
     * @brief SetTimeZone
     * 设置当前时区
     * @param timezone
     * @return
     */
    Q_INVOKABLE void SetTimeZone(const QString &timezone);
    /**
     * @brief GetFormatCountry
     * 获取显示日期，时间货币格式
     * @return QString
     */
    Q_INVOKABLE const QString GetFormatCountry() const;
    /**
     * @brief SetFormatCountry
     * 设置显示日期，时间货币格式
     * @param formatCountry
     * @return
     */
    Q_INVOKABLE void SetFormatCountry(const QString &formatCountry);
    /**
     * @brief GetCalendar
     * 获取日历
     * @return QString
     */
    Q_INVOKABLE const QString GetCalendar() const;
    /**
     * @brief SetCalendar
     * 设置日历
     * @param calendar
     * @return
     */
    Q_INVOKABLE void SetCalendar(const QString &calendar);
    /**
     * @brief getFirstDay
     * 获取一周第一天
     * @return QString
     */
    Q_INVOKABLE const QString GetFirstDay() const;
    /**
     * @brief SetFirstDay
     * 设置一周第一天
     * @param firstDay
     * @return
     */
    Q_INVOKABLE void SetFirstDay(const QString &firstDay);
    /**
     * @brief GetTimeFormat
     * 获取时间格式
     * @return QString
     */
    Q_INVOKABLE const QString GetTimeFormat() const;
    /**
     * @brief GetTimeFormat
     * 获取时间格式
     * @param timeFormat
     * @return
     */
    Q_INVOKABLE void SetTimeFormat(const QString &timeFormat);


signals:
    void TimeZoneChanged();
    void FormatCountryChanged();
    void CalendarChanged();
    void FirstDayChanged();
    void TimeFormatChanged();

private:
    QDBusInterface   *m_areaDBusManager   = nullptr;
    QDBusInterface   *m_dateDBusManager   = nullptr;

    Q_PROPERTY(QString TimeZone READ GetTimeZone WRITE SetTimeZone NOTIFY TimeZoneChanged)
    Q_PROPERTY(QString FormatCountry READ GetFormatCountry WRITE SetFormatCountry NOTIFY FormatCountryChanged)
    Q_PROPERTY(QString Calendar READ GetCalendar WRITE SetCalendar NOTIFY CalendarChanged)
    Q_PROPERTY(QString FirstDay READ GetFirstDay WRITE SetFirstDay NOTIFY FirstDayChanged)
    Q_PROPERTY(QString TimeFormat READ GetTimeFormat WRITE SetTimeFormat NOTIFY TimeFormatChanged)
};

#endif // DATETIME_H
