/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SIGNAL_EVENT_H_
#define _SIGNAL_EVENT_H_

#include <QEvent>

class SignalEvent : public QEvent
{
public:
    typedef enum {
        eLeaveEvent = QEvent::User + 1,
        eEnterEvent = QEvent::User + 2,
    }EventType;

    explicit SignalEvent(unsigned long winId, EventType sigType, int32_t posX, int32_t posY) 
        : QEvent((QEvent::Type)sigType)
        , mWinId(winId)
        , mX(posX)
        , mY(posY){}
    ~SignalEvent() {}

    unsigned long getWinId() {return mWinId;}

private:
    unsigned long mWinId;
    int32_t mX;
    int32_t mY;
};

#endif