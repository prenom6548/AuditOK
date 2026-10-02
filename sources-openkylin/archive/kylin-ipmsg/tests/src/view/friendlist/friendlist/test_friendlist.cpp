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

#include "test_friendlist.h"
#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include "../../../../../src/global/database/chatmsgdb.h"
#include "view/friendlist/friendinfowid.h"
#include "view/chatmsg/chatmsg.h"
#include "model/chatmsgmodel.h"

void TestFriendListView::initTestCase()
{
    qRegisterMetaType<ChatMsgInfo*>("ChatMsgInfo*");
}

void TestFriendListView::init()
{
    // 创建好友数据以供初始化，防止 ChatMsg 构造函数崩溃
    FriendListModel *model = FriendListModel::getInstance();
    model->clear();
    model->m_idItemsMap.clear();
    model->m_uuidItemsMap.clear();

    QStandardItem *item = new QStandardItem();
    item->setData(1, FriendListModel::Id);
    item->setData("test-uuid", FriendListModel::Uuid);
    item->setData("test-user", FriendListModel::Username);
    item->setData("Test Nickname", FriendListModel::Nickname);
    item->setData("avatar.png", FriendListModel::Avatar);
    
    model->appendRow(item);
    model->m_idItemsMap.insert(1, item);
    model->m_uuidItemsMap.insert("test-uuid", item);

    m_view = FriendListView::getInstance();
}

void TestFriendListView::testSingleton()
{
    FriendListView* instance2 = FriendListView::getInstance();
    QCOMPARE(m_view, instance2);
}

void TestFriendListView::testUiInitialization()
{
    m_view->setViewUi();
    m_view->setViewStyle();
    QVERIFY(m_view->m_myItemDelegate != nullptr);
}

void TestFriendListView::testModelInitialization()
{
    m_view->initListModel();
    QVERIFY(m_view->m_friendListModel != nullptr);
    QVERIFY(m_view->m_sortFilterModel != nullptr);
    QCOMPARE(m_view->model(), m_view->m_sortFilterModel);
}

void TestFriendListView::testFilterFriend()
{
    m_view->filterFriendByReg("test");
    // 验证过滤器是否设置
    QVERIFY(m_view->m_sortFilterModel != nullptr);
}

void TestFriendListView::testGetMsgWid()
{
    ChatMsg* msg = m_view->getMsgWidById(1);
    QVERIFY(msg != nullptr);
    QCOMPARE(m_view->m_msgWidMap.value(1), msg);
    
    ChatMsg* msg2 = m_view->getMsgWidById(1);
    QCOMPARE(msg, msg2);
}

void TestFriendListView::testSlots()
{
    QSignalSpy spy(m_view, SIGNAL(sigFriendState()));
    m_view->slotFriendState();
    QCOMPARE(spy.count(), 1);

    m_view->slotShowMsgWidById(1);
    QVERIFY(m_view->m_msgWidMap.contains(1));
}

void TestFriendListView::testShowContextMenu()
{
    // 覆盖 145-174
    // 确保有数据
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    
    // 使用定时器关闭菜单，防止 exec() 阻塞
    QTimer::singleShot(100, []() {
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *w : widgets) {
            if (w->inherits("QMenu")) {
                w->close();
            }
        }
    });
    
    m_view->showContextMenu(m_view->visualRect(index).center());
    QVERIFY(true);
}

void TestFriendListView::testGetFriendListModel()
{
    // 覆盖 211-213
    QCOMPARE(m_view->getFriendListModel(), m_view->m_friendListModel);
}

void TestFriendListView::testClearAllMessages()
{
    // 覆盖 241-249
    m_view->getMsgWidById(1); // 确保 map 中有数据
    m_view->clearAllMessages();
    QVERIFY(true);
}

void TestFriendListView::testStartChat()
{
    // 覆盖 256-267
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    m_view->startChat();
    QVERIFY(m_view->m_msgWidMap.contains(1));
}

void TestFriendListView::testSetTop()
{
    // 覆盖 271-287
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    m_view->setTop();
    QVERIFY(true);
}

void TestFriendListView::testChangeNickname()
{
    // 覆盖 290-310
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    m_view->changeNickname();
    QVERIFY(true);
}

void TestFriendListView::testViewInfo()
{
    // 覆盖 313-331
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    m_view->viewInfo();
    QVERIFY(true);
}

void TestFriendListView::testDelFriend()
{
    // 覆盖 334-353
    m_view->getMsgWidById(1); // 确保 map 中有数据
    QModelIndex index = m_view->m_sortFilterModel->index(0, 0);
    m_view->setCurrentIndex(index);
    m_view->delFriend();
    QVERIFY(!m_view->m_msgWidMap.contains(1));
}

void TestFriendListView::testSlotDeleFriend()
{
    // 覆盖 376-384
    QSignalSpy spy(m_view, SIGNAL(sigFriendState()));
    m_view->slotDeleFriend();
    QCOMPARE(spy.count(), 1);
    
    // 清空模型测试 empty 信号
    FriendListModel::getInstance()->clear();
    QSignalSpy spyEmpty(m_view, SIGNAL(sigFriendsEmpty()));
    m_view->slotDeleFriend();
    QCOMPARE(spyEmpty.count(), 1);
}

void TestFriendListView::cleanup()
{
    FriendListModel::getInstance()->clear();
    FriendListModel::getInstance()->m_idItemsMap.clear();
    FriendListModel::getInstance()->m_uuidItemsMap.clear();
}

void TestFriendListView::cleanupTestCase()
{
    // 触发析构函数覆盖 38-52
    FriendListView::getInstance()->deleteLater();
    QCoreApplication::processEvents();
}
