/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_GLOBALSIZEDATA_H
#define TEST_GLOBALSIZEDATA_H

#include <QtTest/QtTest>
#include <QObject>

// 包含被测类（使用相对路径）
#include "../../../../../src/view/common/globalsizedata.h"

// 测试类
class TestGlobalSizeData : public QObject
{
    Q_OBJECT

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试静态常量
    void testStaticConstants();
    
    // 测试主题颜色枚举
    void testThemeColorEnum();
    
    // 测试静态成员修改
    void testStaticMemberModification();

    // 测试SDK打点函数
    void testSDKPointFunctions();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();
};

#endif // TEST_GLOBALSIZEDATA_H