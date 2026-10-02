/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "core/datacollector.h"
#include <QtTest/QtTest>

class TestDataCollector : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCollectResouceLimitedEnebaledChanged();
    void testCollectSoftFreezeModeEnabledChanged();
};

void TestDataCollector::testCollectResouceLimitedEnebaledChanged()
{
    DataCollector collector;

    // 测试 enabled 为 true 的情况
    collector.collectResouceLimitedEnebaledChanged(true);
    // 这里可以添加断言或日志来验证数据是否正确收集

    // 测试 enabled 为 false 的情况
    collector.collectResouceLimitedEnebaledChanged(false);
    // 这里可以添加断言或日志来验证数据是否正确收集
}

void TestDataCollector::testCollectSoftFreezeModeEnabledChanged()
{
    DataCollector collector;

    // 测试 enabled 为 true 的情况
    collector.collectSoftFreezeModeEnabledChanged(true);
    // 这里可以添加断言或日志来验证数据是否正确收集

    // 测试 enabled 为 false 的情况
    collector.collectSoftFreezeModeEnabledChanged(false);
    // 这里可以添加断言或日志来验证数据是否正确收集
}

QTEST_MAIN(TestDataCollector)
#include "tst_datacollector.moc"