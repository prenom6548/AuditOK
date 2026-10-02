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

#include "../../../src/extension/widget-extension.h"
#include "../../../src/extension/context-menu-extension.h"
#include "../../../src/extension/menu-extension-plugin.h"

using namespace UkuiMenu;

namespace {

int g_deletedPlugins = 0;
int g_deletedWidgets = 0;
int g_deletedMenus = 0;

class FakeWidgetExtension : public WidgetExtension
{
public:
    explicit FakeWidgetExtension(int index)
        : m_index(index)
    {
    }

    ~FakeWidgetExtension() override
    {
        ++g_deletedWidgets;
    }

    int index() const override
    {
        return m_index;
    }

    MetadataMap metadata() const override
    {
        return {};
    }

private:
    int m_index;
};

class FakeContextMenuExtension : public ContextMenuExtension
{
public:
    explicit FakeContextMenuExtension(int index)
        : m_index(index)
    {
    }

    ~FakeContextMenuExtension() override
    {
        ++g_deletedMenus;
    }

    int index() const override
    {
        return m_index;
    }

    QList<QAction *> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location, const QString &locationId) override
    {
        Q_UNUSED(data);
        Q_UNUSED(parent);
        Q_UNUSED(location);
        Q_UNUSED(locationId);
        return {};
    }

private:
    int m_index;
};

class FakePlugin : public MenuExtensionPlugin
{
public:
    FakePlugin(const QString &id, int widgetIndex, int menuIndex)
        : m_id(id), m_widgetIndex(widgetIndex), m_menuIndex(menuIndex)
    {
    }

    ~FakePlugin() override
    {
        ++g_deletedPlugins;
    }

    QString id() override
    {
        return m_id;
    }

    WidgetExtension *createWidgetExtension() override
    {
        return m_widgetIndex == NoExtension ? nullptr : new FakeWidgetExtension(m_widgetIndex);
    }

    ContextMenuExtension *createContextMenuExtension() override
    {
        return m_menuIndex == NoExtension ? nullptr : new FakeContextMenuExtension(m_menuIndex);
    }

    static constexpr int NoExtension = -9999;

private:
    QString m_id;
    int m_widgetIndex;
    int m_menuIndex;
};

} // namespace

#define UKUI_MENU_FAVORITE_EXTENSION_PLUGIN_H
#define UKUI_MENU_APP_MENU_PLUGIN_H
namespace UkuiMenu {
class FavoriteExtensionPlugin : public FakePlugin
{
public:
    FavoriteExtensionPlugin()
        : FakePlugin(QStringLiteral("favorite"), 10, 10)
    {
    }
};

class AppMenuPlugin : public FakePlugin
{
public:
    AppMenuPlugin()
        : FakePlugin(QStringLiteral("app-menu"), FakePlugin::NoExtension, 20)
    {
    }
};
}

#define private public
#include "../../../src/extension/menu-extension-loader.h"
#undef private

#include "../../../src/extension/menu-extension-plugin.cpp"
#include "../../../src/extension/widget-extension.cpp"
#include "../../../src/extension/context-menu-extension.cpp"
#include "../../../src/extension/menu-extension-loader.cpp"

class menuExtensionLoader_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_registerExtension_rejectsBlacklistedAndDuplicatePlugins();
    void test_expand_createsAndSortsExtensions();
    void test_loadInternalExtension_registersBuiltInPlugins();
};

void menuExtensionLoader_test::init()
{
    MenuExtensionLoader::plugins.clear();
    g_deletedPlugins = 0;
    g_deletedWidgets = 0;
    g_deletedMenus = 0;
}

void menuExtensionLoader_test::cleanup()
{
    qDeleteAll(MenuExtensionLoader::plugins);
    MenuExtensionLoader::plugins.clear();
}

void menuExtensionLoader_test::test_registerExtension_rejectsBlacklistedAndDuplicatePlugins()
{
    MenuExtensionLoader loader;
    qDeleteAll(MenuExtensionLoader::plugins);
    MenuExtensionLoader::plugins.clear();
    g_deletedPlugins = 0;

    loader.setBlackList({QStringLiteral("blocked")});
    loader.registerExtension(new FakePlugin(QStringLiteral("blocked"), 1, 1));
    QCOMPARE(MenuExtensionLoader::plugins.size(), 0);
    QCOMPARE(g_deletedPlugins, 1);

    loader.registerExtension(new FakePlugin(QStringLiteral("same"), 1, 1));
    loader.registerExtension(new FakePlugin(QStringLiteral("same"), 2, 2));
    QCOMPARE(MenuExtensionLoader::plugins.size(), 1);
    QCOMPARE(g_deletedPlugins, 2);
}

void menuExtensionLoader_test::test_expand_createsAndSortsExtensions()
{
    MenuExtensionLoader loader;
    qDeleteAll(MenuExtensionLoader::plugins);
    MenuExtensionLoader::plugins.clear();
    g_deletedPlugins = 0;
    loader.m_widgets.clear();
    loader.m_menus.clear();

    loader.registerExtension(new FakePlugin(QStringLiteral("last"), -1, -1));
    loader.registerExtension(new FakePlugin(QStringLiteral("first"), 1, 1));
    loader.registerExtension(new FakePlugin(QStringLiteral("middle"), 5, 5));
    loader.expand();

    QCOMPARE(loader.widgets().size(), 3);
    QCOMPARE(loader.menus().size(), 3);
    QCOMPARE(loader.widgets().at(0)->index(), 1);
    QCOMPARE(loader.widgets().at(1)->index(), 5);
    QCOMPARE(loader.widgets().at(2)->index(), -1);
    QCOMPARE(loader.menus().at(0)->index(), 1);
    QCOMPARE(loader.menus().at(1)->index(), 5);
    QCOMPARE(loader.menus().at(2)->index(), -1);

    qDeleteAll(loader.m_widgets);
    qDeleteAll(loader.m_menus);
    loader.m_widgets.clear();
    loader.m_menus.clear();
}

void menuExtensionLoader_test::test_loadInternalExtension_registersBuiltInPlugins()
{
    MenuExtensionLoader loader;
    qDeleteAll(MenuExtensionLoader::plugins);
    MenuExtensionLoader::plugins.clear();
    g_deletedPlugins = 0;

    loader.loadInternalExtension();

    QVERIFY(MenuExtensionLoader::plugins.contains(QStringLiteral("favorite")));
    QVERIFY(MenuExtensionLoader::plugins.contains(QStringLiteral("app-menu")));
}

QTEST_MAIN(menuExtensionLoader_test)

#include "menu-extension-loader-test.moc"
