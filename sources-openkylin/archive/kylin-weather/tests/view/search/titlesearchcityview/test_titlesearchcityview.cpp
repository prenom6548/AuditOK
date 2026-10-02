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
#include "test_titlesearchcityview.h"
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QLabel>
#include "global/corevar.h"
#include "global/viewvar.h"
#include "model/titlesearchmodel.h"
#include "controller/Interaction.h"

TestTitleSearchCityView::TestTitleSearchCityView(QObject *parent) : QObject(parent), m_view(nullptr)
{
}

void TestTitleSearchCityView::initTestCase()
{
}

void TestTitleSearchCityView::cleanupTestCase()
{
}

void TestTitleSearchCityView::init()
{
    m_view = new TitleSearchCityView();
}

void TestTitleSearchCityView::cleanup()
{
    if (m_view) {
        delete m_view;
        m_view = nullptr;
    }
}

void TestTitleSearchCityView::testConstructor()
{
    QVERIFY(m_view != nullptr);
    QVERIFY(m_view->m_model != nullptr);
    QVERIFY(m_view->m_delegate != nullptr);
    QVERIFY(m_view->m_nullTip != nullptr);
}

void TestTitleSearchCityView::testSearchUiChange()
{
    // Test hasCity = true
    m_view->searchUiChange(true);
    QVERIFY(m_view->m_nullTip->isHidden());
    
    // Test hasCity = false
    m_view->searchUiChange(false);
    QVERIFY(m_view->m_nullTip->isVisible());
}

void TestTitleSearchCityView::testSetViewModel()
{
    m_view->setViewModel(true);
    QVERIFY(!m_view->isHidden());
    QVERIFY(m_view->model() != nullptr);
    
    m_view->setViewModel(false);
    QVERIFY(m_view->isHidden());
}

void TestTitleSearchCityView::testSlotSearchNull()
{
    QSignalSpy spy(m_view, SIGNAL(cityListNull(bool)));
    m_view->slotSearchNull();
    QCOMPARE(spy.count(), 1);
    QVERIFY(m_view->m_nullTip->isVisible());
    QCOMPARE(m_view->height(), 50);
}

void TestTitleSearchCityView::testSlotChangeCityFromTitleSearchList()
{
    CoreVar::GET_CITY_INFO = true;
    
    // Use the model from mock_symbols
    QAbstractItemModel *model = Interaction::getInstance()->getTitleModel();
    QModelIndex index = model->index(0, 0);
    
    QSignalSpy spy(m_view, SIGNAL(sigTitleSearchChangeCityToMain()));
    
    m_view->slotChangeCityFromTitleSearchList(index);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(CoreVar::CURRENT_CITY_ID, QString("101010100"));
}

void TestTitleSearchCityView::testStyleMethods()
{
    // Test private style methods
    m_view->darkListview();
    QVERIFY(ViewVar::s_themeStyle == false);
    
    m_view->lightListView();
    QVERIFY(ViewVar::s_themeStyle == true);
    
    m_view->updateRadius("classical");
    QCOMPARE(ViewVar::SAME_ITEMRADIUS, 0);
    
    m_view->updateRadius("other");
    QCOMPARE(ViewVar::SAME_ITEMRADIUS, 6);
}
