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


#include "clock-skew-notifier.h"

namespace USD {
//U need this cmd: sudo find ./* -exec touch {} +;


ClockSkewNotifier::ClockSkewNotifier(QObject *parent)
    : QObject(parent)
{
   m_fd = timerfd_create(CLOCK_REALTIME, O_CLOEXEC | O_NONBLOCK);
   if (m_fd < 0) {
        USD_LOG(LOG_ERR, "timerfd_create fail...");
        return;
   }

   const itimerspec spec = {};
   const int ret = timerfd_settime(m_fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &spec, nullptr);
   if (ret == -1) {
       USD_LOG(LOG_ERR,"Couldn't create clock skew notifier engine: %s", strerror(errno));
       return;
   }
   const QSocketNotifier *notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
   connect(notifier,SIGNAL(activated(int)), this, SLOT(handleTimerCancelled(int)));
   USD_LOG(LOG_DEBUG,"ClockSkewNotifier create success!");
}

ClockSkewNotifier::~ClockSkewNotifier()
{

}

void ClockSkewNotifier::handleTimerCancelled(int value)
{
    uint64_t expirationCount;
    read(m_fd, &expirationCount, sizeof(expirationCount));
    USD_LOG_SHOW_PARAM1(expirationCount);
    Q_EMIT clockSkewed("");
}

}
