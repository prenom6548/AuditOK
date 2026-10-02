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

#include "klinguist.h"
#include <QCoreApplication>
#include <QDebug>
#include <QLocale>

namespace KServer {



KLinguist *KLinguist::m_linguist = nullptr;

KLinguist::KLinguist()
{
    trans = new QTranslator;
    currentLanguage = QLocale::system().name();
    qDebug() << "72b:"<< currentLanguage;
//    currentLanguage = "zh_CN";
    changeLanguage(currentLanguage);
}

KLinguist *KLinguist::getLinguistInstance()
{
    if (nullptr == m_linguist) {
        m_linguist = new KLinguist;
    }
    return m_linguist;
}

void KLinguist::changeLanguage(QString str)
{
    qDebug() << "KLinguist: " << str;
    if (nullptr != trans)
        qApp->removeTranslator(trans);

    if (currentLanguage == "C")
        str = "en_US";

    bool b = trans->load(":/resources/translations/" + str);

    if (!b) {
        str = "zh_CN";
        trans->load(":/resources/translations/" + str);
    }

    setCurrentLanguage(str);
    qDebug() << str << ".qm install bool: " << b;

    qApp->installTranslator(trans);

    emit languageChange();
}

void KLinguist::setCurrentLanguage(QString str)
{
    currentLanguage = str;
}

QString KLinguist::getCurrentLanguage()
{
    qDebug() << "KLinguist languagez: " << currentLanguage;
    return currentLanguage;
}
}
