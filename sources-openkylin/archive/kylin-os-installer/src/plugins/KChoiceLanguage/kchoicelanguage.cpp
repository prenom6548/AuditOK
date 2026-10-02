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

#include "kchoicelanguage.h"
#include "languageframe.h"

using namespace KInstaller;

KChoiceLanguage::KChoiceLanguage()
{
}

KChoiceLanguage::~KChoiceLanguage()
{
}

QWidget* KChoiceLanguage::CreateObject(QWidget *parent)
{
    m_frameLanguage = new LanguageFrame(parent);
    return m_frameLanguage;
}

QWidget* KChoiceLanguage::getWidget()
{
      return m_frameLanguage;
}

QString KChoiceLanguage::getWidgetName()
{
    QString s = "";
    return (QString)s;
}

void KChoiceLanguage::next()
{

}

void KChoiceLanguage::back()
{

}

void KChoiceLanguage::onActive()
{

}

void KChoiceLanguage::onLeave()
{

}
