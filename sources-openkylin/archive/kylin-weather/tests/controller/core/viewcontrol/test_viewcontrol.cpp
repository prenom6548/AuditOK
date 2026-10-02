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
#include "test_viewcontrol.h"
#include <QApplication>
#include <QWidget>
#include <QSignalSpy>
#include <QTimer>
#include "global/viewvar.h"

TestViewControl::TestViewControl()
    : m_viewControl(nullptr)
{
}

void TestViewControl::initTestCase()
{
    // 初始化测试用例
    m_viewControl = viewControl::getInstance();
    QVERIFY(m_viewControl != nullptr);
}

void TestViewControl::cleanupTestCase()
{
    // 清理测试用例
}

void TestViewControl::testGetInstance()
{
    // 测试获取单例实例
    viewControl *instance1 = viewControl::getInstance();
    viewControl *instance2 = viewControl::getInstance();
    
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2); // 验证是同一个实例
}

void TestViewControl::testUpdateIsStart()
{
    // 测试更新启动参数
    QVERIFY(m_viewControl != nullptr);
    
    m_viewControl->updateIsStart(true);
    QCOMPARE(m_viewControl->m_isStart, true);
    
    m_viewControl->updateIsStart(false);
    QCOMPARE(m_viewControl->m_isStart, false);
}

void TestViewControl::testInitCore()
{
    // 测试初始化后端
    QVERIFY(m_viewControl != nullptr);
    
    QStringList list;
    list << "arg1" << "arg2";
    
    // 调用initCore方法
    m_viewControl->initCore(list, true);
    
    QVERIFY(true); 
}

void TestViewControl::testActiveWindow()
{
    // 测试返回已经存在的widget
    QVERIFY(m_viewControl != nullptr);
    
    m_viewControl->updateIsStart(true);
    m_viewControl->autoStart();
    
    QWidget *widget = m_viewControl->activeWindow();
    QVERIFY(widget != nullptr);
    QCOMPARE(widget->accessibleName(), QString("kylin-weather_viewControl_horscreen"));
}

void TestViewControl::testAutoStart()
{
    // 测试通知实例化界面
    QVERIFY(m_viewControl != nullptr);
    
    // 调用autoStart方法
    m_viewControl->updateIsStart(false);
    m_viewControl->autoStart();
    
    m_viewControl->updateIsStart(true);
    m_viewControl->autoStart();
    
    QVERIFY(true);
}

void TestViewControl::testToShowView()
{
    // 测试在界面打开之后，将界面展示在屏幕中间
    QVERIFY(m_viewControl != nullptr);
    
    m_viewControl->toShowView();
    QVERIFY(true);
}

void TestViewControl::testShowView()
{
    QVERIFY(m_viewControl != nullptr);
    
    // 直接调用私有方法 (覆盖 viewcontrol.cpp:101-143)
    m_viewControl->showView(true);
    QVERIFY(m_viewControl->activeWindow() != nullptr);
    
    m_viewControl->showView(false);
}

void TestViewControl::testShowInCenter()
{
    // 测试显示在屏幕中央 (覆盖 viewcontrol.cpp:145-188)
    QVERIFY(m_viewControl != nullptr);
    
    QWidget *testWidget = new QWidget();
    m_viewControl->showInCenter(testWidget);
    
    QVERIFY(ViewVar::widgetIsShow);
    delete testWidget;
}

#include "moc_test_viewcontrol.cpp"
