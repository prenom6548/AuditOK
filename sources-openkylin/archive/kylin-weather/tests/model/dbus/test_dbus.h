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
#ifndef TEST_DBUS_H
#define TEST_DBUS_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../model/dbus.h"

// 我们不继承真正的 Core，因为那样会引入太多依赖。
// 我们在 test_dbus.cpp 中提供 Core::showCityWeather 的桩实现。
class Core; 

class TestDbus : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDbusConstructor();
    void testRegisterService();
    void testShowCityWeatherNullCore();
    void testShowCityWeatherValidCore();
};

#endif // TEST_DBUS_H
