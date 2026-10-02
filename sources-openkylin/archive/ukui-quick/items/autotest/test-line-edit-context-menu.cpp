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
#include <QMenu>
#include <QSignalSpy>
#include <QtTest>

#include "line-edit-context-menu.h"

using namespace UkuiQuick;

class line_edit_context_menu_test : public QObject
{
    Q_OBJECT

private slots:
    void qmlActionsMaintainInsertionOrder();
    void shortcutStringHonorsApplicationAttribute();
    void showDisplaysPopupMenuWithConfiguredActions();
};

void line_edit_context_menu_test::qmlActionsMaintainInsertionOrder()
{
    LineEditContextMenu menu;
    auto list = menu.qmlActions();
    QAction first(QStringLiteral("Cut"), &menu);
    QAction second(QStringLiteral("Copy"), &menu);

    list.append(&list, &first);
    list.append(&list, &second);

    QCOMPARE(list.count(&list), 2);
    QCOMPARE(list.at(&list, 0), &first);
    QCOMPARE(list.at(&list, 1), &second);
    QCOMPARE(menu.actions().size(), 2);
    QCOMPARE(menu.actions().at(0), &first);
    QCOMPARE(menu.actions().at(1), &second);
}

void line_edit_context_menu_test::shortcutStringHonorsApplicationAttribute()
{
    QCoreApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, false);
    const QString copyShortcut = LineEditContextMenu().getShortCutString(QKeySequence::Copy);

    QVERIFY(!copyShortcut.isEmpty());
    QVERIFY(copyShortcut.startsWith(QLatin1Char('\t')));

    QCoreApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, true);
    QCOMPARE(LineEditContextMenu().getShortCutString(QKeySequence::Copy), QString());

    QCoreApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, false);
}

void line_edit_context_menu_test::showDisplaysPopupMenuWithConfiguredActions()
{
    LineEditContextMenu menu;
    auto list = menu.qmlActions();
    QAction first(QStringLiteral("Cut"), &menu);
    QAction second(QStringLiteral("Paste"), &menu);

    list.append(&list, &first);
    list.append(&list, &second);

    menu.show(10, 10, nullptr);

    QTRY_VERIFY(QApplication::activePopupWidget() != nullptr);
    auto *popupMenu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
    QVERIFY(popupMenu != nullptr);
    QCOMPARE(popupMenu->actions().size(), 2);
    QCOMPARE(popupMenu->actions().at(0)->text(), QStringLiteral("Cut"));
    QCOMPARE(popupMenu->actions().at(1)->text(), QStringLiteral("Paste"));

    menu.hide();
    QTRY_VERIFY(QApplication::activePopupWidget() == nullptr);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QApplication app(argc, argv);
    line_edit_context_menu_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-line-edit-context-menu.moc"
