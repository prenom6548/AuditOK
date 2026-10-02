/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "timelabel.h"
#include "QApplication"
#include <QDebug>
#include <QDateTime>
#include<libkydate.h>

TimeLabel::TimeLabel(QWidget *parent) :
    KLabel(parent)
{
    this->setAlignment(Qt::AlignVCenter);
    this->setContentsMargins(0, 0, 0, 0);
    this->setObjectName("timeClockLable");
    timerID = startTimer(1000);

    areaInterface = new QDBusInterface("org.ukui.ukcc.session",
                                       "/Area",
                                       "org.ukui.ukcc.session.Area",
                                       QDBusConnection::sessionBus(),
                                       this);
    if (areaInterface) {
        timeFormat = areaInterface->property("timeFormat").toString();
    } else {
        if (!areaInterface->isValid()) {
            qCritical() << "org.ukui.ukcc.session.Area DBus error:" << areaInterface->lastError();
        }
    }
    setTimeText();
}

TimeLabel::~TimeLabel()
{
    killTimer(timerID);
}

void TimeLabel::updateLabelFont() {
    QFont font = QApplication::font();
    if (QLocale::system().amText() == "སྔ་དྲོ་") {
        font.setPixelSize(font.pointSize() * 20 / 11);
    } else {
        font.setPixelSize(font.pointSize() * 28 / 11);
    }
    font.setWeight(QFont::Medium);
    font.setBold(true);
    this->setFont(font);
}

void TimeLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerID) {
        setTimeText();
    }
}

void TimeLabel::setTimeText()
{
    QString currentsecStr;
    QDateTime datetime = QDateTime::currentDateTime();
    QLocale locale = QLocale::system();
    if (areaInterface->property("timeFormat").toString() == "24") {
        currentsecStr = datetime.toString("hh : mm : ss");
    } else {
        currentsecStr = kdk_system_second();
        qDebug() << Q_FUNC_INFO << "kdk_system_second" << currentsecStr << locale.name();
    }
    QString dateString;
    if ("cn" == areaInterface->property("dateFormat").toString()) {
        dateString = datetime.toString("yyyy/MM/dd ddd").replace("周", "星期");
    } else {
        dateString = datetime.toString("yyyy-MM-dd ddd").replace("周", "星期");
    }
    if (dateString != date) {
        date = dateString;
        Q_EMIT dateChanged();
    }
    setText(currentsecStr);
    updateLabelFont();
}

QString TimeLabel::dateText()
{
    return date;
}
