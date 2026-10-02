/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "usdqjsonconf.h"
#include "clib-syslog.h"

UsdQJsonConf::UsdQJsonConf()
{

}

QJsonObject UsdQJsonConf::loadJsonObject(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING,"Cannot open file for reading: %s cuz:[%s]" ,
                filePath.toLatin1().data(), file.errorString().toLatin1().data());
        return QJsonObject();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        SYS_LOG(LOG_WARNING,"Error parsing JSON file: %s" ,filePath.toLatin1().data());
        return QJsonObject();
    }
    return doc.object();
}

void UsdQJsonConf::saveJsonObject(const QString &filePath, const QJsonObject &obj)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING,"Cannot open file for reading: %s cuz:[%s]" ,
                filePath.toLatin1().data(),file.errorString().toLatin1().data());
        return;
    }
    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.setPermissions(QFileDevice::Permission(0x644));
    file.flush();
    file.close();
}

bool UsdQJsonConf::mergeJsonObjects(QJsonObject &targetObj, const QJsonObject &srcObj)
{
    if (targetObj == srcObj || srcObj.isEmpty()) {
        return false;
    }

    for (QString key : srcObj.keys()) {
        if (!targetObj.keys().contains(key) || targetObj.value(key) != srcObj.value(key)) {
            targetObj.insert(key, srcObj.value(key));
        }
    }
    return true;
}




