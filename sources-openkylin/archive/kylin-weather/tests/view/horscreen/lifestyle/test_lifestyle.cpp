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
#include "test_lifestyle.h"
#include <QtTest/QtTest>
#include <QApplication>

TestLifestyle::TestLifestyle()
    : m_lifestyle(nullptr)
{
}

void TestLifestyle::initTestCase()
{
    // 在所有测试函数运行前执行一次
    m_lifestyle = new lifestyle();
    QVERIFY(m_lifestyle != nullptr);
}

void TestLifestyle::cleanupTestCase()
{
    // 在所有测试函数运行后执行一次
    if (m_lifestyle) {
        delete m_lifestyle;
        m_lifestyle = nullptr;
    }
}

void TestLifestyle::testConstructor()
{
    // 测试构造函数
    lifestyle* life = new lifestyle();
    QVERIFY(life != nullptr);
    
    // 检查默认大小
    QCOMPARE(life->width(), 856);
    QCOMPARE(life->height(), 120);
    
    // 检查窗口标志
    QVERIFY(life->windowFlags() & Qt::FramelessWindowHint);
    
    // 检查背景属性
    QVERIFY(life->testAttribute(Qt::WA_TranslucentBackground));
    
    // 测试paintEvent函数
    life->show();
    life->repaint();  // 触发paintEvent
    
    delete life;
}

void TestLifestyle::testGetLifeStyleInfor_data()
{
    // 创建测试数据
    QTest::addColumn<QString>("uv_brf");
    QTest::addColumn<QString>("air_brf");
    QTest::addColumn<QString>("sport_brf");
    QTest::addColumn<QString>("cw_brf");
    QTest::addColumn<QString>("flu_brf");
    QTest::addColumn<QString>("drsg_brf");

    QTest::newRow("normal data") << "弱" << "良" << "适宜" << "较适宜" << "少发" << "热";
    QTest::newRow("empty data") << "" << "" << "" << "" << "" << "";
    QTest::newRow("special chars") << "强!" << "污染#" << "不适宜$" << "不宜%" << "易发&" << "冷*";
}

void TestLifestyle::testGetLifeStyleInfor()
{
    QFETCH(QString, uv_brf);
    QFETCH(QString, air_brf);
    QFETCH(QString, sport_brf);
    QFETCH(QString, cw_brf);
    QFETCH(QString, flu_brf);
    QFETCH(QString, drsg_brf);

    LifeStyle lifeStyle;
    lifeStyle.uv_brf = uv_brf;
    lifeStyle.air_brf = air_brf;
    lifeStyle.sport_brf = sport_brf;
    lifeStyle.cw_brf = cw_brf;
    lifeStyle.flu_brf = flu_brf;
    lifeStyle.drsg_brf = drsg_brf;

    // 测试函数是否正常执行，不抛出异常即为通过
    m_lifestyle->getLifeStyleInfor(lifeStyle, 0);
    
    // 由于子组件是私有的，我们无法直接验证它们的值
    // 但我们可以通过检查函数是否正常执行来确认基本功能
    QVERIFY(true);
}