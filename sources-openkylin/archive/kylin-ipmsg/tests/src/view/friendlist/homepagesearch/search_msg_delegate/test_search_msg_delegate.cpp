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

#include "test_search_msg_delegate.h"
#include <QPainter>
#include <QStandardItemModel>
#include "model/chatmsgmodel.h"

void TestSearchMsgDelegate::initTestCase()
{
}

void TestSearchMsgDelegate::cleanupTestCase()
{
}

void TestSearchMsgDelegate::init()
{
    m_delegate = new SearchMsgDelegate();
}

void TestSearchMsgDelegate::cleanup()
{
    delete m_delegate;
}

void TestSearchMsgDelegate::testConstructorDestructor()
{
    SearchMsgDelegate *delegate = new SearchMsgDelegate();
    QVERIFY(delegate != nullptr);
    delete delegate;
}

void TestSearchMsgDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    QSize size = m_delegate->sizeHint(option, index);
    QVERIFY(size.isValid());
}

void TestSearchMsgDelegate::testPaint()
{
    QImage image(400, 400, QImage::Format_RGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    
    // 1. 测试无效索引
    m_delegate->paint(&painter, option, QModelIndex());

    // 2. 测试正常路径
    model.setData(index, 1, ChatMsgModel::FriendId);
    model.setData(index, 10, ChatMsgModel::MsgCount);
    
    // 模拟好友列表数据，因为 paint 内部调用了 getFriendById
    FriendListModel *friendModel = FriendListModel::getInstance();
    QStandardItem *friendItem = new QStandardItem();
    friendItem->setData(1, FriendListModel::Id);
    friendItem->setData("Alice", FriendListModel::Username);
    friendItem->setData("A", FriendListModel::Avatar);
    friendModel->appendRow(friendItem);
    friendModel->m_idItemsMap.insert(1, friendItem);

    m_delegate->paint(&painter, option, index);

    // 3. 测试 MouseOver 状态
    option.state |= QStyle::State_MouseOver;
    m_delegate->paint(&painter, option, index);

    // 4. 测试 HasFocus 状态
    option.state &= ~QStyle::State_MouseOver;
    option.state |= QStyle::State_HasFocus;
    m_delegate->paint(&painter, option, index);

    // 5. 测试 getFriendById 返回 nullptr
    model.setData(index, 999, ChatMsgModel::FriendId);
    m_delegate->paint(&painter, option, index);
}

void TestSearchMsgDelegate::testFindHash()
{
    m_delegate->m_friendIDHash.insert(1, 10);
    QVERIFY(m_delegate->findHash(1, 10));
    QVERIFY(!m_delegate->findHash(1, 5));
}
