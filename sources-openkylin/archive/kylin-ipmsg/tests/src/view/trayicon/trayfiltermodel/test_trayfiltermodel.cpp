/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "test_trayfiltermodel.h"
#include <QStandardItemModel>
#include "model/friendlistmodel.h"

void TestTrayFilterModel::initTestCase()
{
}

void TestTrayFilterModel::cleanupTestCase()
{
}

void TestTrayFilterModel::init()
{
    m_model = new TrayFilterModel();
}

void TestTrayFilterModel::cleanup()
{
    delete m_model;
}

void TestTrayFilterModel::testConstructor()
{
    QVERIFY(m_model != nullptr);
}

void TestTrayFilterModel::testFilterAcceptsRow()
{
    QStandardItemModel sourceModel;
    QStandardItem *item1 = new QStandardItem("Item 1");
    item1->setData(5, FriendListModel::UnreadMsgNum); // 设置未读消息数 > 0
    sourceModel.appendRow(item1);
    
    QStandardItem *item2 = new QStandardItem("Item 2");
    item2->setData(0, FriendListModel::UnreadMsgNum); // 设置未读消息数 = 0
    sourceModel.appendRow(item2);
    
    m_model->setSourceModel(&sourceModel);
    
    // 测试 filterAcceptsRow (覆盖第55行)
    QVERIFY(m_model->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(!m_model->filterAcceptsRow(1, QModelIndex()));
}

void TestTrayFilterModel::testLessThan()
{
    QStandardItemModel sourceModel;
    QStandardItem *itemA = new QStandardItem("A");
    // 设置不同的时间以覆盖 lessThan 逻辑 (覆盖 26-49)
    // 使用正确的格式 "yyyy-MM-dd HH:mm:ss.zzz"
    itemA->setData("2023-10-27 10:00:00.000", FriendListModel::RecentMsgTime);
    
    QStandardItem *itemB = new QStandardItem("B");
    itemB->setData("2023-10-27 09:00:00.000", FriendListModel::RecentMsgTime);
    
    sourceModel.appendRow(itemA);
    sourceModel.appendRow(itemB);
    m_model->setSourceModel(&sourceModel);
    
    QModelIndex indexA = sourceModel.index(0, 0);
    QModelIndex indexB = sourceModel.index(1, 0);
    
    // 测试 lessThan
    // 根据源码：if (compareDataL > compareDataR) return true;
    // 时间 A 晚于时间 B，所以 compareDataL > compareDataR 应该为 true
    bool result = m_model->lessThan(indexA, indexB);
    QVERIFY(result);
    
    bool resultReverse = m_model->lessThan(indexB, indexA);
    QVERIFY(!resultReverse);
}
