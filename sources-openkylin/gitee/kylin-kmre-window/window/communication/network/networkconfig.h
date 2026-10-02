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

#ifndef KMRE_NETWORK_CONFIG_H
#define KMRE_NETWORK_CONFIG_H

#include <QStringList>

namespace kmre {
namespace network {

class NetworkConfig
{
public:
    NetworkConfig(const QString& path);
    ~NetworkConfig();
    void setDnsList(const QStringList& dnsList);
    QStringList getDnsList();
    void setHttpProxyHost(const QString& host);
    QString getHttpProxyHost();
    void setHttpProxyPort(unsigned int port);
    unsigned int getHttpProxyPort();

private:
    QString getValueFromConfig(const QString& group, const QString& key);
    void setValueToConfig(const QString& group, const QString& key, const QString& value);

    QString mConfigFilePath;

};

} // network
} // kmre

#endif // KMRE_NETWORK_CONFIG_H
