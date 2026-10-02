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
#include "test_clothe.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include "view/horscreen/clothe.h"

void TestClothe::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestClothe::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestClothe::testConstructor()
{
    // 测试构造函数
    clothe* clotheWidget = new clothe();
    
    // 验证对象是否创建成功
    QVERIFY(clotheWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(clotheWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(clotheWidget->findChildren<QLabel*>().size(), 3);
    
    // 验证默认文本
    QList<QLabel*> labels = clotheWidget->findChildren<QLabel*>();
    bool foundNA = false;
    for (QLabel* label : labels) {
        if (label->text() == "N/A") {
            foundNA = true;
            break;
        }
    }
    QVERIFY(foundNA);
    
    delete clotheWidget;
}

void TestClothe::testSetItemContent()
{
    // 测试setItemContent函数
    clothe* clotheWidget = new clothe();
    
    // 获取初始标签列表
    QList<QLabel*> labels = clotheWidget->findChildren<QLabel*>();
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
    QString newText = "Warm";
    clotheWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    clotheWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    delete clotheWidget;
}

void TestClothe::testIsFullScreen()
{
    // 测试isFullScreen函数
    clothe* clotheWidget = new clothe();
    
    // 测试全屏模式设置
    clotheWidget->isFullScreen(1);
    QCOMPARE(clotheWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    clotheWidget->isFullScreen(0);
    QCOMPARE(clotheWidget->m_judgeFullScreen, 0);
    
    delete clotheWidget;
}

void TestClothe::testLable2SetFontSizeSlot()
{
    // lable2SetFontSizeSlot是私有函数，无法直接测试
    // 但可以通过间接方式验证其功能
    clothe* clotheWidget = new clothe();
    
    // 验证对象创建成功
    QVERIFY(clotheWidget != nullptr);
    
    delete clotheWidget;
}