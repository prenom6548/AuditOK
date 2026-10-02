/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
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

#ifndef GPSGLOBAL_H
#define GPSGLOBAL_H
#include <QString>
namespace kmre_gps
{
struct ItemData
{
    QString province = "";
    QString cityName = "";
    QString adName = "";
    QString address = "";
    QString name = "";
    QString location = "";
};
const QString DEFAULTLOCATION = "北京市(默认位置)";
const QString DEFAULTGPSDATA = "116.397428, 39.90923";
const QString GPS_GSETTINGS_SERVICENAME = QString("org.kylin-gps.settings");
const QString CODESTR = "NmFjMTJiNDUwMw==";
} // namespace kmre_gps

#endif // GPSGLOBAL_H
