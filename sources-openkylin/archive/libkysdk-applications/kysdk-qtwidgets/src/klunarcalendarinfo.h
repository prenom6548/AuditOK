/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KLUNARCALENDARINFO_H
#define KLUNARCALENDARINFO_H

#include "gui_g.h"
#include <QObject>

namespace kdk
{
class KLunarCalendarInfoPrivate;

/**
 *  @defgroup KLunarCalendarInfo
 *  @since 2.5.2.0
 */

class GUI_EXPORT KLunarCalendarInfo : public QObject
{
    Q_OBJECT
public:
    static KLunarCalendarInfo *Instance();
    explicit KLunarCalendarInfo(QObject *parent = 0);

    /**
     * @brief 计算是否闰年
     * @param year
     * @return
     */
    bool isLoopYear(int year);

    /**
     * @brief 计算指定年月该月共多少天
     * @param year
     * @param month
     * @return
     */
    int getMonthDays(int year, int month);

    /**
     * @brief 计算指定年月对应到该月共多少天
     * @param year
     * @param month
     * @return
     */
    int getTotalMonthDays(int year, int month);

    /**
     * @brief 计算指定年月对应星期几
     * @param year
     * @param month
     * @param FirstDayisSun:第一天是否以周日开始
     * @return
     */
    int getFirstDayOfWeek(int year, int month, bool FirstDayisSun);

    /**
     * @brief 计算国际节日
     * @param month
     * @param day
     * @return
     */
    QString getHoliday(int month, int day);

    /**
     * @brief 计算二十四节气
     * @param year
     * @param month
     * @param day
     * @return
     */
    QString getSolarTerms(int year, int month, int day);

    /**
     * @brief 计算农历节日(必须传入农历年份月份)
     * @param month
     * @param day
     * @return
     */
    QString getLunarFestival(int month, int day);

    /**
     * @brief 计算农历年 天干+地支+生肖
     * @param year
     * @return
     */
    QString getLunarYear(int year);

    /**
     * @brief 计算指定年月日农历信息,包括公历节日和农历节日及二十四节气
     * @param year
     * @param month
     * @param day
     * @param strHoliday
     * @param strSolarTerms
     * @param strLunarFestival
     * @param strLunarYear
     * @param strLunarMonth
     * @param strLunarDay
     */
    void getLunarCalendarInfo(int year, int month, int day,
                              QString &strHoliday,
                              QString &strSolarTerms,
                              QString &strLunarFestival,
                              QString &strLunarYear,
                              QString &strLunarMonth,
                              QString &strLunarDay);

    /**
     * @brief 获取指定年月日农历信息
     * @param year
     * @param month
     * @param day
     * @param yearInfo
     * @param monthInfo
     * @param dayInfo
     * @return
     */
    QString getLunarInfo(int year, int month, int day, bool yearInfo, bool monthInfo, bool dayInfo);

    /**
     * @brief 获取农历年月日
     * @param year
     * @param month
     * @param day
     * @return
     */
    QString getLunarYearMonthDay(int year, int month, int day);

    /**
     * @brief 获取农历月日
     * @param year
     * @param month
     * @param day
     * @return
     */
    QString getLunarMonthDay(int year, int month, int day);

    /**
     * @brief 获取农历日
     * @param year
     * @param month
     * @param day
     * @return
     */
    QString getLunarDay(int year, int month, int day);

private:
    Q_DECLARE_PRIVATE(KLunarCalendarInfo)
    KLunarCalendarInfoPrivate *const d_ptr;
    static QScopedPointer<KLunarCalendarInfo> self;
};
}

#endif // KLUNARCALENDARINFO_H
