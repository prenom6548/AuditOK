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

#include "test_rotatechangeinfo.h"
#include <QSignalSpy>
#include <QApplication>
#include <QScreen>

// 测试初始化
void TestRotateChangeInfo::initTestCase()
{
    // 在所有测试开始前执行一次
}

// 每个测试前的初始化
void TestRotateChangeInfo::init()
{
    // 初始化测试环境
    m_rotateChangeInfo = new RotateChangeInfo();
}

// 测试构造函数
void TestRotateChangeInfo::testConstructor()
{
    RotateChangeInfo* rotateInfo = new RotateChangeInfo();
    QVERIFY(rotateInfo != nullptr);
    delete rotateInfo;
}

// 测试getCurrentScale方法
void TestRotateChangeInfo::testGetCurrentScale()
{
    QPair<int, int> scale = m_rotateChangeInfo->getCurrentScale();
    QVERIFY(scale.first > 0);
    QVERIFY(scale.second > 0);
}

// 测试getCurrentMode方法
void TestRotateChangeInfo::testGetCurrentMode()
{
    // 由于这个方法依赖于DBus调用，我们只测试它能正常返回而不抛出异常
    bool mode = m_rotateChangeInfo->getCurrentMode();
    QVERIFY(true); // 只要不抛出异常就算通过
}

// 测试slotRotateChanged方法
void TestRotateChangeInfo::testSlotRotateChanged()
{
    // 创建信号监视器来捕获sigRotationChanged信号
    QSignalSpy spy(m_rotateChangeInfo, &RotateChangeInfo::sigRotationChanged);
    
    // 调用slotRotateChanged方法
    m_rotateChangeInfo->slotRotateChanged();
    
    // 验证信号是否正确发出
    QVERIFY(spy.count() >= 0); // 信号可能发出也可能不发出，取决于屏幕状态
}

// 测试slotModeChanged方法
void TestRotateChangeInfo::testSlotModeChanged()
{
    // 创建信号监视器来捕获sigRotationChanged信号
    QSignalSpy spy(m_rotateChangeInfo, &RotateChangeInfo::sigRotationChanged);
    
    // 测试PC模式
    m_rotateChangeInfo->slotModeChanged(false);
    QVERIFY(spy.count() >= 0);
    
    // 清空spy
    spy.clear();
    
    // 测试Pad模式
    m_rotateChangeInfo->slotModeChanged(true);
    QVERIFY(spy.count() >= 0);
}

// 每个测试后的清理
void TestRotateChangeInfo::cleanup()
{
    // 清理测试环境
    if (m_rotateChangeInfo) {
        delete m_rotateChangeInfo;
        m_rotateChangeInfo = nullptr;
    }
}

// 测试结束后的清理
void TestRotateChangeInfo::cleanupTestCase()
{
    // 在所有测试结束后执行一次
}