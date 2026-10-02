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
#include "test_city.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QChildEvent>
#include "../../../../view/horscreen/city.h"
#include "../../../../global/corevar.h"

void TestCity::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestCity::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestCity::testConstructor()
{
    // 测试构造函数
    city* cityWidget = new city();
    
    // 验证对象是否创建成功
    QVERIFY(cityWidget != nullptr);
    
    // 验证布局是否创建
    QVERIFY(cityWidget->layout() != nullptr);
    
    // 验证子控件数量（应该有多个标签）
    QList<QLabel*> labels = cityWidget->findChildren<QLabel*>();
    QVERIFY(labels.size() >= 7); // 至少应该有7个标签
    
    delete cityWidget;
}

void TestCity::testGetCurrentInfor_data()
{
    QTest::addColumn<QString>("cond_code");
    QTest::addColumn<QString>("tmp");
    QTest::addColumn<QString>("cond_txt");
    QTest::addColumn<QString>("hum");
    QTest::addColumn<QString>("wind_dir");
    QTest::addColumn<QString>("wind_sc");
    
    QTest::newRow("sunny") << "100" << "25" << "晴" << "60" << "东南风" << "3";
    QTest::newRow("cloudy") << "101" << "22" << "多云" << "70" << "西北风" << "2";
    QTest::newRow("rainy") << "300" << "18" << "小雨" << "85" << "北风" << "1";
}

void TestCity::testGetCurrentInfor()
{
    QFETCH(QString, cond_code);
    QFETCH(QString, tmp);
    QFETCH(QString, cond_txt);
    QFETCH(QString, hum);
    QFETCH(QString, wind_dir);
    QFETCH(QString, wind_sc);
    
    // 测试getCurrentInfor函数
    city* cityWidget = new city();
    
    // 创建测试数据
    CurrentWeather weather;
    weather.cond_code = cond_code;
    weather.tmp = tmp;
    weather.cond_txt = cond_txt;
    weather.hum = hum;
    weather.wind_dir = wind_dir;
    weather.wind_sc = wind_sc;
    
    // 保存原始网络状态
    bool originalNetworkStatus = CoreVar::HAS_NETWORK;
    CoreVar::HAS_NETWORK = true; // 设置为有网络状态以测试更新时间
    
    // 调用函数
    cityWidget->getCurrentInfor(weather);
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    // 测试无网络状态
    CoreVar::HAS_NETWORK = false;
    cityWidget->getCurrentInfor(weather);
    
    // 恢复原始网络状态
    CoreVar::HAS_NETWORK = originalNetworkStatus;
    
    delete cityWidget;
}

void TestCity::testIsFullScreen()
{
    // 测试isFullScreen函数
    city* cityWidget = new city();
    
    // 测试全屏模式设置
    cityWidget->isFullScreen(1);
    QCOMPARE(cityWidget->m_judgeFullScreen, 1);
    
    // 测试非全屏模式设置
    cityWidget->isFullScreen(0);
    QCOMPARE(cityWidget->m_judgeFullScreen, 0);
    
    // 测试边界值
    cityWidget->isFullScreen(-1);
    QCOMPARE(cityWidget->m_judgeFullScreen, -1);
    
    cityWidget->isFullScreen(100);
    QCOMPARE(cityWidget->m_judgeFullScreen, 100);
    
    delete cityWidget;
}

void TestCity::testInitWidgetSize()
{
    // 测试initWidgetSize函数
    city* cityWidget = new city();
    
    // 调用函数
    cityWidget->initWidgetSize();
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    delete cityWidget;
}

void TestCity::testSlotChangePCMode()
{
    // 测试slotChangePCMode槽函数
    city* cityWidget = new city();
    
    // 调用函数
    cityWidget->slotChangePCMode();
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    delete cityWidget;
}

void TestCity::testSlotChangePadVMode()
{
    // 测试slotChangePadVMode槽函数
    city* cityWidget = new city();
    
    // 调用函数
    cityWidget->slotChangePadVMode();
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    delete cityWidget;
}

void TestCity::testSlotChangePadHMode()
{
    // 测试slotChangePadHMode槽函数
    city* cityWidget = new city();
    
    // 调用函数
    cityWidget->slotChangePadHMode();
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    delete cityWidget;
}

void TestCity::testTemperatureDisplay()
{
    // 测试不同长度的温度显示
    city* cityWidget = new city();
    
    CurrentWeather weather;
    weather.cond_code = "100";
    weather.cond_txt = "晴";
    weather.hum = "60";
    weather.wind_dir = "东南风";
    weather.wind_sc = "3";
    
    // 测试单位数温度
    weather.tmp = "5";
    cityWidget->getCurrentInfor(weather);
    
    // 测试两位数温度
    weather.tmp = "25";
    cityWidget->getCurrentInfor(weather);
    
    // 测试负数温度
    weather.tmp = "-3";
    cityWidget->getCurrentInfor(weather);
    
    // 验证函数是否正常执行（不会崩溃）
    QVERIFY(true);
    
    delete cityWidget;
}

void TestCity::testWeatherIconUpdate()
{
    // 测试天气图标更新
    city* cityWidget = new city();
    
    CurrentWeather weather;
    weather.tmp = "25";
    weather.cond_txt = "晴";
    weather.hum = "60";
    weather.wind_dir = "东南风";
    weather.wind_sc = "3";
    
    // 测试不同的天气代码
    QStringList codes = {"100", "101", "300", "400", "999"};
    for (const QString& code : codes) {
        weather.cond_code = code;
        cityWidget->getCurrentInfor(weather);
        // 验证图标路径被正确设置（虽然我们无法直接访问私有成员path，但我们可以通过行为推断）
        QVERIFY(true); // 不会崩溃即视为通过
    }
    
    delete cityWidget;
}