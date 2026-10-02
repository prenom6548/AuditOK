/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef KYLIN_PRINTER_COMMON_H
#define KYLIN_PRINTER_COMMON_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <map>

#define QTC(x) (qstringTochar(x).data())

enum CheckRes{
    CHECK_FALSE = 0,
    MAINWINDOW,
    POPWINDOW,
    DEBUG,
};

enum class PopWinStatus : int
{
    HIDE = 0,
    INSTALLING,
    INSTALL_SUCCESS,
    INSTALL_FAIL,
};

CheckRes checkCommand(const QStringList &command);

QString getRetFromCommand(const QStringList &command);

void runCommand(const QStringList &command);

QByteArray qstringTochar(const QString &qstr);

QString getUserName();

bool isAdministratorUser();

const QMap<QString, QString> mfgConvertMap = {
    std::map<QString, QString>::value_type("Lenovo Image", "Lenovo"),
    std::map<QString, QString>::value_type("Fuji Xerox", "Fuji-Xerox"),
    std::map<QString, QString>::value_type("INDEX BRAILLE", "INDEX-BRAILLE"),
    std::map<QString, QString>::value_type("KONICA MINOLTA", "KONICA-MINOLTA"),
    std::map<QString, QString>::value_type("Hewlett-Packard", "HP")
};

void specialDeviceCheck(QString &vendor, QString &model);

extern QString devicePathCheck(const QString &path);

extern bool ipIsValid(const QString &ipAddr);

extern bool printerNameIsLegal(const QString &printerName);

enum class CommonStatusFlag : int
{
    NONE = 0,
    SUCCESS,
    FAIL,

    ERROR_EXIST_SAME_PRINTER,
};

struct CommonStatus {
    CommonStatusFlag status{CommonStatusFlag::NONE};
    QString message{QString()};
};

extern bool processCheck(const QString &processName);

extern QString getPackageVersion(const QString &packageName);

extern QString getDebVersion(const QString &debName);


#endif // KYLIN_PRINTER_COMMON_H
