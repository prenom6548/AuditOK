/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "spacessettings.h"

#include <QSettings>
#include <QTimer>

#include <QDebug>

static SpacesSettings *instance = nullptr;

SpacesSettings *SpacesSettings::globalInstance()
{
    if (!instance) {
        instance = new SpacesSettings;
    }
    return instance;
}

SpacesSettings::SpacesSettings(QObject *parent)
    : QObject{parent}
{
    delayer = new QTimer(this);
    delayer->setInterval(500);
    delayer->setSingleShot(true);

    QSettings settings;
    settings.beginGroup("Common");
    sortType = settings.value("sortType", 0).toInt();
    sortOrder = settings.value("sortAsc", false).toBool()? Qt::AscendingOrder: Qt::DescendingOrder;
    useGlobalSorting = settings.value("useGlobalSorting", true).toBool();
    userAddedFirstType = settings.value("userAddedFirstType", 7).toInt();
    isSpacesCollapsed = settings.value("isSpacesCollapsed").toBool();
    isRecentCollapsed = settings.value("isRecentCollapsed").toBool();
    isSpaceListMode = settings.value("isSpaceListMode").toBool();
    isPreviewMode = settings.value("isPreviewMode").toBool();
    settings.endGroup();

    auto startTimer = [=]{delayer->start();};
    connect(this, &SpacesSettings::sortOrderChanged, delayer, startTimer);
    connect(this, &SpacesSettings::sortTypeChanged, delayer, startTimer);
    connect(this, &SpacesSettings::useGlobalSortingChanged, delayer, startTimer);
    connect(this, &SpacesSettings::userAddedFirstChanged, delayer, startTimer);
    connect(delayer, &QTimer::timeout, this, [=]{
        QSettings settingsToSync;
        settingsToSync.beginGroup("Common");
        settingsToSync.setValue("sortType", sortType);
        bool sortAsc = (sortOrder == Qt::AscendingOrder);
        settingsToSync.setValue("sortAsc", sortAsc);
        settingsToSync.setValue("useGlobalSorting", useGlobalSorting);
        settingsToSync.setValue("userAddedFirstType", userAddedFirstType);
        settingsToSync.endGroup();
        settingsToSync.sync();
        qDebug() << settingsToSync.fileName();

        Q_EMIT sortConfigChanged();
    });
}

Qt::SortOrder SpacesSettings::getSortOrder() const
{
    return sortOrder;
}

void SpacesSettings::setSortOrder(Qt::SortOrder newSortOrder)
{
    if (sortOrder == newSortOrder)
        return;
    sortOrder = newSortOrder;
    Q_EMIT sortOrderChanged();
}

int SpacesSettings::getSortType() const
{
    return sortType;
}

void SpacesSettings::setSortType(int newSortType)
{
    if (sortType == newSortType)
        return;
    sortType = newSortType;
    Q_EMIT sortTypeChanged();
}

bool SpacesSettings::getUseGlobalSorting() const
{
    return useGlobalSorting;
}

void SpacesSettings::setUseGlobalSorting(bool newUseGlobalSorting)
{
    if (useGlobalSorting == newUseGlobalSorting)
        return;
    useGlobalSorting = newUseGlobalSorting;
    if (useGlobalSorting) {
        QSettings settings;
        settings.beginGroup("Spaces");
        settings.clear();
        settings.endGroup();
        settings.sync();
    }
    Q_EMIT useGlobalSortingChanged();
}

int SpacesSettings::getUserAddedFirst() const
{
    return userAddedFirstType;
}

void SpacesSettings::setUserAddedFirst(int newUserAddedFirst)
{
    if (userAddedFirstType == newUserAddedFirst)
        return;

    setSortType(1);
    userAddedFirstType = newUserAddedFirst;
    Q_EMIT userAddedFirstChanged();
}

void SpacesSettings::setSpacesSortConfig(int spaceId, int sortType, int sortOrder, int userAddedFirst)
{
    QSettings settings;
    settings.beginGroup("Spaces");
    settings.beginWriteArray(QString::number(spaceId), 3);
    settings.setValue("sortType", sortType);
    settings.setValue("sortOrder", sortOrder);
    settings.setValue("userAddedFirst", userAddedFirst);
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

QVariantHash SpacesSettings::getSpacesSortConfig(int spaceId)
{
    QVariantHash result;
    QSettings settings;
    settings.beginGroup("Spaces");
    settings.beginReadArray(QString::number(spaceId));

    QVariant value = settings.value("sortType");
    if (value.isNull()) {
        result.insert("sortType", this->getSortType());
    } else {
        result.insert("sortType", value);
    }

    value = settings.value("sortOrder");
    if (value.isNull()) {
        result.insert("sortOrder", this->getSortOrder() == Qt::AscendingOrder? 0: 1);
    } else {
        result.insert("sortOrder", value);
    }

    value = settings.value("userAddedFirstType");
    if (value.isNull()) {
        result.insert("userAddedFirstType", this->getUserAddedFirst());
    } else {
        result.insert("userAddedFirstType", value);
    }

    settings.endArray();
    settings.endGroup();
    return result;
}

bool SpacesSettings::getIsSpacesCollapsed() const
{
    return isSpacesCollapsed;
}

void SpacesSettings::setIsSpacesCollapsed(bool newIsSpacesCollapsed)
{
    if (isSpacesCollapsed == newIsSpacesCollapsed)
        return;
    isSpacesCollapsed = newIsSpacesCollapsed;
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue("isSpacesCollapsed", newIsSpacesCollapsed);
    settings.endGroup();
    settings.sync();
    Q_EMIT isSpacesCollapsedChanged();
}

bool SpacesSettings::getIsRecentCollapsed() const
{
    return isRecentCollapsed;
}

void SpacesSettings::setIsRecentCollapsed(bool newIsRecentCollapsed)
{
    if (isRecentCollapsed == newIsRecentCollapsed)
        return;
    isRecentCollapsed = newIsRecentCollapsed;
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue("isRecentCollapsed", newIsRecentCollapsed);
    settings.endGroup();
    settings.sync();
    Q_EMIT isRecentCollapsedChanged();
}

bool SpacesSettings::getIsSpaceListMode() const
{
    return isSpaceListMode;
}

void SpacesSettings::setIsSpaceListMode(bool newIsSpaceListMode)
{
    if (isSpaceListMode == newIsSpaceListMode)
        return;
    isSpaceListMode = newIsSpaceListMode;
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue("isSpaceListMode", newIsSpaceListMode);
    settings.endGroup();
    settings.sync();
    Q_EMIT isSpaceListModeChanged();
}

bool SpacesSettings::getIsPreviewMode() const
{
    return isPreviewMode;
}

void SpacesSettings::setIsPreviewMode(bool newIsPreviewMode)
{
    if (isPreviewMode == newIsPreviewMode)
        return;
    isPreviewMode = newIsPreviewMode;
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue("isPreviewMode", newIsPreviewMode);
    settings.endGroup();
    settings.sync();
    Q_EMIT isPreviewModeChanged();
}
