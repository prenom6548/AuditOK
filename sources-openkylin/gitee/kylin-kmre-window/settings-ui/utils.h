/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef UTILS_H
#define UTILS_H

#include <QApplication>
#include <QWidget>
#include <vector>
#include <QDesktopWidget>
#include <QLockFile>
#include <tuple>

namespace kmre {
namespace utils {

    void centerToScreen(QWidget *widget);
    const QString& getUserName();
    const QString& getUid();
    //bool isWayland();
    QString readFileContent(const QString &fileName);
    QString getIconPath(const QString &packageName);
    QString makeContainerName();
    QString makeContainerName(const QString &userName, int uid);
    QByteArray getInstalledAppListJsonStr();
    std::vector<std::pair<QString, QString>> getInstalledAppList();
    QString execCmd(const QString &cmd, int msec = 3000);
    bool checkAndCreateFile(const QString& filePath, int permission);
    QByteArray tryReadFile(const QString& filePath, bool lock, int msecond);
    bool tryWriteFile(const QString& filePath, const QByteArray& data, bool lock, int msecond);
    std::tuple<bool, QString> getElideText(const QString& origString, const uint32_t maxLength, 
            Qt::TextElideMode mode = Qt::ElideRight);
}
}

#endif // UTILS_H
