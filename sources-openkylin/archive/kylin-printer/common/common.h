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
#ifndef KYLIN_PRINTER_COMMON_H
#define KYLIN_PRINTER_COMMON_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <map>
#include "custom_qstring_converter.h"
#include "package_info.h"

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

bool execCommand(const QStringList &command, QString *res, QString *standardError, int *exitCode, int *msec);

void runCommand(const QStringList &command);

QString getUserName();

bool isAdministratorUser();

const QMap<QString, QString> mfgConvertMap {
    {"Lenovo Image",    "Lenovo"},
    {"Fuji Xerox",      "Fuji-Xerox"},
    {"INDEX BRAILLE",   "INDEX-BRAILLE"},
    {"KONICA MINOLTA",  "KONICA-MINOLTA"},
    {"Hewlett-Packard", "HP"},
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

extern bool packageIsInstalled(const QString &name, const QString &version);

extern bool packageListNeedInstall(const QList<PackageInfo> &list);

extern QString encodeUrl(const QString &in);

extern QString  decodeUrl(const QString &in);

extern void delay_msec(unsigned int msec);

extern bool isV11version();

extern bool isDebFile(const QString &filePath);

extern bool isFile(const QString &filePath);

#endif // KYLIN_PRINTER_COMMON_H
