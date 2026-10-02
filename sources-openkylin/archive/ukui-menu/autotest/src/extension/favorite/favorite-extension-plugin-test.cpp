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

#include "../../../../src/extension/widget-extension.h"
#include "../../../../src/extension/context-menu-extension.h"
#include "../../../../src/extension/favorite/favorite-extension-plugin.h"

using namespace UkuiMenu;

namespace UkuiMenu {

class FavoriteWidget : public WidgetExtension
{
public:
    MetadataMap metadata() const override
    {
        return {};
    }
};

class FavoriteContextMenu : public ContextMenuExtension
{
public:
    QList<QAction *> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location, const QString &locationId) override
    {
        Q_UNUSED(data);
        Q_UNUSED(parent);
        Q_UNUSED(location);
        Q_UNUSED(locationId);
        return {};
    }
};

} // namespace UkuiMenu

#define UKUI_MENU_FAVORITE_WIDGET_H
#define UKUI_MENU_FAVORITE_CONTEXT_MENU_H
#include "../../../../src/extension/menu-extension-plugin.cpp"
#include "../../../../src/extension/widget-extension.cpp"
#include "../../../../src/extension/context-menu-extension.cpp"
#include "../../../../src/extension/favorite/favorite-extension-plugin.cpp"

class favoriteExtensionPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void test_idAndFactories();
};

void favoriteExtensionPlugin_test::test_idAndFactories()
{
    FavoriteExtensionPlugin plugin;

    QCOMPARE(plugin.id(), QStringLiteral("favorite"));

    QScopedPointer<WidgetExtension> widget(plugin.createWidgetExtension());
    QVERIFY(widget);
    QVERIFY(dynamic_cast<FavoriteWidget *>(widget.data()));

    QScopedPointer<ContextMenuExtension> menu(plugin.createContextMenuExtension());
    QVERIFY(menu);
    QVERIFY(dynamic_cast<FavoriteContextMenu *>(menu.data()));
}

QTEST_MAIN(favoriteExtensionPlugin_test)

#include "favorite-extension-plugin-test.moc"
