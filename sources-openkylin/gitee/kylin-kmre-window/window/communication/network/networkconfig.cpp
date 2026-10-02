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

#include "networkconfig.h"

#include <QSettings>
#include <QFile>

namespace kmre {
namespace network {


static bool isIpAddressValid(const QString ipAddr)
{
    QRegExp regExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (regExp.exactMatch(ipAddr)) {
        return true;
    }

    return false;
}

NetworkConfig::NetworkConfig(const QString &path)
    : mConfigFilePath(path)
{
}

NetworkConfig::~NetworkConfig()
{
}

QString NetworkConfig::getValueFromConfig(const QString& group, const QString& key)
{
    QString value;
    if (QFile::exists(mConfigFilePath)) {
        QSettings settings(mConfigFilePath, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        if (settings.childGroups().contains(group)) {
            settings.beginGroup(group);
            value = settings.value(key, QString("")).toString();
            settings.endGroup();
        }
    }

    return value;
}

void NetworkConfig::setValueToConfig(const QString& group, const QString& key, const QString& value)
{
    QSettings settings(mConfigFilePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();

}


void NetworkConfig::setDnsList(const QStringList &dnsList)
{
    QString value;

    foreach (QString dns, dnsList) {
        if (isIpAddressValid(dns)) {
            if (value.length() == 0) {
                value += dns;
            } else {
                value += " ";
                value += dns;
            }
        }
    }

    if (value.length() > 0) {
        setValueToConfig("network", "dns_list", value);
    }
}

QStringList NetworkConfig::getDnsList()
{
    QStringList dnsList;
    QString dnsListString = getValueFromConfig("network", "dns_list");
    if (dnsListString.length() > 0) {
        QStringList dnses = dnsListString.split(" ");
        foreach (QString dns, dnses) {
            if (isIpAddressValid(dns)) {
                dnsList.append(dns);
            }
        }
    }

    return dnsList;
}

void NetworkConfig::setHttpProxyHost(const QString &host)
{
    if (host.length() > 0) {
        setValueToConfig("network", "http_proxy_host", host);
    }
}

QString NetworkConfig::getHttpProxyHost()
{
    return getValueFromConfig("network", "http_proxy_host");
}

void NetworkConfig::setHttpProxyPort(unsigned int port)
{
    QString portString = QString::number(port);
    if (portString.length() > 0) {
        setValueToConfig("network", "http_proxy_port", portString);
    }
}

unsigned int NetworkConfig::getHttpProxyPort()
{
    QString portString = getValueFromConfig("network", "http_proxy_port");
    if (portString.length() > 0) {
        int port = portString.toInt();
        if (port >= 0) {
            return portString.toUInt();
        }
    }

    return 0;
}

} // network
} // kmre
