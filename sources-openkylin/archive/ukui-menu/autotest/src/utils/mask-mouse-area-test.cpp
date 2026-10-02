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
#include <QQmlComponent>
#include <QQmlEngine>

#include "../../../src/utils/mask-mouse-area.h"

class maskMouseArea_test : public QObject
{
    Q_OBJECT

private slots:
    void test_constructor_unsetsCursor();
    void test_defineModule_registersQmlType();
};

void maskMouseArea_test::test_constructor_unsetsCursor()
{
    MaskMouseArea area;

    QCOMPARE(area.cursor().shape(), Qt::ArrowCursor);
}

void maskMouseArea_test::test_defineModule_registersQmlType()
{
    MaskMouseArea::defineModule("org.ukui.menu.utils", 1, 0);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.15\nimport org.ukui.menu.utils 1.0\nMaskMouseArea {}", QUrl());

    QObject *object = component.create();
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    QVERIFY(qobject_cast<MaskMouseArea *>(object));
    delete object;
}

QTEST_MAIN(maskMouseArea_test)

#include "mask-mouse-area-test.moc"
