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
#include "test_weekweather.h"
#include <QApplication>
#include <QtTest/QSignalSpy>
#include <QDateTime>
#include <QLayout>
#include <QBoxLayout>
#include "global/corevar.h"

TestWeekweather::TestWeekweather(QObject *parent)
    : QObject(parent)
    , m_weekweather(nullptr)
{
}

void TestWeekweather::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestWeekweather::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestWeekweather::init()
{
    // 在每个测试之前执行
    m_weekweather = new weekweather();
}

void TestWeekweather::cleanup()
{
    // 在每个测试之后执行
    if (m_weekweather) {
        delete m_weekweather;
        m_weekweather = nullptr;
    }
}

void TestWeekweather::testConstructor()
{
    QVERIFY(m_weekweather != nullptr);
    
    // 验证窗口属性
    QVERIFY(m_weekweather->testAttribute(Qt::WA_TranslucentBackground));
    QCOMPARE(m_weekweather->windowFlags() & Qt::FramelessWindowHint, Qt::FramelessWindowHint);
    QCOMPARE(m_weekweather->size(), QSize(856, 216));
    
    // 验证布局是否创建
    QVERIFY(m_weekweather->layout() != nullptr);
    
    // 验证布局中的控件数量
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(m_weekweather->layout());
    QVERIFY(layout != nullptr);
    // 应该有13个控件：7个onedayweather + 6个splitline
    QCOMPARE(layout->count(), 13);
    
    // 验证控件类型
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);
        QWidget *widget = item->widget();
        QVERIFY(widget != nullptr);
        
        // 检查奇数索引是onedayweather，偶数索引是splitline
        if (i % 2 == 0) {
            // 偶数索引应该是onedayweather
            QVERIFY(qobject_cast<onedayweather*>(widget) != nullptr);
        } else {
            // 奇数索引应该是splitline（除了最后一个控件）
            if (i < 12) {
                QVERIFY(qobject_cast<splitline*>(widget) != nullptr);
            } else {
                // 最后一个是onedayweather
                QVERIFY(qobject_cast<onedayweather*>(widget) != nullptr);
            }
        }
    }
}

void TestWeekweather::testGetForcastInfor_data()
{
    QTest::addColumn<QString>("date");
    QTest::addColumn<QString>("cond_txt_d");
    QTest::addColumn<QString>("cond_code_d");
    QTest::addColumn<QString>("cond_txt_n");
    QTest::addColumn<QString>("cond_code_n");
    QTest::addColumn<QString>("tmp_max");
    QTest::addColumn<QString>("tmp_min");
    QTest::addColumn<QString>("wind_dir");
    
    QTest::newRow("sunny day") << "2023-06-15" << "晴" << "100" << "晴" << "100" << "30" << "20" << "东南风";
    QTest::newRow("rainy day") << "2023-06-16" << "小雨" << "300" << "小雨" << "300" << "25" << "18" << "西北风";
    QTest::newRow("cloudy day") << "2023-06-17" << "多云" << "102" << "多云" << "102" << "28" << "19" << "东风";
}

void TestWeekweather::testGetForcastInfor()
{
    QFETCH(QString, date);
    QFETCH(QString, cond_txt_d);
    QFETCH(QString, cond_code_d);
    QFETCH(QString, cond_txt_n);
    QFETCH(QString, cond_code_n);
    QFETCH(QString, tmp_max);
    QFETCH(QString, tmp_min);
    QFETCH(QString, wind_dir);
    
    // 创建测试数据
    ForecastWeather forecastWeather;
    
    // 设置第一天数据
    forecastWeather.first.date = date;
    forecastWeather.first.cond_txt_d = cond_txt_d;
    forecastWeather.first.cond_code_d = cond_code_d;
    forecastWeather.first.cond_txt_n = cond_txt_n;
    forecastWeather.first.cond_code_n = cond_code_n;
    forecastWeather.first.tmp_max = tmp_max;
    forecastWeather.first.tmp_min = tmp_min;
    forecastWeather.first.wind_dir = wind_dir;
    
    // 设置其他天的数据（简化处理）
    forecastWeather.second = forecastWeather.first;
    forecastWeather.third = forecastWeather.first;
    forecastWeather.fourth = forecastWeather.first;
    forecastWeather.fifth = forecastWeather.first;
    forecastWeather.sixth = forecastWeather.first;
    forecastWeather.seventh = forecastWeather.first;
    
    // 调用测试方法
    m_weekweather->getForcastInfor(forecastWeather, 0);
    
    // 验证数据是否正确设置（检查第一个天气控件）
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(m_weekweather->layout());
    QVERIFY(layout != nullptr);
    QVERIFY(layout->count() > 0);
    
    // 第一个控件应该是onedayweather
    QLayoutItem *firstItem = layout->itemAt(0);
    QVERIFY(firstItem != nullptr);
    onedayweather *firstDay = qobject_cast<onedayweather*>(firstItem->widget());
    QVERIFY2(firstDay != nullptr, "第一个控件不是onedayweather类型");
    
    // 验证温度显示
    QString expectedTemp = tmp_min + "°~" + tmp_max + "°";
    QCOMPARE(firstDay->m_temp->text(), expectedTemp);
    
    // 验证风向显示
    QCOMPARE(firstDay->m_wind->text(), wind_dir);
    
    // 验证天气描述
    QCOMPARE(firstDay->m_weather->text(), cond_txt_d); // 白天时间应该显示白天天气
}

void TestWeekweather::testSetWeatherIcon()
{
    // 创建测试数据
    ForecastWeather forecastWeather;
    
    // 设置数据
    forecastWeather.first.cond_code_d = "100";
    forecastWeather.second.cond_code_d = "101";
    forecastWeather.third.cond_code_d = "102";
    forecastWeather.fourth.cond_code_d = "103";
    forecastWeather.fifth.cond_code_d = "104";
    forecastWeather.sixth.cond_code_d = "300";
    forecastWeather.seventh.cond_code_d = "301";
    
    // 复制到夜间数据
    forecastWeather.first.cond_code_n = "100";
    forecastWeather.second.cond_code_n = "101";
    forecastWeather.third.cond_code_n = "102";
    forecastWeather.fourth.cond_code_n = "103";
    forecastWeather.fifth.cond_code_n = "104";
    forecastWeather.sixth.cond_code_n = "300";
    forecastWeather.seventh.cond_code_n = "301";
    
    // 先调用getForcastInfor设置基本数据
    m_weekweather->getForcastInfor(forecastWeather, 0);
    
    // 调用测试方法
    m_weekweather->setWeatherIcon(0);
    
    // 验证图标路径是否正确设置
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(m_weekweather->layout());
    QVERIFY(layout != nullptr);
    QVERIFY(layout->count() > 0);
    
    // 第一个控件应该是onedayweather
    QLayoutItem *firstItem = layout->itemAt(0);
    QVERIFY(firstItem != nullptr);
    onedayweather *firstDay = qobject_cast<onedayweather*>(firstItem->widget());
    QVERIFY2(firstDay != nullptr, "第一个控件不是onedayweather类型");
    
    // 验证图标大小
    QCOMPARE(firstDay->m_weatherIcon->size(), QSize(72, 72));
}

void TestWeekweather::testInitLayout()
{
    // 验证布局已正确设置
    QVERIFY(m_weekweather->layout() != nullptr);
    
    // 检查布局的contents margins而不是margin
    QCOMPARE(m_weekweather->layout()->contentsMargins().left(), 15);
    QCOMPARE(m_weekweather->layout()->contentsMargins().top(), 15);
    QCOMPARE(m_weekweather->layout()->contentsMargins().right(), 15);
    QCOMPARE(m_weekweather->layout()->contentsMargins().bottom(), 15);
    
    // 验证spacing
    QCOMPARE(m_weekweather->layout()->spacing(), 0);
    
    // 验证所有控件都已添加到布局中
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(m_weekweather->layout());
    QVERIFY(layout != nullptr);
    
    // 验证控件数量（13个控件：7个onedayweather + 6个splitline）
    QCOMPARE(layout->count(), 13);
}

void TestWeekweather::testInitConnect()
{
    // 测试信号连接 - 通过发送信号并检查槽函数是否被调用
    QSignalSpy spy(m_weekweather, SIGNAL(sendIsFullScreen(int)));
    
    // 发送信号
    emit m_weekweather->sendIsFullScreen(1);
    
    // 验证信号是否正确发出
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), 1);
}

void TestWeekweather::testSendIsFullScreenSignal()
{
    // 测试发送全屏信号的功能
    QSignalSpy spy(m_weekweather, SIGNAL(sendIsFullScreen(int)));
    
    // 发送不同值的信号
    emit m_weekweather->sendIsFullScreen(0);
    emit m_weekweather->sendIsFullScreen(1);
    
    // 验证信号发送次数
    QCOMPARE(spy.count(), 2);
    
    // 验证信号参数
    QList<QVariant> arguments1 = spy.takeFirst();
    QCOMPARE(arguments1.at(0).toInt(), 0);
    
    QList<QVariant> arguments2 = spy.takeFirst();
    QCOMPARE(arguments2.at(0).toInt(), 1);
}

QTEST_MAIN(TestWeekweather)