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

#include "../../../src/data-entity.h"
#include "../../../src/extension/context-menu-manager.h"
#include "../../../src/libappdata/app-list-model.h"

using namespace UkuiMenu;

namespace {

DataEntity g_lastMenuEntity;
MenuInfo::Location g_lastMenuLocation = MenuInfo::Location::AppList;

QStandardItem *createListItem(const QString &name, const QString &group, const QString &id)
{
    auto *item = new QStandardItem(name);
    DataEntity entity;
    entity.setId(id);
    entity.setName(name);
    entity.setGroup(group);
    item->setData(name, DataEntity::Name);
    item->setData(group, DataEntity::Group);
    item->setData(QVariant::fromValue(entity), DataEntity::Entity);
    return item;
}

}

namespace UkuiMenu {

ContextMenuManager *ContextMenuManager::instance()
{
    static ContextMenuManager manager;
    return &manager;
}

ContextMenuManager::ContextMenuManager()
    : QObject(nullptr)
{
}

ContextMenuManager::~ContextMenuManager() = default;

void ContextMenuManager::showMenu(const QString &appid, MenuInfo::Location location, const QString &lid, const QPoint &point)
{
    Q_UNUSED(appid);
    Q_UNUSED(location);
    Q_UNUSED(lid);
    Q_UNUSED(point);
}

void ContextMenuManager::showMenu(const DataEntity &data, MenuInfo::Location location, const QString &lid, const QPoint &point)
{
    Q_UNUSED(lid);
    Q_UNUSED(point);
    g_lastMenuEntity = data;
    g_lastMenuLocation = location;
}

void ContextMenuManager::setMainWindow(QWindow *mainWindow)
{
    Q_UNUSED(mainWindow);
}

bool ContextMenuManager::closeMenu()
{
    return true;
}

}

class FakeListPlugin : public AppListPluginInterface
{
    Q_OBJECT
public:
    FakeListPlugin()
        : m_actionA(new QAction(QStringLiteral("Action A"), this))
        , m_actionB(new QAction(QStringLiteral("Action B"), this))
    {
        m_model.setColumnCount(1);
        m_model.appendRow(createListItem(QStringLiteral("Alpha"), QStringLiteral("A"), QStringLiteral("/tmp/alpha.desktop")));
        m_model.appendRow(createListItem(QStringLiteral("Beta"), QStringLiteral("B"), QStringLiteral("/tmp/beta.desktop")));
        m_bottle.setLabels({LabelItem(QStringLiteral("A"), QStringLiteral("A"))});
    }

    AppListPluginGroup::Group group() override
    {
        return AppListPluginGroup::Display;
    }

    QString name() override
    {
        return QStringLiteral("FakeListPlugin");
    }

    QString title() override
    {
        return m_title;
    }

    QList<QAction *> actions() override
    {
        return {m_actionA, m_actionB};
    }

    QAbstractItemModel *dataModel() override
    {
        return &m_model;
    }

    LabelBottle *labelBottle() override
    {
        return &m_bottle;
    }

    void changeTitle(const QString &title)
    {
        m_title = title;
        Q_EMIT titleChanged();
    }

    void emitLabelChanged()
    {
        Q_EMIT labelChanged();
    }

private:
    QString m_title {QStringLiteral("Initial Title")};
    QStandardItemModel m_model;
    LabelBottle m_bottle;
    QAction *m_actionA {nullptr};
    QAction *m_actionB {nullptr};
};

class appListModel_test : public QObject
{
    Q_OBJECT

private slots:
    void test_installPlugin_updatesHeaderAndSourceModel();
    void test_pluginSignals_updateHeaderAndLabelBottle();
    void test_unInstallPlugin_resetsHeaderAndSourceModel();
    void test_findLabelIndex_returnsRowFromSourceModel();
    void test_openMenu_forwardsEntityToContextMenuManager();
};

void appListModel_test::test_installPlugin_updatesHeaderAndSourceModel()
{
    AppListModel model;
    FakeListPlugin plugin;

    model.installPlugin(&plugin);

    QCOMPARE(model.sourceModel(), plugin.dataModel());
    QCOMPARE(model.getHeader()->title(), QStringLiteral("Initial Title"));
    QCOMPARE(model.getHeader()->visible(), true);
    QCOMPARE(model.getHeader()->actions().size(), 2);
    QCOMPARE(model.labelBottle(), plugin.labelBottle());
}

void appListModel_test::test_pluginSignals_updateHeaderAndLabelBottle()
{
    AppListModel model;
    FakeListPlugin plugin;
    QSignalSpy labelSpy(&model, &AppListModel::labelBottleChanged);

    model.installPlugin(&plugin);
    plugin.changeTitle(QStringLiteral("Updated Title"));
    plugin.emitLabelChanged();

    QCOMPARE(model.getHeader()->title(), QStringLiteral("Updated Title"));
    QVERIFY(labelSpy.count() >= 2);
}

void appListModel_test::test_unInstallPlugin_resetsHeaderAndSourceModel()
{
    AppListModel model;
    FakeListPlugin plugin;

    model.installPlugin(&plugin);
    model.unInstallPlugin();

    QCOMPARE(model.sourceModel(), nullptr);
    QCOMPARE(model.getHeader()->title(), QString());
    QCOMPARE(model.getHeader()->visible(), false);
    QCOMPARE(model.getHeader()->actions().size(), 0);
    QCOMPARE(model.labelBottle(), nullptr);
}

void appListModel_test::test_findLabelIndex_returnsRowFromSourceModel()
{
    AppListModel model;
    FakeListPlugin plugin;

    model.installPlugin(&plugin);

    QCOMPARE(model.findLabelIndex(QStringLiteral("A")), 0);
    QCOMPARE(model.findLabelIndex(QStringLiteral("B")), 1);
    QCOMPARE(model.findLabelIndex(QStringLiteral("C")), -1);
}

void appListModel_test::test_openMenu_forwardsEntityToContextMenuManager()
{
    AppListModel model;
    FakeListPlugin plugin;
    g_lastMenuEntity = DataEntity();

    model.installPlugin(&plugin);
    model.openMenu(1, MenuInfo::AppList);

    QCOMPARE(g_lastMenuEntity.id(), QStringLiteral("/tmp/beta.desktop"));
    QCOMPARE(g_lastMenuLocation, MenuInfo::AppList);
}

QTEST_MAIN(appListModel_test)

#include "app-list-model-test.moc"
