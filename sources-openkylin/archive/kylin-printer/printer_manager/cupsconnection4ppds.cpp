/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include "cupsconnection4ppds.h"
#include "ukui_printer.h"

//QMutex CupsConnection4PPDs::m_mutex;
CupsConnection4PPDs::CupsConnection4PPDs()
{
    qDebug() << "CUPS开始建立" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    m_server = cupsServer();

    m_ippPortNum = ippPort();

    m_encryptionType = cupsEncryption();

    int cancel = 0;
    m_ppdRequestConnection = httpConnect2(m_server, m_ippPortNum, nullptr, AF_UNSPEC, (http_encryption_t)m_encryptionType, 1, 30000, &cancel);

    if (!m_ppdRequestConnection)
    {
        throw std::runtime_error("Failed to construct ppdRequestConnection!");
    }
    else
    {
        qDebug() << "CUPS服务ppdRequestConnection建立成功!";
    }


    qDebug() << "CUPS建立完成" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
}

CupsConnection4PPDs::~CupsConnection4PPDs()
{
    delete m_server;
}
http_t* CupsConnection4PPDs::getCupsConnection() const
{
    return m_ppdRequestConnection;
}