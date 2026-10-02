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
#include "test_xatomhelper.h"
#include "view/horscreen/xatom-helper.h"

#include <QX11Info>
#include <X11/Xlib.h>

void TestXAtomHelper::initTestCase()
{
    // 初始化测试用例
}

void TestXAtomHelper::cleanupTestCase()
{
    // 清理测试用例
}

void TestXAtomHelper::testGetInstance()
{
    // 测试获取单例实例
    XAtomHelper* instance1 = XAtomHelper::getInstance();
    XAtomHelper* instance2 = XAtomHelper::getInstance();
    
    QVERIFY(instance1 != nullptr);
    QCOMPARE(instance1, instance2); // 验证是否为同一实例
}

void TestXAtomHelper::testIsFrameLessWindow()
{
    // 测试是否为无边框窗口
    // 使用无效的winId进行测试
    bool result = XAtomHelper::isFrameLessWindow(0);
    QCOMPARE(result, false);
}

void TestXAtomHelper::testIsWindowDecorateBorderOnly()
{
    // 测试窗口是否只有装饰边框
    bool result = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(0);
    QCOMPARE(result, false);
}

void TestXAtomHelper::testIsWindowMotifHintDecorateBorderOnly()
{
    // 测试Motif提示是否只有装饰边框
    MotifWmHints hint;
    hint.flags = 0;
    hint.decorations = 0;
    
    bool result = XAtomHelper::getInstance()->isWindowMotifHintDecorateBorderOnly(hint);
    QCOMPARE(result, false);
    
    // 测试另一种情况：flags包含MWM_HINTS_DECORATIONS但不等于MWM_HINTS_DECORATIONS
    hint.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
    hint.decorations = MWM_DECOR_BORDER;
    
    result = XAtomHelper::getInstance()->isWindowMotifHintDecorateBorderOnly(hint);
    QCOMPARE(result, true);
    
    // 测试flags等于MWM_HINTS_DECORATIONS的情况，应该返回false
    hint.flags = MWM_HINTS_DECORATIONS;
    hint.decorations = MWM_DECOR_BORDER;
    
    result = XAtomHelper::getInstance()->isWindowMotifHintDecorateBorderOnly(hint);
    QCOMPARE(result, false);
}

void TestXAtomHelper::testIsUKUICsdSupported()
{
    // 测试UKUI CSD支持
    bool result = XAtomHelper::getInstance()->isUKUICsdSupported();
    QCOMPARE(result, false); // 根据实现，当前固定返回false
}

void TestXAtomHelper::testIsUKUIDecorationWindow()
{
    // 测试是否为UKUI装饰窗口
    bool result = XAtomHelper::getInstance()->isUKUIDecorationWindow(0);
    QCOMPARE(result, false);
}

void TestXAtomHelper::testGetWindowBorderRadius()
{
    // 测试获取窗口边框半径
    UnityCorners corners = XAtomHelper::getInstance()->getWindowBorderRadius(0);
    
    // 默认值应为0
    QCOMPARE(corners.topLeft, 0ul);
    QCOMPARE(corners.topRight, 0ul);
    QCOMPARE(corners.bottomLeft, 0ul);
    QCOMPARE(corners.bottomRight, 0ul);
}

void TestXAtomHelper::testSetWindowBorderRadiusWithStruct()
{
    // 测试使用结构体设置窗口边框半径
    UnityCorners corners;
    corners.topLeft = 10;
    corners.topRight = 20;
    corners.bottomLeft = 30;
    corners.bottomRight = 40;
    
    // 注意：由于我们无法创建真实的X11窗口，这里主要测试代码路径
    // 实际的X11调用会失败，但我们仍可以测试代码逻辑
    XAtomHelper::getInstance()->setWindowBorderRadius(0, corners);
    
    // 验证代码能正常执行到这里
    QVERIFY(true);
}

void TestXAtomHelper::testSetWindowBorderRadiusWithParams()
{
    // 测试使用参数设置窗口边框半径
    // 注意：由于我们无法创建真实的X11窗口，这里主要测试代码路径
    XAtomHelper::getInstance()->setWindowBorderRadius(0, 10, 20, 30, 40);
    
    // 验证代码能正常执行到这里
    QVERIFY(true);
}

void TestXAtomHelper::testSetUKUIDecoraiontHint()
{
    // 测试设置UKUI装饰提示
    // 注意：由于我们无法创建真实的X11窗口，这里主要测试代码路径
    XAtomHelper::getInstance()->setUKUIDecoraiontHint(0, true);
    
    // 验证代码能正常执行到这里
    QVERIFY(true);
}

void TestXAtomHelper::testSetWindowMotifHint()
{
    // 测试设置窗口Motif提示
    MotifWmHints hints;
    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = MWM_DECOR_BORDER;
    
    // 注意：由于我们无法创建真实的X11窗口，这里主要测试代码路径
    XAtomHelper::getInstance()->setWindowMotifHint(0, hints);
    
    // 验证代码能正常执行到这里
    QVERIFY(true);
}

void TestXAtomHelper::testGetWindowMotifHint()
{
    // 测试获取窗口Motif提示
    MotifWmHints hints = XAtomHelper::getInstance()->getWindowMotifHint(0);
    
    // 默认值应为0
    QCOMPARE(hints.flags, 0ul);
    QCOMPARE(hints.functions, 0ul);
    QCOMPARE(hints.decorations, 0ul);
    QCOMPARE(hints.input_mode, 0l);
    QCOMPARE(hints.status, 0ul);
}
