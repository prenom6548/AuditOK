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

#include "kpartition.h"
#include <QApplication>
#include <QLibrary>
#include <QDebug>

KPartition::KPartition()
{
    this->setObjectName("KPartition");
}
KPartition::~KPartition()
{
}

QWidget* KPartition::CreateObject(QWidget *parent)
{
    m_mainPartFrame = new MainPartFrame(parent);
    return m_mainPartFrame;
}
MainPartFrame* KPartition::getWidget()
{
      return (MainPartFrame*)m_mainPartFrame;
}
QString KPartition::getWidgetName()
{
    QString s = "KPartition";
    return s;
}

void KPartition::next()
{

}
void KPartition::back()
{

}
void KPartition::onActive()
{

}
void KPartition::onLeave()
{

}
