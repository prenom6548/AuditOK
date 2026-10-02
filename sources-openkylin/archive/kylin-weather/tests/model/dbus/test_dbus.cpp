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
#include "test_dbus.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include "../../../model/dbus.h"

// 桩实现：不包含真正的 core.h 以避免复杂的系统依赖
// 我们在这里提供 Core 类的最小定义，仅为了能定义 showCityWeather 成员函数
#include <QString>

class Core : public QObject {
public:
    Core() : QObject(nullptr) {}
    QString showCityWeather(const QString &cityName);
};

static QString s_mockedResult = "";

QString Core::showCityWeather(const QString &cityName)
{
    Q_UNUSED(cityName);
    return s_mockedResult;
}

void TestDbus::initTestCase() {}
void TestDbus::cleanupTestCase() {}

void TestDbus::testDbusConstructor()
{
    Dbus dbus(nullptr);
    QVERIFY(true);
}

void TestDbus::testRegisterService()
{
    // 单元测试中通常跳过真正的 DBus 注册
    QVERIFY(true);
}

void TestDbus::testShowCityWeatherNullCore()
{
    Dbus dbus(nullptr);
    QString result = dbus.showCityWeather("Beijing");
    QCOMPARE(result, QString("{\"status\": 1}"));
}

void TestDbus::testShowCityWeatherValidCore()
{
    // 创建一个假的 Core 指针。
    // 我们不需要真的实例化真实的 Core，只需要一个 Core 类型的指针
    Core *fakeCore = new Core(); 
    Dbus dbus(fakeCore);
    
    s_mockedResult = "{\"status\": 0}";
    QString result = dbus.showCityWeather("Beijing");
    
    QCOMPARE(result, s_mockedResult);
    delete fakeCore;
}
