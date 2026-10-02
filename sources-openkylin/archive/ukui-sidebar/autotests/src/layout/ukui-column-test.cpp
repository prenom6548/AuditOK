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
#include <QMatrix4x4>
#include <QQuickItem>
#include <QQmlEngine>
#include <private/qquickanchors_p_p.h>
#include "../../../src/layout/ukui-column.h"

class TestUkuiColumn : public UkuiColumn
{
    Q_OBJECT

public:
    explicit TestUkuiColumn(QQuickItem *parent = nullptr)
        : UkuiColumn(parent)
    {
    }

    using UkuiColumn::componentComplete;

    int positionedCount() const { return positionedItems.count(); }
    int unpositionedCount() const { return unpositionedItems.count(); }
    QQuickItem *unpositionedItemAt(int index) const { return unpositionedItems.at(index).item; }
    void callPositionItemX(qreal x, PositionedItem *target) { positionItemX(x, target); }
    void callPositionItemY(qreal y, PositionedItem *target) { positionItemY(y, target); }
    void callPositionItem(qreal x, qreal y, PositionedItem *target) { positionItem(x, y, target); }
    PositionedItem makePositionedItem(QQuickItem *item) { return PositionedItem(item); }
    int transformCount(QQuickItem *item) const
    {
        QQmlListProperty<QQuickTransform> prop = item->transform();
        return prop.count(&prop);
    }
};

class ukuiColumn_test : public QObject
{
    Q_OBJECT

private slots:
    void test_ukuiScale_defaults();
    void test_ukuiScale_setters_emitSignals();
    void test_ukuiScale_applyTo_scalesAroundOrigin();
    void test_positionerAttached_defaults();
    void test_positionerAttached_setters_emitSignals();
    void test_column_defaults();
    void test_column_spacingAndTransitions();
    void test_column_paddingAccessorsAndReset();
    void test_column_nonFoldLayout_positionsChildren();
    void test_column_foldLayout_stacksChildren();
    void test_column_attachedProperties_updateAfterLayout();
    void test_positionerAttached_ctor_readsParentLayoutState();
    void test_column_zeroSizedChild_becomesUnpositioned();
    void test_column_childRemoval_updatesLayout();
    void test_column_resetPaddingAndNoopPaddingSetters();
    void test_column_visibilityAndGeometryChangesRelayout();
    void test_column_anchorConflictSkipsPositioning();
    void test_column_foldLayoutTallFoldedItemsUseYScale();
    void test_positionedItem_helpersHandleNullAndTransforms();
    void test_basePositioner_positionAxisGuards();
    void test_column_destructorUnwatchesUnpositionedItems();
};

void ukuiColumn_test::test_ukuiScale_defaults()
{
    UkuiScale scale;
    QCOMPARE(scale.origin(), QVector3D());
    QCOMPARE(scale.xScale(), 1.0);
    QCOMPARE(scale.yScale(), 1.0);
    QCOMPARE(scale.zScale(), 1.0);
}

void ukuiColumn_test::test_ukuiScale_setters_emitSignals()
{
    UkuiScale scale;
    QSignalSpy originSpy(&scale, &UkuiScale::originChanged);
    QSignalSpy xSpy(&scale, &UkuiScale::xScaleChanged);
    QSignalSpy ySpy(&scale, &UkuiScale::yScaleChanged);
    QSignalSpy zSpy(&scale, &UkuiScale::zScaleChanged);
    QSignalSpy allScaleSpy(&scale, &UkuiScale::scaleChanged);

    scale.setOrigin(QVector3D(1.0f, 2.0f, 3.0f));
    scale.setXScale(1.5);
    scale.setYScale(0.75);
    scale.setZScale(2.0);

    QCOMPARE(scale.origin(), QVector3D(1.0f, 2.0f, 3.0f));
    QCOMPARE(scale.xScale(), 1.5);
    QCOMPARE(scale.yScale(), 0.75);
    QCOMPARE(scale.zScale(), 2.0);
    QCOMPARE(originSpy.count(), 1);
    QCOMPARE(xSpy.count(), 1);
    QCOMPARE(ySpy.count(), 1);
    QCOMPARE(zSpy.count(), 1);
    QCOMPARE(allScaleSpy.count(), 3);

    scale.setXScale(1.5);
    scale.setYScale(0.75);
    scale.setZScale(2.0);
    QCOMPARE(xSpy.count(), 1);
    QCOMPARE(ySpy.count(), 1);
    QCOMPARE(zSpy.count(), 1);
    QCOMPARE(allScaleSpy.count(), 3);
}

void ukuiColumn_test::test_ukuiScale_applyTo_scalesAroundOrigin()
{
    UkuiScale scale;
    scale.setOrigin(QVector3D(10.0f, 20.0f, 0.0f));
    scale.setXScale(2.0);
    scale.setYScale(0.5);

    QMatrix4x4 matrix;
    scale.applyTo(&matrix);

    const QVector3D transformed = matrix.map(QVector3D(12.0f, 24.0f, 0.0f));
    QCOMPARE(transformed, QVector3D(14.0f, 22.0f, 0.0f));
}

void ukuiColumn_test::test_positionerAttached_defaults()
{
    UkuiPositionerAttached attached(nullptr);
    QCOMPARE(attached.index(), -1);
    QCOMPARE(attached.isFirstItem(), false);
    QCOMPARE(attached.isLastItem(), false);
}

void ukuiColumn_test::test_positionerAttached_setters_emitSignals()
{
    UkuiPositionerAttached attached(nullptr);
    QSignalSpy indexSpy(&attached, &UkuiPositionerAttached::indexChanged);
    QSignalSpy firstSpy(&attached, &UkuiPositionerAttached::isFirstItemChanged);
    QSignalSpy lastSpy(&attached, &UkuiPositionerAttached::isLastItemChanged);

    attached.setIndex(2);
    attached.setIsFirstItem(true);
    attached.setIsLastItem(true);

    QCOMPARE(attached.index(), 2);
    QCOMPARE(attached.isFirstItem(), true);
    QCOMPARE(attached.isLastItem(), true);
    QCOMPARE(indexSpy.count(), 1);
    QCOMPARE(firstSpy.count(), 1);
    QCOMPARE(lastSpy.count(), 1);

    attached.setIndex(2);
    attached.setIsFirstItem(true);
    attached.setIsLastItem(true);
    QCOMPARE(indexSpy.count(), 1);
    QCOMPARE(firstSpy.count(), 1);
    QCOMPARE(lastSpy.count(), 1);
}

void ukuiColumn_test::test_column_defaults()
{
    TestUkuiColumn column;
    QCOMPARE(column.isFoldMode(), true);
    QCOMPARE(column.heightOffset(), 0);
    QCOMPARE(column.spacing(), 0.0);
    QCOMPARE(column.padding(), 0.0);
    QCOMPARE(column.topPadding(), 0.0);
    QCOMPARE(column.leftPadding(), 0.0);
    QCOMPARE(column.rightPadding(), 0.0);
    QCOMPARE(column.bottomPadding(), 0.0);
}

void ukuiColumn_test::test_column_spacingAndTransitions()
{
    TestUkuiColumn column;
    QSignalSpy spacingSpy(&column, &UkuiColumn::spacingChanged);
    QSignalSpy populateSpy(&column, &UkuiColumn::populateChanged);
    QSignalSpy moveSpy(&column, &UkuiColumn::moveChanged);
    QSignalSpy addSpy(&column, &UkuiColumn::addChanged);
    QSignalSpy removeSpy(&column, &UkuiColumn::removeTransitionChanged);

    QQuickTransition populate;
    QQuickTransition move;
    QQuickTransition add;
    QQuickTransition remove;

    column.setSpacing(6.0);
    column.setSpacing(6.0);
    QCOMPARE(column.spacing(), 6.0);
    QCOMPARE(spacingSpy.count(), 1);

    column.setPopulate(&populate);
    column.setPopulate(&populate);
    QCOMPARE(column.populate(), &populate);
    QCOMPARE(populateSpy.count(), 1);

    column.setMove(&move);
    column.setMove(&move);
    QCOMPARE(column.move(), &move);
    QCOMPARE(moveSpy.count(), 1);

    column.setAdd(&add);
    column.setAdd(&add);
    QCOMPARE(column.add(), &add);
    QCOMPARE(addSpy.count(), 1);

    column.setRemoveTransition(&remove);
    column.setRemoveTransition(&remove);
    QCOMPARE(column.removeTransition(), &remove);
    QCOMPARE(removeSpy.count(), 1);
}

void ukuiColumn_test::test_column_paddingAccessorsAndReset()
{
    TestUkuiColumn column;
    QSignalSpy paddingSpy(&column, &UkuiColumn::paddingChanged);
    QSignalSpy topSpy(&column, &UkuiColumn::topPaddingChanged);
    QSignalSpy leftSpy(&column, &UkuiColumn::leftPaddingChanged);
    QSignalSpy rightSpy(&column, &UkuiColumn::rightPaddingChanged);
    QSignalSpy bottomSpy(&column, &UkuiColumn::bottomPaddingChanged);

    column.setPadding(10.0);
    QCOMPARE(column.padding(), 10.0);
    QCOMPARE(column.topPadding(), 10.0);
    QCOMPARE(column.leftPadding(), 10.0);
    QCOMPARE(column.rightPadding(), 10.0);
    QCOMPARE(column.bottomPadding(), 10.0);

    column.setTopPadding(1.0);
    column.setLeftPadding(2.0);
    column.setRightPadding(3.0);
    column.setBottomPadding(4.0);
    QCOMPARE(column.topPadding(), 1.0);
    QCOMPARE(column.leftPadding(), 2.0);
    QCOMPARE(column.rightPadding(), 3.0);
    QCOMPARE(column.bottomPadding(), 4.0);

    column.resetTopPadding();
    column.resetLeftPadding();
    column.resetRightPadding();
    column.resetBottomPadding();
    QCOMPARE(column.topPadding(), 10.0);
    QCOMPARE(column.leftPadding(), 10.0);
    QCOMPARE(column.rightPadding(), 10.0);
    QCOMPARE(column.bottomPadding(), 10.0);

    QVERIFY(paddingSpy.count() >= 1);
    QVERIFY(topSpy.count() >= 2);
    QVERIFY(leftSpy.count() >= 2);
    QVERIFY(rightSpy.count() >= 2);
    QVERIFY(bottomSpy.count() >= 2);
}

void ukuiColumn_test::test_column_nonFoldLayout_positionsChildren()
{
    TestUkuiColumn column;
    column.setFoldMode(false);
    column.setSpacing(5.0);
    column.setPadding(4.0);

    QQuickItem first(&column);
    first.setWidth(100.0);
    first.setHeight(30.0);

    QQuickItem second(&column);
    second.setWidth(80.0);
    second.setHeight(20.0);

    QQuickItem third(&column);
    third.setWidth(120.0);
    third.setHeight(10.0);

    column.componentComplete();

    QCOMPARE(first.x(), 4.0);
    QCOMPARE(first.y(), 4.0);
    QCOMPARE(second.x(), 4.0);
    QCOMPARE(second.y(), 39.0);
    QCOMPARE(third.x(), 4.0);
    QCOMPARE(third.y(), 64.0);
    QCOMPARE(column.implicitWidth(), 128.0);
    QCOMPARE(column.implicitHeight(), 78.0);
    QCOMPARE(column.heightOffset(), 0);
    QCOMPARE(first.scale(), second.scale());
    QCOMPARE(second.scale(), third.scale());
}

void ukuiColumn_test::test_column_foldLayout_stacksChildren()
{
    TestUkuiColumn column;
    column.setPadding(4.0);

    QQuickItem first(&column);
    first.setWidth(100.0);
    first.setHeight(100.0);

    QQuickItem second(&column);
    second.setWidth(80.0);
    second.setHeight(50.0);

    QQuickItem third(&column);
    third.setWidth(70.0);
    third.setHeight(40.0);

    QQuickItem fourth(&column);
    fourth.setWidth(60.0);
    fourth.setHeight(30.0);

    column.componentComplete();

    QCOMPARE(first.x(), 4.0);
    QCOMPARE(first.y(), 4.0);
    QCOMPARE(first.z(), 3.0);

    QCOMPARE(second.x(), 4.0);
    QCOMPARE(second.y(), 64.5);
    QCOMPARE(second.z(), 2.0);

    QCOMPARE(third.x(), 4.0);
    QCOMPARE(third.y(), 84.0);
    QCOMPARE(third.z(), 1.0);

    QCOMPARE(fourth.x(), second.x());
    QCOMPARE(fourth.y(), 64.0);
    QCOMPARE(fourth.z(), 0.0);
    QVERIFY(second.y() > first.y());
    QVERIFY(third.y() > second.y());

    QCOMPARE(column.implicitWidth(), 108.0);
    QCOMPARE(column.implicitHeight(), 124.0);
    QCOMPARE(column.heightOffset(), 8);
}

void ukuiColumn_test::test_column_attachedProperties_updateAfterLayout()
{
    TestUkuiColumn column;
    column.setFoldMode(false);

    QQuickItem first(&column);
    first.setWidth(40.0);
    first.setHeight(20.0);

    QQuickItem second(&column);
    second.setWidth(30.0);
    second.setHeight(15.0);

    QQuickItem third(&column);
    third.setWidth(20.0);
    third.setHeight(10.0);

    UkuiPositionerAttached firstAttached(&first);
    UkuiPositionerAttached secondAttached(&second);
    UkuiPositionerAttached thirdAttached(&third);

    column.componentComplete();

    column.updateAttachedProperties(&firstAttached, &first);
    column.updateAttachedProperties(&secondAttached, &second);
    column.updateAttachedProperties(&thirdAttached, &third);

    QCOMPARE(firstAttached.index(), 0);
    QCOMPARE(firstAttached.isFirstItem(), true);
    QCOMPARE(firstAttached.isLastItem(), false);

    QCOMPARE(secondAttached.index(), 1);
    QCOMPARE(secondAttached.isFirstItem(), false);
    QCOMPARE(secondAttached.isLastItem(), false);

    QCOMPARE(thirdAttached.index(), 2);
    QCOMPARE(thirdAttached.isFirstItem(), false);
    QCOMPARE(thirdAttached.isLastItem(), true);
}

void ukuiColumn_test::test_positionerAttached_ctor_readsParentLayoutState()
{
    TestUkuiColumn column;
    column.setFoldMode(false);

    QQuickItem first(&column);
    first.setWidth(10.0);
    first.setHeight(10.0);

    QQuickItem second(&column);
    second.setWidth(20.0);
    second.setHeight(20.0);

    column.componentComplete();

    UkuiPositionerAttached firstAttached(&first);
    UkuiPositionerAttached secondAttached(&second);

    QCOMPARE(firstAttached.index(), 0);
    QCOMPARE(firstAttached.isFirstItem(), true);
    QCOMPARE(firstAttached.isLastItem(), false);
    QCOMPARE(secondAttached.index(), 1);
    QCOMPARE(secondAttached.isFirstItem(), false);
    QCOMPARE(secondAttached.isLastItem(), true);
}

void ukuiColumn_test::test_column_zeroSizedChild_becomesUnpositioned()
{
    TestUkuiColumn column;
    column.setFoldMode(false);

    QQuickItem visible(&column);
    visible.setWidth(40.0);
    visible.setHeight(20.0);

    QQuickItem zeroSized(&column);
    zeroSized.setWidth(0.0);
    zeroSized.setHeight(10.0);

    column.componentComplete();

    QCOMPARE(column.positionedCount(), 1);
    QCOMPARE(column.unpositionedCount(), 1);
    QCOMPARE(column.unpositionedItemAt(0), &zeroSized);

    UkuiPositionerAttached zeroAttached(&zeroSized);
    QCOMPARE(zeroAttached.index(), -1);
    QCOMPARE(zeroAttached.isFirstItem(), false);
    QCOMPARE(zeroAttached.isLastItem(), false);
}

void ukuiColumn_test::test_column_childRemoval_updatesLayout()
{
    TestUkuiColumn column;
    column.setFoldMode(false);
    column.setSpacing(5.0);

    auto *first = new QQuickItem(&column);
    first->setWidth(30.0);
    first->setHeight(10.0);

    auto *second = new QQuickItem(&column);
    second->setWidth(30.0);
    second->setHeight(20.0);

    column.componentComplete();
    QCOMPARE(column.positionedCount(), 2);
    QCOMPARE(column.implicitHeight(), 35.0);

    delete second;
    column.forceLayout();

    QCOMPARE(column.positionedCount(), 1);
    QCOMPARE(column.unpositionedCount(), 0);
    QCOMPARE(column.implicitHeight(), 10.0);
    QCOMPARE(first->y(), 0.0);
}

void ukuiColumn_test::test_column_resetPaddingAndNoopPaddingSetters()
{
    TestUkuiColumn column;
    QSignalSpy paddingSpy(&column, &UkuiColumn::paddingChanged);
    QSignalSpy topSpy(&column, &UkuiColumn::topPaddingChanged);
    QSignalSpy leftSpy(&column, &UkuiColumn::leftPaddingChanged);
    QSignalSpy rightSpy(&column, &UkuiColumn::rightPaddingChanged);
    QSignalSpy bottomSpy(&column, &UkuiColumn::bottomPaddingChanged);

    column.resetPadding();
    column.resetTopPadding();
    column.resetLeftPadding();
    column.resetRightPadding();
    column.resetBottomPadding();
    QCOMPARE(paddingSpy.count(), 0);
    QCOMPARE(topSpy.count(), 0);
    QCOMPARE(leftSpy.count(), 0);
    QCOMPARE(rightSpy.count(), 0);
    QCOMPARE(bottomSpy.count(), 0);

    column.setPadding(8.0);
    column.setTopPadding(3.0);
    column.setLeftPadding(4.0);
    column.setRightPadding(5.0);
    column.setBottomPadding(6.0);

    column.setTopPadding(3.0);
    column.setLeftPadding(4.0);
    column.setRightPadding(5.0);
    column.setBottomPadding(6.0);
    QCOMPARE(topSpy.count(), 2);
    QCOMPARE(leftSpy.count(), 2);
    QCOMPARE(rightSpy.count(), 2);
    QCOMPARE(bottomSpy.count(), 2);

    column.resetPadding();
    QCOMPARE(column.padding(), 0.0);
    QCOMPARE(column.topPadding(), 3.0);
    QCOMPARE(column.leftPadding(), 4.0);
    QCOMPARE(column.rightPadding(), 5.0);
    QCOMPARE(column.bottomPadding(), 6.0);
}

void ukuiColumn_test::test_column_visibilityAndGeometryChangesRelayout()
{
    TestUkuiColumn column;
    column.setFoldMode(false);
    column.setSpacing(2.0);

    QQuickItem first(&column);
    first.setWidth(20.0);
    first.setHeight(10.0);

    QQuickItem second(&column);
    second.setWidth(20.0);
    second.setHeight(20.0);
    second.setVisible(false);

    column.componentComplete();
    QCOMPARE(column.positionedCount(), 1);
    QCOMPARE(column.unpositionedCount(), 1);

    second.setVisible(true);
    column.forceLayout();
    QCOMPARE(column.positionedCount(), 2);
    QCOMPARE(column.unpositionedCount(), 0);
    QCOMPARE(second.y(), 12.0);

    second.setHeight(0.0);
    column.forceLayout();
    QCOMPARE(column.positionedCount(), 1);
    QCOMPARE(column.unpositionedCount(), 1);
    QCOMPARE(column.unpositionedItemAt(0), &second);

    second.setHeight(30.0);
    column.forceLayout();
    QCOMPARE(column.positionedCount(), 2);
    QCOMPARE(second.y(), 12.0);
}

void ukuiColumn_test::test_column_anchorConflictSkipsPositioning()
{
    TestUkuiColumn column;
    column.setFoldMode(false);
    column.setSpacing(5.0);

    QQuickItem child(&column);
    child.setWidth(40.0);
    child.setHeight(20.0);
    child.setY(77.0);
    QQuickItemPrivate::get(&child)->anchors()->setTop(QQuickAnchorLine(&column, QQuickAnchors::TopAnchor));

    column.componentComplete();

    QCOMPARE(column.positionedCount(), 1);
    QCOMPARE(column.implicitHeight(), 0.0);
}

void ukuiColumn_test::test_column_foldLayoutTallFoldedItemsUseYScale()
{
    TestUkuiColumn column;
    column.setPadding(0.0);

    QQuickItem first(&column);
    first.setWidth(100.0);
    first.setHeight(40.0);

    QQuickItem second(&column);
    second.setWidth(100.0);
    second.setHeight(200.0);

    QQuickItem third(&column);
    third.setWidth(100.0);
    third.setHeight(160.0);

    QQuickItem fourth(&column);
    fourth.setWidth(100.0);
    fourth.setHeight(180.0);

    column.componentComplete();

    QCOMPARE(column.positionedCount(), 4);
    QVERIFY(column.transformCount(&second) > 0);
    QVERIFY(column.transformCount(&fourth) > 0);
    QCOMPARE(second.y(), 8.0);
    QCOMPARE(fourth.y(), second.y());
}

void ukuiColumn_test::test_positionedItem_helpersHandleNullAndTransforms()
{
    TestUkuiColumn column;
    QQuickItem item;
    item.setWidth(20.0);
    item.setHeight(10.0);

    auto positioned = column.makePositionedItem(&item);
    QCOMPARE(positioned.itemX(), 0.0);
    QCOMPARE(positioned.itemY(), 0.0);

    positioned.moveTo(QPointF(3.0, 4.0));
    QCOMPARE(item.position(), QPointF(3.0, 4.0));

    positioned.updatePadding(1.0, 2.0, 3.0, 4.0);
    QCOMPARE(positioned.leftPadding, 1.0);
    QCOMPARE(positioned.topPadding, 2.0);
    QCOMPARE(positioned.rightPadding, 3.0);
    QCOMPARE(positioned.bottomPadding, 4.0);

    positioned.setYScale(0.5);
    QCOMPARE(positioned.yScale, 0.5);
    QCOMPARE(column.transformCount(&item), 1);

    positioned.setYScale(0.5);
    QCOMPARE(column.transformCount(&item), 1);

    positioned.setYScale(1.0);
    QCOMPARE(positioned.yScale, 1.0);
    QCOMPARE(column.transformCount(&item), 0);

    positioned.setScaleTransition(1.0, 1.2);
    positioned.applyCustomTransitions();
    QVERIFY(!positioned.hasScaleTransition);

    UkuiScale transform;
    auto nullItem = column.makePositionedItem(nullptr);
    nullItem.setYScale(0.4);
    nullItem.addTransformToItem(&transform);
    nullItem.clearTransformFromItem();
    nullItem.applyCustomTransitions();
}

void ukuiColumn_test::test_basePositioner_positionAxisGuards()
{
    TestUkuiColumn column;
    QQuickItem item;
    item.setPosition(QPointF(1.0, 2.0));
    auto positioned = column.makePositionedItem(&item);

    column.callPositionItemX(10.0, &positioned);
    QCOMPARE(item.x(), 1.0);

    column.callPositionItemY(20.0, &positioned);
    QCOMPARE(item.y(), 20.0);

    column.callPositionItemY(20.0, &positioned);
    QCOMPARE(item.y(), 20.0);

    column.callPositionItem(7.0, 8.0, &positioned);
    QCOMPARE(item.position(), QPointF(7.0, 8.0));
}

void ukuiColumn_test::test_column_destructorUnwatchesUnpositionedItems()
{
    auto *column = new TestUkuiColumn;
    auto *zeroSized = new QQuickItem(column);
    zeroSized->setWidth(0.0);
    zeroSized->setHeight(10.0);

    column->componentComplete();
    QCOMPARE(column->positionedCount(), 0);
    QCOMPARE(column->unpositionedCount(), 1);

    delete column;
}

QTEST_MAIN(ukuiColumn_test)
#include "ukui-column-test.moc"
