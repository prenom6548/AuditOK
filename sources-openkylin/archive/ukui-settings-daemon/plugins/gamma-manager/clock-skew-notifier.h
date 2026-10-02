/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLOCKSKEWNOTIFIER_H
#define CLOCKSKEWNOTIFIER_H

#include <QSocketNotifier>
#include <QObject>

#include <sys/timerfd.h>
#include <fcntl.h>


#include "clib-syslog.h"

namespace USD {

/**
  *  After modifying the time in 2303, it is often necessary to wait for one minute before setting gamma.
  *  Now, you can use timerfd_create with CLOCK_ REALTIME, TFD_TIMER_CANCEL_ON_SET monitoring
  *  clock jumps and solves real-time issues
*/

class ClockSkewNotifier : public QObject
{
    Q_OBJECT

public:
    ClockSkewNotifier(QObject *parent = nullptr);
    ~ClockSkewNotifier() override;

private Q_SLOTS:
    void handleTimerCancelled(int value);

Q_SIGNALS:

    /**
     * when system clock are changed by 'settimeofday' or 'clock_settime' will this signal would emit
     */
    void clockSkewed(QString key);
private:
    int m_fd = -1;
};

}


#endif // CLOCKSKEWNOTIFIER_H
