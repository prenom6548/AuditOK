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

#ifndef USDQJSONCONF_H
#define USDQJSONCONF_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>


class UsdQJsonConf
{
public:
    UsdQJsonConf();

    /**
     * @brief readJsonFile
     * @param filePath
     * @return Json object
     */
    static QJsonObject loadJsonObject(const QString &filePath);

    /**
     * @brief writeJsonFile
     * @param filePath
     * @param obj
     */
    static void saveJsonObject(const QString &filePath, const QJsonObject &obj);

    /**
     * @brief mergeJsonObjects
     * @param targetObj
     * @param srcObj
     * @return
     */
    static bool mergeJsonObjects(QJsonObject &targetObj, const QJsonObject &srcObj);
};

#endif // USDQJSONCONF_H
