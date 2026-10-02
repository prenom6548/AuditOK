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
#include "test_ultravioletrays.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include "view/horscreen/ultravioletrays.h"

void TestUltravioletrays::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestUltravioletrays::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestUltravioletrays::testConstructor()
{
    // 测试构造函数
    ultravioletrays* uvWidget = new ultravioletrays();
    
    // 验证对象是否创建成功
    QVERIFY(uvWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(uvWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(uvWidget->findChildren<QLabel*>().size(), 3);
    
    // 验证默认文本
    QList<QLabel*> labels = uvWidget->findChildren<QLabel*>();
    bool foundUV = false;
    bool foundNA = false;
    for (QLabel* label : labels) {
        if (label->text() == "UV") {
            foundUV = true;
        }
        if (label->text() == "N/A") {
            foundNA = true;
        }
    }
    QVERIFY(foundUV);
    QVERIFY(foundNA);
    
    delete uvWidget;
}

void TestUltravioletrays::testSetItemContent()
{
    // 测试setItemContent函数
    ultravioletrays* uvWidget = new ultravioletrays();
    
    // 获取初始标签列表
    QList<QLabel*> labels = uvWidget->findChildren<QLabel*>();
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
    QString newText = "High";
    uvWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    uvWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    // 测试长字符串
    QString longText = "Very High UV Index";
    uvWidget->setItemContent(longText);
    QCOMPARE(degreeLabel->text(), longText);
    
    delete uvWidget;
}

void TestUltravioletrays::testIsFullScreen()
{
    // 测试isFullScreen函数
    ultravioletrays* uvWidget = new ultravioletrays();
    
    // 测试全屏模式设置
    uvWidget->isFullScreen(1);
    QCOMPARE(uvWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    uvWidget->isFullScreen(0);
    QCOMPARE(uvWidget->m_judgeFullScreen, 0);
    
    delete uvWidget;
}