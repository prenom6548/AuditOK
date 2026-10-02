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

#include "../../../src/libappdata/app-list-plugin.h"

using namespace UkuiMenu;

class FakeListPluginForInterfaceTest : public AppListPluginInterface
{
    Q_OBJECT
public:
    explicit FakeListPluginForInterfaceTest(QObject *parent = nullptr)
        : AppListPluginInterface(parent)
    {
    }

    AppListPluginGroup::Group group() override
    {
        return AppListPluginGroup::Display;
    }

    QString name() override
    {
        return QStringLiteral("FakeListPluginForInterfaceTest");
    }

    QString title() override
    {
        return QStringLiteral("Fake Title");
    }

    QList<QAction *> actions() override
    {
        return {};
    }

    QAbstractItemModel *dataModel() override
    {
        return nullptr;
    }
};

class appListPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void test_labelItem_returnsConstructorValues();
    void test_labelBottle_updatesPropertiesAndSignals();
    void test_appListPluginInterface_defaultHelpers();
};

void appListPlugin_test::test_labelItem_returnsConstructorValues()
{
    const LabelItem item(QStringLiteral("office"), QStringLiteral("Office"), LabelItem::Icon);

    QCOMPARE(item.labelName(), QStringLiteral("office"));
    QCOMPARE(item.displayName(), QStringLiteral("Office"));
    QCOMPARE(item.type(), LabelItem::Icon);
}

void appListPlugin_test::test_labelBottle_updatesPropertiesAndSignals()
{
    LabelBottle bottle;
    QSignalSpy columnSpy(&bottle, &LabelBottle::columnChanged);
    QSignalSpy visibleSpy(&bottle, &LabelBottle::visibleChanged);
    QSignalSpy labelsSpy(&bottle, &LabelBottle::labelsChanged);

    bottle.setColumn(5);
    bottle.setVisible(true);
    bottle.setLabels({
        LabelItem(QStringLiteral("A"), QStringLiteral("A")),
        LabelItem(QStringLiteral("B"), QStringLiteral("B"), LabelItem::Icon)
    });

    QCOMPARE(bottle.column(), 5);
    QCOMPARE(bottle.visible(), true);
    QCOMPARE(bottle.labels().size(), 2);
    QCOMPARE(bottle.labels().at(1).type(), LabelItem::Icon);
    QCOMPARE(columnSpy.count(), 1);
    QCOMPARE(visibleSpy.count(), 1);
    QCOMPARE(labelsSpy.count(), 1);
}

void appListPlugin_test::test_appListPluginInterface_defaultHelpers()
{
    FakeListPluginForInterfaceTest plugin;

    plugin.search(QStringLiteral("keyword"));
    QCOMPARE(plugin.labelBottle(), nullptr);
}

QTEST_MAIN(appListPlugin_test)

#include "app-list-plugin-test.moc"
