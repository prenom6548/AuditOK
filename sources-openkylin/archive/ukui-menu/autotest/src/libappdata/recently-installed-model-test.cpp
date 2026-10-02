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
#include <QStandardItemModel>
#include <QTimerEvent>

#include "fakes/user-config.h"

#include "../../../src/data-entity.h"
#define protected public
#include "../../../src/libappdata/recently-installed-model.h"
#undef protected
#include "../../../src/libappdata/basic-app-filter-model.h"

using namespace UkuiMenu;

namespace {

QStandardItemModel *g_recentSourceModel = nullptr;

QStandardItem *createRecentItem(const QString &id,
                                const QString &name,
                                const QString &installTime,
                                const QString &firstLetter,
                                int launched = 0,
                                int favorite = 0,
                                int launchTimes = 0)
{
    auto *item = new QStandardItem(name);
    DataEntity entity;
    entity.setId(id);
    entity.setName(name);
    entity.setInsertTime(installTime);
    entity.setFirstLetter(firstLetter);
    entity.setLaunched(launched);
    entity.setFavorite(favorite);
    entity.setLaunchTimes(launchTimes);
    item->setData(id, DataEntity::Id);
    item->setData(name, DataEntity::Name);
    item->setData(installTime, DataEntity::InstallationTime);
    item->setData(firstLetter, DataEntity::FirstLetter);
    item->setData(launched, DataEntity::IsLaunched);
    item->setData(favorite, DataEntity::Favorite);
    item->setData(launchTimes, DataEntity::LaunchTimes);
    item->setData(QVariant::fromValue(entity), DataEntity::Entity);
    return item;
}

}

namespace UkuiMenu {

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel model;
    return &model;
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const { return nullptr; }
DataEntity BasicAppFilterModel::appOfIndex(int row) const { return data(index(row, 0), DataEntity::Entity).value<DataEntity>(); }
int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    if (!g_recentSourceModel) return -1;
    for (int row = 0; row < g_recentSourceModel->rowCount(); ++row) {
        if (g_recentSourceModel->index(row, 0).data(DataEntity::Id).toString() == appid) return row;
    }
    return -1;
}
bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    int row = indexOfApp(appid);
    if (row < 0) return false;
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
BasicAppFilterModel::BasicAppFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

}

class recentlyInstalledModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_isRecentInstall_checksRecentUnlaunchedUnfavoritedApps();
    void test_modeChanges_filterRowsAndGroupRole();
    void test_lessThan_coversRecentAndDegreeBranches();
    void test_eventAndDataChangedBranches();

private:
    QStandardItemModel m_sourceModel;
};

void recentlyInstalledModel_test::init()
{
    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    const QString recent = QDateTime::currentDateTime().addDays(-5).toString("yyyy-MM-dd hh:mm:ss");
    const QString old = QDateTime::currentDateTime().addDays(-40).toString("yyyy-MM-dd hh:mm:ss");
    const QString invalid = QStringLiteral("not-a-date");
    const QString future = QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-dd hh:mm:ss");
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha"), recent, QStringLiteral("A")));
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/b.desktop"), QStringLiteral("Beta"), old, QStringLiteral("B"), 0, 0, 3));
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/c.desktop"), QStringLiteral("Gamma"), recent, QStringLiteral("G"), 1, 0, 5));
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/d.desktop"), QStringLiteral("Delta"), recent, QStringLiteral("D"), 0, 2));
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/e.desktop"), QStringLiteral("Invalid"), invalid, QStringLiteral("I")));
    m_sourceModel.appendRow(createRecentItem(QStringLiteral("/tmp/f.desktop"), QStringLiteral("Future"), future, QStringLiteral("F")));
    g_recentSourceModel = &m_sourceModel;
    BasicAppFilterModel::instance()->setSourceModel(&m_sourceModel);
    UserConfig::instance()->initPreInstalledLists({});
}

void recentlyInstalledModel_test::test_isRecentInstall_checksRecentUnlaunchedUnfavoritedApps()
{
    const DataEntity recent = m_sourceModel.index(0, 0).data(DataEntity::Entity).value<DataEntity>();
    const DataEntity old = m_sourceModel.index(1, 0).data(DataEntity::Entity).value<DataEntity>();
    const DataEntity launched = m_sourceModel.index(2, 0).data(DataEntity::Entity).value<DataEntity>();
    const DataEntity favorite = m_sourceModel.index(3, 0).data(DataEntity::Entity).value<DataEntity>();
    const DataEntity invalid = m_sourceModel.index(4, 0).data(DataEntity::Entity).value<DataEntity>();
    const DataEntity future = m_sourceModel.index(5, 0).data(DataEntity::Entity).value<DataEntity>();

    QVERIFY(RecentlyInstalledModel::isRecentInstall(recent));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(old));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(launched));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(favorite));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(invalid));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(future));

    UserConfig::instance()->addPreInstalledApp(QStringLiteral("/tmp/a.desktop"));
    QVERIFY(!RecentlyInstalledModel::isRecentInstall(recent));
}

void recentlyInstalledModel_test::test_modeChanges_filterRowsAndGroupRole()
{
    RecentlyInstalledModel model;

    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Group).toString(), QString());
    QVERIFY(!model.data(model.index(0, 0), DataEntity::Name).toString().isEmpty());
    QCOMPARE(model.data(model.index(0, 0), DataEntity::RecentInstall).toBool(), true);

    model.setMode(AppCategoryModel::Category);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), true);
    QCOMPARE(model.filterAcceptsRow(1, QModelIndex()), false);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Group).toString(), QStringLiteral("Recently Installed"));
    QCOMPARE(model.data(model.index(0, 0), DataEntity::RecentInstall).toBool(), true);

    model.setMode(AppCategoryModel::Category);
    QCOMPARE(model.rowCount(), 1);
}

void recentlyInstalledModel_test::test_lessThan_coversRecentAndDegreeBranches()
{
    RecentlyInstalledModel model;
    QStandardItemModel source;
    source.setColumnCount(1);

    const QString newest = QDateTime::currentDateTime().addDays(-1).toString("yyyy-MM-dd hh:mm:ss");
    const QString olderRecent = QDateTime::currentDateTime().addDays(-2).toString("yyyy-MM-dd hh:mm:ss");
    const QString sameRecent = QDateTime::currentDateTime().addDays(-3).toString("yyyy-MM-dd hh:mm:ss");
    const QString old = QDateTime::currentDateTime().addDays(-40).toString("yyyy-MM-dd hh:mm:ss");

    source.appendRow(createRecentItem(QStringLiteral("/tmp/new.desktop"), QStringLiteral("New"), newest, QStringLiteral("N")));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/old-recent.desktop"), QStringLiteral("Old Recent"), olderRecent, QStringLiteral("O")));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/same-b.desktop"), QStringLiteral("Same B"), sameRecent, QStringLiteral("B")));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/same-a.desktop"), QStringLiteral("Same A"), sameRecent, QStringLiteral("A")));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/used-high.desktop"), QStringLiteral("Used High"), old, QStringLiteral("H"), 1, 0, 30));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/used-low.desktop"), QStringLiteral("Used Low"), old, QStringLiteral("L"), 1, 0, 1));
    source.appendRow(createRecentItem(QStringLiteral("/tmp/invalid.desktop"), QStringLiteral("Invalid"), QStringLiteral("invalid"), QStringLiteral("I"), 1));

    QVERIFY(model.lessThan(source.index(0, 0), source.index(1, 0)));
    QVERIFY(!model.lessThan(source.index(1, 0), source.index(0, 0)));
    QVERIFY(model.lessThan(source.index(3, 0), source.index(2, 0)));
    QVERIFY(!model.lessThan(source.index(2, 0), source.index(3, 0)));
    QVERIFY(model.lessThan(source.index(4, 0), source.index(5, 0)));
    QVERIFY(!model.lessThan(source.index(5, 0), source.index(4, 0)));
    QVERIFY(model.lessThan(source.index(4, 0), source.index(6, 0)));
    QVERIFY(model.lessThan(source.index(0, 0), source.index(4, 0)));
    QVERIFY(!model.lessThan(source.index(4, 0), source.index(0, 0)));
}

void recentlyInstalledModel_test::test_eventAndDataChangedBranches()
{
    RecentlyInstalledModel model;
    QSignalSpy resetSpy(&model, &RecentlyInstalledModel::modelReset);
    QSignalSpy dataSpy(&model, &RecentlyInstalledModel::dataChanged);

    QTimerEvent timerEvent(1);
    QCOMPARE(model.event(&timerEvent), true);
    QCOMPARE(resetSpy.count(), 1);

    QTimerEvent zeroTimerEvent(0);
    model.event(&zeroTimerEvent);

    QEvent userEvent(QEvent::User);
    QCOMPARE(model.event(&userEvent), true);

    dataSpy.clear();
    const QModelIndex changed = BasicAppFilterModel::instance()->index(0, 0);
    Q_EMIT BasicAppFilterModel::instance()->dataChanged(changed, changed, {DataEntity::IsLaunched});
    QCOMPARE(dataSpy.count(), 2);

    Q_EMIT BasicAppFilterModel::instance()->dataChanged(changed, changed, {});
    QCOMPARE(dataSpy.count(), 4);

    Q_EMIT BasicAppFilterModel::instance()->dataChanged(changed, changed, {DataEntity::Name});
    QCOMPARE(dataSpy.count(), 5);
}

QTEST_MAIN(recentlyInstalledModel_test)

#include "recently-installed-model-test.moc"
