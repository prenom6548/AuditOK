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
#include "test_sport.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include "view/horscreen/sport.h"

void TestSport::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestSport::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestSport::testConstructor()
{
    // 测试构造函数
    sport* sportWidget = new sport();
    
    // 验证对象是否创建成功
    QVERIFY(sportWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(sportWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有3个标签）
    QCOMPARE(sportWidget->findChildren<QLabel*>().size(), 3);
    
    // 验证默认文本
    QList<QLabel*> labels = sportWidget->findChildren<QLabel*>();
    bool foundNA = false;
    for (QLabel* label : labels) {
        if (label->text() == "N/A") {
            foundNA = true;
            break;
        }
    }
    QVERIFY(foundNA);
    
    delete sportWidget;
}

void TestSport::testSetItemContent()
{
    // 测试setItemContent函数
    sport* sportWidget = new sport();
    
    // 获取初始标签列表
    QList<QLabel*> labels = sportWidget->findChildren<QLabel*>();
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
    sportWidget->setItemContent(newText);
    
    // 验证内容是否正确设置
    QCOMPARE(degreeLabel->text(), newText);
    
    // 测试空字符串
    sportWidget->setItemContent("");
    QCOMPARE(degreeLabel->text(), QString(""));
    
    delete sportWidget;
}

void TestSport::testIsFullScreen()
{
    // 测试isFullScreen函数
    sport* sportWidget = new sport();
    
    // 测试全屏模式设置
    sportWidget->isFullScreen(1);
    // 注意：m_judgeFullScreen是私有成员，无法直接访问验证
    
    // 测试非全屏模式设置
    sportWidget->isFullScreen(0);
    // 注意：m_judgeFullScreen是私有成员，无法直接访问验证
    
    delete sportWidget;
}

void TestSport::testLable2SetFontSizeSlot()
{
    // lable2SetFontSizeSlot是私有函数，无法直接测试
    // 但可以通过间接方式验证其功能
    sport* sportWidget = new sport();
    
    // 验证对象创建成功
    QVERIFY(sportWidget != nullptr);
    
    delete sportWidget;
}
