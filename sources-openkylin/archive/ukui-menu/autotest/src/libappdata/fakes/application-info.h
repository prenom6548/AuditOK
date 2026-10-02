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

#ifndef AUTOTEST_FAKE_APPLICATION_INFO_H
#define AUTOTEST_FAKE_APPLICATION_INFO_H

#include <QObject>
#include <QHash>

#include <application-property.h>

namespace UkuiSearch {

class ApplicationInfo : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationInfo(QObject *parent = nullptr);
    ~ApplicationInfo() override;

    QVariant getInfo(const QString &desktopFile, ApplicationProperty::Property property);
    ApplicationPropertyMap getInfo(const QString &desktopFile, ApplicationProperties properties);
    ApplicationInfoMap getInfo(ApplicationProperties properties);
    ApplicationInfoMap getInfo(ApplicationProperties properties, ApplicationPropertyMap restrictions);
    bool desktopFilePathFromName(const QString &desktopName, QString &desktopFilePath);

    void setAppToFavorites(const QString &desktopFilePath);
    void removeAppFromFavorites(const QString &desktopFilePath);
    void setFavoritesOfApp(const QString &desktopFilePath, size_t num);
    void setAppToTop(const QString &desktopFilePath);
    void setTopOfApp(const QString &desktopFilePath, size_t num);
    void setAppLaunchedState(const QString &desktopFilePath, bool launched = true);

Q_SIGNALS:
    void DBOpenFailed();
    void appDBItems2BUpdate(UkuiSearch::ApplicationInfoMap);
    void appDBItems2BUpdateAll(QStringList);
    void appDBItems2BAdd(QStringList);
    void appDBItems2BDelete(QStringList);
};

namespace FakeAppInfoStore {
extern ApplicationInfoMap g_apps;
extern QHash<QString, QString> g_desktopPathByName;
extern QList<QPair<QString, int>> g_favoriteCalls;
extern QList<QPair<QString, int>> g_topCalls;
extern QList<QPair<QString, bool>> g_launchedCalls;
void reset();
ApplicationPropertyMap createApp(const QString &id,
                                 const QString &name,
                                 const QString &category,
                                 const QString &firstLetter,
                                 const QString &insertTime,
                                 int dontDisplay = 0,
                                 int autoStart = 0);
}

}

#endif
