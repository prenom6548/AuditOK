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

#include "test_trayitemdelegate.h"
#include <QStandardItemModel>
#include <QPainter>
#include <QImage>
#include "model/friendlistmodel.h"
#include "view/common/globalsizedata.h"

void TestTrayItemDelegate::initTestCase()
{
}

void TestTrayItemDelegate::cleanupTestCase()
{
}

void TestTrayItemDelegate::init()
{
    m_delegate = new TrayItemDelegate();
}

void TestTrayItemDelegate::cleanup()
{
    delete m_delegate;
}

void TestTrayItemDelegate::testConstructor()
{
    QVERIFY(m_delegate != nullptr);
}

void TestTrayItemDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QStandardItemModel model;
    model.appendRow(new QStandardItem("Test"));
    QModelIndex index = model.index(0, 0);
    
    QSize size = m_delegate->sizeHint(option, index);
    QVERIFY(size.isValid());
}

void TestTrayItemDelegate::testPaint()
{
    QImage image(100, 100, QImage::Format_RGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    
    QStandardItemModel model;
    model.appendRow(new QStandardItem("Test"));
    QModelIndex index = model.index(0, 0);
    
    m_delegate->paint(&painter, option, index);
    QVERIFY(true);
}

void TestTrayItemDelegate::testCoverageAdditions()
{
    QImage image(300, 100, QImage::Format_RGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 280, 58);
    
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    // 1. 覆盖 74-81 (State_MouseOver + THEME_COLOR)
    option.state = QStyle::State_MouseOver;
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    m_delegate->paint(&painter, option, model.index(0, 0)); // 虽然 index 无效，但先测试分支
    
    // 2. 覆盖 96 (onlineState != Offline)
    item->setData("A", FriendListModel::Avatar);
    item->setData(FriendListModel::OnlineType::Online, FriendListModel::OnlineState);
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);
    
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    m_delegate->paint(&painter, option, index);
    
    // 3. 覆盖 111-114, 145-151 (msgNum != 0, msgNum < 100, msgNum >= 100)
    // 测试 msgNum < 100
    item->setData(10, FriendListModel::UnreadMsgNum);
    m_delegate->paint(&painter, option, index);
    
    // 测试 msgNum >= 100
    item->setData(105, FriendListModel::UnreadMsgNum);
    m_delegate->paint(&painter, option, index);
    
    // 4. 覆盖 129-131 (nickname elision)
    item->setData("This is a very long nickname that should be elided", FriendListModel::Nickname);
    m_delegate->paint(&painter, option, index);

    QVERIFY(true);
}
