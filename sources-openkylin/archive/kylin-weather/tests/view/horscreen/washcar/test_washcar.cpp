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
#include "test_washcar.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include "view/horscreen/washcar.h"

void TestWashcar::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestWashcar::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestWashcar::testConstructor()
{
    // 测试构造函数
    washcar* washcarWidget = new washcar();
    
    // 验证对象是否创建成功
    QVERIFY(washcarWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(washcarWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(washcarWidget->findChildren<QLabel*>().size(), 3);
    
    // 验证默认文本
    QList<QLabel*> labels = washcarWidget->findChildren<QLabel*>();
    bool foundWashCar = false;
    bool foundNA = false;
    for (QLabel* label : labels) {
        if (label->text() == "Wash Car") {
            foundWashCar = true;
        }
        if (label->text() == "N/A") {
            foundNA = true;
        }
    }
    QVERIFY(foundWashCar);
    QVERIFY(foundNA);
    
    delete washcarWidget;
}

void TestWashcar::testSetItemContent()
{
    // 测试setItemContent函数
    washcar* washcarWidget = new washcar();
    
    // 获取初始标签列表
    QList<QLabel*> labels = washcarWidget->findChildren<QLabel*>();
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
    washcarWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    washcarWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    // 测试特殊字符
    QString specialText = "!@#$%^&*()";
    washcarWidget->setItemContent(specialText);
    QCOMPARE(degreeLabel->text(), specialText);
    
    delete washcarWidget;
}

void TestWashcar::testIsFullScreen()
{
    // 测试isFullScreen函数
    washcar* washcarWidget = new washcar();
    
    // 测试全屏模式设置
    washcarWidget->isFullScreen(1);
    QCOMPARE(washcarWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    washcarWidget->isFullScreen(0);
    QCOMPARE(washcarWidget->m_judgeFullScreen, 0);
    
    // 测试负数值
    washcarWidget->isFullScreen(-1);
    QCOMPARE(washcarWidget->m_judgeFullScreen, -1);
    
    delete washcarWidget;
}