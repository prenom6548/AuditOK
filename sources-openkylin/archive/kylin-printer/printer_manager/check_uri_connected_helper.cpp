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
#include "check_uri_connected_helper.h"
#include "lpinfo_browser.h"
#include "common.h"
#include "connect.h"
#include "gsettings_monitor.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CheckUriConnectedHelper::CheckUriConnectedHelper(const QString &uri) :
    m_uri(uri)
{

}

CheckUriConnectedHelper::~CheckUriConnectedHelper()
{

}


bool check(const QJsonObject &jsonObject, const QString &uri)
{
    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        if (it.key() != uri) {
            continue;
        }

        QJsonObject subObj = it.value().toObject();

        if (subObj.contains("path") && subObj.value("path").isString()) {
            QString path = subObj.value("path").toString();
            if (path.isEmpty()) {
                continue;
            } else {
                return true;
            }
        }
    }
    return false;
}

bool checkInGsetting(const QString &uri)
{
    QByteArray s = GsettingsMonitor::getInstance()->printerDevicesGet();

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(s, &jsonParserError);
    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug () << "json解析失败";
        return false;
    }
    if (!jsonDocument.isObject()) {
        return false;
    }
    QJsonObject jsonObject = jsonDocument.object();
    qDebug() << jsonObject;
    if (jsonObject.size() == 0) {
        qDebug() << "json is empty!";
        return false;
    }

    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        if (it.key() == "uris" && it.value().isObject()) {
            if (check(it.value().toObject(), uri)) {
                return true;
            } else {
                continue;
            }
        }
    }
    return false;
}


bool checkUri(const QString &uri)
{
    auto mp = LpinfoBrowser::findAllPrinter();
    if (mp.contains(uri)) {
        return true;
    }
    // usb的也在gsetting里面扫一下
    if (checkInGsetting(uri)) {
        return true;
    }

    const QString uriSplit = ":/";
    QString uriScheme = uri.left(uri.indexOf(uriSplit));
    if (uriScheme != "ipp"
     && uriScheme != "ipps"
     && uriScheme != "http"
     && uriScheme != "https"
     && uriScheme != "socket"
     && uriScheme != "smb"
     && uriScheme != "lpd") {
        return false;
     }

    QString ip = uri.right(uri.size() - uriSplit.size() - uri.indexOf(uriSplit));
    if (ip.isEmpty()) {
        return false;
    }

    if (ip.left(1) == "/") {
        ip.remove(0, 1);
    }

    if (ip.isEmpty()) {
        return false;
    }

    if (ip.contains("/")) {
        ip = ip.split("/").at(0);
    }

    if (ip.isEmpty()) {
        return false;
    }

    if (ip.contains(":")) {
        ip = ip.split(":").at(0);
    }

    if (!ipIsValid(ip)) {
        return false;
    }

    if (pingCheck(ip)) {
        return true;
    }
    return false;
}

void CheckUriConnectedHelper::dowork()
{
    if (m_uri.isEmpty()) {
        return;
    }

    emit result(checkUri(m_uri));
}
