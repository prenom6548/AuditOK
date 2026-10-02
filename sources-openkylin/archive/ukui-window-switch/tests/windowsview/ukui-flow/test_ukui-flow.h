/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TEST_UKUI_FLOW_H
#define TEST_UKUI_FLOW_H

#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>

class UkuiFlow;

class TestUkuiFlow : public QObject
{
    Q_OBJECT
public:
    explicit TestUkuiFlow(QObject *parent = nullptr);
    ~TestUkuiFlow();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test constructors
    void test_constructor();
    void test_constructorWithParent();

    // Test UkuiBasePositioner PositionedItem
    void testConstructorInitialization();
    void testUpdatePadding();
    void testItemXYWithoutTransition();
    void testMoveToWithoutTransition();
    void testPositionedItem();
    void testPositionedItemTransition();

    // Test UkuiFlow properties (combined)
    void testFlow();
    void testLayoutDirection();
    void testLineHorizontalCenterAlignment();

    // Test UkuiBasePositioner properties (combined)
    void testSpacing();
    void testTransitions();

    // Test padding properties (combined)
    void testPadding();

    // Test signals (merged)
    void testSignals();

    // Test UkuiPositionerAttached (combined)
    void testAttachedProperties();

    // Test boundary conditions
    void test_boundaryConditionsSpacing();
    void test_boundaryConditionsPadding();
    void test_boundaryConditionsLayoutDirection();

    // Test default values
    void test_defaultValues();
    void test_defaultValuesUkuiPositionerAttached();

    // Test forceLayout
    void test_forceLayout();

    // Test function
    void testComponentComplete();
    void testItemChange();
    void testAttachedPropertiesUpdate();
    void testMoveCurrentIndex();

    void testUkuiPositionerAttachedIsLastItem();
    void testPositionedItemDestructor();
    
    // Test UkuiFlow directions (merged)
    void testUkuiFlowDirections();
    
    // Test transitions (merged)
    void testUkuiFlowTransitions();
    
    void testPrePositioningWithUnpositionedItems();
    
    // Test same value setters (merged)
    void testSameValueSetters();
    
    void testIsLeftToRightVertical();
    void testGetEffectiveLayoutDirectionWithMirror();
    
    // Additional coverage tests - merged
    void testPrePositioningCases();
    void testDoPositioningCases();
    void testPositionedItemCases();
    void testPaddingCases();
    void testSpacingCases();
    void testTransitionCases();
    void testDestructorCases();
    void testAttachedPropertyCases();
    
    // New test functions for increased coverage (merged)
    void testItemChangeCases();
    void testPaddingSameValueCases();
    void testDoPositioningCases2();
    void testPrePositioningCases2();
    void testResetPaddingCases();
    void testMoveCurrentIndexCases();
    void testPositionedItemCases2();
    
    // Additional branch coverage tests
    void testPrePositioningNotComplete();
    void testPrePositioningDoingPositioning();
    void testPrePositioningWithTransitioner();
    void testPrePositioningWithAnchorConflict();
    void testPositionItemBranches();
    void testPositionItemXBranches();
    void testPositionItemYBranches();
    void testUpdateAttachedPropertiesWithSpecificProperty();
    void testUpdateAttachedPropertiesWithNullItem();
    void testDoPositioningWithLineHorizontalCenterAlignment();
    void testDoPositioningTopToBottom();
    void testDoPositioningRightToLeft();
    void testReportConflictingAnchors();
    void testRemoveAndClearPositionedItems();
    void testPositionedItemFind();
    void testSetPaddingWithExtraData();
    void testPrePositioningWithTransparentItem();
    void testMoveCurrentIndexUpSingleRow();
    void testMoveCurrentIndexDownSingleRow();
    void testFindUpDownItemIndexEdgeCases();
    void testMoveCurrentIndexLeftAtBoundary();
    void testMoveCurrentIndexRightAtBoundary();
    
    // Additional branch coverage tests - part 2
    void testUkuiBasePositionerConstructor();
    void testSetPopulateNewTransitioner();
    void testSetPopulateSameValue();
    void testSetMoveNewTransitioner();
    void testSetMoveSameValue();
    void testSetAddNewTransitioner();
    void testSetAddSameValue();
    void testUpdatePolish();
    void testComponentCompleteWithTransitioner();
    void testItemChangeChildAdded();
    void testItemChangeVisibleHasChanged();
    void testPrePositioningWithTransitionerAndItems();
    void testDoPositioningWithItems();
    void testDoPositioningWithTopToBottomAndHeightValid();
    void testDoPositioningLastLineCenterAlignment();
    void testPositionedItemDestructorWithTransitionableItem();
    void testExtraDataPaddingCases();
    void testExtraDataPaddingResetCases();
    void testPositionedItemMultipleTransitions();
    void testFlowSetSameValue();
    void testLayoutDirectionSetSameValue();
    void testLineHorizontalCenterAlignmentSetSameValue();
    void testMoveCurrentIndexUpAtFirstRow();
    void testMoveCurrentIndexDownAtLastRow();
    
    // Tests for prePositioning branches - merged
    void testPrePositioningComprehensive();
    void testPrePositioningWithTransitionerComprehensive();
    void testDoPositioningWithDifferentFlows();
    void testDoPositioningWithCenterAlignment();
    void testQmlComprehensive();
    void testPositionedItemTransitions();
    void testAttachedPropertyUpdate();
    void testItemChangeWithUnpositionedItems();
    void testPrePositioningWithAddedIndex();
    void testPositionerAttachedCreation();
    void testDoPositioningWithRightToLeftAndCenterAlignment();
    void testPrePositioningWithDoingPositioning();

private:
    QQuickView *m_view;
    UkuiFlow *m_flowItem;
};

#endif // TEST_UKUI_FLOW_H
