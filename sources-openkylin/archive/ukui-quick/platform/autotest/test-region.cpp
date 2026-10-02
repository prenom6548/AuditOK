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

#include <QSignalSpy>
#include <QtTest>

#include "region.h"

using namespace UkuiQuick;

class region_test : public QObject
{
    Q_OBJECT

private slots:
    void rectRegionPropertiesEmitOnlyOnChange();
    void regionTracksAppendedRectChangesAndRemoval();
    void appendRectListAndClearMaintainUnion();
};

void region_test::rectRegionPropertiesEmitOnlyOnChange()
{
    RectRegion rect;
    QSignalSpy xSpy(&rect, &RectRegion::xChanged);
    QSignalSpy ySpy(&rect, &RectRegion::yChanged);
    QSignalSpy widthSpy(&rect, &RectRegion::widthChanged);
    QSignalSpy heightSpy(&rect, &RectRegion::heightChanged);
    QSignalSpy radiusSpy(&rect, &RectRegion::radiusChanged);

    rect.setX(10);
    rect.setY(20);
    rect.setWidth(30);
    rect.setHeight(40);
    rect.setRadius(5);
    rect.setX(10);
    rect.setY(20);
    rect.setWidth(30);
    rect.setHeight(40);
    rect.setRadius(5);

    QCOMPARE(rect.x(), 10);
    QCOMPARE(rect.y(), 20);
    QCOMPARE(rect.width(), 30);
    QCOMPARE(rect.height(), 40);
    QCOMPARE(rect.radius(), 5);
    QCOMPARE(xSpy.count(), 1);
    QCOMPARE(ySpy.count(), 1);
    QCOMPARE(widthSpy.count(), 1);
    QCOMPARE(heightSpy.count(), 1);
    QCOMPARE(radiusSpy.count(), 1);
}

void region_test::regionTracksAppendedRectChangesAndRemoval()
{
    Region region;
    RectRegion rect;
    rect.setX(10);
    rect.setY(20);
    rect.setWidth(30);
    rect.setHeight(40);
    QSignalSpy regionSpy(&region, &Region::regionChanged);

    region.appendRect(&rect);

    QCOMPARE(regionSpy.count(), 1);
    QCOMPARE(region.region().boundingRect(), QRect(10, 20, 30, 40));

    rect.setX(15);
    QCOMPARE(regionSpy.count(), 2);
    QCOMPARE(region.region().boundingRect(), QRect(15, 20, 30, 40));

    region.removeRect(&rect);
    QCOMPARE(regionSpy.count(), 3);
    QVERIFY(region.region().isEmpty());

    rect.setX(25);
    QCOMPARE(regionSpy.count(), 3);
}

void region_test::appendRectListAndClearMaintainUnion()
{
    Region region;
    RectRegion first;
    first.setX(15);
    first.setY(20);
    first.setWidth(30);
    first.setHeight(40);
    RectRegion second;
    second.setX(0);
    second.setY(0);
    second.setWidth(10);
    second.setHeight(10);
    QSignalSpy regionSpy(&region, &Region::regionChanged);

    region.appendRectList({&first, &second});

    QCOMPARE(regionSpy.count(), 1);
    QCOMPARE(region.region().boundingRect(), QRect(0, 0, 45, 60));

    region.clear();
    QCOMPARE(regionSpy.count(), 2);
    QVERIFY(region.region().isEmpty());

    second.setWidth(20);
    QCOMPARE(regionSpy.count(), 2);
}

QTEST_GUILESS_MAIN(region_test)

#include "test-region.moc"
