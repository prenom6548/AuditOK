/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "test_empty.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidget>
#include "view/horscreen/empty.h"

void TestEmpty::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestEmpty::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestEmpty::testConstructor()
{
    // 测试构造函数
    Empty* emptyWidget = new Empty();
    
    // 验证对象是否创建成功
    QVERIFY(emptyWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(emptyWidget->layout() != nullptr);
    
    // 验证控件数量（应该有一个内部控件）
    QCOMPARE(emptyWidget->findChildren<QWidget*>().size(), 1);
    
    delete emptyWidget;
}

void TestEmpty::testResizeLayout()
{
    // 测试resizeLayout函数
    Empty* emptyWidget = new Empty();
    
    // 记录初始大小
    int originalWidth = emptyWidget->width();
    int originalHeight = emptyWidget->height();
    
    // 测试小于等于1.0的情况
    emptyWidget->resizeLayout(1.0);
    // 由于这是GUI组件，我们主要验证函数能正常调用而不崩溃
    
    // 测试大于1.0的情况
    emptyWidget->resizeLayout(2.0);
    // 由于这是GUI组件，我们主要验证函数能正常调用而不崩溃
    
    // 测试边界值0
    emptyWidget->resizeLayout(0.0);
    // 由于这是GUI组件，我们主要验证函数能正常调用而不崩溃
    
    // 测试负数
    emptyWidget->resizeLayout(-1.0);
    // 由于这是GUI组件，我们主要验证函数能正常调用而不崩溃
    
    delete emptyWidget;
}