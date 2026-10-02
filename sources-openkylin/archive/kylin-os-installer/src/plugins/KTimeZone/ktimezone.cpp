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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "ktimezone.h"
#include "timezoneframe.h"

using namespace KInstaller;

KTimeZone::KTimeZone()
{
    this->setObjectName("KTimeZone");
}

KTimeZone::~KTimeZone()
{
}

QWidget* KTimeZone::CreateObject(QWidget *parent)
{
    m_frameTimeZone = new TimeZoneFrame(parent);
    return m_frameTimeZone;
}

QWidget* KTimeZone::getWidget()
{
      return m_frameTimeZone;
}

QString KTimeZone::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KTimeZone::next()
{

}

void KTimeZone::back()
{

}

void KTimeZone::onActive()
{

}

void KTimeZone::onLeave()
{

}
