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

#include <QtTest>
#include <QDateTime>
#include <QSignalSpy>
#include <QStandardItemModel>

#include "../../../src/data-entity.h"
#include "../../../src/libappdata/app-search-plugin.h"
#include "../../../src/libappdata/basic-app-filter-model.h"
#include "../../../src/libappdata/recently-installed-model.h"
#include "../../../src/libappdata/app-database-interface.h"
#include "fakes/UkuiSearchTask.h"

using namespace UkuiMenu;

namespace {

QStandardItemModel *g_searchSourceModel = nullptr;

QStandardItem *createSearchApp(const QString &id, const QString &name)
{
    auto *item = new QStandardItem(name);
    DataEntity entity;
    entity.setId(id);
    entity.setName(name);
    entity.setInsertTime(QDateTime::currentDateTime().addDays(-2).toString("yyyy-MM-dd hh:mm:ss"));
    item->setData(id, DataEntity::Id);
    item->setData(name, DataEntity::Name);
    item->setData(entity.insertTime(), DataEntity::InstallationTime);
    item->setData(QVariant::fromValue(entity), DataEntity::Entity);
    return item;
}

}

namespace UkuiMenu {

AppDatabaseInterface::AppDatabaseInterface(QObject *parent)
    : QObject(parent)
{
}

DataEntityVector AppDatabaseInterface::apps() const
{
    return {};
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    Q_UNUSED(appid);
    Q_UNUSED(state);
}

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel model;
    return &model;
}

BasicAppFilterModel::BasicAppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    static AppDatabaseInterface databaseInterface;
    return &databaseInterface;
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    return data(index(row, 0), DataEntity::Entity).value<DataEntity>();
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    if (!g_searchSourceModel) {
        return -1;
    }

    for (int row = 0; row < g_searchSourceModel->rowCount(); ++row) {
        if (g_searchSourceModel->index(row, 0).data(DataEntity::Id).toString() == appid) {
            return row;
        }
    }

    return -1;
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    const int row = indexOfApp(appid);
    if (row < 0) {
        return false;
    }

    app = appOfIndex(row);
    return true;
}

QVariant BasicAppFilterModel::data(const QModelIndex &index, int role) const
{
    return QSortFilterProxyModel::data(index, role);
}

bool BasicAppFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool BasicAppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

bool RecentlyInstalledModel::isRecentInstall(const DataEntity &data)
{
    Q_UNUSED(data);
    return true;
}

RecentlyInstalledModel::RecentlyInstalledModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool RecentlyInstalledModel::event(QEvent *event)
{
    return QObject::event(event);
}

QVariant RecentlyInstalledModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    return {};
}

void RecentlyInstalledModel::setMode(AppCategoryModel::Mode mode)
{
    Q_UNUSED(mode);
}

bool RecentlyInstalledModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

bool RecentlyInstalledModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    Q_UNUSED(source_left);
    Q_UNUSED(source_right);
    return false;
}

qreal RecentlyInstalledModel::getDegree(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 0;
}

void RecentlyInstalledModel::executeInvalidate()
{
}

}

class appSearchPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_search_metadata();
    void test_search_populatesModelFromSearchResults();
    void test_emptySearch_clearsResultsAndSkipsSearch();
    void test_dataModel_reflectsSourceDataAndRecentInstallRole();
    void test_sourceRemoval_updatesSearchResults();

private:
    QStandardItemModel m_sourceModel;
};

void appSearchPlugin_test::init()
{
    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    m_sourceModel.appendRow(createSearchApp(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha")));
    m_sourceModel.appendRow(createSearchApp(QStringLiteral("/tmp/b.desktop"), QStringLiteral("Beta")));
    m_sourceModel.appendRow(createSearchApp(QStringLiteral("/tmp/c.desktop"), QStringLiteral("Gamma")));

    g_searchSourceModel = &m_sourceModel;
    BasicAppFilterModel::instance()->setSourceModel(&m_sourceModel);
    UkuiSearch::FakeSearchStore::reset();
}

void appSearchPlugin_test::test_search_metadata()
{
    auto *plugin = new AppSearchPlugin;

    QCOMPARE(plugin->group(), AppListPluginGroup::Search);
    QCOMPARE(plugin->name(), QStringLiteral("Search"));
    QCOMPARE(plugin->title(), QStringLiteral("Search"));
    QCOMPARE(plugin->actions().size(), 0);
    QVERIFY(plugin->dataModel());
}

void appSearchPlugin_test::test_search_populatesModelFromSearchResults()
{
    auto *plugin = new AppSearchPlugin;
    auto *model = plugin->dataModel();

    UkuiSearch::FakeSearchStore::g_resultIds = QStringList({
        QStringLiteral("/tmp/a.desktop"),
        QStringLiteral("/tmp/b.desktop"),
        QStringLiteral("/tmp/a.desktop"),
        QStringLiteral("/tmp/not-found.desktop")
    });

    plugin->search(QStringLiteral("alpha"));
    QTRY_COMPARE(model->rowCount(), 2);

    QCOMPARE(UkuiSearch::FakeSearchStore::g_lastKeyword, QStringLiteral("alpha"));
    QCOMPARE(model->index(0, 0).data(DataEntity::Id).toString(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(model->index(1, 0).data(DataEntity::Id).toString(), QStringLiteral("/tmp/b.desktop"));
}

void appSearchPlugin_test::test_emptySearch_clearsResultsAndSkipsSearch()
{
    auto *plugin = new AppSearchPlugin;

    UkuiSearch::FakeSearchStore::g_resultIds = QStringList({ QStringLiteral("/tmp/a.desktop") });
    plugin->search(QStringLiteral("alpha"));
    QTRY_COMPARE(plugin->dataModel()->rowCount(), 1);

    plugin->search(QString());

    QCOMPARE(plugin->dataModel()->rowCount(), 0);
    QCOMPARE(UkuiSearch::FakeSearchStore::g_lastKeyword, QStringLiteral("alpha"));
}

void appSearchPlugin_test::test_dataModel_reflectsSourceDataAndRecentInstallRole()
{
    auto *plugin = new AppSearchPlugin;
    auto *model = plugin->dataModel();

    UkuiSearch::FakeSearchStore::g_resultIds = QStringList({ QStringLiteral("/tmp/a.desktop") });
    plugin->search(QStringLiteral("alpha"));
    QTRY_COMPARE(model->rowCount(), 1);

    const QModelIndex resultIndex = model->index(0, 0);
    QCOMPARE(resultIndex.data(DataEntity::Name).toString(), QStringLiteral("Alpha"));
    QCOMPARE(resultIndex.data(DataEntity::RecentInstall).toBool(), true);

    m_sourceModel.item(0, 0)->setData(QStringLiteral("Alpha Updated"), DataEntity::Name);
    QCoreApplication::processEvents();

    QCOMPARE(resultIndex.data(DataEntity::Name).toString(), QStringLiteral("Alpha Updated"));
}

void appSearchPlugin_test::test_sourceRemoval_updatesSearchResults()
{
    auto *plugin = new AppSearchPlugin;
    auto *model = plugin->dataModel();

    UkuiSearch::FakeSearchStore::g_resultIds = QStringList({
        QStringLiteral("/tmp/a.desktop"),
        QStringLiteral("/tmp/b.desktop")
    });
    plugin->search(QStringLiteral("alpha"));
    QTRY_COMPARE(model->rowCount(), 2);

    m_sourceModel.removeRow(0);
    QCoreApplication::processEvents();

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->index(0, 0).data(DataEntity::Id).toString(), QStringLiteral("/tmp/b.desktop"));
}

QTEST_MAIN(appSearchPlugin_test)

#include "app-search-plugin-test.moc"
