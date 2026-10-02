/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef SLEEPTIME_H
#define SLEEPTIME_H

#include "scconfiguration.h"
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QList>
#include <QDateTime>
#include <QHBoxLayout>

class SleepTime : public QWidget
{
    Q_OBJECT
public:
    explicit SleepTime(QWidget *parent = nullptr);
    ~SleepTime();
    int setTime(QDateTime time);
    void setSmallMode();

protected:
    void changeEvent(QEvent *event);

private:
    SCConfiguration *configuration;
    QLabel *restTime;
    QList<QLabel *> list;
    QHBoxLayout *layout;
    long long sleepTime;
    long long m_nLastSleepLeave;
    long long m_nLastSleepTimeSecs;
    QDateTime initTime;
    QDateTime m_lastTime;
    double curFontSize;
    double m_ptToPx = 1.0;
    QFont sysFont;

    void init();
    void setHour(int hour);
    void setSeconds(int seconds);
    void setMinute(int minutes);
    bool QLabelSetText(QLabel *label, QString string);
};

#endif // SLEEPTIME_H
