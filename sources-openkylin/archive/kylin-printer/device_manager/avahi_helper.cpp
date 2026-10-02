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
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/thread-watch.h>
#include <iostream>
#include <arpa/inet.h>

#include <QUrl>
#include <QRegularExpression>

#include "avahi_helper.h"
#include "common.h"

// 全局变量声明（避免重复）
AvahiThreadedPoll* threaded_poll = nullptr;
AvahiServiceResolver* resolver = nullptr;
QString ipAddress{QString()};

void resolve_callback(
    AvahiServiceResolver* r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char* name,
    const char* type,
    const char* domain,
    const char* host_name,
    const AvahiAddress* address,
    uint16_t port,
    AvahiStringList* txt,
    AvahiLookupResultFlags flags,
    void* userdata) {

    if (event == AVAHI_RESOLVER_FOUND) {
        char ip[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(ip, sizeof(ip), address);
        ipAddress = QString(ip);
    }

    avahi_service_resolver_free(r);
    resolver = nullptr;
    avahi_threaded_poll_quit(threaded_poll);
}

bool searchip(const char *instanceName,
              const char *serviceType,
              const char *domain)
{
    if (!instanceName || !serviceType || !domain) {
        return false;
    }

    // 初始化线程轮询对象
    threaded_poll = avahi_threaded_poll_new();
    if (!threaded_poll) {
        qDebug() << "Failed to create threaded poll object.";
        return false;
    }

    // 创建 Avahi 客户端
    AvahiClient* client = avahi_client_new(
        avahi_threaded_poll_get(threaded_poll),
        AVAHI_CLIENT_NO_FAIL,
        nullptr, nullptr, nullptr);
    if (!client) {
        qDebug() << "Failed to create Avahi client: " 
                  << avahi_strerror(avahi_client_errno(client));
        avahi_threaded_poll_free(threaded_poll);
        return false;
    }

    // 解析服务
    resolver = avahi_service_resolver_new(
        client,
        AVAHI_IF_UNSPEC,
        AVAHI_PROTO_UNSPEC,
        instanceName,
        serviceType,
        domain,
        AVAHI_PROTO_UNSPEC,
        AVAHI_LOOKUP_USE_MULTICAST,
        resolve_callback,
        nullptr);
    if (!resolver) {
        qDebug()  << "Failed to create resolver: " 
                  << avahi_strerror(avahi_client_errno(client));
        avahi_client_free(client);
        avahi_threaded_poll_free(threaded_poll);
        return false;
    }

    // 启动事件循环
    avahi_threaded_poll_start(threaded_poll);

    // 等待回调完成
    while (resolver != nullptr) {
        avahi_threaded_poll_lock(threaded_poll);
        avahi_threaded_poll_unlock(threaded_poll);
    }

    // 清理资源
    if (client) avahi_client_free(client);
    if (threaded_poll) avahi_threaded_poll_free(threaded_poll);

    return true;
}

bool parseDnssdUrl(const QString &url, QString &instanceName, QString &serviceType, QString &domain) {
    // 示例输入: "ipp://Kyocera%20ECOSYS%20P3045dn._ipp._tcp.local/"
    // 1. 提取主机名部分（去掉协议头和末尾斜杠）
    QString fullServiceName;
    int protocolEnd = url.indexOf("://");
    if (protocolEnd != -1) {
        fullServiceName = url.mid(protocolEnd + 3); // "Kyocera%20ECOSYS%20P3045dn._ipp._tcp.local/"
        fullServiceName = fullServiceName.split('/').first(); // 去掉末尾斜杠
    } else {
        qWarning() << "Invalid URL: Missing protocol";
        return false;
    }

    // 2. URL 解码（将 %20 转为空格等）
    fullServiceName = QUrl::fromPercentEncoding(fullServiceName.toUtf8());

    // 2. 使用正则表达式拆分
    QRegularExpression regex(
        R"(^(.*?)\._([a-z]+)\._([a-z]+)\.([a-z]+)$)", 
        QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = regex.match(fullServiceName);

    if (match.hasMatch()) {
        instanceName = match.captured(1);  // "Kyocera ECOSYS P3045dn"
        serviceType = "_" + match.captured(2) + "._" + match.captured(3); // "_ipp._tcp"
        domain = match.captured(4);       // "local"

        // qDebug() << "Original URL:" << url;
        // qDebug() << "Instance Name:" << instanceName;
        // qDebug() << "Service Type:" << serviceType;
        // qDebug() << "Domain:" << domain;
    
        return true;
    }
    
    // qDebug() << "Invalid DNSSD service URL format";

    return false;
}


AvahiHelper::AvahiHelper()
{

}

AvahiHelper::~AvahiHelper()
{

}

QString AvahiHelper::search(const QString &uri)
{
    QString instanceName;
    QString serviceType;
    QString domain;
    if (!parseDnssdUrl(uri, instanceName, serviceType, domain) ) {
        return QString();
    }

    if (!searchip(QTC(instanceName), QTC(serviceType), QTC(domain))){
        return QString();
    }

    return ipAddress;    
}
