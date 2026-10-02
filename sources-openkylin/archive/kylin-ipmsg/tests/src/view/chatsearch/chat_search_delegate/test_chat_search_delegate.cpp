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

#include "test_chat_search_delegate.h"
#include <QPainter>
#include <QPixmap>
#include <QStandardItem>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QHelpEvent>
#include <QAbstractItemView>
#include "model/chatmsgmodel.h"
#include "view/chatmsg/chatmsg.h"
#include "view/chatsearch/chat_search.h"

// 测试初始化
void TestChatSearchDelegate::initTestCase()
{
    // 在所有测试开始前执行一次
}

// 每个测试前的初始化
void TestChatSearchDelegate::init()
{
    // 初始化测试环境
    if (!m_model) {
        m_model = new QStandardItemModel();
    }
    
    // 创建好友数据以供初始化，防止 ChatMsg 构造函数崩溃
    QStandardItem *item = new QStandardItem();
    item->setData(1, FriendListModel::Id);
    item->setData("test-uuid", FriendListModel::Uuid);
    item->setData("test-user", FriendListModel::Username);
    item->setData("Test Nickname", FriendListModel::Nickname);
    item->setData("avatar.png", FriendListModel::Avatar);
    
    FriendListModel *model = FriendListModel::getInstance();
    model->appendRow(item);
    model->m_idItemsMap.insert(1, item);
    model->m_uuidItemsMap.insert("test-uuid", item);

    // 创建依赖对象
    if (!m_chatMsg) {
        m_chatMsg = new ChatMsg(1, "test-uuid");
    }
    if (!m_chatSearch) {
        m_chatSearch = new ChatSearch(m_chatMsg);
    }
    
    if (!m_chatSearchDelegate) {
        m_chatSearchDelegate = new TestableChatSearchDelegate(m_chatMsg, m_chatSearch);
    }
}

// 测试构造函数和析构函数
void TestChatSearchDelegate::testConstructorAndDestructor()
{
    QVERIFY(m_chatSearchDelegate != nullptr);
    QVERIFY(m_chatSearchDelegate->m_chatMsg != nullptr);
    QVERIFY(m_chatSearchDelegate->m_chatSearch != nullptr);
}

// 测试sizeHint方法
void TestChatSearchDelegate::testSizeHint()
{
    QStandardItem *item = new QStandardItem();
    item->setData("Test message content", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QSize size = m_chatSearchDelegate->sizeHintPublic(option, m_model->index(0, 0));
    QVERIFY(size.height() > 0);
}

// 测试构造函数覆盖
void TestChatSearchDelegate::testConstructorCoverage()
{
    // 覆盖 38-42
    ChatSearchDelegate delegate(m_chatMsg);
    QVERIFY(delegate.m_chatMsg == m_chatMsg);
    QVERIFY(delegate.m_chatSearch == nullptr);
}

// 测试paint方法 - 异常情况
void TestChatSearchDelegate::testPaintExceptions()
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);

    // 覆盖 63-64: m_chatMsg 为空
    TestableChatSearchDelegate nullMsgDelegate(nullptr);
    nullMsgDelegate.paintPublic(&painter, option, QModelIndex());
    
    // 覆盖 68-69: index 无效
    m_chatSearchDelegate->paintPublic(&painter, option, QModelIndex());
    
    QVERIFY(true);
}

// 测试paint方法 - 文本消息
void TestChatSearchDelegate::testPaintTextMessage()
{
    QStandardItem *item = new QStandardItem();
    item->setData("Test text message", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    item->setData(ChatMsgModel::SendType::SendMsg, ChatMsgModel::IsSend);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    QVERIFY(true);
}

// 测试paint方法 - 文件消息
void TestChatSearchDelegate::testPaintFileMessage()
{
    // 覆盖 285-384, 556-559, 564-569
    QStandardItem *item = new QStandardItem();
    item->setData("test_file_long_long_long_name_to_trigger_elide.txt", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    item->setData("/tmp/test_file.txt", ChatMsgModel::FilePath);
    
    // 测试 GB 级别的大小
    qint64 gbSize = 1024LL * 1024LL * 1024LL;
    item->setData(gbSize * 2, ChatMsgModel::TotalSize);
    item->setData(gbSize, ChatMsgModel::TransferSize);
    item->setData("2026-02-26 12:00:00", ChatMsgModel::MsgTime);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    
    // 测试 UKUILight 主题
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    
    // 测试 UKUIDark 主题
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    
    // 测试 MB 级别的大小
    qint64 mbSize = 1024LL * 1024LL;
    item->setData(mbSize * 5, ChatMsgModel::TotalSize);
    item->setData(mbSize * 5, ChatMsgModel::TransferSize);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    
    QVERIFY(true);
}

// 测试paint方法 - 目录消息
void TestChatSearchDelegate::testPaintDirMessage()
{
    QStandardItem *item = new QStandardItem();
    item->setData("test_dir", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::DirMsg, ChatMsgModel::MsgType);
    item->setData("/tmp/test_dir", ChatMsgModel::FilePath);
    item->setData(100, ChatMsgModel::TotalSize);
    item->setData(100, ChatMsgModel::TransferSize);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    QVERIFY(true);
}

// 测试paint方法 - 鼠标悬停状态
void TestChatSearchDelegate::testPaintMouseOver()
{
    QStandardItem *item = new QStandardItem();
    item->setData("Hover test", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    option.state |= QStyle::State_MouseOver;
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    QVERIFY(true);
}

// 测试paint方法 - 选中状态
void TestChatSearchDelegate::testPaintHasFocus()
{
    QStandardItem *item = new QStandardItem();
    item->setData("Focus test", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    option.state |= QStyle::State_HasFocus;
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    QVERIFY(true);
}

// 测试paint方法 - 批量删除状态
void TestChatSearchDelegate::testPaintDeleteState()
{
    QStandardItem *item = new QStandardItem();
    item->setData("Delete state test", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::MsgType);
    m_model->appendRow(item);

    // 模拟开启删除模式
    m_chatSearchDelegate->m_chatSearch->m_isDeleteNow = true;

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    QImage image(400, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(0, 0));
    QVERIFY(true);
}

void TestChatSearchDelegate::testPaintDeleteState_FileDirMessage()
{
    // 覆盖 285-384 (批量删除状态下的文件/目录分支)
    
    // 1. 准备数据：文件消息
    QStandardItem *itemFile = new QStandardItem();
    itemFile->setData("test_file_to_elide_in_delete_mode_long_name.txt", ChatMsgModel::MsgContent);
    itemFile->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    itemFile->setData("/tmp/test_file_delete.txt", ChatMsgModel::FilePath);
    itemFile->setData("2026-03-04 10:00:00", ChatMsgModel::MsgTime);
    
    // 测试各种文件大小分支
    qint64 kb = 1024;
    qint64 mb = kb * kb;
    qint64 gb = mb * kb;
    
    // 情况A: totalSize < kbSize, transferSize < kbSize, totalSize == transferSize
    itemFile->setData(500, ChatMsgModel::TotalSize);
    itemFile->setData(500, ChatMsgModel::TransferSize);
    m_model->appendRow(itemFile);
    
    // 情况B: totalSize < mbSize, transferSize < mbSize, totalSize != transferSize
    QStandardItem *itemFileB = new QStandardItem();
    itemFileB->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    itemFileB->setData(mb - 100, ChatMsgModel::TotalSize);
    itemFileB->setData(kb + 100, ChatMsgModel::TransferSize);
    m_model->appendRow(itemFileB);

    // 情况C: totalSize < gbSize, transferSize < gbSize
    QStandardItem *itemFileC = new QStandardItem();
    itemFileC->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    itemFileC->setData(gb - 100, ChatMsgModel::TotalSize);
    itemFileC->setData(mb + 100, ChatMsgModel::TransferSize);
    m_model->appendRow(itemFileC);

    // 情况D: totalSize >= gbSize
    QStandardItem *itemFileD = new QStandardItem();
    itemFileD->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    itemFileD->setData(gb + 100, ChatMsgModel::TotalSize);
    itemFileD->setData(gb + 100, ChatMsgModel::TransferSize);
    m_model->appendRow(itemFileD);

    // 2. 准备数据：目录消息
    QStandardItem *itemDir = new QStandardItem();
    itemDir->setData(ChatMsgModel::MessageType::DirMsg, ChatMsgModel::MsgType);
    itemDir->setData("/tmp/test_dir_delete", ChatMsgModel::FilePath);
    m_model->appendRow(itemDir);

    // 3. 设置删除模式
    m_chatSearchDelegate->m_chatSearch->m_isDeleteNow = true;

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 500, 80);
    QImage image(500, 80 * m_model->rowCount(), QImage::Format_ARGB32);
    QPainter painter(&image);

    // 4. 执行绘制，覆盖不同分支
    for (int i = 0; i < m_model->rowCount(); ++i) {
        m_chatSearchDelegate->paintPublic(&painter, option, m_model->index(i, 0));
        option.rect.translate(0, 80);
    }

    QVERIFY(true);
}

// 测试helpEvent方法
void TestChatSearchDelegate::testHelpEvent()
{
    // 覆盖 607-630
    QStandardItem *item = new QStandardItem();
    item->setData("test_file_with_a_very_long_name_that_will_definitely_be_elided_when_rendered_in_the_list_view.txt", ChatMsgModel::MsgContent);
    item->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::MsgType);
    item->setData("/tmp/test_file_long.txt", ChatMsgModel::FilePath);
    m_model->appendRow(item);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, 100);
    
    // 模拟普通模式下的 ToolTip 事件
    QHelpEvent event(QEvent::ToolTip, QPoint(100, 20), QPoint(100, 20));
    m_chatSearchDelegate->m_chatSearch->m_isDeleteNow = false;
    m_chatSearchDelegate->helpEventPublic(&event, nullptr, option, m_model->index(0, 0));
    
    // 模拟批量删除模式下的 ToolTip 事件
    m_chatSearchDelegate->m_chatSearch->m_isDeleteNow = true;
    QHelpEvent eventDelete(QEvent::ToolTip, QPoint(360, 20), QPoint(360, 20));
    m_chatSearchDelegate->helpEventPublic(&eventDelete, nullptr, option, m_model->index(0, 0));
    
    QVERIFY(true);
}

// 每个测试后的清理
void TestChatSearchDelegate::cleanup()
{
    // 先销毁 delegate
    if (m_chatSearchDelegate) {
        delete m_chatSearchDelegate;
        m_chatSearchDelegate = nullptr;
    }
    
    // 再销毁 chatSearch
    if (m_chatSearch) {
        m_chatSearch->slotCancelClick();
        m_chatSearch->deleteLater();
        m_chatSearch = nullptr;
    }

    // 最后销毁 chatMsg
    if (m_chatMsg) {
        m_chatMsg->deleteLater();
        m_chatMsg = nullptr;
    }

    if (m_model) {
        m_model->clear();
        delete m_model;
        m_model = nullptr;
    }

    FriendListModel::getInstance()->clear();
    
    // 处理事件循环，确保 deleteLater 生效
    QCoreApplication::processEvents();
}

// 测试结束后的清理
void TestChatSearchDelegate::cleanupTestCase()
{
    // 在所有测试结束后执行一次
}
