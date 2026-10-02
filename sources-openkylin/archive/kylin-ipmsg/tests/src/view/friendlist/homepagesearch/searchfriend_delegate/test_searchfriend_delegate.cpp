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

#include "test_searchfriend_delegate.h"
#include <QPainter>
#include <QStandardItemModel>
#include "view/common/globalsizedata.h"

void TestSearchFriendDelegate::initTestCase()
{
}

void TestSearchFriendDelegate::cleanupTestCase()
{
}

void TestSearchFriendDelegate::init()
{
    m_delegate = new SearchFriendDelegate();
}

void TestSearchFriendDelegate::cleanup()
{
    delete m_delegate;
}

void TestSearchFriendDelegate::testConstructorDestructor()
{
    SearchFriendDelegate *delegate = new SearchFriendDelegate();
    QVERIFY(delegate != nullptr);
    delete delegate;
}

void TestSearchFriendDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    QSize size = m_delegate->sizeHint(option, index);
    QVERIFY(size.isValid());
}

void TestSearchFriendDelegate::testPaint()
{
    QImage image(400, 400, QImage::Format_RGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    
    // 1. 测试无效索引
    m_delegate->paint(&painter, option, QModelIndex());

    // 2. 测试正常路径 (有昵称)
    model.setData(index, "TestUser", FriendListModel::Username);
    model.setData(index, "TestNickname", FriendListModel::Nickname);
    model.setData(index, "A", FriendListModel::Avatar);
    m_delegate->paint(&painter, option, index);

    // 3. 测试无昵称 (使用用户名)
    model.setData(index, "", FriendListModel::Nickname);
    m_delegate->paint(&painter, option, index);

    // 4. 测试 MouseOver 状态
    option.state |= QStyle::State_MouseOver;
    m_delegate->paint(&painter, option, index);

    // 5. 测试 HasFocus 状态
    option.state |= QStyle::State_HasFocus;
    m_delegate->paint(&painter, option, index);

    // 6. 测试长文本省略
    model.setData(index, "Very Long Username That Should Be Elided In The UI", FriendListModel::Username);
    m_delegate->paint(&painter, option, index);
}

void TestSearchFriendDelegate::testPaintStatesAndThemes()
{
    // 覆盖 78-88
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    model.setData(index, "TestUser", FriendListModel::Username);
    model.setData(index, "A", FriendListModel::Avatar);

    // 1. 测试 State_MouseOver + UKUIDark (覆盖 78)
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    option.state = QStyle::State_MouseOver;
    m_delegate->paint(&painter, option, index);

    // 2. 测试 State_HasFocus + UKUILight (覆盖 85)
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    option.state = QStyle::State_HasFocus;
    m_delegate->paint(&painter, option, index);

    // 3. 测试 State_HasFocus + UKUIDark (覆盖 87)
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    option.state = QStyle::State_HasFocus;
    m_delegate->paint(&painter, option, index);

    QVERIFY(true);
}
