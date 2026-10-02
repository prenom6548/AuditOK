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
#ifndef TEST_CORE_H
#define TEST_CORE_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include "../../../../controller/core/core.h"

class TestCore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试初始化核心功能
    void testInitCore();
    
    // 测试切换城市功能
    void testChangeCity();
    
    // 测试从搜索列表切换城市功能
    void testChangeCityFromSearch();
    
    // 测试添加收藏城市功能
    void testAddCollectCity();
    
    // 测试从部件添加收藏城市功能
    void testAddCollectCityFromWidget();
    
    // 测试收藏城市功能
    void testCollectCity();
    
    // 测试从收藏列表切换城市功能
    void testChangeCityFromCollect();
    
    // 测试删除收藏城市功能
    void testDelCollectCity();
    
    // 测试从部件删除收藏城市功能
    void testDelCollectCityFromWidget();
    
    // 测试搜索城市功能
    void testSearchCity();
    
    // 测试加载收藏城市功能
    void testLoadCollectCity();
    
    // 测试显示在屏幕中心功能
    void testShowInCenter();
    
    // 测试获取城市模型功能
    void testGetCityModel();
    
    // 测试获取标题栏城市模型功能
    void testGetCityTitleModel();
    
    // 测试获取收藏模型功能
    void testGetCollectModel();
    
    // 测试获取底部收藏模型功能
    void testGetBottomCollectModel();
    
    // 测试允许定位功能
    void testAllowLocationYes();
    
    // 测试拒绝定位功能
    void testAllowLocationNo();
    
    // 测试程序退出功能
    void testProgremExit();
    
    // 测试刷新城市功能
    void testRefreshCity();
    
    // 测试显示NA数据功能
    void testShowNAData();

    // 测试DBus接口显示城市天气
    void testShowCityWeather();

    // 新增：覆盖率完善测试
    void testNetworkState();
    void testUpdateCollect();
    // void testCoreOperateTooOften();
    void testLocationFunctions();
    void testModelFunctions();
    void testDbusAndCommand();
    void testRefreshCityMore();
    void testDelCollectCityFromWidgetMore();

private:
    Core* m_core;
};

#endif // TEST_CORE_H
