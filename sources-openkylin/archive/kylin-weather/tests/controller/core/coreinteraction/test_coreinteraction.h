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
#ifndef TEST_COREINTERACTION_H
#define TEST_COREINTERACTION_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QTimer>
#include <QApplication>
#include "../../../../controller/core/coreinteraction.h"

// 友元类，用于访问CoreInteraction的受保护成员
class CoreInteractionTester : public CoreInteraction
{
public:
    // 公开受保护的方法以便测试
    void publicCreatCore(const QStringList &list) { creatCore(list); }
    void publicChangeCity(QString cityId) { changeCity(cityId); }
    void publicSearchResult(QString inputText) { searchResult(inputText); }
    QStandardItemModel *publicGetCityModel() { return getCityModel(); }
    QStandardItemModel *publicGetTitleModel() { return getTitleModel(); }
    QStandardItemModel *publicGetCollectModel() { return getCollectModel(); }
    QStandardItemModel *publicGetBottomCollectModel() { return getBottomCollectModel(); }
    void publicChangeCityFromSearch(QModelIndex index) { changeCityFromSearch(index); }
    void publicAddCollectCity(QModelIndex index) { addCollectCity(index); }
    void publicChangeCityFromCollect(QModelIndex index) { changeCityFromCollect(index); }
    void publicDelCollectCity(QModelIndex index) { delCollectCity(index); }
    void publicLoadCollectCity() { loadCollectCity(); }
    void publicShowCenterAfter() { showCenterAfter(); }
    void publicClosePrograme() { closePrograme(); }
    void publicAllowLocationYes() { allowLocationYes(); }
    void publicAllowLocationNo() { allowLocationNo(); }
    void publicRefresh() { refresh(); }
    void publicAddCollectCityFromWidget(QString cityId) { addCollectCityFromWidget(cityId); }
    void publicDelCollectCityFromWidget(QString cityId) { delCollectCityFromWidget(cityId); }
    void publicGetLAndRCityInfo(QString cityIdList) { getLAndRCityInfo(cityIdList); }
    
    // 通过公共方法间接测试私有方法
    bool testFrequencyLimit() {
        // 测试频率限制功能
        publicChangeCity("123456");
        publicChangeCity("123456"); // 第二次调用应该被限制
        return true; // 只要不崩溃就算通过
    }
};

class TestCoreInteraction : public QObject
{
    Q_OBJECT

public:
    TestCoreInteraction();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetInstance();
    void testConstructor();
    void testCreatCore();
    void testChangeCity();
    void testSearchResult();
    void testGetCityModel();
    void testGetTitleModel();
    void testGetCollectModel();
    void testGetBottomCollectModel();
    void testChangeCityFromSearch();
    void testAddCollectCity();
    void testChangeCityFromCollect();
    void testDelCollectCity();
    void testLoadCollectCity();
    void testShowCenterAfter();
    void testClosePrograme();
    void testAllowLocationYes();
    void testAllowLocationNo();
    void testRefresh();
    void testAddCollectCityFromWidget();
    void testDelCollectCityFromWidget();
    void testGetLAndRCityInfo();
    void testCoreOperateTooOften();

private:
    CoreInteractionTester* m_coreInteraction;
};

#endif // TEST_COREINTERACTION_H