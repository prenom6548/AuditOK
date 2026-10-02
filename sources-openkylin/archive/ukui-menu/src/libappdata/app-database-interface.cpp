/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */


#include "app-database-interface.h"
#include "user-config.h"
#include "settings.h"

#include <application-info.h>
#include <QDebug>

#define APP_ICON_PREFIX "image://theme/"

namespace UkuiMenu {

class AppDatabaseWorkerPrivate : public QObject
{
    Q_OBJECT
    friend class AppDatabaseInterface;

public:
    explicit AppDatabaseWorkerPrivate(AppDatabaseInterface *parent = nullptr);
    DataEntityVector getAllApps();
    DataEntity createAppEntity(const UkuiSearch::ApplicationPropertyMap &appInfo);
    bool getApp(const QString &appid, DataEntity &app);

    // 数据库操作函数
    void setAppProperty(const QString &appid, const UkuiSearch::ApplicationPropertyMap &propertyMap);
    void setAppProperty(const QString &appid, const UkuiSearch::ApplicationProperty::Property &property, const QVariant &value);

private Q_SLOTS:
    /**
     * 应用数据库的添加信号处理函数
     * @param infos 新增应用的id列表
     */
    void onAppDatabaseAdded(const QStringList &infos);
    void onAppDatabaseUpdate(const UkuiSearch::ApplicationInfoMap &infoMap);
    void onAppDatabaseUpdateAll(const QStringList &infos);
    void onAppDatabaseDeleted(const QStringList &infos);

private:
    static void addInfoToApp(const QMap<UkuiSearch::ApplicationProperty::Property, QVariant> &info, DataEntity &app);
    static bool hasVisibilityProperty(const UkuiSearch::ApplicationPropertyMap &appInfo);
    QString getUniqueDesktop(const QString &appid) const;

private:
    AppDatabaseInterface *q {nullptr};

    UkuiSearch::ApplicationInfo       *appDatabase {nullptr};
    UkuiSearch::ApplicationProperties  properties;
    // 设置我们需要的属性和值
    UkuiSearch::ApplicationPropertyMap filter;
};

AppDatabaseWorkerPrivate::AppDatabaseWorkerPrivate(AppDatabaseInterface *parent) : QObject(parent), q(parent)
{
    // 注册需要在信号和槽函数中使用的数据结构
    qRegisterMetaType<UkuiMenu::DataEntityVector>("DataEntityVector");

    // 初始化应用数据库链接
    appDatabase = new UkuiSearch::ApplicationInfo(this);

    // 首次启动时，为某些应用设置标志位
    if (UserConfig::instance()->isFirstStartUp()) {
        // 预装应用
        QStringList appList;
        UkuiSearch::ApplicationProperties  appProperty;
        UkuiSearch::ApplicationPropertyMap appFilter;
        appProperty.append(UkuiSearch::ApplicationProperty::Property::DesktopFilePath);
        appFilter.insert(UkuiSearch::ApplicationProperty::Property::DontDisplay, 0);
        appFilter.insert(UkuiSearch::ApplicationProperty::Property::AutoStart, 0);

        UkuiSearch::ApplicationInfoMap appInfos = appDatabase->getInfo(appProperty, appFilter);

        for (const auto &info : appInfos) {
            appList.append(info.value(UkuiSearch::ApplicationProperty::Property::DesktopFilePath).toString());
        }
        UserConfig::instance()->initPreInstalledLists(appList);

        // 默认收藏应用
        for (const auto &appid : GlobalSetting::instance()->defaultFavoriteApps()) {
            appDatabase->setAppToFavorites(appid);
            appDatabase->setAppLaunchedState(appid, true);
        }
    }

    // 设置从数据库查询哪些属性
    properties << UkuiSearch::ApplicationProperty::Property::Top
                  << UkuiSearch::ApplicationProperty::Property::Lock
                  << UkuiSearch::ApplicationProperty::Property::Favorites
                  << UkuiSearch::ApplicationProperty::Property::LaunchTimes
                  << UkuiSearch::ApplicationProperty::Property::DesktopFilePath
                  << UkuiSearch::ApplicationProperty::Property::Icon
                  << UkuiSearch::ApplicationProperty::Property::LocalName
                  << UkuiSearch::ApplicationProperty::Property::Category
                  << UkuiSearch::ApplicationProperty::Property::FirstLetterAll
                  << UkuiSearch::ApplicationProperty::Property::DontDisplay
                  << UkuiSearch::ApplicationProperty::Property::AutoStart
                  << UkuiSearch::ApplicationProperty::Property::InsertTime
                  << UkuiSearch::ApplicationProperty::Property::Launched;

    // 需要从数据库过滤的属性，满足该条件的数据才会被查询出来
    filter.insert(UkuiSearch::ApplicationProperty::Property::DontDisplay, 0);
    filter.insert(UkuiSearch::ApplicationProperty::Property::AutoStart, 0);

    // 链接数据库信号
    connect(appDatabase, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, &AppDatabaseWorkerPrivate::onAppDatabaseAdded);
    connect(appDatabase, &UkuiSearch::ApplicationInfo::appDBItems2BUpdate, this, &AppDatabaseWorkerPrivate::onAppDatabaseUpdate);
    connect(appDatabase, &UkuiSearch::ApplicationInfo::appDBItems2BUpdateAll, this, &AppDatabaseWorkerPrivate::onAppDatabaseUpdateAll);
    connect(appDatabase, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &AppDatabaseWorkerPrivate::onAppDatabaseDeleted);
    connect(appDatabase, &UkuiSearch::ApplicationInfo::DBOpenFailed, q, &AppDatabaseInterface::appDatabaseOpenFailed);
}

void AppDatabaseWorkerPrivate::addInfoToApp(const UkuiSearch::ApplicationPropertyMap &info, DataEntity &app)
{
    app.setTop(info.value(UkuiSearch::ApplicationProperty::Property::Top).toInt());
    app.setLock(info.value(UkuiSearch::ApplicationProperty::Property::Lock).toInt() == 1);
    app.setFavorite(info.value(UkuiSearch::ApplicationProperty::Property::Favorites).toInt());
    app.setLaunchTimes(info.value(UkuiSearch::ApplicationProperty::Property::LaunchTimes).toInt());
    app.setId(info.value(UkuiSearch::ApplicationProperty::Property::DesktopFilePath).toString());
    app.setIcon(info.value(UkuiSearch::ApplicationProperty::Property::Icon).toString());
    app.setName(info.value(UkuiSearch::ApplicationProperty::Property::LocalName).toString());
    app.setCategory(info.value(UkuiSearch::ApplicationProperty::Property::Category).toString());
    app.setFirstLetter(info.value(UkuiSearch::ApplicationProperty::Property::FirstLetterAll).toString());
    app.setInsertTime(info.value(UkuiSearch::ApplicationProperty::Property::InsertTime).toString());
    app.setLaunched(info.value(UkuiSearch::ApplicationProperty::Property::Launched).toInt());
    app.setDontDisplay(info.value(UkuiSearch::ApplicationProperty::Property::DontDisplay).toInt());
    app.setAutoStart(info.value(UkuiSearch::ApplicationProperty::Property::AutoStart).toInt());
}

bool AppDatabaseWorkerPrivate::hasVisibilityProperty(const UkuiSearch::ApplicationPropertyMap &appInfo)
{
    return appInfo.contains(UkuiSearch::ApplicationProperty::Property::DontDisplay)
           || appInfo.contains(UkuiSearch::ApplicationProperty::Property::AutoStart);
}

QString AppDatabaseWorkerPrivate::getUniqueDesktop(const QString &appid) const
{
    if (appid.isEmpty()) {
        return {};
    }

    QFileInfo file(appid);
    QString path = "";
    if (appDatabase->desktopFilePathFromName(file.completeBaseName(), path) && !path.isEmpty()) {
        return path;
    }

    return appid;
}

DataEntityVector AppDatabaseWorkerPrivate::getAllApps()
{
    UkuiSearch::ApplicationInfoMap appInfos = appDatabase->getInfo(properties, filter);
    if (appInfos.isEmpty()) {
        return {};
    }

    DataEntityVector apps;
    for (const auto &info : appInfos) {
        DataEntity app;
        addInfoToApp(info, app);
        apps.append(app);
    }

    return apps;
}

DataEntity AppDatabaseWorkerPrivate::createAppEntity(const UkuiSearch::ApplicationPropertyMap &appInfo)
{
    DataEntity app;
    addInfoToApp(appInfo, app);

    const QString uniqueId = getUniqueDesktop(app.id());
    if (!uniqueId.isEmpty() && uniqueId != app.id()) {
        const UkuiSearch::ApplicationPropertyMap uniqueInfo = appDatabase->getInfo(uniqueId, properties);
        if (uniqueInfo.isEmpty()) {
            return {};
        }
        addInfoToApp(uniqueInfo, app);
    }

    return app;
}

void AppDatabaseWorkerPrivate::onAppDatabaseAdded(const QStringList &infos)
{
    if (infos.isEmpty()) {
        return;
    }

    DataEntityVector apps;
    for (const QString &appid : infos) {
        const UkuiSearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);
        apps.append(createAppEntity(appInfo));
    }

    if (apps.isEmpty()) {
        return;
    }

    Q_EMIT q->appAdded(apps);
}

void AppDatabaseWorkerPrivate::onAppDatabaseUpdate(const UkuiSearch::ApplicationInfoMap &infoMap)
{
    if (infoMap.isEmpty()) {
        return;
    }

    QVector<QPair<DataEntity, QVector<int> > > updates;
    QMapIterator<QString, UkuiSearch::ApplicationPropertyMap> iterator(infoMap);

    while (iterator.hasNext()) {
        iterator.next();

        if (hasVisibilityProperty(iterator.value())) {
            const UkuiSearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(iterator.key(), properties);
            updates.append({createAppEntity(appInfo), {}});
            continue;
        }

        // 如果没有特殊属性，处理其他更新
        DataEntity app;
        QVector<int> roles;

        QMapIterator<UkuiSearch::ApplicationProperty::Property, QVariant> it(iterator.value());
        while (it.hasNext()) {
            it.next();

            switch (it.key()) {
                case UkuiSearch::ApplicationProperty::LocalName:
                    app.setName(it.value().toString());
                    roles.append(DataEntity::Name);
                    break;
                case UkuiSearch::ApplicationProperty::FirstLetterAll:
                    app.setFirstLetter(it.value().toString());
                    roles.append(DataEntity::FirstLetter);
                    break;
                case UkuiSearch::ApplicationProperty::Icon:
                    app.setIcon(it.value().toString());
                    roles.append(DataEntity::Icon);
                    break;
                case UkuiSearch::ApplicationProperty::InsertTime:
                    app.setInsertTime(it.value().toString());
                    roles.append(DataEntity::Icon);
                    break;
                case UkuiSearch::ApplicationProperty::Category:
                    app.setCategory(it.value().toString());
                    roles.append(DataEntity::Category);
                    break;
                case UkuiSearch::ApplicationProperty::LaunchTimes:
                    app.setLaunchTimes(it.value().toInt());
                    roles.append(DataEntity::LaunchTimes);
                    break;
                case UkuiSearch::ApplicationProperty::Favorites:
                    app.setFavorite(it.value().toInt());
                    roles.append(DataEntity::Favorite);
                    break;
                case UkuiSearch::ApplicationProperty::Launched:
                    app.setLaunched(it.value().toInt());
                    roles.append(DataEntity::IsLaunched);
                    break;
                case UkuiSearch::ApplicationProperty::Top:
                    app.setTop(it.value().toInt());
                    roles.append(DataEntity::Top);
                    break;
                case UkuiSearch::ApplicationProperty::Lock:
                    app.setLock(it.value().toBool());
                    roles.append(DataEntity::IsLocked);
                    break;
                default:
                    break;
            }
        }

        // 这个函数中，没有更新列不会发送信号
        if (roles.isEmpty()) {
            continue;
        }

        app.setId(iterator.key());
        updates.append({app, roles});
    }

    if (!updates.isEmpty()) {
        Q_EMIT q->appUpdated(updates);
    }
}

void AppDatabaseWorkerPrivate::onAppDatabaseDeleted(const QStringList &infos)
{
    if (infos.empty()) {
        return;
    }

    QStringList deletedApps;
    QVector<QPair<DataEntity, QVector<int> > > replacements;

    for (const auto &appid : infos) {
        if (appid.isEmpty()) {
            continue;
        }

        deletedApps.append(appid);

        QFileInfo file(appid);
        QString replacementPath;
        if (!appDatabase->desktopFilePathFromName(file.completeBaseName(), replacementPath)
                || replacementPath.isEmpty()
                || replacementPath == appid) {
            continue;
        }

        const UkuiSearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(replacementPath, properties);
        DataEntity replacement = createAppEntity(appInfo);
        if (!replacement.id().isEmpty()) {
            replacements.append({replacement, {}});
        }
    }

    if (!replacements.isEmpty()) {
        Q_EMIT q->appUpdated(replacements);
    }

    if (!deletedApps.isEmpty()) {
        Q_EMIT q->appDeleted(deletedApps);
    }
}

void AppDatabaseWorkerPrivate::onAppDatabaseUpdateAll(const QStringList &infos)
{
    if (infos.isEmpty()) {
        return;
    }

    QVector<QPair<DataEntity, QVector<int> > > updates;

    for (const auto &appid : infos) {
        const UkuiSearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);

        DataEntity app = createAppEntity(appInfo);
        updates.append({app, {}});
    }

    if (!updates.isEmpty()) {
        Q_EMIT q->appUpdated(updates);
    }
}

void AppDatabaseWorkerPrivate::setAppProperty(const QString &appid, const UkuiSearch::ApplicationPropertyMap &propertyMap)
{
    QMapIterator<UkuiSearch::ApplicationProperty::Property, QVariant> iterator(propertyMap);
    while (iterator.hasNext()) {
        iterator.next();
        setAppProperty(appid, iterator.key(), iterator.value());
    }
}

void AppDatabaseWorkerPrivate::setAppProperty(const QString &appid, const UkuiSearch::ApplicationProperty::Property &property,
                                              const QVariant &value)
{
    switch (property) {
        case UkuiSearch::ApplicationProperty::Favorites:
            appDatabase->setFavoritesOfApp(appid, value.toInt());
            break;
        case UkuiSearch::ApplicationProperty::Launched:
            appDatabase->setAppLaunchedState(appid, value.toBool());
            break;
        case UkuiSearch::ApplicationProperty::Top:
            appDatabase->setTopOfApp(appid, value.toInt());
            break;
        default:
            break;
    }
}

bool AppDatabaseWorkerPrivate::getApp(const QString &appid, DataEntity &app)
{
    if (appid.isEmpty()) {
        return false;
    }

    const UkuiSearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);
    addInfoToApp(appInfo, app);
    return true;
}

// ====== AppDatabaseInterface ====== //
// TODO: 多线程
AppDatabaseInterface::AppDatabaseInterface(QObject *parent) : QObject(parent), d(new AppDatabaseWorkerPrivate(this))
{

}

DataEntityVector AppDatabaseInterface::apps() const
{
    return d->getAllApps();
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    if (index < 0) {
        index = 0;
    }

    d->setAppProperty(appid, UkuiSearch::ApplicationProperty::Top, index);
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    if (index < 0) {
        index = 0;
    }

    d->setAppProperty(appid, UkuiSearch::ApplicationProperty::Favorites, index);
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    d->setAppProperty(appid, UkuiSearch::ApplicationProperty::Launched, state);
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    return d->getApp(appid, app);
}

} // UkuiMenu

#include "app-database-interface.moc"
