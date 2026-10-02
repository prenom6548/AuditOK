/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "serviceinterface.h"
#include <QDBusConnection>
#include <QDebug>

ServiceInterface::ServiceInterface() : m_securityQuestionAnswer(new SecurityQuestionAnswer(this))
{
    bool res = QDBusConnection::systemBus().registerService("org.kylin.KprBackend");
    if (!res) {
        qInfo() << "registerService org.kylin.KprBackend failed!!";
        exit(0);
    }

    res = QDBusConnection::systemBus().registerObject(
        "/SecurityQuestionAnswer",
        "org.kylin.KprBackend",
        m_securityQuestionAnswer,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    if (!res) {
        qInfo() << "registerObject /SecurityQuestionAnswer failed!!";
    }
}
