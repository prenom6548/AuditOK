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

#include "application-info.h"

namespace UkuiSearch {
namespace FakeAppInfoStore {

ApplicationInfoMap g_apps;
QHash<QString, QString> g_desktopPathByName;
QList<QPair<QString, int>> g_favoriteCalls;
QList<QPair<QString, int>> g_topCalls;
QList<QPair<QString, bool>> g_launchedCalls;

void reset()
{
    g_apps.clear();
    g_desktopPathByName.clear();
    g_favoriteCalls.clear();
    g_topCalls.clear();
    g_launchedCalls.clear();
}

ApplicationPropertyMap createApp(const QString &id,
                                 const QString &name,
                                 const QString &category,
                                 const QString &firstLetter,
                                 const QString &insertTime,
                                 int dontDisplay,
                                 int autoStart)
{
    ApplicationPropertyMap app;
    app.insert(ApplicationProperty::DesktopFilePath, id);
    app.insert(ApplicationProperty::LocalName, name);
    app.insert(ApplicationProperty::Category, category);
    app.insert(ApplicationProperty::FirstLetterAll, firstLetter);
    app.insert(ApplicationProperty::InsertTime, insertTime);
    app.insert(ApplicationProperty::DontDisplay, dontDisplay);
    app.insert(ApplicationProperty::AutoStart, autoStart);
    app.insert(ApplicationProperty::Top, 0);
    app.insert(ApplicationProperty::Lock, 0);
    app.insert(ApplicationProperty::Favorites, 0);
    app.insert(ApplicationProperty::LaunchTimes, 0);
    app.insert(ApplicationProperty::Icon, QStringLiteral("icon-name"));
    app.insert(ApplicationProperty::Launched, 0);
    return app;
}

}

ApplicationInfo::ApplicationInfo(QObject *parent)
    : QObject(parent)
{
}

ApplicationInfo::~ApplicationInfo() = default;

QVariant ApplicationInfo::getInfo(const QString &desktopFile, ApplicationProperty::Property property)
{
    return FakeAppInfoStore::g_apps.value(desktopFile).value(property);
}

ApplicationPropertyMap ApplicationInfo::getInfo(const QString &desktopFile, ApplicationProperties properties)
{
    Q_UNUSED(properties);
    return FakeAppInfoStore::g_apps.value(desktopFile);
}

ApplicationInfoMap ApplicationInfo::getInfo(ApplicationProperties properties)
{
    Q_UNUSED(properties);
    return FakeAppInfoStore::g_apps;
}

ApplicationInfoMap ApplicationInfo::getInfo(ApplicationProperties properties, ApplicationPropertyMap restrictions)
{
    Q_UNUSED(properties);

    ApplicationInfoMap result;
    for (auto it = FakeAppInfoStore::g_apps.constBegin(); it != FakeAppInfoStore::g_apps.constEnd(); ++it) {
        bool accepted = true;
        for (auto filterIt = restrictions.constBegin(); filterIt != restrictions.constEnd(); ++filterIt) {
            if (it.value().value(filterIt.key()) != filterIt.value()) {
                accepted = false;
                break;
            }
        }

        if (accepted) {
            result.insert(it.key(), it.value());
        }
    }

    return result;
}

bool ApplicationInfo::desktopFilePathFromName(const QString &desktopName, QString &desktopFilePath)
{
    if (!FakeAppInfoStore::g_desktopPathByName.contains(desktopName)) {
        return false;
    }

    desktopFilePath = FakeAppInfoStore::g_desktopPathByName.value(desktopName);
    return true;
}

void ApplicationInfo::setAppToFavorites(const QString &desktopFilePath)
{
    FakeAppInfoStore::g_favoriteCalls.append({desktopFilePath, 1});
}

void ApplicationInfo::removeAppFromFavorites(const QString &desktopFilePath)
{
    FakeAppInfoStore::g_favoriteCalls.append({desktopFilePath, 0});
}

void ApplicationInfo::setFavoritesOfApp(const QString &desktopFilePath, size_t num)
{
    FakeAppInfoStore::g_favoriteCalls.append({desktopFilePath, static_cast<int>(num)});
}

void ApplicationInfo::setAppToTop(const QString &desktopFilePath)
{
    FakeAppInfoStore::g_topCalls.append({desktopFilePath, 1});
}

void ApplicationInfo::setTopOfApp(const QString &desktopFilePath, size_t num)
{
    FakeAppInfoStore::g_topCalls.append({desktopFilePath, static_cast<int>(num)});
}

void ApplicationInfo::setAppLaunchedState(const QString &desktopFilePath, bool launched)
{
    FakeAppInfoStore::g_launchedCalls.append({desktopFilePath, launched});
}

}
