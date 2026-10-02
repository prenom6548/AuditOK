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
#include "test_searchmarch.h"
#include "model/searchmarch.h"
#include <QtTest/QtTest>
#include <QApplication>

void TestSearchMarch::initTestCase()
{
    // 在所有测试之前运行
}

void TestSearchMarch::cleanupTestCase()
{
    // 在所有测试之后运行
}

void TestSearchMarch::init()
{
    // 实例化 SearchMarch，它会启动线程加载 CSV
    searchMarch = new SearchMarch();
    
    // 关键：增加重试逻辑，确保 CSV 数据加载完成后再开始测试
    // 否则 m_locatonList 为空，匹配逻辑分支将无法覆盖
    int retry = 0;
    while (searchMarch->m_locatonList.isEmpty() && retry < 50) {
        QTest::qWait(100);
        retry++;
    }
}

void TestSearchMarch::cleanup()
{
    delete searchMarch;
}

void TestSearchMarch::testConstructor()
{
    QVERIFY(searchMarch != nullptr);
}

void TestSearchMarch::testDataLoading()
{
    // 验证数据是否成功从 CSV 加载，这是后续测试的基础
    QVERIFY(!searchMarch->m_locatonList.isEmpty());
}

void TestSearchMarch::testSetAutomaticCity_data()
{
    QTest::addColumn<QString>("cityName");
    QTest::addColumn<QString>("expectedId");

    QTest::newRow("empty city name") << QString("") << QString("");
    QTest::newRow("valid city name - beijing") << QString("beijing") << QString("101010100");
    QTest::newRow("valid city name - 北京") << QString("北京") << QString("101010100");
    QTest::newRow("valid city name - 北京市") << QString("北京市") << QString("101010100");
    QTest::newRow("valid city name - 朝阳区") << QString("朝阳区") << QString("101010300");
    QTest::newRow("valid city name - 延庆县") << QString("延庆县") << QString("101011000");
    QTest::newRow("case insensitive") << QString("BeiJing") << QString("101010100");
    QTest::newRow("invalid city name") << QString("nonexistentcity") << QString("");
}

void TestSearchMarch::testSetAutomaticCity()
{
    QFETCH(QString, cityName);
    QFETCH(QString, expectedId);

    QString result = searchMarch->setAutomaticCity(cityName);
    QCOMPARE(result, expectedId);
}

void TestSearchMarch::testMatchCityName_data()
{
    QTest::addColumn<QString>("cityId");
    QTest::addColumn<QString>("expectedName");

    QTest::newRow("empty city id") << QString("") << QString("");
    QTest::newRow("valid city id - beijing") << QString("101010100") << QString("北京");
    QTest::newRow("valid city id - changsha") << QString("101250101") << QString("长沙");
    QTest::newRow("invalid city id") << QString("999999999") << QString("");
}

void TestSearchMarch::testMatchCityName()
{
    QFETCH(QString, cityId);
    QFETCH(QString, expectedName);

    QString result = searchMarch->matchCityName(cityId);
    QCOMPARE(result, expectedName);
}

void TestSearchMarch::testExactMatchCity_data()
{
    QTest::addColumn<QString>("inputText");
    QTest::addColumn<bool>("shouldFound");

    QTest::newRow("empty input") << QString("") << false;
    QTest::newRow("valid city shorthand - bj") << QString("bj") << true;
    QTest::newRow("valid city id - 101010100") << QString("101010100") << true;
    QTest::newRow("valid city chinese start") << QString("北") << true;
    QTest::newRow("valid city en start") << QString("beij") << true;
    QTest::newRow("valid city en match") << QString("beijing") << true;
    QTest::newRow("invalid input") << QString("nonexistent") << false;
}

void TestSearchMarch::testExactMatchCity()
{
    QFETCH(QString, inputText);
    QFETCH(bool, shouldFound);

    QList<LocationData> result = searchMarch->exactMatchCity(inputText);
    if (shouldFound) {
        QVERIFY(!result.isEmpty());
    } else {
        QVERIFY(result.isEmpty());
    }
}

void TestSearchMarch::testMatchRanking()
{
    // 测试排序逻辑：完全匹配项 (city_en == inputText) 应插入到列表首位
    QString input = "beijing";
    QList<LocationData> result = searchMarch->exactMatchCity(input);
    
    QVERIFY(!result.isEmpty());
    // 验证第一个结果是否为完全匹配项
    QCOMPARE(result.first().city_en, input);
}
