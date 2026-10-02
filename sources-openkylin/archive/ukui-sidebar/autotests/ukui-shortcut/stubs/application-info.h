/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#ifndef TEST_APPLICATION_INFO_H
#define TEST_APPLICATION_INFO_H

#include <QObject>
#include <QHash>
#include <QStringList>

namespace UkuiSearch {

class ApplicationInfo : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationInfo(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void desktopFilePathFromName(const QString &name, QString &path)
    {
        path = s_desktopFiles.value(name);
    }

    static void setDesktopFilePath(const QString &name, const QString &path)
    {
        s_desktopFiles.insert(name, path);
    }

    static void clearDesktopFilePaths()
    {
        s_desktopFiles.clear();
    }

Q_SIGNALS:
    void appDBItems2BDelete(const QStringList &desktopFiles);
    void appDBItems2BAdd(const QStringList &desktopFiles);

private:
    inline static QHash<QString, QString> s_desktopFiles;
};

} // namespace UkuiSearch

#endif // TEST_APPLICATION_INFO_H
