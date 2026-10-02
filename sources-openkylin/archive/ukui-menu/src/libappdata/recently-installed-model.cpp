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

#include "recently-installed-model.h"
#include "basic-app-filter-model.h"
#include "user-config.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QTimerEvent>
#include <QDBusConnection>

#define KYSDK_TIMERSERVER       "com.kylin.kysdk.TimeServer"
#define KYSDK_TIMERPATH         "/com/kylin/kysdk/Timer"
#define KYSDK_TIMERINTERFACE    "com.kylin.kysdk.TimeInterface"

namespace UkuiMenu {

// ====== RecentlyInstalledModel ====== //
RecentlyInstalledModel::RecentlyInstalledModel(QObject *parent) : QSortFilterProxyModel(parent), m_timer(new QTimer(this))
{
    QSortFilterProxyModel::setSourceModel(BasicAppFilterModel::instance());
    // 触发排序动作
//    QSortFilterProxyModel::sort(0, Qt::DescendingOrder);
    QSortFilterProxyModel::sort(0);

    // 每24小时主动刷新
    this->startTimer(24 * 3600000);

    QDBusConnection::systemBus().connect(KYSDK_TIMERSERVER,
                                         KYSDK_TIMERPATH,
                                         KYSDK_TIMERINTERFACE,
                                         "TimeChangeSignal",
                                         this,
                                         SLOT(executeInvalidate()));
    connect(BasicAppFilterModel::instance(), &QAbstractProxyModel::dataChanged, this, [&](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()){
        if (roles.isEmpty()
                || roles.contains(DataEntity::Id)
                || roles.contains(DataEntity::IsLaunched)
                || roles.contains(DataEntity::Favorite)
                || roles.contains(DataEntity::InstallationTime)) {
            Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), QVector<int>({DataEntity::RecentInstall}));
        }
    });
}

bool RecentlyInstalledModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_mode == AppCategoryModel::All) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    } else {
        QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
        DataEntity data = sourceIndex.data(DataEntity::Entity).value<DataEntity>();
        return isRecentInstall(data);
    }
}

bool RecentlyInstalledModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    DataEntity left_app = source_left.data(DataEntity::Entity).value<DataEntity>(), right_app = source_right.data(DataEntity::Entity).value<DataEntity>();
    bool isLeftRecent = isRecentInstall(left_app), isRightRecent = isRecentInstall(right_app);

    if (isLeftRecent && isRightRecent) {
        QDateTime leftInstallDate = QDateTime::fromString(source_left.data(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");
        QDateTime rightInstallDate = QDateTime::fromString(source_right.data(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");

        qint64 xt = leftInstallDate.toSecsSinceEpoch() - rightInstallDate.toSecsSinceEpoch();
        if (xt == 0) {
            return source_left.data(DataEntity::FirstLetter).value<QString>() < source_right.data(DataEntity::FirstLetter).value<QString>();
        }
        return xt >= 0;

    } else if (!isLeftRecent && !isRightRecent) {
        qreal leftDegree = getDegree(source_left);
        qreal rightDegree = getDegree(source_right);
        return leftDegree > rightDegree;

    } else if (isLeftRecent) {
        return true;
    } else {
        return false;
    }
}

bool RecentlyInstalledModel::isRecentInstall(const DataEntity &data)
{
    // 是否为预装应用
    if (UserConfig::instance()->isPreInstalledApps(data.getValue(DataEntity::Id).toString())) {
        return false;
    }
    // 是否打开过
    if (data.getValue(DataEntity::IsLaunched).toInt() != 0) {
        return false;
    }

    // 是否收藏
    if (data.getValue(DataEntity::Favorite).toInt() > 0) {
        return false;
    }

    QDateTime installDate = QDateTime::fromString(data.getValue(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");
    if (!installDate.isValid()) {
        return false;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 安装时间在30天内
    qint64 xt = currentDateTime.toSecsSinceEpoch() - installDate.toSecsSinceEpoch();
    return (xt >= 0) && (xt <= 30 * 24 * 3600);
}

qreal RecentlyInstalledModel::getDegree(const QModelIndex &index) const
{
    qreal degree;
    QDateTime installTime = QDateTime::fromString(index.data(DataEntity::InstallationTime).toString(), "yyyy-MM-dd hh:mm:ss");
    if (!installTime.isValid()) {
        return -1;
    }
    qreal installDays = installTime.daysTo(QDateTime::currentDateTime());
    qreal LaunchTimes = index.data(DataEntity::LaunchTimes).toInt();

    if (installDays > 10) {
        degree = LaunchTimes * (240 / (installDays - 6));
    } else {
        degree = LaunchTimes * (-0.4 * installDays * installDays + 100);
    }
    return degree;
}

void RecentlyInstalledModel::executeInvalidate()
{
    invalidate();
    beginResetModel();
    endResetModel();
}

bool RecentlyInstalledModel::event(QEvent *event)
{
    if (event->type() == QEvent::Timer) {
        auto timerEvent = static_cast<QTimerEvent*>(event);
        if (timerEvent->timerId()) {
            executeInvalidate();
            return true;
        }
    }

    return QObject::event(event);
}

QVariant RecentlyInstalledModel::data(const QModelIndex &index, int role) const
{
    if (role == DataEntity::Group) {
        if (m_mode == AppCategoryModel::All) {
            return "";
        }
        return tr("Recently Installed");
    }

    if (role == DataEntity::RecentInstall) {
        if (m_mode == AppCategoryModel::All) {
            return isRecentInstall(index.data(DataEntity::Entity).value<DataEntity>());
        } else {
            return true;
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

void RecentlyInstalledModel::setMode(AppCategoryModel::Mode mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        executeInvalidate();
    }
}

} // UkuiMenu
