/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "klabel.h"
#include "../../ukui-greeter/common/configuration.h"
#include <QFont>
#include <QApplication>
#include <QToolTip>

KLabel::KLabel(QWidget *parent)
    : QLabel(parent),
      configuration(Configuration::instance())
{
     sysFont = qApp->font();
     m_ptToPx = configuration->getPtToPx();
}

void KLabel::setFontSize(int fontSize)
{
    sysFont.setPointSize(fontSize * m_ptToPx);
    this->setFont(sysFont);
}
