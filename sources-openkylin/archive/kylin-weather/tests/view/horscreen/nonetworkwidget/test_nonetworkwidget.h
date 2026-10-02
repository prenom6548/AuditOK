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
#ifndef TEST_NONETWORKWIDGET_H
#define TEST_NONETWORKWIDGET_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/horscreen/nonetworkwidget.h"

class TestNoNetworkWidget : public QObject
{
    Q_OBJECT

public:
    TestNoNetworkWidget();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试界面初始化
    void testInitUI();
    
    // 测试连接初始化
    void testInitConnect();
    
    // 测试绘图事件
    void testPaintEvent();
    
    // 测试设置按钮点击事件
    void testOnSettingClicked();

private:
    NoNetworkWidget *m_noNetworkWidget;
};

#endif // TEST_NONETWORKWIDGET_H