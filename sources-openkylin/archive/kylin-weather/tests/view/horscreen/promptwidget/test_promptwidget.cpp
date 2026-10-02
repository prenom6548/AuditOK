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
#include "test_promptwidget.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include "view/horscreen/promptwidget.h"

void TestPromptWidget::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestPromptWidget::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestPromptWidget::testConstructor()
{
    // 测试构造函数
    PromptWidget* promptWidget = new PromptWidget();
    
    // 验证对象是否创建成功
    QVERIFY(promptWidget != nullptr);
    
    // 验证初始状态
    QCOMPARE(promptWidget->m_judgeFullScreen, 0);
    
    // 验证对象能正常创建和销毁
    delete promptWidget;
}

void TestPromptWidget::testInitLayout()
{
    // 测试initLayout函数
    PromptWidget* promptWidget = new PromptWidget();
    
    // 调用initLayout
    QString testError = "Network connection error";
    promptWidget->initLayout(testError, 0);
    
    // 验证函数能正常执行（不会崩溃）
    // 由于UI组件是私有的，我们主要验证函数能正常调用
    
    // 测试另一种情况
    QString anotherError = "Server timeout";
    promptWidget->initLayout(anotherError, 1);
    
    delete promptWidget;
}

void TestPromptWidget::testIsFullScreen()
{
    // 测试isFullScreen函数
    PromptWidget* promptWidget = new PromptWidget();
    
    // 验证初始状态
    QCOMPARE(promptWidget->m_judgeFullScreen, 0);
    
    // 调用isFullScreen
    promptWidget->isFullScreen(1);
    
    // 验证状态是否正确更新
    QCOMPARE(promptWidget->m_judgeFullScreen, 1);
    
    // 测试另一种情况
    promptWidget->isFullScreen(0);
    QCOMPARE(promptWidget->m_judgeFullScreen, 0);
    
    delete promptWidget;
}