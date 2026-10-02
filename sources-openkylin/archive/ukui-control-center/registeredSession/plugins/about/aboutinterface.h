/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef ABOUTINTERFACE_H
#define ABOUTINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QMutex>
#include <QDBusArgument>
#include <QVariant>
#include <QDBusMetaType>
#include <QGSettings>
#include "../../universalinterface.h"
#include <kysdk/kysdk-system/libkysysinfo.h>

struct dnsInfo
{
    char *servname;
    pthread_t pid;
};

class AboutInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.About")
    Q_PROPERTY(QStringList versionInfo READ getSystemVersion)
    Q_PROPERTY(QString themeMode READ getThemeMode)
    Q_PROPERTY(QStringList build READ getBuild)
    Q_PROPERTY(QString desktop READ getDesktopEnv)
    Q_PROPERTY(QString memory READ getTotalMemory)
    Q_PROPERTY(QString blockInfo READ getBlockInfo)
    Q_PROPERTY(QString diskInfo READ getDiskInfo)
    Q_PROPERTY(QString upgradeDate READ getUpgradeDate)
    Q_PROPERTY(QString netDate READ getNtpTime)
    Q_PROPERTY(QString availableMemory READ getAvailableMemory)
public:
    AboutInterface();

protected:
    QStringList getSystemVersion();
    QString getThemeMode();
    QStringList getBuild();
    QString getDesktopEnv();
    QString getTotalMemory();
    QString getBlockInfo();
    QString getDiskInfo();
    QString getUpgradeDate();
    QString getNtpTime();
    QString getAvailableMemory();

public Q_SLOTS:
    bool openActivation();
    void setThemeMode(QString styleName);

Q_SIGNALS:
    void changed(const QString &key);

private:
    QStringList readFile(QString filepath);
    int ntpGethostbyname(char *servname);
    char *ntpdate(char *hostname);
    static void *threadGetAddrInfo(void *arg);
    void getSystemVersionList();
    void getBuildList();
    void getDesktopEnvStr();
    void getTotalMemoryStr();
    void getBlockInfoStr();

private:
    QGSettings *mStyleGsettings = nullptr;
    QStringList mNtphostNameList;
    static QStringList ipList;
    static QMutex ipListMutex;
    QStringList mSystemVersion;
    QStringList mBuild;
    QString mDesktopEnv;
    QString mTotalMem;
    QString mBlockInfo;
    QString m_availableMem;

};
//QStringList AboutInterface::ipList = QStringList();

#endif // ABOUTINTERFACE_H
