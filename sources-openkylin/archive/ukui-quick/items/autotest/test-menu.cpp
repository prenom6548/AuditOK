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

#include <QApplication>
#include <QMenu>
#include <QQuickItem>
#include <QSignalSpy>
#include <QtTest>

#include "menu.h"

using namespace UkuiQuick;

class menu_test : public QObject
{
    Q_OBJECT

private slots:
    void contentMaintainsMenuItemsInOrder();
    void transientParentEmitsOnlyOnChange();
    void openShowsPopupAndCloseHidesIt();
};

void menu_test::contentMaintainsMenuItemsInOrder()
{
    Menu menu;
    auto list = menu.content();
    MenuItem first(&menu);
    MenuItem second(&menu);
    first.setText(QStringLiteral("Open"));
    second.setText(QStringLiteral("Close"));

    list.append(&list, &first);
    list.append(&list, &second);

    QCOMPARE(list.count(&list), 2);
    QCOMPARE(list.at(&list, 0), &first);
    QCOMPARE(list.at(&list, 1), &second);
}

void menu_test::transientParentEmitsOnlyOnChange()
{
    Menu menu;
    QQuickItem firstParent;
    QQuickItem secondParent;
    QSignalSpy parentSpy(&menu, &Menu::transientParentChanged);

    menu.setTransientParent(&firstParent);
    menu.setTransientParent(&firstParent);
    menu.setTransientParent(&secondParent);

    QCOMPARE(parentSpy.count(), 2);
}

void menu_test::openShowsPopupAndCloseHidesIt()
{
    Menu menu;
    auto list = menu.content();
    MenuItem first(&menu);
    MenuItem second(&menu);
    first.setText(QStringLiteral("Copy"));
    second.setText(QStringLiteral("Paste"));

    list.append(&list, &first);
    list.append(&list, &second);

    QSignalSpy visibleSpy(&menu, &Menu::visibleChanged);

    menu.open(10, 10);

    QTRY_VERIFY(QApplication::activePopupWidget() != nullptr);
    auto *popupMenu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
    QVERIFY(popupMenu != nullptr);
    QVERIFY(menu.visible());
    QCOMPARE(popupMenu->actions().size(), 2);
    QCOMPARE(popupMenu->actions().at(0)->text(), QStringLiteral("Copy"));
    QCOMPARE(popupMenu->actions().at(1)->text(), QStringLiteral("Paste"));

    menu.close();

    QTRY_VERIFY(QApplication::activePopupWidget() == nullptr);
    QVERIFY(!menu.visible());
    QVERIFY(visibleSpy.count() >= 2);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QApplication app(argc, argv);
    menu_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-menu.moc"
