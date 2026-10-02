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
#ifndef SMBCLIENT_HELPER
#define SMBCLIENT_HELPER

#include <QString>
#include <QStringList>
#include <QObject>
#include <QThread>
#include "device_information.h"

class SMBAuthData
{
public:
    QString host{QString()};
    QString share{QString()};
    QString workgroup{QString()};
    QString username{QString()};
    QString password{QString()};
    QList<DeviceInformation> smbPrinterList{QList<DeviceInformation>()};
};

class SmbClientHelper : public QObject
{
    Q_OBJECT
public:
    SmbClientHelper(SMBAuthData &data, QObject *parent = nullptr);
    ~SmbClientHelper();
    
    DeviceInformation constructSmbDevice(const QString &share);
    QString constructSmbUri(const QString &host,
                            const QString &share,
                            const QString &workgroup = QString(),
                            const QString &username = QString(),
                            const QString &password = QString());

    int cconnect(const QString &host, const QString &share);
    int browse(const QString &host);
    bool checkSmbServer(const QString &host);

    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
Q_SIGNALS:
    void finished();
    void succeed(int status, QString errroMsg);
    void failed();
private:
    void dowork();
    SMBAuthData &m_data;
    QThread *m_thread{nullptr};
};


#endif // SMBCLIENT_HELPER
