/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "../../biometric-manager/src/mytreeview.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QModelIndex>
#include <QDebug>
#include <QToolTip>


class MyTreeViewTest : public ::testing::Test {
protected:
    MyTreeView* treeView;


    void SetUp() override {
        treeView = new MyTreeView();
    }


    void TearDown() override {
        delete treeView;
    }
};


// 测试构造函数
TEST_F(MyTreeViewTest, ConstructorTest) {
    EXPECT_TRUE(treeView->eventFilter(treeView, nullptr));
    EXPECT_TRUE(treeView->testAttribute(Qt::WA_Hover));
}


// 测试 event 函数
TEST_F(MyTreeViewTest, EventTest) {
    // 测试 QEvent::ToolTip 事件
    QEvent::Type tooltipEventType = QEvent::ToolTip;
    QEvent tooltipEvent(tooltipEventType);
    QPoint cursorPos(100, 100);
    QCursor::setPos(cursorPos);
    QPoint windowPoint = treeView->mapFromGlobal(cursorPos);
    QModelIndex mockIndex;
    mockIndex = treeView->indexAt(QPoint(windowPoint.x(), windowPoint.y() - 40));
//    QString mockData = "Test Data";
//    mockIndex.model()->setData(mockIndex, mockData);


    // 模拟 ToolTip 事件
    bool result = treeView->event(&tooltipEvent);
    EXPECT_TRUE(result);
    EXPECT_TRUE(QToolTip::isVisible());
//    EXPECT_EQ(QToolTip::text(), mockData);


    // 测试 QEvent::HoverMove 事件
    QEvent::Type hoverMoveEventType = QEvent::HoverMove;
    QEvent hoverMoveEvent(hoverMoveEventType);
    QToolTip::showText(cursorPos, "testText");


    // 模拟 HoverMove 事件
    result = treeView->event(&hoverMoveEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(QToolTip::isVisible());


    // 测试其他事件
    QEvent::Type otherEventType = QEvent::KeyPress;
    QEvent otherEvent(otherEventType);


    // 模拟其他事件
    result = treeView->event(&otherEvent);
    EXPECT_TRUE(result);
}


// 测试 mousePressEvent 函数
TEST_F(MyTreeViewTest, MousePressEventTest) {
//    QSignalSpy spy(treeView, SIGNAL(rightClicked(QModelIndex)));


    // 模拟右键点击
    QMouseEvent rightClickEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    treeView->mousePressEvent(&rightClickEvent);


    // 检查信号是否被发射
//    EXPECT_EQ(spy.count(), 1);


    // 模拟右键点击无效的 QModelIndex
    QMouseEvent invalidRightClickEvent(QEvent::MouseButtonPress, QPoint(-1, -1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    treeView->mousePressEvent(&invalidRightClickEvent);


    // 检查信号是否未被发射
//    EXPECT_EQ(spy.count(), 1);
}
