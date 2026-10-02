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
#include "test_hotcitybtn.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "view/search/hotcitybtn.h"

TestHotcityBtn::TestHotcityBtn(QObject *parent)
    : QObject(parent)
    , m_hotcityBtn(nullptr)
{
}

void TestHotcityBtn::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestHotcityBtn::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestHotcityBtn::init()
{
    // 在每个测试之前执行的初始化代码
    m_hotcityBtn = nullptr;
}

void TestHotcityBtn::cleanup()
{
    // 在每个测试之后执行的清理代码
    if (m_hotcityBtn) {
        delete m_hotcityBtn;
        m_hotcityBtn = nullptr;
    }
}

void TestHotcityBtn::testConstructor()
{
    // 测试构造函数
    QString cityName = "北京";
    QString cityCode = "101010100";
    
    m_hotcityBtn = new HotcityBtn(nullptr, cityName, cityCode);
    QVERIFY(m_hotcityBtn != nullptr);
    
    // 验证按钮文本
    QCOMPARE(m_hotcityBtn->text(), cityName);
    
    // 验证按钮大小是否设置正确（通过ViewVar::HOT_BTN）
    QCOMPARE(m_hotcityBtn->size(), ViewVar::HOT_BTN);
    
    // 验证焦点策略
    QCOMPARE(m_hotcityBtn->focusPolicy(), Qt::NoFocus);
    
    // 验证调色板设置
    QPalette palette = m_hotcityBtn->palette();
    QCOMPARE(palette.color(QPalette::Button), Qt::transparent);
}

void TestHotcityBtn::testHotCityClickSignal()
{
    // 测试hotCityClick信号
    QString cityName = "上海";
    QString cityCode = "101020100";
    
    m_hotcityBtn = new HotcityBtn(nullptr, cityName, cityCode);
    QVERIFY(m_hotcityBtn != nullptr);
    
    // 创建信号监视器
    QSignalSpy spy(m_hotcityBtn, SIGNAL(hotCityClick(QString)));
    QVERIFY(spy.isValid());
    
    // 触发按钮点击
    QTest::mouseClick(m_hotcityBtn, Qt::LeftButton);
    
    // 验证信号是否正确发出
    QCOMPARE(spy.count(), 1);
    
    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toString() == cityCode);
}

void TestHotcityBtn::testButtonProperties()
{
    // 测试按钮的各种属性
    QString cityName = "广州";
    QString cityCode = "101280101";
    
    m_hotcityBtn = new HotcityBtn(nullptr, cityName, cityCode);
    QVERIFY(m_hotcityBtn != nullptr);
    
    // 验证按钮文本
    QCOMPARE(m_hotcityBtn->text(), cityName);
    
    // 验证按钮大小
    QCOMPARE(m_hotcityBtn->size(), ViewVar::HOT_BTN);
    
    // 验证焦点策略
    QCOMPARE(m_hotcityBtn->focusPolicy(), Qt::NoFocus);
    
    // 验证调色板
    QPalette palette = m_hotcityBtn->palette();
    QCOMPARE(palette.color(QPalette::Button), Qt::transparent);
}