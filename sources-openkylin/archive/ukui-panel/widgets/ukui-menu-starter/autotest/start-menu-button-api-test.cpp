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
 */

#include "../plugin/start-menu-button.h"

#include <QTest>

class StartMenuButtonApiTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDoesNotExposeCurrentScreenObjectStateToQml();
};

void StartMenuButtonApiTest::testDoesNotExposeCurrentScreenObjectStateToQml()
{
    const auto &metaObject = StartMenuButton::staticMetaObject;

    QCOMPARE(metaObject.indexOfProperty("currentScreenId"), -1);
    QCOMPARE(metaObject.indexOfMethod("screenId(QScreen*)"), -1);
    QVERIFY(metaObject.indexOfProperty("currentScreenValid") >= 0);
    QVERIFY(metaObject.indexOfMethod("isCurrentScreen(QScreen*)") >= 0);
}

QTEST_MAIN(StartMenuButtonApiTest)

#include "start-menu-button-api-test.moc"
