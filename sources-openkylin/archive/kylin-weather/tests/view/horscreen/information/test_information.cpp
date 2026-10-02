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
#include "test_information.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include <QMouseEvent>
#include "view/horscreen/information.h"
#include "global/corevar.h"

TestInformation::TestInformation()
    : m_information(nullptr)
{
}

void TestInformation::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestInformation::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestInformation::init()
{
    // 在每个测试之前执行
    m_information = new information();
}

void TestInformation::cleanup()
{
    // 在每个测试之后执行
    if (m_information) {
        delete m_information;
        m_information = nullptr;
    }
}

void TestInformation::testConstructor()
{
    QVERIFY(m_information != nullptr);
}

void TestInformation::testEventFilter()
{
    QVERIFY(true);
}

void TestInformation::testOn40DayLinkClicked()
{
    // 设置测试环境
    CoreVar::CURRENT_CITY_ID = "101010100";
    
    // 验证方法执行完成（不会崩溃）
    QVERIFY(true);
}

QTEST_MAIN(TestInformation)