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

#include "test_chatmsgdelegate.h"
#include <QFont>
#include <QStandardItemModel>
#include <QPainter>
#include <QImage>
#include "model/chatmsgmodel.h"
#include "model/friendlistmodel.h"
#include "view/localinfo/localinfo.h"
#include "global/utils/global_data.h"

TestChatMsgDelegate::TestChatMsgDelegate(QObject *parent)
    : QObject(parent)
{
}

TestChatMsgDelegate::~TestChatMsgDelegate()
{
}

void TestChatMsgDelegate::initTestCase()
{
    // 初始化全局数据，避免 paint 中 getInstance() 崩溃
    GlobalData::getInstance();
}

void TestChatMsgDelegate::cleanupTestCase()
{
}

void TestChatMsgDelegate::testSizeFormat()
{
    QCOMPARE(ChatMsgDelegate::sizeFormat(500), QString("500 B"));
    QCOMPARE(ChatMsgDelegate::sizeFormat(1024), QString("1.0 KB"));
    QCOMPARE(ChatMsgDelegate::sizeFormat(1024 * 1024), QString("1.0 MB"));
    QCOMPARE(ChatMsgDelegate::sizeFormat(1024LL * 1024 * 1024), QString("1.0 GB"));
}

void TestChatMsgDelegate::testGetMsgRectSize()
{
    QFont font;
    font.setPixelSize(14);
    
    // 测试短文本
    QSize size1 = ChatMsgDelegate::getMsgRectSize("hello", font, 200);
    QVERIFY(size1.width() > 0);
    QVERIFY(size1.height() > 0);
    
    // 测试带换行符的文本
    QSize size2 = ChatMsgDelegate::getMsgRectSize("hello\nworld", font, 200);
    QVERIFY(size2.height() > size1.height());
    
    // 测试长文本自动换行
    QString longText = "This is a very long text that should definitely wrap when the width is small enough.";
    QSize size3 = ChatMsgDelegate::getMsgRectSize(longText, font, 50);
    QVERIFY(size3.height() > size1.height());
}

void TestChatMsgDelegate::testSizeHint()
{
    ChatMsgDelegate delegate;
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    // 设置文本消息数据
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    item->setData("Test message content", ChatMsgModel::MsgContent);
    model.appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QSize size = delegate.sizeHint(option, model.index(0, 0));
    QVERIFY(size.height() > 0);
    
    // 设置时间消息数据
    item->setData(ChatMsgModel::MessageType::TimeMsg, ChatMsgModel::MsgType);
    size = delegate.sizeHint(option, model.index(0, 0));
    QCOMPARE(size.height(), 25);
}

void TestChatMsgDelegate::testPaintTimeMsg()
{
    ChatMsgDelegate delegate;
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    item->setData(ChatMsgModel::MessageType::TimeMsg, ChatMsgModel::MsgType);
    item->setData("2023-10-27T10:00:00", ChatMsgModel::MsgTime);
    model.appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 30);
    
    QImage image(400, 30, QImage::Format_ARGB32);
    QPainter painter(&image);
    delegate.paint(&painter, option, model.index(0, 0));
    QVERIFY(true);
}

void TestChatMsgDelegate::testPaintTextMsgRecv()
{
    ChatMsgDelegate delegate;
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    // 准备好友数据，避免 paint 中 getFriendById 崩溃
    FriendListModel *fmodel = FriendListModel::getInstance();
    QStandardItem *fitem = new QStandardItem();
    fitem->setData(1, FriendListModel::Id);
    fitem->setData("Avatar", FriendListModel::Avatar);
    fmodel->appendRow(fitem);
    fmodel->m_idItemsMap.insert(1, fitem);

    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    item->setData("Hello from friend", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::SendType::RecvMsg, ChatMsgModel::IsSend);
    item->setData(1, ChatMsgModel::FriendId);
    model.appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    delegate.paint(&painter, option, model.index(0, 0));
    QVERIFY(true);
}

void TestChatMsgDelegate::testPaintTextMsgSend()
{
    ChatMsgDelegate delegate;
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    item->setData("Hello to friend", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::SendType::SendMsg, ChatMsgModel::IsSend);
    item->setData(ChatMsgModel::MesssageState::SendSucceed, ChatMsgModel::SendState);
    model.appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    delegate.paint(&painter, option, model.index(0, 0));
    QVERIFY(true);
}

void TestChatMsgDelegate::testPaintFileMsg()
{
    ChatMsgDelegate delegate;
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem();
    
    item->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    item->setData("test_file.txt", ChatMsgModel::MsgContent);
    item->setData("/tmp/test_file.txt", ChatMsgModel::FilePath);
    item->setData(ChatMsgModel::SendType::SendMsg, ChatMsgModel::IsSend);
    item->setData(1024, ChatMsgModel::TotalSize);
    item->setData(1024, ChatMsgModel::TransferSize);
    model.appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    delegate.paint(&painter, option, model.index(0, 0));
    QVERIFY(true);
}
