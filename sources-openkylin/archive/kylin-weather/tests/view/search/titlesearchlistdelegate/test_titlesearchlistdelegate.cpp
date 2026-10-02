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
#include "test_titlesearchlistdelegate.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QPainter>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QStyleOption>
#include <QSignalSpy>
#include "model/titlesearchmodel.h"
#include "global/viewvar.h"

void TestTitleSearchListDelegate::initTestCase()
{
}

void TestTitleSearchListDelegate::cleanupTestCase()
{
}

void TestTitleSearchListDelegate::init()
{
    m_delegate = new TitleSearchListDelegate();
    m_model = new QStandardItemModel();
}

void TestTitleSearchListDelegate::cleanup()
{
    delete m_delegate;
    delete m_model;
}

void TestTitleSearchListDelegate::testConstructor()
{
    QVERIFY(m_delegate != nullptr);
}

void TestTitleSearchListDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QSize size = m_delegate->sizeHint(option, index);
    QCOMPARE(size, QSize(292, 48));
}

void TestTitleSearchListDelegate::testPaint()
{
    QImage image(300, 300, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 292, 48);
    option.state = QStyle::State_Enabled;
    
    // Test with city data
    QStandardItem* item = new QStandardItem();
    item->setData("Beijing", TitleSearchModel::CityName);
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // Normal state
    m_delegate->paint(&painter, option, index);
    
    // MouseOver state
    option.state |= QStyle::State_MouseOver;
    m_delegate->paint(&painter, option, index);
    
    // Selected state
    option.state |= QStyle::State_Selected;
    m_delegate->paint(&painter, option, index);
    
    // White theme
    ViewVar::s_themeStyle = true;
    m_delegate->paint(&painter, option, index);
    ViewVar::s_themeStyle = false;

    // Long city name
    item->setData("Very Long City Name That Should Be Elided", TitleSearchModel::CityName);
    m_delegate->paint(&painter, option, index);

    painter.end();
    QVERIFY(true);
}

void TestTitleSearchListDelegate::testEditorEvent()
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 292, 48);
    
    QStandardItem* item = new QStandardItem();
    item->setData("Beijing", TitleSearchModel::CityName);
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // MouseRelease event
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(50, 24), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QSignalSpy spy(m_delegate, SIGNAL(changeCityFromSearch(QModelIndex)));
    
    bool handled = m_delegate->editorEvent(&releaseEvent, m_model, option, index);
    QVERIFY(handled);
    QCOMPARE(spy.count(), 1);
    
    // Other event types
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(50, 24), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    handled = m_delegate->editorEvent(&moveEvent, m_model, option, index);
    QVERIFY(handled);
}

void TestTitleSearchListDelegate::testSignals()
{
    const QMetaObject *metaObj = m_delegate->metaObject();
    QVERIFY(metaObj->indexOfSignal("changeCityFromSearch(QModelIndex)") != -1);
}
