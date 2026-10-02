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
#include "test_bottomlistview.h"
#include <QSignalSpy>
#include <QStandardItemModel>
#include "global/corevar.h"
#include "global/viewvar.h"
#include "model/titlesearchmodel.h"
#include "controller/Interaction.h"

TestBottomListView::TestBottomListView(QObject *parent) : QObject(parent), m_view(nullptr)
{
}

void TestBottomListView::initTestCase()
{
}

void TestBottomListView::cleanupTestCase()
{
}

void TestBottomListView::init()
{
    m_view = new BottomListView();
}

void TestBottomListView::cleanup()
{
    if (m_view) {
        delete m_view;
        m_view = nullptr;
    }
}

void TestBottomListView::testConstructor()
{
    QVERIFY(m_view != nullptr);
    QCOMPARE(m_view->height(), 50);
}

void TestBottomListView::testSetModelforView()
{
    // Initially m_initFirst is true
    QVERIFY(m_view->m_initFirst);
    
    m_view->setModelforView();
    
    QVERIFY(!m_view->m_initFirst);
    QVERIFY(m_view->model() != nullptr);
}

void TestBottomListView::testSetModelforView_MultipleCalls()
{
    m_view->setModelforView();
    QAbstractItemModel *firstModel = m_view->model();
    
    // Multiple calls should not change model if m_initFirst is false
    m_view->setModelforView();
    QCOMPARE(m_view->model(), firstModel);
}

void TestBottomListView::testSlotChangeCityFromTitleSearchList()
{
    CoreVar::GET_CITY_INFO = true;
    
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *item = new QStandardItem();
    item->setData("101010100", TitleSearchModel::CityId);
    item->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
    model->appendRow(item);
    
    QModelIndex index = model->index(0, 0);
    
    QSignalSpy spy(m_view, SIGNAL(sigTitleSearchChangeCityToMain()));
    
    m_view->slotChangeCityFromTitleSearchList(index);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(CoreVar::CURRENT_CITY_TYPE, CityType::Location);
    
    delete model;
}

void TestBottomListView::testSlotChangeCityFromTitleSearchList_NoInfo()
{
    CoreVar::GET_CITY_INFO = false;
    
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    item->setData("101010100", TitleSearchModel::CityId);
    model.appendRow(item);
    
    QSignalSpy spy(m_view, SIGNAL(sigTitleSearchChangeCityToMain()));
    m_view->slotChangeCityFromTitleSearchList(model.index(0, 0));
    
    QCOMPARE(spy.count(), 0); // Should return early
}

void TestBottomListView::testSlotChangeCityFromTitleSearchList_Types()
{
    CoreVar::GET_CITY_INFO = true;
    QStandardItemModel model;
    
    // Test Collection type
    QStandardItem *itemColl = new QStandardItem();
    itemColl->setData(TitleSearchModel::CityType::Collection, TitleSearchModel::CityStyle);
    model.appendRow(itemColl);
    m_view->slotChangeCityFromTitleSearchList(model.index(0, 0));
    QCOMPARE(CoreVar::CURRENT_CITY_TYPE, CityType::Collection);
    
    // Test Other type (any other value)
    QStandardItem *itemOther = new QStandardItem();
    itemOther->setData(999, TitleSearchModel::CityStyle);
    model.appendRow(itemOther);
    m_view->slotChangeCityFromTitleSearchList(model.index(1, 0));
    QCOMPARE(CoreVar::CURRENT_CITY_TYPE, CityType::Other);
}

void TestBottomListView::testSlotShowBottomList()
{
    m_view->slotShowBottomList(5);
    QCOMPARE(m_view->width(), 5 * 30); // LIST_ITEM_WIDTH_SIZE = 30
    QVERIFY(m_view->model() != nullptr);
    
    m_view->slotShowBottomList(0);
    QCOMPARE(m_view->width(), 0);
}

void TestBottomListView::testWidgetStyle()
{
    // widget style is called in constructor
    QCOMPARE(m_view->focusPolicy(), Qt::NoFocus);
    QCOMPARE(m_view->flow(), QListView::LeftToRight);
    QVERIFY(m_view->m_model != nullptr);
    QVERIFY(m_view->m_delegate != nullptr);
    QCOMPARE(m_view->editTriggers(), QAbstractItemView::NoEditTriggers);
}
