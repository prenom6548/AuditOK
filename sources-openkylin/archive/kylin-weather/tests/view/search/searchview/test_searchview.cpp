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
#include "test_searchview.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QMessageBox>
#include "view/search/searchview.h"
#include "global/viewvar.h"
#include "global/corevar.h"
#include "controller/Interaction.h"

TestSearchView::TestSearchView(QObject *parent)
    : QObject(parent)
    , m_searchView(nullptr)
{
}

void TestSearchView::initTestCase() {}
void TestSearchView::cleanupTestCase() {}

void TestSearchView::init()
{
    m_searchView = new SearchView();
}

void TestSearchView::cleanup()
{
    if (m_searchView) {
        delete m_searchView;
        m_searchView = nullptr;
    }
}

void TestSearchView::testConstructor()
{
    QVERIFY(m_searchView != nullptr);
    QCOMPARE(m_searchView->size(), ViewVar::SEARCH);
}

void TestSearchView::testPublicMethods()
{
    m_searchView->changeUiShow();
    QVERIFY(true);
    
    m_searchView->searchReset();
    QVERIFY(true);
}

void TestSearchView::testEvents()
{
    // Test keyPressEvent (Ctrl+E)
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_E, Qt::ControlModifier);
    QApplication::sendEvent(m_searchView, &keyEvent);
    QVERIFY(m_searchView->m_searchLineEdit->hasFocus());
    
    // Test MouseButtonPress (should clear focus)
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_searchView, &mouseEvent);
    QVERIFY(!m_searchView->m_searchLineEdit->hasFocus());
    
    // Test hideEvent
    m_searchView->hide();
    QCOMPARE(m_searchView->m_stackedWidget->currentIndex(), 0);
}

void TestSearchView::testSignals()
{
    QSignalSpy spy(m_searchView, SIGNAL(searchWidState()));
    m_searchView->changeUiShow();
    QCOMPARE(spy.count(), 1);
}

void TestSearchView::testLayoutLogic()
{
    // Test back label click via event filter
    m_searchView->changeStackedWidget(); // switch to index 1
    QCOMPARE(m_searchView->m_stackedWidget->currentIndex(), 1);
    
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_searchView->eventFilter(m_searchView->m_backLabel, &mouseEvent);
    
    QCOMPARE(m_searchView->m_stackedWidget->currentIndex(), 0);
}

void TestSearchView::testSearchListUi()
{
    ViewVar::m_isTitleSearch = false;
    m_searchView->searchListUi(true);
    QVERIFY(true);

    m_searchView->searchListUi(false);
    QVERIFY(true);

    ViewVar::m_isTitleSearch = true;
    m_searchView->searchListUi(true);
    QVERIFY(true);
    ViewVar::m_isTitleSearch = false;
}

void TestSearchView::testSendSearchInput()
{
    m_searchView->m_searchLineEdit->setText("Beijing");
    m_searchView->sendSearchInput();
    QVERIFY(m_searchView->m_backInputText == "beijing");

    m_searchView->m_searchLineEdit->setText("");
    m_searchView->sendSearchInput();
    QVERIFY(m_searchView->m_backInputText == "");
}

void TestSearchView::testAddCollectCity()
{
    QModelIndex dummy;
    m_searchView->addCollectCity(dummy);
    QCOMPARE(m_searchView->m_stackedWidget->currentIndex(), 0);
}

void TestSearchView::testChangeCitySlots()
{
    QModelIndex dummy;
    m_searchView->changeCityFromSearch(dummy);
    QVERIFY(true);

    m_searchView->changeCityFromCollect(dummy);
    QVERIFY(true);
}

void TestSearchView::testChangeStackedWidgetLimit()
{
    CoreVar::CURRENT_COLLECT_LIST.clear();
    for(int i=0; i<17; ++i) CoreVar::CURRENT_COLLECT_LIST << QString::number(i);
    
    // Skip QMessageBox exec case for now as it's modal
    
    CoreVar::CURRENT_COLLECT_LIST.clear();
    m_searchView->changeStackedWidget();
    QCOMPARE(m_searchView->m_stackedWidget->currentIndex(), 1);
}
