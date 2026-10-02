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
#ifndef TIMEBTN_H
#define TIMEBTN_H
#include <QPushButton>
#include "lightlabel.h"
#include <QGSettings>
#include <QTimeZone>
#include <QLabel>
#include "ukccframe.h"
#include <QTimerEvent>
#include <klabel.h>

using namespace kdk;

class TimeBtn : public UkccFrame
{
    Q_OBJECT
public:
    TimeBtn(const QString &timezone, const QString &kyTimezone = "");
    ~TimeBtn();
    void updateTime(bool hour_24);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void timerEvent(QTimerEvent *e);

private:
    void styleSettingChangeSlot(const QString& key);
    void updateGMT();

private:
    QTimeZone thisZone;
    int timerId = 0;
    KLabel       *labelInfo     = nullptr;
    LightLabel     *labelTime     = nullptr;
    QDBusInterface *areaInterface = nullptr;
    QPushButton    *deleteBtn     = nullptr;
    QGSettings *m_fontSetting = nullptr;

Q_SIGNALS:
    void deleted();
};


#endif // TIMEBTN_H
