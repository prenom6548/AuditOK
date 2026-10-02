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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QAction>
#include <QApplication>
#include <QSignalSpy>
#include <QtTest>

#include "menu-item.h"

using namespace UkuiQuick;

class menu_item_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultActionIsUsable();
    void propertiesForwardToCurrentAction();
    void actionSignalsAreForwarded();
    void externalActionCanBeReplacedAndCleared();
};

void menu_item_test::defaultActionIsUsable()
{
    MenuItem item;

    QVERIFY(item.action() != nullptr);
    QCOMPARE(item.text(), QString());
    QVERIFY(item.isEnabled());
    QVERIFY(!item.checkable());
    QVERIFY(!item.checked());
}

void menu_item_test::propertiesForwardToCurrentAction()
{
    MenuItem item;

    item.setText(QStringLiteral("Open"));
    item.setEnabled(false);
    item.setCheckable(true);
    item.setChecked(true);

    QCOMPARE(item.text(), QStringLiteral("Open"));
    QCOMPARE(item.action()->text(), QStringLiteral("Open"));
    QVERIFY(!item.isEnabled());
    QVERIFY(!item.action()->isEnabled());
    QVERIFY(item.checkable());
    QVERIFY(item.action()->isCheckable());
    QVERIFY(item.checked());
    QVERIFY(item.action()->isChecked());
}

void menu_item_test::actionSignalsAreForwarded()
{
    MenuItem item;
    item.setCheckable(true);
    QSignalSpy clickedSpy(&item, &MenuItem::clicked);
    QSignalSpy toggledSpy(&item, &MenuItem::toggled);

    item.action()->trigger();

    QCOMPARE(clickedSpy.count(), 1);
    QCOMPARE(toggledSpy.count(), 1);
    QCOMPARE(toggledSpy.takeFirst().at(0).toBool(), true);
    QVERIFY(item.checked());
}

void menu_item_test::externalActionCanBeReplacedAndCleared()
{
    MenuItem item;
    QAction externalAction(QStringLiteral("External"));
    externalAction.setEnabled(false);
    QSignalSpy actionSpy(&item, &MenuItem::actionChanged);

    item.setAction(&externalAction);

    QCOMPARE(item.action(), &externalAction);
    QCOMPARE(item.text(), QStringLiteral("External"));
    QVERIFY(!item.isEnabled());
    QCOMPARE(actionSpy.count(), 1);

    item.setAction(nullptr);

    QVERIFY(item.action() != nullptr);
    QVERIFY(item.action() != &externalAction);
    QVERIFY(!item.action()->isVisible());
    QCOMPARE(actionSpy.count(), 2);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    menu_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-menu-item.moc"
