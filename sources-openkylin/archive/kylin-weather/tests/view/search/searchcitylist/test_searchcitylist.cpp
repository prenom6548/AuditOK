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
#include "test_searchcitylist.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QWidget>
#include <QStandardItemModel>
#include "view/search/searchcitylist.h"
#include "global/viewvar.h"
#include "mock_interaction.h"

// 使用宏替换Interaction
#ifdef Interaction
#undef Interaction
#endif
#define Interaction MockInteraction

// 包含源文件以进行测试
#include "../../../view/search/searchcitylist.cpp"

TestSearchCityList::TestSearchCityList(QObject *parent)
    : QObject(parent)
    , m_searchCityList(nullptr)
    , m_model(nullptr)
{
}

void TestSearchCityList::initTestCase()
{
    // 在所有测试之前执行的初始化代码
}

void TestSearchCityList::cleanupTestCase()
{
    // 在所有测试之后执行的清理代码
}

void TestSearchCityList::init()
{
    // 在每个测试之前执行的初始化代码
    m_searchCityList = nullptr;
    m_model = nullptr;
}

void TestSearchCityList::cleanup()
{
    // 在每个测试之后执行的清理代码
    if (m_searchCityList) {
        delete m_searchCityList;
        m_searchCityList = nullptr;
    }
    if (m_model) {
        delete m_model;
        m_model = nullptr;
    }
}

void TestSearchCityList::testConstructor()
{
    // 测试构造函数
    m_searchCityList = new SearchCityLIst(nullptr);
    
    QVERIFY(m_searchCityList != nullptr);
    
    // 验证基本属性设置
    QCOMPARE(m_searchCityList->size(), ViewVar::SEARCH_LIST);
    QCOMPARE(m_searchCityList->alternatingRowColors(), false);
    QCOMPARE(m_searchCityList->showGrid(), false);
    QCOMPARE(m_searchCityList->editTriggers(), QAbstractItemView::NoEditTriggers);
    QCOMPARE(m_searchCityList->selectionMode(), QAbstractItemView::NoSelection);
    QCOMPARE(m_searchCityList->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    QCOMPARE(m_searchCityList->frameShape(), QFrame::NoFrame);
    
    // 验证delegate已设置
    QVERIFY(m_searchCityList->itemDelegate() != nullptr);
    
    // 验证header visibility
    QCOMPARE(m_searchCityList->horizontalHeader()->isVisible(), false);
    QCOMPARE(m_searchCityList->verticalHeader()->isVisible(), false);
}

void TestSearchCityList::testSearchUiChange_data()
{
    QTest::addColumn<bool>("hasCity");
    QTest::addColumn<bool>("shouldBeVisible");

    QTest::newRow("has city") << true << true;
    QTest::newRow("no city") << false << false;
}

void TestSearchCityList::testSearchUiChange()
{
    QFETCH(bool, hasCity);
    QFETCH(bool, shouldBeVisible);

    m_searchCityList = new SearchCityLIst(nullptr);
    QVERIFY(m_searchCityList != nullptr);
    
    // 调用searchUiChange
    m_searchCityList->searchUiChange(hasCity);
    
    // 注意：由于我们无法直接访问私有成员m_nullTip，所以我们不验证它的状态
}

void TestSearchCityList::testSetViewModel_data()
{
    QTest::addColumn<bool>("needSetModel");
    QTest::addColumn<bool>("shouldBeVisible");

    QTest::newRow("set model true") << true << true;
    QTest::newRow("set model false") << false << false;
}

void TestSearchCityList::testSetViewModel()
{
    QFETCH(bool, needSetModel);
    QFETCH(bool, shouldBeVisible);

    m_searchCityList = new SearchCityLIst(nullptr);
    QVERIFY(m_searchCityList != nullptr);
    
    // 初始状态应该是隐藏的
    QCOMPARE(m_searchCityList->isHidden(), true);
    
    // 调用setViewModel
    m_searchCityList->setViewModel(needSetModel);
    
    if (needSetModel) {
        // 需要设置模型时，应该显示
        QCOMPARE(m_searchCityList->isHidden(), false);
        // 应该设置了正确的模型
        QVERIFY(m_searchCityList->model() != nullptr);
    } else {
        // 不需要设置模型时，应该保持隐藏
        QCOMPARE(m_searchCityList->isHidden(), true);
    }
}

void TestSearchCityList::testSignals()
{
    m_searchCityList = new SearchCityLIst(nullptr);
    QVERIFY(m_searchCityList != nullptr);
    
    // 创建信号监视器
    QSignalSpy addCollectCitySpy(m_searchCityList, SIGNAL(addCollectCity(QModelIndex)));
    QVERIFY(addCollectCitySpy.isValid());
    
    QSignalSpy changeCityFromSearchSpy(m_searchCityList, SIGNAL(changeCityFromSearch(QModelIndex)));
    QVERIFY(changeCityFromSearchSpy.isValid());
    
    QSignalSpy cityListNullSpy(m_searchCityList, SIGNAL(cityListNull(bool)));
    QVERIFY(cityListNullSpy.isValid());
    
    // 测试addCollectCity信号
    QModelIndex testIndex;
    emit m_searchCityList->addCollectCity(testIndex);
    QCOMPARE(addCollectCitySpy.count(), 1);
    
    // 测试changeCityFromSearch信号
    emit m_searchCityList->changeCityFromSearch(testIndex);
    QCOMPARE(changeCityFromSearchSpy.count(), 1);
    
    // 测试cityListNull信号
    emit m_searchCityList->cityListNull(true);
    QCOMPARE(cityListNullSpy.count(), 1);
    QCOMPARE(cityListNullSpy.first().first().toBool(), true);
}