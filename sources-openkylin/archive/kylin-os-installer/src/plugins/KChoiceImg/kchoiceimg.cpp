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

#include "kchoiceimg.h"
#include "imgframe.h"
using namespace KInstaller;
KChoiceImg::KChoiceImg()
{
    this->setObjectName("KChoiceImg");
}


KChoiceImg::~KChoiceImg()
{
}

QWidget* KChoiceImg::CreateObject(QWidget *parent)
{
    m_frameLicense = new ImgFrame(parent);
    return m_frameLicense;
}
QWidget* KChoiceImg::getWidget()
{
      return m_frameLicense;
}

QString KChoiceImg::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KChoiceImg::next()
{

}
void KChoiceImg::back()
{

}
void KChoiceImg::onActive()
{

}
void KChoiceImg::onLeave()
{

}
