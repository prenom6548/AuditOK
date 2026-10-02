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
#include "test_flu.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include "view/horscreen/flu.h"

void TestFlu::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestFlu::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestFlu::testConstructor()
{
    // 测试构造函数
    flu* fluWidget = new flu();
    
    // 验证对象是否创建成功
    QVERIFY(fluWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(fluWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(fluWidget->findChildren<QLabel*>().size(), 3);
    
    // 验证默认文本
    QList<QLabel*> labels = fluWidget->findChildren<QLabel*>();
    bool foundNA = false;
    bool foundFlu = false;
    for (QLabel* label : labels) {
        if (label->text() == "N/A") {
            foundNA = true;
        }
        if (label->text() == "Flu") {
            foundFlu = true;
        }
    }
    QVERIFY(foundNA);
    QVERIFY(foundFlu);
    
    delete fluWidget;
}

void TestFlu::testSetItemContent()
{
    // 测试setItemContent函数
    flu* fluWidget = new flu();
    
    // 获取初始标签列表
    QList<QLabel*> labels = fluWidget->findChildren<QLabel*>();
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
    QString newText = "Low Risk";
    fluWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    fluWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    delete fluWidget;
}

void TestFlu::testIsFullScreen()
{
    // 测试isFullScreen函数
    flu* fluWidget = new flu();
    
    // 测试全屏模式设置
    fluWidget->isFullScreen(1);
    QCOMPARE(fluWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    fluWidget->isFullScreen(0);
    QCOMPARE(fluWidget->m_judgeFullScreen, 0);
    
    delete fluWidget;
}