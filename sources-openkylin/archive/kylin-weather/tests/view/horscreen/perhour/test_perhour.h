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
#ifndef TEST_PERHOUR_H
#define TEST_PERHOUR_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/horscreen/perhour.h"

class TestPerhour : public QObject
{
    Q_OBJECT

public:
    TestPerhour();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试setHourlyInfor方法
    void testSetHourlyInfor_data();
    void testSetHourlyInfor();
    
    // 测试setHourlyIcon方法
    void testSetHourlyIcon();
    
    // 测试模式切换方法
    void testModeSwitching();

private:
    perhour* m_perhour;
};

#endif // TEST_PERHOUR_H