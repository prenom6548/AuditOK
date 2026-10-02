/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef UTIL_H
#define UTIL_H

#include <QWidget>
#include <QString>
#include <QTextStream>
//#include "recent.h"

class Settings;

namespace Util {

// platform specific
QString VersionFileUrl();
QString DownloadFileUrl();

bool DimLightsSupported();
void SetAlwaysOnTop(WId wid, bool);
QString SettingsLocation();

bool IsValidFile(QString path);
bool IsValidLocation(QString loc); // combined file and url

void ShowInFolder(QString path, QString file);

QString MonospaceFont();

// common
bool IsValidUrl(QString url);

QString FormatTime(int time, int totalTime);
QString FormatRelativeTime(int time);
QString FormatNumber(int val, int length);
QString FormatNumberWithAmpersand(int val, int length);
QString HumanSize(qint64 size);
//QString ShortenPathToParent(const Recent &recent);
QStringList ToNativeSeparators(QStringList list);
QStringList FromNativeSeparators(QStringList list);
int GCD(int v, int u);
QString Ratio(int w, int h);

}

inline QTextStream& qStdout()
{
    static QTextStream r{stdout};
    return r;
}

#endif // UTIL_H
