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
#include <arpa/inet.h>
#include "network.h"
#include "common.h"

bool ipv6IsValid(const QString &ipAddr)
{
    struct sockaddr_in6 sa;
    return inet_pton(AF_INET6, QTC(ipAddr), &(sa.sin6_addr)) == 1;
}

bool ipv6UriCheck(const QString &uri)
{
    // the uri is just like 'socket://[fe80::3e52:82ff:fe26:4b20%enp1s0]' via smfpnetdiscovery
    const QString uriSplit = "://";
    if(!uri.contains(uriSplit)) {
        return false;
    }

    QString uriScheme = uri.left(uri.indexOf(uriSplit));
    QString host = uri.right(uri.size() - uriSplit.size() - uri.indexOf(uriSplit));

    if (uriScheme == "socket") {
        if (host.contains('[')) {
            host.remove('[');
        }

        if (host.contains(']')) {
            host.remove(']');
        }

        if(host.isEmpty()) {
            return false;
        }

        QString ipv6Address(host);
        if (host.contains("%")) {
            ipv6Address = host.split("%").at(0);
        }


        return ipv6IsValid(ipv6Address);
    }


    return false;
}
