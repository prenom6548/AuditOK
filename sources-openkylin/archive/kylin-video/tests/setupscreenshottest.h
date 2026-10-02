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
#ifndef SETUPSCREENSHOTTEST_H
#define SETUPSCREENSHOTTEST_H

#include <QObject>
#include <QtTest/QtTest>

class SetupScreenshot;

class SetupScreenshotTest : public QObject
{
    Q_OBJECT

public:
    explicit SetupScreenshotTest(QObject *parent = nullptr);

private slots:
    // 测试 SetupScreenshot 构造函数
    void testConstructor();
    
    // 测试 SetupScreenshot 析构函数
    void testDestructor();
    
    // 测试 initData 函数
    void testInitData();
    
    // 测试 initLayout 函数
    void testInitLayout();
    
    // 测试 initConnect 函数
    void testInitConnect();
    
    // 测试 initConnect 中的信号连接
    void testInitConnectSignals();
    
    // 测试 setBlackTheme 函数
    void testSetBlackTheme();
    
    // 测试 setDefaultTheme 函数
    void testSetDefaultTheme();
    
    // 测试 setWidgetFont 函数
    void testSetWidgetFont();
    
    // 测试 paintEvent 函数
    void testPaintEvent();
    
    // 测试未覆盖的 lambda 函数
    void testUncoveredLambdas();
    
private:
    SetupScreenshot* m_screenshot { nullptr };
};

#endif // SETUPSCREENSHOTTEST_H