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
#ifndef TEST_VERSCREEN_H
#define TEST_VERSCREEN_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../view/verscreen/verscreen.h"

class TestVerscreen : public QObject
{
    Q_OBJECT

public:
    TestVerscreen();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();

    // 测试默认属性
    void testDefaultProperties();

private:
    verscreen* m_verscreen;
};

#endif // TEST_VERSCREEN_H