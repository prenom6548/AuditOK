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

#define private public
#define protected public
#include "../../../src/utils/user-menu.h"
#undef private
#undef protected

#include <QtTest>
#include <QAction>
#include <QSignalSpy>

class userMenu_test : public QObject
{
    Q_OBJECT

private slots:
    void test_addMiniButton_rejectsInvalidAction();
    void test_addMiniButton_createsButtonAndEmitsSignal();
    void test_updateAllButtonPos_hidesButtonWithoutGeometry();
    void test_eventFilter_clearsActiveActionOnEnter();
};

void userMenu_test::test_addMiniButton_rejectsInvalidAction()
{
    UserMenu menu;
    QAction foreignAction(QStringLiteral("foreign"), nullptr);

    menu.addMiniButton(nullptr, QStringLiteral("Log Out"), QSize(50, 20));
    menu.addMiniButton(&foreignAction, QStringLiteral("Log Out"), QSize(50, 20));

    QCOMPARE(menu.m_actionBtnMap.size(), 0);
}

void userMenu_test::test_addMiniButton_createsButtonAndEmitsSignal()
{
    UserMenu menu;
    QAction *action = menu.addAction(QStringLiteral("Current User"));
    QSignalSpy spy(&menu, &UserMenu::miniButtonClicked);

    menu.addMiniButton(action, QStringLiteral("Log Out"), QSize(56, 26));

    QCOMPARE(menu.m_actionBtnMap.size(), 1);
    QPushButton *button = menu.m_actionBtnMap.value(action);
    QVERIFY(button != nullptr);
    QCOMPARE(button->text(), QStringLiteral("Log Out"));
    QCOMPARE(button->size(), QSize(56, 26));
    QCOMPARE(button->isVisible(), false);

    button->click();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<QAction *>(), action);
    QCOMPARE(spy.at(0).at(1).value<QPushButton *>(), button);
}

void userMenu_test::test_updateAllButtonPos_hidesButtonWithoutGeometry()
{
    UserMenu menu;
    QAction *action = menu.addAction(QStringLiteral("Current User"));
    menu.addMiniButton(action, QStringLiteral("Log Out"), QSize(56, 26));
    QPushButton *button = menu.m_actionBtnMap.value(action);
    QVERIFY(button != nullptr);
    button->setVisible(true);

    menu.updateAllButtonPos();

    QCOMPARE(button->isVisible(), false);
}

void userMenu_test::test_eventFilter_clearsActiveActionOnEnter()
{
    UserMenu menu;
    QAction *action = menu.addAction(QStringLiteral("Current User"));
    menu.addMiniButton(action, QStringLiteral("Log Out"), QSize(56, 26));
    QPushButton *button = menu.m_actionBtnMap.value(action);
    QVERIFY(button != nullptr);

    menu.setActiveAction(action);
    QEvent event(QEvent::Enter);

    QVERIFY(menu.eventFilter(button, &event) == false);
    QCOMPARE(menu.activeAction(), nullptr);
}

QTEST_MAIN(userMenu_test)

#include "user-menu-test.moc"
