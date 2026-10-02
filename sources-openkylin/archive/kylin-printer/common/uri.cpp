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
#include "uri.h"
#include "common.h"

Uri::Uri(const QString &str) :
    m_rawString(str)
{
    scheme = m_rawString.left(m_rawString.indexOf(uriSplit));
    protocolType = protocolMap[scheme];
    getDeviceType();
    if (protocolType == ProtocolType::IPP
     || protocolType == ProtocolType::IPPS
     || protocolType == ProtocolType::SOCKET) {

        getIpAddress();
    }
}

Uri::~Uri()
{

}

DeviceType Uri::getDeviceType()
{
    switch (protocolType)
    {
    case ProtocolType::NONE:
        deviceType = DeviceType::NONE;
        break;
    case ProtocolType::LPD:
        deviceType = DeviceType::LPD;
        break;
    case ProtocolType::IPP:
    case ProtocolType::IPPS:
        deviceType = DeviceType::IPP;
        // driverless uri:
        // maybe ipp://printername(id)._ipp._tcp.local/
        // or ipps://printername(serial)._ipps._tcp.local/
        if (m_rawString.contains("._ipp._tcp.local") || m_rawString.contains("._ipps._tcp.local")) {

            QString host = m_rawString.right(m_rawString.size() - uriSplit.size() - m_rawString.indexOf(uriSplit));
            if (host.isEmpty()) {
                break;
            }

            while (host[0] == "/") {
                host.remove(0, 1);
            }

            if (host.contains("/")) {
                host = host.left(host.indexOf("/"));
            }

            if (host.isEmpty()) {
                break;
            }

            host.remove("._ipp._tcp.local");
            host.remove("._ipps._tcp.local");

            if (!host.isEmpty()) {
                driverlessDomain = decodeUrl(host);
                deviceType = DeviceType::DRIVERLESS;
            }
        }
        break;
    case ProtocolType::HTTP:
    case ProtocolType::HTTPS:
        deviceType = DeviceType::HTTP;
        break;
    case ProtocolType::SOCKET:
        deviceType = DeviceType::SOCKET;
        break;
    case ProtocolType::DNSSD:
        deviceType = DeviceType::DNSSD;
        break;
    case ProtocolType::SERIAL:
    case ProtocolType::PARALLEL:
        deviceType = DeviceType::SERIAL_AND_PRALALLEL;
        break;
    case ProtocolType::SMB:
        deviceType = DeviceType::SMB;
        break;
    case ProtocolType::USB:
        deviceType = DeviceType::USB;
        break;
    case ProtocolType::CUPS_PDF:
    case ProtocolType::CUPS_BRF:
        deviceType = DeviceType::VIRTUAL;
        break;
    case ProtocolType::GUTENPRINT:
        deviceType = DeviceType::GUTENPRINT;
        break;
    default:
        deviceType = DeviceType::NONE;
        break;
    }
    return deviceType;
}

QString Uri::getIpAddress()
{
    if (protocolType != ProtocolType::IPP
     && protocolType != ProtocolType::IPPS
     && protocolType != ProtocolType::SOCKET) {

        return QString();
    }

    QString host = m_rawString.right(m_rawString.size() - uriSplit.size() - m_rawString.indexOf(uriSplit));
    if (host.isEmpty()) {
        return QString();
    }

    while (host[0] == "/") {
        host.remove(0, 1);
    }

    if (host.contains("/")) {
        host = host.left(host.indexOf("/"));
    }

    if (host.isEmpty()) {
        return QString();
    }

    if (host.contains(":")) {
        host = host.split(":").at(0);
    }

    if (!ipIsValid(host)) {
        return QString();
    }

    ipAddress = host;

    return ipAddress;
}
