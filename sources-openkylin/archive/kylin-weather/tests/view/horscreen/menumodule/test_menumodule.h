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
#ifndef TEST_MENUMODULE_H
#define TEST_MENUMODULE_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/horscreen/menumodule.h"

class TestMenuModule : public QObject
{
    Q_OBJECT

public:
    TestMenuModule();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();

    // 测试设置允许定位
    void testSetMenuModuleAllowLocationYes();

    // 测试主题变更监控
    void testMonitorThemeChanged_data();
    void testMonitorThemeChanged();

    // 测试信号连接
    void testSignals();

    // 测试菜单触发
    void testTriggerMenu();

    // 测试获取版本号
    void testGetVersion();

    // 测试帮助和关于动作
    void testHelpAction();
    void testAboutAction();

    // 增加：测试私有逻辑方法
    void testExecuteCMD();
    void testInitAbout();
    void testInternalState();

private:
    menuModule* m_menuModule;
};

#endif // TEST_MENUMODULE_H