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
#ifndef DATEGROUPWIDGET_H
#define DATEGROUPWIDGET_H

#include "dateedit.h"
#include "calendarwidget.h"
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimerEvent>
#include <kysdk/kysdk-system/libkydate.h>
#include "klunardateedit.h"

using namespace kdk;

class DategroupWidget : public QWidget
{
    Q_OBJECT
public:
    DategroupWidget(QWidget *parent = nullptr);
    ~DategroupWidget();
    void initHour();
    void initMinAndSec();
    void updateTime(bool force = false);
    void initConnect();

public Q_SLOTS:
    void dataChangedSlot(QString key);
    void resetDateEdit(QString date);

Q_SIGNALS:
    void dateChanged(QDate date, QTime time);

protected:
    void timerEvent(QTimerEvent *e);

private:
    KLunarDateEdit *m_lunarDateEdit    = nullptr;
    QComboBox      *hourComboBox      = nullptr;
    QComboBox      *minComboBox       = nullptr;
    QComboBox      *secComboBox       = nullptr;
    QLabel         *hourMinColonLabel = nullptr;
    QLabel         *MinSecColonLabel  = nullptr;

private:
    QDBusInterface *areaInterface = nullptr;
    int timerID;
};


#endif // DATEGROUPWIDGET_H
