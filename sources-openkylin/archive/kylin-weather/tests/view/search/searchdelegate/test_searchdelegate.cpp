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
#include "test_searchdelegate.h"
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QPainter>
#include <QStandardItemModel>
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QListView>
#include <QStyledItemDelegate>
#include "global/corevar.h"
#include "global/viewvar.h"
#include "controller/Interaction.h"

TestSearchDelegate::TestSearchDelegate(QObject *parent) : QObject(parent)
{
}

void TestSearchDelegate::initTestCase()
{
}

void TestSearchDelegate::cleanupTestCase()
{
}

void TestSearchDelegate::testConstructor()
{
    SearchDelegate* del = new SearchDelegate(nullptr);
    QVERIFY(del != nullptr);
    delete del;
}

void TestSearchDelegate::testPaintAndEvents()
{
    SearchDelegate delegate(nullptr);
    QStandardItemModel model;
    
    // 1. Setup normal city data
    ItemData data;
    data.cityName = "Beijing";
    data.cityId = "101010100";
    
    QStandardItem* item = new QStandardItem();
    item->setData(QVariant::fromValue(data), Qt::UserRole);
    model.appendRow(item);
    
    QModelIndex index = model.index(0, 0);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 50);
    option.state = QStyle::State_Enabled;
    
    QImage image(200, 200, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    
    // 2. Test Paint - Normal state
    delegate.paint(&painter, option, index);
    
    // 3. Test Paint - MouseOver state
    option.state |= QStyle::State_MouseOver;
    delegate.paint(&painter, option, index);
    
    // 4. Test Paint - Selected state
    option.state |= QStyle::State_Selected;
    delegate.paint(&painter, option, index);
    
    // 5. Test Paint - White theme
    ViewVar::s_themeStyle = true;
    delegate.paint(&painter, option, index);
    ViewVar::s_themeStyle = false; // reset

    // 6. Test editorEvent - MouseRelease on Add Button (around x=180)
    QSignalSpy spyAdd(&delegate, SIGNAL(addCollectCity(QModelIndex)));
    QMouseEvent clickAddEvent(QEvent::MouseButtonRelease, QPointF(180, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    delegate.editorEvent(&clickAddEvent, &model, option, index);
    QCOMPARE(spyAdd.count(), 1);
    
    // 7. Test editorEvent - MouseRelease on City Name (around x=50)
    QSignalSpy spyChange(&delegate, SIGNAL(changeCityFromSearch(QModelIndex)));
    QMouseEvent clickChangeEvent(QEvent::MouseButtonRelease, QPointF(50, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    delegate.editorEvent(&clickChangeEvent, &model, option, index);
    QCOMPARE(spyChange.count(), 1);

    painter.end();
    QVERIFY(true);
}

void TestSearchDelegate::testSignals()
{
    SearchDelegate delegate(nullptr);
    const QMetaObject *metaObj = delegate.metaObject();
    QVERIFY(metaObj->indexOfSignal("addCollectCity(QModelIndex)") != -1);
    QVERIFY(metaObj->indexOfSignal("changeCityFromSearch(QModelIndex)") != -1);
}
