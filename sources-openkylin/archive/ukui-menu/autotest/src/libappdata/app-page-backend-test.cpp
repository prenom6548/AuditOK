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
#include "../../../src/libappdata/app-page-backend.h"
#include "../../../src/libappdata/app-list-model.h"
#include "../../../src/libappdata/app-category-plugin.h"
#include "../../../src/libappdata/app-search-plugin.h"
#include "../../../src/extension/context-menu-manager.h"

using namespace UkuiMenu;

namespace {

QString g_lastSearchKeyword;
QHash<const UkuiMenu::AppCategoryPlugin *, QStandardItemModel *> g_categoryModels;
QHash<const UkuiMenu::AppSearchPlugin *, QStandardItemModel *> g_searchModels;

QStandardItem *createBackendItem(const QString &name)
{
    auto *item = new QStandardItem(name);
    item->setData(name, DataEntity::Name);
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
    Q_UNUSED(data);
    Q_UNUSED(location);
    Q_UNUSED(lid);
    Q_UNUSED(point);
}

void ContextMenuManager::setMainWindow(QWindow *mainWindow)
{
    Q_UNUSED(mainWindow);
}

bool ContextMenuManager::closeMenu()
{
    return true;
}

AppCategoryPlugin::AppCategoryPlugin(QObject *parent)
    : AppListPluginInterface(parent)
{
    auto *model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->appendRow(createBackendItem(QStringLiteral("DisplayPlugin")));
    g_categoryModels.insert(this, model);
}

AppListPluginGroup::Group AppCategoryPlugin::group()
{
    return AppListPluginGroup::Display;
}

QString AppCategoryPlugin::name()
{
    return QStringLiteral("DisplayPlugin");
}

QString AppCategoryPlugin::title()
{
    return QStringLiteral("DisplayPlugin");
}

QList<QAction *> AppCategoryPlugin::actions()
{
    return {};
}

QAbstractItemModel *AppCategoryPlugin::dataModel()
{
    return g_categoryModels.value(this, nullptr);
}

LabelBottle *AppCategoryPlugin::labelBottle()
{
    return nullptr;
}

AppSearchPlugin::AppSearchPlugin(QObject *parent)
    : AppListPluginInterface(parent)
{
    auto *model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->appendRow(createBackendItem(QStringLiteral("SearchPlugin")));
    g_searchModels.insert(this, model);
}

AppSearchPlugin::~AppSearchPlugin() = default;

AppListPluginGroup::Group AppSearchPlugin::group()
{
    return AppListPluginGroup::Search;
}

QString AppSearchPlugin::name()
{
    return QStringLiteral("SearchPlugin");
}

QString AppSearchPlugin::title()
{
    return QStringLiteral("SearchPlugin");
}

QList<QAction *> AppSearchPlugin::actions()
{
    return {};
}

QAbstractItemModel *AppSearchPlugin::dataModel()
{
    return g_searchModels.value(this, nullptr);
}

void AppSearchPlugin::search(const QString &keyword)
{
    g_lastSearchKeyword = keyword;
}

}

class appPageBackend_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_defaultGroup_andInstalledPlugin();
    void test_setGroup_switchesPluginAndEmitsSignal();
    void test_startSearch_onlyForSearchGroup();
};

void appPageBackend_test::init()
{
    g_lastSearchKeyword.clear();
}

void appPageBackend_test::test_defaultGroup_andInstalledPlugin()
{
    AppPageBackend *backend = AppPageBackend::instance();

    QCOMPARE(backend->group(), AppListPluginGroup::Display);
    QVERIFY(backend->appModel() != nullptr);
    QCOMPARE(backend->appModel()->sourceModel()->rowCount(), 1);
}

void appPageBackend_test::test_setGroup_switchesPluginAndEmitsSignal()
{
    AppPageBackend *backend = AppPageBackend::instance();
    QSignalSpy spy(backend, &AppPageBackend::groupChanged);

    backend->setGroup(AppListPluginGroup::Search);

    QCOMPARE(backend->group(), AppListPluginGroup::Display);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(backend->appModel()->sourceModel()->data(backend->appModel()->sourceModel()->index(0, 0), DataEntity::Name).toString(),
             QStringLiteral("SearchPlugin"));

    backend->setGroup(AppListPluginGroup::Display);
}

void appPageBackend_test::test_startSearch_onlyForSearchGroup()
{
    AppPageBackend *backend = AppPageBackend::instance();

    backend->setGroup(AppListPluginGroup::Display);
    backend->startSearch(QStringLiteral("ignored"));
    QCOMPARE(g_lastSearchKeyword, QString());

    backend->setGroup(AppListPluginGroup::Search);
    backend->startSearch(QStringLiteral("terminal"));
    QCOMPARE(g_lastSearchKeyword, QStringLiteral("terminal"));

    backend->setGroup(AppListPluginGroup::Display);
}

QTEST_MAIN(appPageBackend_test)

#include "app-page-backend-test.moc"
