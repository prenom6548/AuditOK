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

#ifndef FORMATCONVERSION_H
#define FORMATCONVERSION_H
#include <QObject>
#include <QString>
namespace kmre_dataformat
{
class FormatConversion : public QObject
{
public:
    static QString dataConversion(QString locationData);
    static QString decodeToOrigin();

private:
    static QString dataCalculation(QString locationData);
    static QString toGPSPoint(QString locationData);
    static QString calDev(double wgLat, double wgLon);
    static bool isOutOfChina(double lat, double lon);
    static double calLat(double x, double y);
    static double calLon(double x, double y);
};

} // namespace kmre_dataformat

#endif // FORMATCONVERSION_H
