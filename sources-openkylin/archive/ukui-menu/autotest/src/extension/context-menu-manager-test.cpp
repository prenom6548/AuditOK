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
#include <QAction>
#include <QCursor>
#include <QList>
#include <QMenu>
#include <QPointer>
#include <QWindow>

class FakeMenu : public QMenu
{
    Q_OBJECT
public:
    explicit FakeMenu(QObject *parent = nullptr)
        : QMenu(nullptr)
    {
        Q_UNUSED(parent);
        s_lastCreated = this;
    }

    ~FakeMenu() override
    {
        if (s_lastCreated == this) {
            s_lastCreated = nullptr;
        }
    }

    void popup(const QPoint &point)
    {
        m_popupPoint = point;
        m_popupCalled = true;
    }

    WId winId()
    {
        m_winIdCalled = true;
        return 1;
    }

    QWindow *windowHandle()
    {
        return &m_windowHandle;
    }

    void close()
    {
        m_closed = true;
        QMenu::close();
    }

    bool popupCalled() const { return m_popupCalled; }
    QPoint popupPoint() const { return m_popupPoint; }
    bool closed() const { return m_closed; }
    bool winIdCalled() const { return m_winIdCalled; }
    QWindow *fakeWindowHandle() { return &m_windowHandle; }

    static FakeMenu *lastCreated() { return s_lastCreated; }
    static void resetLastCreated() { s_lastCreated = nullptr; }

private:
    QPoint m_popupPoint;
    bool m_popupCalled = false;
    bool m_closed = false;
    bool m_winIdCalled = false;
    QWindow m_windowHandle;

    static FakeMenu *s_lastCreated;
};

FakeMenu *FakeMenu::s_lastCreated = nullptr;

#define private public
#include "../../../src/extension/context-menu-manager.h"
#undef private

#include "fakes/menu-extension-loader.h"
#include "fakes/basic-app-filter-model.h"

using namespace UkuiMenu;

namespace {

QHash<QString, DataEntity> g_apps;
QList<ContextMenuExtension*> g_extensions;
int g_deletedExtensions = 0;

class FakeContextMenuExtension : public ContextMenuExtension
{
public:
    enum Mode {
        ReturnAction,
        PopulateMenuOnly,
        ReturnNothing
    };

    explicit FakeContextMenuExtension(Mode mode, const QString &label = QStringLiteral("Action"))
        : m_mode(mode), m_label(label)
    {
    }

    ~FakeContextMenuExtension() override
    {
        ++g_deletedExtensions;
    }

    QList<QAction *> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location, const QString &locationId) override
    {
        m_seenData = data;
        m_seenLocation = location;
        m_seenLocationId = locationId;

        switch (m_mode) {
        case ReturnAction:
            return {new QAction(m_label, parent)};
        case PopulateMenuOnly:
            parent->addAction(m_label);
            return {};
        case ReturnNothing:
        default:
            return {};
        }
    }

    DataEntity seenData() const { return m_seenData; }
    MenuInfo::Location seenLocation() const { return m_seenLocation; }
    QString seenLocationId() const { return m_seenLocationId; }

private:
    Mode m_mode;
    QString m_label;
    DataEntity m_seenData;
    MenuInfo::Location m_seenLocation = MenuInfo::AppList;
    QString m_seenLocationId;
};

} // namespace

namespace UkuiMenu {

MenuExtensionLoader *MenuExtensionLoader::instance()
{
    static MenuExtensionLoader loader;
    return &loader;
}

QList<WidgetExtension*> MenuExtensionLoader::widgets() const
{
    return {};
}

QList<ContextMenuExtension*> MenuExtensionLoader::menus() const
{
    return g_extensions;
}

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel model;
    return &model;
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    return nullptr;
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    Q_UNUSED(row);
    return {};
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    return g_apps.contains(appid) ? 0 : -1;
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    if (!g_apps.contains(appid)) {
        return false;
    }

    app = g_apps.value(appid);
    return true;
}

} // namespace UkuiMenu

#define QMenu FakeMenu
#define UKUI_MENU_MENU_EXTENSION_LOADER_H
#define BASICAPPFILTERMODEL_H
#include "../../../src/extension/context-menu-manager.cpp"
#undef QMenu

class contextMenuManager_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_constructor_and_destructor_manageExtensions();
    void test_showMenu_byAppId_ignoresMissingApp();
    void test_showMenu_byAppId_forwardsResolvedEntity();
    void test_showMenu_deletesEmptyMenu();
    void test_showMenu_acceptsMenuOnlyContribution();
    void test_showMenu_addsActionsAndTracksMenu();
    void test_setMainWindow_and_closeMenuBranches();
    void test_checkPoint_usesCursorForNullPoint();
};

void contextMenuManager_test::init()
{
    g_extensions.clear();
    g_apps.clear();
    g_deletedExtensions = 0;
    FakeMenu::resetLastCreated();
}

void contextMenuManager_test::test_constructor_and_destructor_manageExtensions()
{
    g_extensions = {
        new FakeContextMenuExtension(FakeContextMenuExtension::ReturnNothing),
        new FakeContextMenuExtension(FakeContextMenuExtension::ReturnNothing)
    };

    auto *manager = new ContextMenuManager;
    QCOMPARE(manager->d->extensions.size(), 2);

    delete manager;
    QCOMPARE(g_deletedExtensions, 2);
    g_extensions.clear();
}

void contextMenuManager_test::test_showMenu_byAppId_ignoresMissingApp()
{
    g_extensions = { new FakeContextMenuExtension(FakeContextMenuExtension::ReturnAction) };
    ContextMenuManager manager;

    manager.showMenu(QStringLiteral("missing.desktop"), MenuInfo::Extension, QStringLiteral("lid"), QPoint(12, 34));

    QCOMPARE(FakeMenu::lastCreated(), nullptr);
}

void contextMenuManager_test::test_showMenu_byAppId_forwardsResolvedEntity()
{
    auto *extension = new FakeContextMenuExtension(FakeContextMenuExtension::ReturnAction, QStringLiteral("Resolved"));
    g_extensions = { extension };

    DataEntity entity;
    entity.setId(QStringLiteral("known.desktop"));
    entity.setName(QStringLiteral("Known"));
    g_apps.insert(entity.id(), entity);

    ContextMenuManager manager;
    manager.showMenu(entity.id(), MenuInfo::FullScreen, QStringLiteral("all"), QPoint(20, 30));

    QCOMPARE(extension->seenData().id(), QStringLiteral("known.desktop"));
    QCOMPARE(extension->seenLocation(), MenuInfo::FullScreen);
    QCOMPARE(extension->seenLocationId(), QStringLiteral("all"));
}

void contextMenuManager_test::test_showMenu_deletesEmptyMenu()
{
    g_extensions = { new FakeContextMenuExtension(FakeContextMenuExtension::ReturnNothing) };
    ContextMenuManager manager;

    DataEntity entity;
    entity.setId(QStringLiteral("empty.desktop"));
    manager.showMenu(entity, MenuInfo::Folder, QStringLiteral("none"), QPoint(3, 4));

    QCOMPARE(manager.d->contextMenu, nullptr);
}

void contextMenuManager_test::test_showMenu_acceptsMenuOnlyContribution()
{
    g_extensions = { new FakeContextMenuExtension(FakeContextMenuExtension::PopulateMenuOnly, QStringLiteral("Injected")) };
    ContextMenuManager manager;

    DataEntity entity;
    entity.setId(QStringLiteral("menu-only.desktop"));
    manager.showMenu(entity, MenuInfo::Extension, QStringLiteral("ext"), QPoint(5, 6));

    QVERIFY(manager.d->contextMenu != nullptr);
    QCOMPARE(manager.d->contextMenu->actions().size(), 1);
    QCOMPARE(manager.d->contextMenu->actions().first()->text(), QStringLiteral("Injected"));
}

void contextMenuManager_test::test_showMenu_addsActionsAndTracksMenu()
{
    g_extensions = {
        new FakeContextMenuExtension(FakeContextMenuExtension::ReturnAction, QStringLiteral("Action A")),
        new FakeContextMenuExtension(FakeContextMenuExtension::ReturnAction, QStringLiteral("Action B"))
    };
    ContextMenuManager manager;
    QWindow mainWindow;

    manager.setMainWindow(&mainWindow);

    DataEntity entity;
    entity.setId(QStringLiteral("actions.desktop"));
    manager.showMenu(entity, MenuInfo::AppList, QStringLiteral("favorite"), QPoint(10, 11));

    QVERIFY(manager.d->contextMenu != nullptr);
    QCOMPARE(manager.d->contextMenu->actions().size(), 2);
    QCOMPARE(manager.d->contextMenu->actions().at(0)->text(), QStringLiteral("Action A"));
    QCOMPARE(manager.d->contextMenu->actions().at(1)->text(), QStringLiteral("Action B"));
    QCOMPARE(manager.d->mainWindow, &mainWindow);
    QCOMPARE(manager.d->contextMenu->popupPoint(), QPoint(10, 11));
    QVERIFY(manager.d->contextMenu->popupCalled());
    QVERIFY(manager.d->contextMenu->winIdCalled());
    QCOMPARE(manager.d->contextMenu->fakeWindowHandle()->transientParent(), &mainWindow);
}

void contextMenuManager_test::test_setMainWindow_and_closeMenuBranches()
{
    g_extensions = { new FakeContextMenuExtension(FakeContextMenuExtension::ReturnAction) };
    ContextMenuManager manager;
    QWindow mainWindow;

    QCOMPARE(manager.closeMenu(), false);

    manager.setMainWindow(&mainWindow);
    QCOMPARE(manager.d->mainWindow, &mainWindow);

    DataEntity entity;
    entity.setId(QStringLiteral("close.desktop"));
    manager.showMenu(entity, MenuInfo::AppList, QString(), QPoint(7, 8));
    QVERIFY(manager.d->contextMenu != nullptr);
    QVERIFY(manager.closeMenu());
    QVERIFY(manager.d->contextMenu->closed());
}

void contextMenuManager_test::test_checkPoint_usesCursorForNullPoint()
{
    ContextMenuManager manager;

    QCOMPARE(manager.checkPoint(QPoint()), QCursor::pos());
    QCOMPARE(manager.checkPoint(QPoint(9, 10)), QPoint(9, 10));
}

QTEST_MAIN(contextMenuManager_test)

#include "context-menu-manager-test.moc"
