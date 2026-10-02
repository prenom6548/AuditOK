/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef SCHEDULESTRUCT_H
#define SCHEDULESTRUCT_H
#include <QDateTime>
struct MarkInfo
{
    bool isKuaDay;
    QString m_markId;
    QDate m_markStartDate; //开始日期
    QDate m_markEndDate;   //结束日期
    QString m_descript;    //描述
    int m_markStartTime;   //开始时间
    int m_markEndTime;     //结束时间
    int m_markWidth;       // mark的宽
    int m_markHeight;      // mark的高
    QString m_themeColor;  //主题颜色
    bool m_isRepeat;       //是否重复
    QDate m_ReminderTime;  //提醒时间，例如提前5/10/15..分钟
    bool isExtra;          //是否是还有...条日程的标志，用来点击弹出菜单用的
    int year;
    int day; //开始天
    int month; //开始月
    int week; //开始周
    int minute; //开始分钟
    int hour; //开始小时
    int displayHour;//crontab显示的时间，根据12/24小时制变化
    /**
     * @brief end_ 用到日程提醒和 日程视图中
     */
    int end_year;
    int end_day; //结束天
    int end_month;//结束月
    int end_week;//结束周
    int end_minute; //结束分钟
    int end_hour; //结束小时

    int Alarm_day; //提醒开始天
    int Alarm_month; //提醒开始月
    int Alarm_week; //提醒开始周
    int Alarm_minute; //提醒开始分钟
    int Alarm_hour; //提醒开始小时
    /**
     * @brief end_ 用到日程提醒和 日程视图中
     */
    int Alarm_end_day; //提醒结束天
    int Alarm_end_month;//提醒结束月
    int Alarm_end_week;//提醒结束周
    int Alarm_end_minute; //提醒结束分钟
    int Alarm_end_hour; //提醒结束小时


    QString timeLong; //时间间隔
    QString timePrefix = ""; //时间前缀
    QTime startTime;
    QTime endTime;

    QString remind;//非全天
    QString AlldayRemind;
    QString repeat;
    QString beginrepeat;
    QDateTime endrepeat;
    QDateTime beginDateTime;
    QDateTime endDateTime;

    bool m_isAllDay;       //是否是全天
    bool m_isLunar;       //是否是农历

};
#endif // SCHEDULESTRUCT_H
