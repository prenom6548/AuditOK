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

#include "kylicense.h"
#include "licenseframe.h"
using namespace KInstaller;
KyLicense::KyLicense()
{
}


KyLicense::~KyLicense()
{
}

QWidget* KyLicense::CreateObject(QWidget *parent)
{
    m_frameLicense = new LicenseFrame(parent);
    return m_frameLicense;
}
QWidget* KyLicense::getWidget()
{
      return m_frameLicense;
}

QString KyLicense::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KyLicense::next()
{

}
void KyLicense::back()
{

}
void KyLicense::onActive()
{

}
void KyLicense::onLeave()
{

}
