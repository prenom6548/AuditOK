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
#include "test_air.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include "view/horscreen/air.h"

void TestAir::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestAir::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestAir::testConstructor()
{
    // 测试构造函数
    air* airWidget = new air();
    
    // 验证对象是否创建成功
    QVERIFY(airWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(airWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(airWidget->findChildren<QLabel*>().size(), 3);
    
    delete airWidget;
}

void TestAir::testSetItemContent()
{
    // 测试setItemContent函数
    air* airWidget = new air();
    
    // 获取初始标签列表
    QList<QLabel*> labels = airWidget->findChildren<QLabel*>();
    QLabel* degreeLabel = nullptr;
    
    // 查找显示"N/A"的标签（应该是degree标签）
    for (QLabel* label : labels) {
        if (label->text() == "N/A") {
            degreeLabel = label;
            break;
        }
    }
    
    QVERIFY(degreeLabel != nullptr);
    
    // 保存原始文本
    QString originalText = degreeLabel->text();
    
    // 设置新内容
    QString newText = "Good";
    airWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    airWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    delete airWidget;
}

void TestAir::testIsFullScreen()
{
    // 测试isFullScreen函数
    air* airWidget = new air();
    
    // 测试全屏模式设置
    airWidget->isFullScreen(1);
    QCOMPARE(airWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    airWidget->isFullScreen(0);
    QCOMPARE(airWidget->m_judgeFullScreen, 0);
    
    delete airWidget;
}