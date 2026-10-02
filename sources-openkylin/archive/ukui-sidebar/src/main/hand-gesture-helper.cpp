/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "hand-gesture-helper.h"
#include <mutex>
#include <QDebug>

#include "screen-monitor.h"
using namespace Sidebar;

static std::once_flag flag;
static HandGestureHelper *global_intance = nullptr;
HandGestureHelper *HandGestureHelper::getInstance()
{
    std::call_once(flag, [ & ] {
       global_intance = new HandGestureHelper();
   });
   return global_intance;
}

HandGestureHelper::~HandGestureHelper()
{
}

void HandGestureHelper::callNotificationCenter(int posY)
{
//   qDebug() << "callNotificationCenter---" << posY;
   Q_EMIT notificationCenterCalled(posY);
}

void HandGestureHelper::callControlCenter(int posX)
{
//    qDebug() << "callControlCenter---" << posX;
    Q_EMIT controlCenterCalled(posX);
}

void HandGestureHelper::top2BottomRelease(int posX, int posY)
{
    Q_EMIT top2BottomReleased(posX, posY);

}

void HandGestureHelper::right2LeftRelease(int posX, int posY)
{
    Q_EMIT right2LeftReleased(posX, posY);
}

HandGestureHelper::HandGestureHelper(QObject *parent) : QObject(parent)
{
}
