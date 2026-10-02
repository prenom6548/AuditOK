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

#include <QtTest>

#include "margin.h"

using namespace UkuiQuick;

class margin_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructorHasZeroMargins();
    void parameterConstructorStoresMargins();
    void settersUpdateValuesAndEmitSignals();
    void settersIgnoreUnchangedValues();
};

void margin_test::defaultConstructorHasZeroMargins()
{
    Margin margin;

    QCOMPARE(margin.left(), 0);
    QCOMPARE(margin.top(), 0);
    QCOMPARE(margin.right(), 0);
    QCOMPARE(margin.bottom(), 0);
}

void margin_test::parameterConstructorStoresMargins()
{
    Margin margin(1, 2, 3, 4);

    QCOMPARE(margin.left(), 1);
    QCOMPARE(margin.top(), 2);
    QCOMPARE(margin.right(), 3);
    QCOMPARE(margin.bottom(), 4);
}

void margin_test::settersUpdateValuesAndEmitSignals()
{
    Margin margin;
    QSignalSpy leftSpy(&margin, &Margin::leftChanged);
    QSignalSpy topSpy(&margin, &Margin::topChanged);
    QSignalSpy rightSpy(&margin, &Margin::rightChanged);
    QSignalSpy bottomSpy(&margin, &Margin::bottomChanged);

    margin.setLeft(10);
    margin.setTop(20);
    margin.setRight(30);
    margin.setBottom(40);

    QCOMPARE(margin.left(), 10);
    QCOMPARE(margin.top(), 20);
    QCOMPARE(margin.right(), 30);
    QCOMPARE(margin.bottom(), 40);
    QCOMPARE(leftSpy.count(), 1);
    QCOMPARE(topSpy.count(), 1);
    QCOMPARE(rightSpy.count(), 1);
    QCOMPARE(bottomSpy.count(), 1);
}

void margin_test::settersIgnoreUnchangedValues()
{
    Margin margin(5, 6, 7, 8);
    QSignalSpy leftSpy(&margin, &Margin::leftChanged);
    QSignalSpy topSpy(&margin, &Margin::topChanged);
    QSignalSpy rightSpy(&margin, &Margin::rightChanged);
    QSignalSpy bottomSpy(&margin, &Margin::bottomChanged);

    margin.setLeft(5);
    margin.setTop(6);
    margin.setRight(7);
    margin.setBottom(8);

    QCOMPARE(leftSpy.count(), 0);
    QCOMPARE(topSpy.count(), 0);
    QCOMPARE(rightSpy.count(), 0);
    QCOMPARE(bottomSpy.count(), 0);
}

QTEST_GUILESS_MAIN(margin_test)

#include "test-margin.moc"
