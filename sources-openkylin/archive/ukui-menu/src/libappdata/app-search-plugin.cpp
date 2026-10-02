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
#include "app-search-plugin.h"
#include "data-entity.h"
#include "basic-app-filter-model.h"

#include <UkuiSearchTask>
#include <QThread>
#include <QTimer>
#include <QAbstractListModel>
#include <QDebug>
#include <atomic>
#include "recently-installed-model.h"

namespace UkuiMenu {

// ====== AppSearchPluginPrivate ======
class AppSearchPluginPrivate : public QThread
{
    Q_OBJECT
public:
    explicit AppSearchPluginPrivate(QObject *parent = nullptr);

Q_SIGNALS:
    void searchedOne(const QString &id);

public Q_SLOTS:
    void startSearch(const QString &keyword);
    void stopSearch();

protected:
    void run() override;

private:
    std::atomic_size_t m_searchId{0};
    QTimer *m_timer{nullptr};
    UkuiSearch::UkuiSearchTask *m_appSearchTask {nullptr};
    UkuiSearch::DataQueue<UkuiSearch::ResultItem> *m_dataQueue{nullptr};
};

AppSearchPluginPrivate::AppSearchPluginPrivate(QObject *parent) : QThread(parent), m_appSearchTask(new UkuiSearch::UkuiSearchTask(this))
{
    m_dataQueue = m_appSearchTask->init();

    m_appSearchTask->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::Application);
    m_appSearchTask->setSearchOnlineApps(false);

    UkuiSearch::SearchResultProperties searchResultProperties;
    searchResultProperties << UkuiSearch::SearchProperty::SearchResultProperty::ApplicationDesktopPath;
    m_appSearchTask->setResultProperties(UkuiSearch::SearchProperty::SearchType::Application, searchResultProperties);

    m_timer = new QTimer;
    m_timer->setInterval(3000);
    m_timer->moveToThread(this);
}

void AppSearchPluginPrivate::startSearch(const QString &keyword)
{
    m_appSearchTask->clearKeyWords();
    m_appSearchTask->addKeyword(keyword);
    m_searchId.store(m_appSearchTask->startSearch(UkuiSearch::SearchProperty::SearchType::Application), std::memory_order_release);

    if (!this->isRunning()) {
        this->start();
    }
}

void AppSearchPluginPrivate::stopSearch()
{
    m_appSearchTask->stop();
    this->requestInterruption();
}

void AppSearchPluginPrivate::run()
{
    while (!isInterruptionRequested()) {
        UkuiSearch::ResultItem result = m_dataQueue->tryDequeue();
        if(result.getSearchId() == 0 && result.getItemKey().isEmpty() && result.getAllValue().isEmpty()) {
            if(!m_timer->isActive()) {
                // 超时退出
                m_timer->start();
            }
            msleep(100);
        } else {
            m_timer->stop();
            if (result.getSearchId() == m_searchId.load(std::memory_order_acquire)) {
                QString id = result.getValue(UkuiSearch::SearchProperty::ApplicationDesktopPath).toString();
                Q_EMIT this->searchedOne(id);
            }
        }

        if(m_timer->isActive() && m_timer->remainingTime() < 0.01 && m_dataQueue->isEmpty()) {
            this->requestInterruption();
        }
    }
}

// ====== AppSearchModel ====== //
class AppSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AppSearchModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void appendApp(const QString &id);
    void clear();

private:
    QVector<QPersistentModelIndex> m_apps;

private Q_SLOTS:
    void onAppRemoved(const QModelIndex &parent, int first, int last);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
};

AppSearchModel::AppSearchModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(BasicAppFilterModel::instance(), &BasicAppFilterModel::rowsAboutToBeRemoved, this, &AppSearchModel::onAppRemoved);
    connect(BasicAppFilterModel::instance(), &BasicAppFilterModel::dataChanged, this, &AppSearchModel::onDataChanged);
}

int AppSearchModel::rowCount(const QModelIndex &parent) const
{
    return m_apps.size();
}

int AppSearchModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant AppSearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_apps.size()) {
        return {};
    }

    if (index.row() < m_apps.count()) {
        if (role == DataEntity::RecentInstall) {
            return RecentlyInstalledModel::isRecentInstall(m_apps.at(index.row()).data(DataEntity::Entity).value<DataEntity>());
        }
        return m_apps.at(index.row()).data(role);
    }
    return QVariant();
}

void AppSearchModel::appendApp(const QString &id)
{
    int index = BasicAppFilterModel::instance()->indexOfApp(id);
    if (index < 0) return;

    QPersistentModelIndex modelIndex(BasicAppFilterModel::instance()->index(index, 0, QModelIndex()));

    if (m_apps.contains(modelIndex) || !modelIndex.isValid()) {
        return;
    }

    beginInsertRows(QModelIndex(), m_apps.count(), m_apps.count());
    m_apps.append(modelIndex);
    endInsertRows();
}

void AppSearchModel::clear()
{
    beginResetModel();
    m_apps.clear();
    endResetModel();
}

void AppSearchModel::onAppRemoved(const QModelIndex &parent, int first, int last)
{
    for(int row = first; row <= last; ++row) {
        QModelIndex sourceIndex = BasicAppFilterModel::instance()->index(row, 0, {});
        QPersistentModelIndex index(sourceIndex);

        if (!m_apps.contains(index)) {
            return;
        }

        beginRemoveRows(QModelIndex(), m_apps.indexOf(index), m_apps.indexOf(index));
        m_apps.removeOne(index);
        endRemoveRows();
    }
}

void AppSearchModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex sourceIndex;
    QPersistentModelIndex persistentModelIndex;
    int indexRow;

    for (int row = topLeft.row(); row <= bottomRight.row(); row ++) {
        sourceIndex = BasicAppFilterModel::instance()->index(row, 0, {});
        persistentModelIndex = QPersistentModelIndex(sourceIndex);
        indexRow = m_apps.indexOf(persistentModelIndex);
        if (indexRow == -1) continue;
        Q_EMIT dataChanged(index(indexRow, 0, QModelIndex()), index(indexRow, 0, QModelIndex()));
    }
}

// ====== AppSearchPlugin ====== //
AppSearchPlugin::AppSearchPlugin(QObject *parent) : AppListPluginInterface(parent)
    , m_searchPluginPrivate(new AppSearchPluginPrivate(this)), m_model(new AppSearchModel(this))
{
    connect(m_searchPluginPrivate, &AppSearchPluginPrivate::searchedOne, m_model, &AppSearchModel::appendApp);
}

AppListPluginGroup::Group AppSearchPlugin::group()
{
    return AppListPluginGroup::Search;
}

QString AppSearchPlugin::name()
{
    return "Search";
}

QString AppSearchPlugin::title()
{
    return "Search";
}

QList<QAction *> AppSearchPlugin::actions()
{
    // TODO: 搜索结果排序选项
    return {};
}

QAbstractItemModel *AppSearchPlugin::dataModel()
{
    return m_model;
}

void AppSearchPlugin::search(const QString &keyword)
{
    m_model->clear();
    if (keyword.isEmpty()) {
        return;
    }

    m_searchPluginPrivate->startSearch(keyword);
}

AppSearchPlugin::~AppSearchPlugin()
{
    m_searchPluginPrivate->stopSearch();
    m_searchPluginPrivate->quit();
    m_searchPluginPrivate->wait();
    m_searchPluginPrivate->deleteLater();
}

} // UkuiMenu

#include "app-search-plugin.moc"
