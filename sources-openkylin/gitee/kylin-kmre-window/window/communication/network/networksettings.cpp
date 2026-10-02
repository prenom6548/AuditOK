/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "networksettings.h"
#include "utils.h"
#include "dbusclient.h"
#include "kmreenv.h"

#include <QStandardPaths>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

namespace kmre {
namespace network {

NetworkSettings::NetworkSettings()
    : mConfig(nullptr)
{
    mConfig = new NetworkConfig(KmreEnv::GetKmreConfigFile());
    updateNetworkSettings();
    updateTunMetaData();
}

NetworkSettings::~NetworkSettings()
{
    delete mConfig;
}

void NetworkSettings::resetDnsProperties()
{
    int index = 1;
    foreach (QString dns, mDnsList) {
        QString dnsProp = "sys.custom.dns" + QString::number(index++);
        kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), dnsProp, QString(""));
        kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), dnsProp, QString(""));
    }

    mDnsList.clear();
}

void NetworkSettings::updateNetworkSettings()
{
    QString httpProxyHost;
    unsigned int httpProxyPort;
    QStringList dnsList;
    bool shouldResetDnsList = false;

    httpProxyHost = mConfig->getHttpProxyHost();
    httpProxyPort = mConfig->getHttpProxyPort();
    dnsList = mConfig->getDnsList();

    if (httpProxyHost.length() > 0 && httpProxyPort > 0) {
        if (httpProxyHost != mHttpProxyHost) {
            mHttpProxyHost = httpProxyHost;
            kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.httpproxy.host", mHttpProxyHost);
            kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.httpproxy.host", mHttpProxyHost);
        }

        if (httpProxyPort != mHttpProxyPort) {
            mHttpProxyPort = httpProxyPort;
            kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.httpproxy.port", QString::number(mHttpProxyPort));
            kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.httpproxy.port", QString::number(mHttpProxyPort));
        }
    }

    if (dnsList.size() != mDnsList.size()) {
        shouldResetDnsList = true;
        goto resetDnsList;
    }

    foreach (QString dns, dnsList) {
        if (!mDnsList.contains(dns)) {
            shouldResetDnsList = true;
            break;
        }
    }

resetDnsList:
    if (shouldResetDnsList) {
        int index = 1;
        resetDnsProperties();
        mDnsList = dnsList;

        foreach (QString dns, mDnsList) {
            QString dnsProp = "sys.custom.dns" + QString::number(index++);
            kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), dnsProp, dns);
            kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), dnsProp, dns);
        }
    }
}

void NetworkSettings::updateTunMetaData()
{
    struct stat sb;
    int tunMajor = 0;
    int tunMinor = 0;

    if (stat("/dev/net/tun", &sb) != 0) {
        return;
    }

    if ((sb.st_mode & S_IFMT) != S_IFCHR) {
        return;
    }

    tunMajor = major(sb.st_rdev);
    tunMinor = minor(sb.st_rdev);

    if (tunMajor > 0 && tunMinor > 0) {
        kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.tun.device.major", QString::number(tunMajor));
        kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.tun.device.major", QString::number(tunMajor));
        kmre::DbusClient::getInstance()->SetDefaultPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.tun.device.minor", QString::number(tunMinor));
        kmre::DbusClient::getInstance()->SetPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.tun.device.minor", QString::number(tunMinor));
    }
}

} // namespace network
} // namespace kmre
