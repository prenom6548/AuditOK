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

#include "shadow-data.h"

using namespace UkuiQuick;

class shadow_data_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructorUsesEmptyShadow();
    void parameterConstructorStoresValues();
    void settersUpdateValuesAndEmitSignals();
    void settersIgnoreUnchangedValues();
    void childShadowIsClonedAndTracksSourceChanges();
    void childShadowUsesParentOwnershipAndAvoidsDuplicateConnections();
    void childShadowDisconnectsPreviousSource();
    void childShadowClonesNestedSourceRecursively();
    void childShadowRejectsSelfReference();
    void childShadowRejectsMutualReference();
    void copyConstructorAndAssignmentCloneChildShadow();
    void inequalityComparesFlatAndNestedValues();
};

void shadow_data_test::defaultConstructorUsesEmptyShadow()
{
    ShadowData shadow;

    QCOMPARE(shadow.xOffset(), 0);
    QCOMPARE(shadow.yOffset(), 0);
    QCOMPARE(shadow.blurRadius(), 0);
    QCOMPARE(shadow.spread(), 0);
    QCOMPARE(shadow.shadowColor(), QColor(0, 0, 0, 0));
    QCOMPARE(shadow.isMultiShadow(), false);
    QCOMPARE(shadow.childShadow(), nullptr);
}

void shadow_data_test::parameterConstructorStoresValues()
{
    ShadowData child(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData shadow(10, 20, 30, 40, QColor(50, 60, 70, 80), true, &child);

    QCOMPARE(shadow.xOffset(), 10);
    QCOMPARE(shadow.yOffset(), 20);
    QCOMPARE(shadow.blurRadius(), 30);
    QCOMPARE(shadow.spread(), 40);
    QCOMPARE(shadow.shadowColor(), QColor(50, 60, 70, 80));
    QCOMPARE(shadow.isMultiShadow(), true);
    QVERIFY(shadow.childShadow() != nullptr);
    QVERIFY(shadow.childShadow() != &child);
    QCOMPARE(shadow.childShadow()->xOffset(), 1);
    QCOMPARE(shadow.childShadow()->shadowColor(), QColor(5, 6, 7, 8));
}

void shadow_data_test::settersUpdateValuesAndEmitSignals()
{
    ShadowData shadow;
    QSignalSpy xOffsetSpy(&shadow, &ShadowData::xOffsetChanged);
    QSignalSpy yOffsetSpy(&shadow, &ShadowData::yOffsetChanged);
    QSignalSpy blurRadiusSpy(&shadow, &ShadowData::blurRadiusChanged);
    QSignalSpy spreadSpy(&shadow, &ShadowData::spreadChanged);
    QSignalSpy colorSpy(&shadow, &ShadowData::shadowColorChanged);
    QSignalSpy multiShadowSpy(&shadow, &ShadowData::isMultiShadowChanged);

    shadow.setXOffset(1);
    shadow.setYOffset(2);
    shadow.setBlurRadius(3);
    shadow.setSpread(4);
    shadow.setShadowColor(QColor(5, 6, 7, 8));
    shadow.setIsMultiShadow(true);

    QCOMPARE(shadow.xOffset(), 1);
    QCOMPARE(shadow.yOffset(), 2);
    QCOMPARE(shadow.blurRadius(), 3);
    QCOMPARE(shadow.spread(), 4);
    QCOMPARE(shadow.shadowColor(), QColor(5, 6, 7, 8));
    QCOMPARE(shadow.isMultiShadow(), true);
    QCOMPARE(xOffsetSpy.count(), 1);
    QCOMPARE(yOffsetSpy.count(), 1);
    QCOMPARE(blurRadiusSpy.count(), 1);
    QCOMPARE(spreadSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(multiShadowSpy.count(), 1);
}

void shadow_data_test::settersIgnoreUnchangedValues()
{
    ShadowData shadow(1, 2, 3, 4, QColor(5, 6, 7, 8), true);
    QSignalSpy xOffsetSpy(&shadow, &ShadowData::xOffsetChanged);
    QSignalSpy yOffsetSpy(&shadow, &ShadowData::yOffsetChanged);
    QSignalSpy blurRadiusSpy(&shadow, &ShadowData::blurRadiusChanged);
    QSignalSpy spreadSpy(&shadow, &ShadowData::spreadChanged);
    QSignalSpy colorSpy(&shadow, &ShadowData::shadowColorChanged);
    QSignalSpy multiShadowSpy(&shadow, &ShadowData::isMultiShadowChanged);

    shadow.setXOffset(1);
    shadow.setYOffset(2);
    shadow.setBlurRadius(3);
    shadow.setSpread(4);
    shadow.setShadowColor(QColor(5, 6, 7, 8));
    shadow.setIsMultiShadow(true);

    QCOMPARE(xOffsetSpy.count(), 0);
    QCOMPARE(yOffsetSpy.count(), 0);
    QCOMPARE(blurRadiusSpy.count(), 0);
    QCOMPARE(spreadSpy.count(), 0);
    QCOMPARE(colorSpy.count(), 0);
    QCOMPARE(multiShadowSpy.count(), 0);
}

void shadow_data_test::childShadowIsClonedAndTracksSourceChanges()
{
    ShadowData sourceChild(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData shadow;
    QSignalSpy childShadowSpy(&shadow, &ShadowData::childShadowChanged);
    QSignalSpy multiShadowSpy(&shadow, &ShadowData::isMultiShadowChanged);

    shadow.setChildShadow(&sourceChild);

    QVERIFY(shadow.childShadow() != nullptr);
    QVERIFY(shadow.childShadow() != &sourceChild);
    QCOMPARE(shadow.isMultiShadow(), true);
    QCOMPARE(shadow.childShadow()->xOffset(), sourceChild.xOffset());
    QCOMPARE(shadow.childShadow()->yOffset(), sourceChild.yOffset());
    QCOMPARE(shadow.childShadow()->blurRadius(), sourceChild.blurRadius());
    QCOMPARE(shadow.childShadow()->spread(), sourceChild.spread());
    QCOMPARE(shadow.childShadow()->shadowColor(), sourceChild.shadowColor());
    QCOMPARE(childShadowSpy.count(), 1);
    QCOMPARE(multiShadowSpy.count(), 1);

    QSignalSpy clonedXOffsetSpy(shadow.childShadow(), &ShadowData::xOffsetChanged);
    sourceChild.setXOffset(11);

    QCOMPARE(shadow.childShadow()->xOffset(), 11);
    QCOMPARE(clonedXOffsetSpy.count(), 1);

    shadow.setChildShadow(nullptr);

    QCOMPARE(shadow.childShadow(), nullptr);
    QCOMPARE(shadow.isMultiShadow(), false);
    QCOMPARE(childShadowSpy.count(), 2);
    QCOMPARE(multiShadowSpy.count(), 2);
}

void shadow_data_test::childShadowUsesParentOwnershipAndAvoidsDuplicateConnections()
{
    ShadowData sourceChild(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData shadow;
    QSignalSpy childShadowSpy(&shadow, &ShadowData::childShadowChanged);

    shadow.setChildShadow(&sourceChild);
    QPointer<ShadowData> clonedChild = shadow.childShadow();
    QVERIFY(clonedChild != nullptr);
    QCOMPARE(clonedChild->parent(), &shadow);

    shadow.setChildShadow(&sourceChild);
    QCOMPARE(childShadowSpy.count(), 1);

    QSignalSpy clonedXOffsetSpy(clonedChild, &ShadowData::xOffsetChanged);
    sourceChild.setXOffset(11);

    QCOMPARE(clonedChild->xOffset(), 11);
    QCOMPARE(clonedXOffsetSpy.count(), 1);

    shadow.setChildShadow(nullptr);
    QVERIFY(clonedChild.isNull());
}

void shadow_data_test::childShadowDisconnectsPreviousSource()
{
    ShadowData firstSource(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData secondSource(10, 20, 30, 40, QColor(50, 60, 70, 80));
    ShadowData shadow;

    shadow.setChildShadow(&firstSource);
    shadow.setChildShadow(&secondSource);

    QVERIFY(shadow.childShadow() != nullptr);
    QCOMPARE(shadow.childShadow()->xOffset(), secondSource.xOffset());

    firstSource.setXOffset(99);
    QCOMPARE(shadow.childShadow()->xOffset(), secondSource.xOffset());

    secondSource.setXOffset(77);
    QCOMPARE(shadow.childShadow()->xOffset(), 77);
}

void shadow_data_test::childShadowClonesNestedSourceRecursively()
{
    ShadowData grandChild(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData source(10, 20, 30, 40, QColor(50, 60, 70, 80));
    source.setChildShadow(&grandChild);
    QVERIFY(source.isMultiShadow());
    QVERIFY(source.childShadow() != nullptr);

    ShadowData shadow;
    shadow.setChildShadow(&source);

    // 克隆体应递归包含源的子阴影，不能停留在 isMultiShadow==true && childShadow()==nullptr
    QVERIFY(shadow.childShadow() != nullptr);
    QVERIFY(shadow.childShadow()->isMultiShadow());
    QVERIFY(shadow.childShadow()->childShadow() != nullptr);
    QCOMPARE(shadow.childShadow()->childShadow()->xOffset(), 1);

    // 源深层子阴影的标量变化应沿克隆链同步
    grandChild.setXOffset(99);
    QCOMPARE(shadow.childShadow()->childShadow()->xOffset(), 99);

    // 源整体替换子阴影时，克隆链应跟随
    ShadowData grandChild2(100, 200, 300, 400, QColor(110, 120, 130, 140));
    source.setChildShadow(&grandChild2);
    QVERIFY(shadow.childShadow()->childShadow() != nullptr);
    QCOMPARE(shadow.childShadow()->childShadow()->xOffset(), 100);
}

void shadow_data_test::childShadowRejectsSelfReference()
{
    ShadowData shadow;
    QSignalSpy childShadowSpy(&shadow, &ShadowData::childShadowChanged);
    QSignalSpy multiShadowSpy(&shadow, &ShadowData::isMultiShadowChanged);

    shadow.setChildShadow(&shadow);

    QCOMPARE(shadow.childShadow(), nullptr);
    QCOMPARE(shadow.isMultiShadow(), false);
    QCOMPARE(childShadowSpy.count(), 0);
    QCOMPARE(multiShadowSpy.count(), 0);

    ShadowData sourceChild(1, 2, 3, 4, QColor(5, 6, 7, 8));
    shadow.setChildShadow(&sourceChild);
    QPointer<ShadowData> clonedChild = shadow.childShadow();
    QVERIFY(clonedChild != nullptr);
    QCOMPARE(childShadowSpy.count(), 1);
    QCOMPARE(multiShadowSpy.count(), 1);

    shadow.setChildShadow(&shadow);

    QCOMPARE(shadow.childShadow(), clonedChild.data());
    QVERIFY(!clonedChild.isNull());
    QCOMPARE(shadow.childShadow()->xOffset(), sourceChild.xOffset());
    QCOMPARE(shadow.isMultiShadow(), true);
    QCOMPARE(childShadowSpy.count(), 1);
    QCOMPARE(multiShadowSpy.count(), 1);
}

void shadow_data_test::childShadowRejectsMutualReference()
{
    ShadowData first;
    ShadowData second;

    first.setChildShadow(&second);
    QVERIFY(first.childShadow() != nullptr);
    QVERIFY(first.isMultiShadow());

    second.setChildShadow(&first);

    QCOMPARE(second.childShadow(), nullptr);
    QCOMPARE(second.isMultiShadow(), false);
    QVERIFY(first.childShadow() != nullptr);
    QVERIFY(first.isMultiShadow());
}

void shadow_data_test::copyConstructorAndAssignmentCloneChildShadow()
{
    ShadowData child(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData original(10, 20, 30, 40, QColor(50, 60, 70, 80), true, &child);

    ShadowData copied(original);
    ShadowData assigned;
    assigned = original;

    QCOMPARE(copied.xOffset(), original.xOffset());
    QCOMPARE(copied.yOffset(), original.yOffset());
    QCOMPARE(copied.blurRadius(), original.blurRadius());
    QCOMPARE(copied.spread(), original.spread());
    QCOMPARE(copied.shadowColor(), original.shadowColor());
    QCOMPARE(copied.isMultiShadow(), original.isMultiShadow());
    QVERIFY(copied.childShadow() != nullptr);
    QVERIFY(copied.childShadow() != original.childShadow());
    QCOMPARE(copied.childShadow()->shadowColor(), original.childShadow()->shadowColor());

    QCOMPARE(assigned.xOffset(), original.xOffset());
    QCOMPARE(assigned.yOffset(), original.yOffset());
    QCOMPARE(assigned.blurRadius(), original.blurRadius());
    QCOMPARE(assigned.spread(), original.spread());
    QCOMPARE(assigned.shadowColor(), original.shadowColor());
    QCOMPARE(assigned.isMultiShadow(), original.isMultiShadow());
    QVERIFY(assigned.childShadow() != nullptr);
    QVERIFY(assigned.childShadow() != original.childShadow());
    QCOMPARE(assigned.childShadow()->shadowColor(), original.childShadow()->shadowColor());
}

void shadow_data_test::inequalityComparesFlatAndNestedValues()
{
    ShadowData child(1, 2, 3, 4, QColor(5, 6, 7, 8));
    ShadowData first(10, 20, 30, 40, QColor(50, 60, 70, 80), true, &child);
    ShadowData second(first);

    QVERIFY(!(first != second));

    second.setSpread(41);
    QVERIFY(first != second);

    second = first;
    second.childShadow()->setYOffset(22);
    QVERIFY(first != second);
}

QTEST_GUILESS_MAIN(shadow_data_test)

#include "test-shadow-data.moc"
