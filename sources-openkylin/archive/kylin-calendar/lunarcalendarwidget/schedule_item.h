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
#ifndef SCHEDULE_ITEM_H
#define SCHEDULE_ITEM_H
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGSettings>
class LunarCalendarWidget;
//class QWidget;
class ColorArea; // 假设ColorArea是一个已经定义好的类
class schedule_item : public QWidget
{
    Q_OBJECT

public:
    schedule_item(QWidget *parent = nullptr, bool ishasSchedule = false);

    // 假设你想要设置开始时间和结束时间的函数
    void setStartTime(const QString &startTime);
    void setEndTime(int end_hour, int end_minute, QDate date, QString shangwu);
    void setColor(const QString &color);
    void setDescription(const QString &description);
    void adjustDate(QDate date);
    void setId(const QString &idd);

    QString getId();
    QString this_id;
    QString timemode;

    QLabel *id = nullptr;
    QMenu *mnu_menu = nullptr;
    QString styleSheet;
    bool wasSchedule = false;
    bool hasSchdule;
    QLabel *lbl_dateTimeLabel = nullptr;

    ColorArea *colorArea = nullptr;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private:

    QLabel *lbl_eventLabel = nullptr;

    QGSettings *styleGsettings = nullptr;

    QHBoxLayout *mainLayout = nullptr;
    QVBoxLayout *eventLayout = nullptr;
    QVBoxLayout *textLayout = nullptr;

    QColor selectBgColor;
signals:
    void updateWindow(QString id);
    void ScheduleClick(QString id);
    void EditSchedule(QString id);
    void updateSchedule1();
public slots:

    void slotEditSchedule();
    void slotDeleteSchedule();
};

#endif // SCHEDULE_ITEM_H

