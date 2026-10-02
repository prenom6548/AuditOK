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

#ifndef TEST_ROTATECHANGEINFO_H
#define TEST_ROTATECHANGEINFO_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QScreen>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingReply>

// 包含被测类的头文件
#include "../../../../../../src/global/utils/rotatechangeinfo.h"

// 测试类
class TestRotateChangeInfo : public QObject
{
    Q_OBJECT

public:
    explicit TestRotateChangeInfo(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试getCurrentScale方法
    void testGetCurrentScale();
    
    // 测试getCurrentMode方法
    void testGetCurrentMode();
    
    // 测试slotRotateChanged方法
    void testSlotRotateChanged();
    
    // 测试slotModeChanged方法
    void testSlotModeChanged();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    RotateChangeInfo* m_rotateChangeInfo;
};

#endif // TEST_ROTATECHANGEINFO_H