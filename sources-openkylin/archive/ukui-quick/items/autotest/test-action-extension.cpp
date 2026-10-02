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
#include <QtTest>

#include "action-extension.h"

using namespace UkuiQuick;

class action_extension_test : public QObject
{
    Q_OBJECT

private slots:
    void iconNameIsStoredAndEmitsOnlyOnChange();
    void separatorStateForwardsToAction();
    void explicitMenuForwardsToAction();
    void subActionsMaintainMenuAndListOrder();
};

void action_extension_test::iconNameIsStoredAndEmitsOnlyOnChange()
{
    QAction action;
    ActionExtension extension(&action);
    QSignalSpy iconNameSpy(&extension, &ActionExtension::iconNameChanged);

    extension.setIconName(QStringLiteral("document-open"));
    extension.setIconName(QStringLiteral("document-open"));

    QCOMPARE(extension.iconName(), QStringLiteral("document-open"));
    QCOMPARE(iconNameSpy.count(), 1);
}

void action_extension_test::separatorStateForwardsToAction()
{
    QAction action;
    ActionExtension extension(&action);
    QSignalSpy separatorSpy(&extension, &ActionExtension::isSeparatorChanged);

    extension.setSeparator(true);
    extension.setSeparator(true);

    QVERIFY(extension.isSeparator());
    QVERIFY(action.isSeparator());
    QCOMPARE(separatorSpy.count(), 1);

    extension.setSeparator(false);
    QVERIFY(!extension.isSeparator());
    QVERIFY(!action.isSeparator());
    QCOMPARE(separatorSpy.count(), 2);
}

void action_extension_test::explicitMenuForwardsToAction()
{
    QAction action;
    ActionExtension extension(&action);
    QSignalSpy menuSpy(&extension, &ActionExtension::menuChanged);
    auto menu = new QMenu;

    extension.setMenu(menu);
    extension.setMenu(menu);

    QCOMPARE(extension.menu(), menu);
    QCOMPARE(action.menu(), menu);
    QCOMPARE(menuSpy.count(), 1);

    extension.setMenu(nullptr);
    QCOMPARE(extension.menu(), nullptr);
    QCOMPARE(action.menu(), nullptr);
    QCOMPARE(menuSpy.count(), 2);
}

void action_extension_test::subActionsMaintainMenuAndListOrder()
{
    QAction action;
    ActionExtension extension(&action);
    auto list = extension.subActions();
    QAction first(QStringLiteral("first"), &action);
    QAction second(QStringLiteral("second"), &action);
    QAction replacement(QStringLiteral("replacement"), &action);

    list.append(&list, &first);
    list.append(&list, &second);
    list.append(&list, &first);

    QCOMPARE(list.count(&list), 2);
    QCOMPARE(list.at(&list, 0), &first);
    QCOMPARE(list.at(&list, 1), &second);
    QVERIFY(action.menu() != nullptr);
    QCOMPARE(action.menu()->actions().size(), 2);
    QCOMPARE(action.menu()->actions().at(0), &first);
    QCOMPARE(action.menu()->actions().at(1), &second);

    list.replace(&list, 0, &replacement);
    QCOMPARE(list.at(&list, 0), &replacement);
    QCOMPARE(action.menu()->actions().at(0), &replacement);

    list.removeLast(&list);
    QCOMPARE(list.count(&list), 1);
    QVERIFY(action.menu() != nullptr);

    list.clear(&list);
    QCOMPARE(list.count(&list), 0);
    QCOMPARE(action.menu(), nullptr);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    action_extension_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-action-extension.moc"
