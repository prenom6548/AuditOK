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

#include "fakes/user-config.h"

#include "../../../src/data-entity.h"
#include "../../../src/libappdata/basic-app-model.h"
#include "../../../src/libappdata/app-database-interface.h"

using namespace UkuiMenu;

namespace {

DataEntityVector g_initialApps;
QVector<QPair<QString, int>> g_favoriteWrites;
QVector<QPair<QString, int>> g_topWrites;
QVector<QPair<QString, bool>> g_launchedWrites;

DataEntity createBasicApp(const QString &id, const QString &name)
{
    DataEntity app;
    app.setId(id);
    app.setName(name);
    return app;
}

}

namespace UkuiMenu {

AppDatabaseInterface::AppDatabaseInterface(QObject *parent)
    : QObject(parent)
{
}

DataEntityVector AppDatabaseInterface::apps() const
{
    return g_initialApps;
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    for (const auto &item : g_initialApps) {
        if (item.id() == appid) {
            app = item;
            return true;
        }
    }
    return false;
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    g_topWrites.append({appid, index});
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    g_favoriteWrites.append({appid, index});
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    g_launchedWrites.append({appid, state});
}

}

class basicAppModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void test_initialAppsLoaded();
    void test_databaseSignals_updateModel();
    void test_safeBranches_forInvalidIndexesAndUnknownUpdates();
    void test_accessorsAndDatabaseFailureBranch();
    void test_visibilityAndDesktopKeyFullUpdates();

private:
    BasicAppModel *m_model = nullptr;
};

void basicAppModel_test::initTestCase()
{
    g_initialApps = {
        createBasicApp(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha")),
        createBasicApp(QStringLiteral("/tmp/b.desktop"), QStringLiteral("Beta"))
    };
    m_model = BasicAppModel::instance();
}

void basicAppModel_test::init()
{
    g_favoriteWrites.clear();
    g_topWrites.clear();
    g_launchedWrites.clear();
}

void basicAppModel_test::test_initialAppsLoaded()
{
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);
    QCOMPARE(m_model->data(m_model->index(0, 0), DataEntity::Name).toString(), QStringLiteral("Alpha"));
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/b.desktop")), 1);
}

void basicAppModel_test::test_databaseSignals_updateModel()
{
    auto *db = const_cast<AppDatabaseInterface *>(m_model->databaseInterface());
    QSignalSpy insertSpy(m_model, &BasicAppModel::rowsInserted);
    QSignalSpy dataSpy(m_model, &BasicAppModel::dataChanged);
    QSignalSpy removeSpy(m_model, &BasicAppModel::rowsRemoved);

    const DataEntity app = createBasicApp(QStringLiteral("/tmp/c.desktop"), QStringLiteral("Gamma"));
    Q_EMIT db->appAdded({app});
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/c.desktop")), 2);

    DataEntity updated = app;
    updated.setName(QStringLiteral("Gamma Updated"));
    Q_EMIT db->appUpdated({{updated, {DataEntity::Name}}});
    QVERIFY(dataSpy.count() >= 1);
    QCOMPARE(m_model->data(m_model->index(2, 0), DataEntity::Name).toString(), QStringLiteral("Gamma Updated"));

    UserConfig::instance()->addPreInstalledApp(QStringLiteral("/tmp/c.desktop"));
    Q_EMIT db->appDeleted({QStringLiteral("/tmp/c.desktop")});
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/c.desktop")), -1);
}

void basicAppModel_test::test_safeBranches_forInvalidIndexesAndUnknownUpdates()
{
    auto *db = const_cast<AppDatabaseInterface *>(m_model->databaseInterface());
    QSignalSpy insertSpy(m_model, &BasicAppModel::rowsInserted);
    QSignalSpy dataSpy(m_model, &BasicAppModel::dataChanged);
    QSignalSpy removeSpy(m_model, &BasicAppModel::rowsRemoved);

    QCOMPARE(m_model->data(QModelIndex(), DataEntity::Name), QVariant());
    QCOMPARE(m_model->setData(QModelIndex(), QStringLiteral("x"), DataEntity::Name), false);
    QCOMPARE(m_model->indexOfApp(QString()), -1);
    QCOMPARE(m_model->appOfIndex(-1).id(), QString());

    DataEntity missing;
    QCOMPARE(m_model->getAppById(QStringLiteral("/tmp/missing.desktop"), missing), false);

    const DataEntity duplicate = createBasicApp(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha Duplicate"));
    Q_EMIT db->appAdded({duplicate});
    QCOMPARE(insertSpy.count(), 0);

    DataEntity unknown = createBasicApp(QStringLiteral("/tmp/missing.desktop"), QStringLiteral("Missing"));
    Q_EMIT db->appUpdated({{unknown, {DataEntity::Name}}});
    QCOMPARE(dataSpy.count(), 0);

    DataEntity fullReplace = createBasicApp(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha Replaced"));
    Q_EMIT db->appUpdated({{fullReplace, {}}});
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(m_model->data(m_model->index(0, 0), DataEntity::Name).toString(), QStringLiteral("Alpha Replaced"));

    Q_EMIT db->appDeleted({QStringLiteral("/tmp/not-found.desktop")});
    QCOMPARE(removeSpy.count(), 0);
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);
}

void basicAppModel_test::test_accessorsAndDatabaseFailureBranch()
{
    auto *db = const_cast<AppDatabaseInterface *>(m_model->databaseInterface());

    QCOMPARE(m_model->columnCount(QModelIndex()), 1);
    QVERIFY(m_model->roleNames().contains(DataEntity::Name));
    QVERIFY(m_model->roleNames().contains(DataEntity::DesktopName));

    DataEntity app;
    QCOMPARE(m_model->getAppById(QStringLiteral("/tmp/a.desktop"), app), true);
    QCOMPARE(app.id(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(m_model->appOfIndex(0).id(), QStringLiteral("/tmp/a.desktop"));

    Q_EMIT db->appDatabaseOpenFailed();
    QCOMPARE(m_model->rowCount(QModelIndex()), 0);

    QSignalSpy insertSpy(m_model, &BasicAppModel::rowsInserted);
    Q_EMIT db->appAdded(g_initialApps);
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);
}

void basicAppModel_test::test_visibilityAndDesktopKeyFullUpdates()
{
    auto *db = const_cast<AppDatabaseInterface *>(m_model->databaseInterface());

    Q_EMIT db->appDatabaseOpenFailed();
    Q_EMIT db->appAdded(g_initialApps);
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);

    QSignalSpy insertSpy(m_model, &BasicAppModel::rowsInserted);
    QSignalSpy dataSpy(m_model, &BasicAppModel::dataChanged);
    QSignalSpy removeSpy(m_model, &BasicAppModel::rowsRemoved);

    DataEntity hidden = createBasicApp(QStringLiteral("/tmp/hidden.desktop"), QStringLiteral("Hidden"));
    hidden.setDontDisplay(1);
    Q_EMIT db->appAdded({hidden});
    QCOMPARE(insertSpy.count(), 0);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/hidden.desktop")), -1);

    DataEntity autostart = createBasicApp(QStringLiteral("/tmp/autostart.desktop"), QStringLiteral("Autostart"));
    autostart.setAutoStart(1);
    Q_EMIT db->appAdded({autostart});
    QCOMPARE(insertSpy.count(), 0);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/autostart.desktop")), -1);

    DataEntity duplicatePath = createBasicApp(QStringLiteral("/opt/share/a.desktop"), QStringLiteral("Alpha Replacement"));
    Q_EMIT db->appAdded({duplicatePath});
    QCOMPARE(insertSpy.count(), 0);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/a.desktop")), -1);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/opt/share/a.desktop")), 0);
    QCOMPARE(m_model->data(m_model->index(0, 0), DataEntity::Name).toString(), QStringLiteral("Alpha Replacement"));

    DataEntity hiddenReplacement = createBasicApp(QStringLiteral("/usr/local/share/a.desktop"), QStringLiteral("Hidden Replacement"));
    hiddenReplacement.setDontDisplay(1);
    Q_EMIT db->appUpdated({{hiddenReplacement, {}}});
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(m_model->rowCount(QModelIndex()), 1);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/opt/share/a.desktop")), -1);

    DataEntity visibleNew = createBasicApp(QStringLiteral("/tmp/new.desktop"), QStringLiteral("New App"));
    Q_EMIT db->appUpdated({{visibleNew, {}}});
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(m_model->indexOfApp(QStringLiteral("/tmp/new.desktop")), 1);
}

QTEST_MAIN(basicAppModel_test)

#include "basic-app-model-test.moc"
