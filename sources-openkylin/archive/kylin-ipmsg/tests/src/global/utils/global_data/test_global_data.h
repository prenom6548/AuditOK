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

#ifndef TEST_GLOBAL_DATA_H
#define TEST_GLOBAL_DATA_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QScreen>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingReply>
#include <QString>
#include <QFont>

// 包含被测类的头文件
#include "../../../../../../src/global/utils/global_data.h"

// 测试类
class TestGlobalData : public QObject
{
    Q_OBJECT

public:
    explicit TestGlobalData(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数和单例模式
    void testSingleton();
    
    // 测试初始化方法
    void testInit();
    
    // 测试获取TCP监听IP
    void testGetTcpListenIp();
    
    // 测试获取TCP监听端口
    void testGetTcpListenPort();
    
    // 测试获取UDP监听端口
    void testGetUdpListenPort();
    
    // 测试获取UUID
    void testGetUuid();
    
    // 测试获取系统主题
    void testGetSystemTheme();
    
    // 测试获取系统字体大小
    void testGetSystemFontSize();
    
    // 测试获取系统透明度
    void testGetSystemTransparency();
    
    // 测试获取12px字体
    void testGetFontSize12px();
    
    // 测试获取14px字体
    void testGetFontSize14px();
    
    // 测试获取16px字体
    void testGetFontSize16px();
    
    // 测试获取18px字体
    void testGetFontSize18px();
    
    // 测试获取字体名称
    void testGetFontName();
    
    // 测试获取DBus数据
    void testGetDbusData();
    
    // 测试获取当前模式
    void testGetCurrentMode();
    
    // 新增覆盖测试
    void testCoverageAdditions();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    GlobalData* m_globalData;
};

#endif // TEST_GLOBAL_DATA_H