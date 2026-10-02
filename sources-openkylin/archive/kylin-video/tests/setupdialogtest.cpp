/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "setupdialogtest.h"
#include "widget/setup/setupdialog.h"
#include <QPaintEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QApplication>
#include <iostream>

SetUpDialogTest::SetUpDialogTest(QObject *parent) : QObject(parent)
{
    // 初始化成员变量为nullptr
    m_dialog = nullptr;
}

void SetUpDialogTest::testHideEvent()
{
    std::cout << "=== Testing HideEvent ===" << std::endl;
    
    // 创建SetUpDialog实例
    m_dialog = new SetUpDialog();
    
    // 创建QHideEvent事件
    QHideEvent hideEvent;
    
    // 调用hideEvent函数
    m_dialog->hideEvent(&hideEvent);
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    // 清理资源
    delete m_dialog;
    m_dialog = nullptr;
    
    std::cout << "HideEvent test completed." << std::endl;
}

void SetUpDialogTest::testShowEvent()
{
    std::cout << "=== Testing ShowEvent ===" << std::endl;
    
    // 创建SetUpDialog实例
    m_dialog = new SetUpDialog();
    
    // 创建QShowEvent事件
    QShowEvent showEvent;
    
    // 调用showEvent函数
    m_dialog->showEvent(&showEvent);
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    // 清理资源
    delete m_dialog;
    m_dialog = nullptr;
    
    std::cout << "ShowEvent test completed." << std::endl;
}

void SetUpDialogTest::testPaintEvent()
{
    std::cout << "=== Testing PaintEvent ===" << std::endl;
    
    // 创建SetUpDialog实例
    m_dialog = new SetUpDialog();
    
    // 创建QPaintEvent事件
    QPaintEvent paintEvent(m_dialog->rect());
    
    // 调用paintEvent函数
    m_dialog->paintEvent(&paintEvent);
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    // 清理资源
    delete m_dialog;
    m_dialog = nullptr;
    
    std::cout << "PaintEvent test completed." << std::endl;
}

void SetUpDialogTest::testEvent()
{
    std::cout << "=== Testing Event ===" << std::endl;
    
    // 创建SetUpDialog实例
    m_dialog = new SetUpDialog();
    
    // 创建一个普通事件
    QEvent event(QEvent::None);
    
    // 调用event函数
    bool result = m_dialog->event(&event);
    
    // 验证函数返回值
    QVERIFY(result == true);
    
    // 清理资源
    delete m_dialog;
    m_dialog = nullptr;
    
    std::cout << "Event test completed." << std::endl;
}