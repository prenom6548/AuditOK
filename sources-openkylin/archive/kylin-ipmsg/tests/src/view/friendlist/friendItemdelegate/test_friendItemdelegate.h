/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_FRIENDITEMDELEGATE_H
#define TEST_FRIENDITEMDELEGATE_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QModelIndex>
#include <QStandardItemModel>

// 包含被测类的头文件
#include "../../../../../../src/view/friendlist/friendItemdelegate.h"

// 测试类
class TestFriendItemDelegate : public QObject
{
    Q_OBJECT

public:
    explicit TestFriendItemDelegate(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数和析构函数
    void testConstructorAndDestructor();
    
    // 测试sizeHint方法
    void testSizeHint();
    
    // 测试paint方法
    void testPaint();
    
    // 测试paint方法 - 鼠标悬停状态
    void testPaintMouseOver();
    
    // 测试paint方法 - 选中状态
    void testPaintHasFocus();
    
    // 测试paint方法 - 置顶状态
    void testPaintStayTop();
    
    // 测试paint方法 - 离线状态
    void testPaintOffline();
    
    // 测试paint方法 - 有未读消息
    void testPaintWithUnreadMsg();
    
    // 测试changeHighLightColor方法
    void testChangeHighLightColor();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    FriendItemDelegate* m_friendItemDelegate;
    QStandardItemModel* m_model;
};

#endif // TEST_FRIENDITEMDELEGATE_H