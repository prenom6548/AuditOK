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

#include "test_searchmsglist.h"
#include <QStandardItem>
#include "model/chatmsgmodel.h"
#include <QSignalSpy>

void TestSearchMsgList::initTestCase()
{
}

void TestSearchMsgList::cleanupTestCase()
{
}

void TestSearchMsgList::init()
{
    m_list = SearchMsgList::getInstance();
}

void TestSearchMsgList::cleanup()
{
}

void TestSearchMsgList::testConstructorDestructor()
{
    SearchMsgList *list = new SearchMsgList();
    QVERIFY(list != nullptr);
    delete list;
}

void TestSearchMsgList::testGetInstance()
{
    SearchMsgList *instance1 = SearchMsgList::getInstance();
    QVERIFY(instance1 != nullptr);
    SearchMsgList *instance2 = SearchMsgList::getInstance();
    QCOMPARE(instance1, instance2);
}

void TestSearchMsgList::testSetViewUi()
{
    m_list->setViewUi();
    QVERIFY(m_list->m_chatMsgDelegate != nullptr);
}

void TestSearchMsgList::testFilterFriendByReg()
{
    QSignalSpy spy(m_list, &SearchMsgList::sigSelectMsg);
    
    // 1. 测试空字符串 (应该直接返回，不发送信号)
    m_list->filterFriendByReg("");
    QCOMPARE(spy.count(), 0);

    // 2. 测试有效字符串
    m_list->filterFriendByReg("test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString("test"));
}

void TestSearchMsgList::testStartChat()
{
    // 1. 测试无效索引
    m_list->setCurrentIndex(QModelIndex());
    QSignalSpy spy(m_list, &SearchMsgList::sigStartChat);
    m_list->startChat();
    // 注意：如果 currentIndex 无效，data() 返回无效 QVariant，toInt() 为 0
    // 逻辑上应该发射 sigStartChat(0)，除非代码里有有效性检查
    // 根据源码：int friendId = friendIndex.data(ChatMsgModel::FriendId).toInt();
    // 如果 friendIndex 无效，friendId 为 0。
    // 所以 spy.count() 应该是 1
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);

    // 2. 测试有效索引
    SearchMsgModel *model = SearchMsgModel::getInstance();
    model->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(1, ChatMsgModel::FriendId);
    model->appendRow(item);
    
    m_list->setModel(model);
    m_list->setCurrentIndex(model->index(0, 0));
    
    m_list->startChat();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toInt(), 1);
}
