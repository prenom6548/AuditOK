/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kylinagent.h"
#include <QApplication>
#include <QTranslator>

KylinAgent::KylinAgent(QObject *parent) : QObject(parent)
{
    QLocale local;
    QString locale = /*QLocale::system().*/local.name();
    QTranslator* translator = new QTranslator(this);
    if (translator->load("/usr/share/kylin-nm/kylin-keyring/kylin-keyring_" + locale)) {
        QApplication::installTranslator(translator);
    } else {
//        qWarning() << "Translations load fail";
    }
}

KylinAgent::~KylinAgent()
{

}

void KylinAgent::startKylinAgent()
{
    m_secretAgnet = new KylinSecretAgent("kylinScreenAgent", pWidget, this);
    // 转发信号
    connect(m_secretAgnet, &KylinSecretAgent::requestWirelessSecret,
            this, &KylinAgent::requestWirelessSecret);
    connect(m_secretAgnet, &KylinSecretAgent::requestEnterpriseSecret,
            this, &KylinAgent::requestEnterpriseSecret);
}

void KylinAgent::provideWirelessSecret(const QString &requestId,
                                       const QString &password,
                                       bool autoConnect)
{
    if (m_secretAgnet)
        m_secretAgnet->provideWirelessSecret(requestId, password, autoConnect);
}

void KylinAgent::provideEnterpriseSecret(const QString &requestId,
                                         const QVariantMap &secretMap)
{
    if (m_secretAgnet)
        m_secretAgnet->provideEnterpriseSecret(requestId, secretMap);
}

void KylinAgent::cancelSecretRequest(const QString &requestId)
{
    if (m_secretAgnet)
        m_secretAgnet->cancelSecretRequest(requestId);
}

//void KylinAgent::stopKylinAgent()
//{
//    delete m_kylinAgnetObject;
//    m_kylinAgnetObject = nullptr;
//    return;
//}
