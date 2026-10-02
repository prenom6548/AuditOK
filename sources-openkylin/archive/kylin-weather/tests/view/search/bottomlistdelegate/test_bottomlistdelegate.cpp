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
#include "test_bottomlistdelegate.h"
#include <QtTest>
#include <QSignalSpy>
#include <QPainter>
#include <QStandardItemModel>
#include <QApplication>
#include <QWidget>
#include "model/titlesearchmodel.h"
#include "global/corevar.h"

// 注册枚举类型
Q_DECLARE_METATYPE(TitleSearchModel::CityType)

TestBottomListDelegate::TestBottomListDelegate(QObject *parent) : QObject(parent)
{
}

void TestBottomListDelegate::initTestCase()
{
    // 注册类型
    qRegisterMetaType<TitleSearchModel::CityType>("TitleSearchModel::CityType");
    delegate = new BottomListDelegate();
}

void TestBottomListDelegate::cleanupTestCase()
{
    delete delegate;
}

void TestBottomListDelegate::testConstructor()
{
    QVERIFY(delegate != nullptr);
}

void TestBottomListDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QSize size = delegate->sizeHint(option, index);
    QCOMPARE(size, QSize(30, 25));
}

void TestBottomListDelegate::testPaint_data()
{
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<int>("cityType");
    QTest::addColumn<QString>("cityId");
    
    QTest::newRow("invalid index") << false << 0 << "";
    QTest::newRow("valid location type") << true << (int)TitleSearchModel::Location << "101010100";
    QTest::newRow("valid collection type") << true << (int)TitleSearchModel::Collection << "101010100";
    QTest::newRow("valid location type empty id") << true << (int)TitleSearchModel::Location << "";
    QTest::newRow("valid collection type empty id") << true << (int)TitleSearchModel::Collection << "";
}

void TestBottomListDelegate::testPaint()
{
    QFETCH(bool, isValid);
    QFETCH(int, cityType);
    QFETCH(QString, cityId);
    
    // 创建模型和索引
    QStandardItemModel model;
    QModelIndex index;
    
    if (isValid) {
        QStandardItem* item = new QStandardItem("Test Item");
        item->setData(cityType, TitleSearchModel::CityStyle);
        item->setData(cityId, TitleSearchModel::CityId);
        model.appendRow(item);
        index = model.index(0, 0);
    }
    
    // 创建绘图设备
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    
    // 调用paint方法
    delegate->paint(&painter, option, index);
    
    // 验证没有崩溃
    QVERIFY(true);
}