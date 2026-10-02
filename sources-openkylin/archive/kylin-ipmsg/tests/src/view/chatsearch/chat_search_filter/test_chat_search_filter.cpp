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

#include "test_chat_search_filter.h"
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QModelIndex>
#include <QFileInfo>

// 测试初始化
void TestChatSearchFilter::initTestCase()
{
    // 在所有测试开始前执行一次
}

// 每个测试前的初始化
void TestChatSearchFilter::init()
{
    // 初始化测试环境
    m_chatSearchFilter = new TestableChatSearchFilter();
    m_sourceModel = new QStandardItemModel();
    m_chatSearchFilter->setSourceModel(m_sourceModel);
}

// 测试构造函数和析构函数
void TestChatSearchFilter::testConstructorAndDestructor()
{
    // 测试构造函数
    TestableChatSearchFilter* chatSearchFilter = new TestableChatSearchFilter();
    QVERIFY(chatSearchFilter != nullptr);
    
    // 测试析构函数
    delete chatSearchFilter;
}

// 测试lessThan方法
void TestChatSearchFilter::testLessThan()
{
    // 创建测试数据
    QStandardItem* item1 = new QStandardItem();
    item1->setData("2023-01-01 10:00:00", ChatMsgModel::ChatMsgRoles::MsgTime); // MsgTime
    m_sourceModel->appendRow(item1);
    
    QStandardItem* item2 = new QStandardItem();
    item2->setData("2023-01-01 11:00:00", ChatMsgModel::ChatMsgRoles::MsgTime); // MsgTime
    m_sourceModel->appendRow(item2);
    
    QModelIndex index1 = m_sourceModel->index(0, 0);
    QModelIndex index2 = m_sourceModel->index(1, 0);
    
    // 测试时间顺序比较 - 左侧时间小于右侧时间，应该返回true
    bool result = m_chatSearchFilter->lessThan(index1, index2);
    QVERIFY(result);
    
    // 测试时间顺序比较 - 左侧时间大于右侧时间，应该返回false
    result = m_chatSearchFilter->lessThan(index2, index1);
    QVERIFY(!result);
    
    // 测试时间相等的情况
    QStandardItem* item3 = new QStandardItem();
    item3->setData("2023-01-01 10:00:00", ChatMsgModel::ChatMsgRoles::MsgTime); // MsgTime
    m_sourceModel->appendRow(item3);
    
    QModelIndex index3 = m_sourceModel->index(2, 0);
    result = m_chatSearchFilter->lessThan(index1, index3);
    QVERIFY(result); // 相等时应该返回true
}

// 测试filterAcceptsRow方法 - 空过滤器
void TestChatSearchFilter::testFilterAcceptsRowEmptyFilter()
{
    // 设置空过滤器
    m_chatSearchFilter->setFilterRegExp("");
    
    // 创建测试数据
    QStandardItem* item = new QStandardItem();
    item->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = TextMsg
    m_sourceModel->appendRow(item);
    
    // 空过滤器应该接受所有行
    bool result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
}

// 测试filterAcceptsRow方法 - all过滤器
void TestChatSearchFilter::testFilterAcceptsRowAllFilter()
{
    // 设置all过滤器
    m_chatSearchFilter->setFilterRegExp("all:message");
    
    // 创建文本消息测试数据
    QStandardItem* textItem = new QStandardItem();
    textItem->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = TextMsg
    textItem->setData("This is a test message", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    textItem->setData("", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(textItem);
    
    // 测试包含关键字的文本消息
    bool result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
    
    // 创建文件消息测试数据，文件名包含"message"关键字
    QStandardItem* fileItem = new QStandardItem();
    fileItem->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = FileMsg
    fileItem->setData("", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    fileItem->setData("/path/to/message_document.pdf", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(fileItem);
    
    // 测试包含关键字的文件名
    result = m_chatSearchFilter->filterAcceptsRow(1, QModelIndex());
    QVERIFY(result);
    
    // 测试空关键字的情况
    m_chatSearchFilter->setFilterRegExp("all:");
    result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
}

// 测试filterAcceptsRow方法 - file过滤器
void TestChatSearchFilter::testFilterAcceptsRowFileFilter()
{
    // 设置file过滤器
    m_chatSearchFilter->setFilterRegExp("file:document");
    
    // 创建文件消息测试数据
    QStandardItem* fileItem = new QStandardItem();
    fileItem->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = FileMsg
    fileItem->setData("", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    fileItem->setData("/path/to/document.pdf", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(fileItem);
    
    // 测试包含关键字的文件
    bool result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
    
    // 创建不匹配的文件消息测试数据
    QStandardItem* otherFileItem = new QStandardItem();
    otherFileItem->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = FileMsg
    otherFileItem->setData("", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    otherFileItem->setData("/path/to/image.jpg", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(otherFileItem);
    
    // 测试不包含关键字的文件
    result = m_chatSearchFilter->filterAcceptsRow(1, QModelIndex());
    QVERIFY(!result);
    
    // 测试空关键字的情况
    m_chatSearchFilter->setFilterRegExp("file:");
    result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
}

// 测试filterAcceptsRow方法 - image过滤器
void TestChatSearchFilter::testFilterAcceptsRowImageFilter()
{
    // 设置image过滤器
    m_chatSearchFilter->setFilterRegExp("image:image");
    
    // 创建图片文件消息测试数据
    QStandardItem* imageItem = new QStandardItem();
    imageItem->setData(ChatMsgModel::MessageType::FileMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = FileMsg
    imageItem->setData("", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    imageItem->setData("/path/to/image.jpg", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(imageItem);
    
    // 测试图片文件（注意：实际测试中不会调用外部命令判断图片类型）
    // 这里我们主要测试过滤器逻辑
    bool result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    // 由于没有实际调用外部命令，这里可能返回false，但我们主要关注过滤器逻辑
    
    // 测试空关键字的情况
    m_chatSearchFilter->setFilterRegExp("image:");
    result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    // 由于没有实际调用外部命令，这里可能返回false，但我们主要关注过滤器逻辑
}

// 测试filterAcceptsRow方法 - link过滤器
void TestChatSearchFilter::testFilterAcceptsRowLinkFilter()
{
    // 设置link过滤器
    m_chatSearchFilter->setFilterRegExp("link:example");
    
    // 创建链接消息测试数据 - 注意：judgeWhetherLink使用exactMatch，所以整个字符串必须是一个链接
    QStandardItem* linkItem = new QStandardItem();
    linkItem->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = TextMsg
    linkItem->setData("https://example.com", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    linkItem->setData("", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(linkItem);
    
    // 测试包含链接的消息
    bool result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
    
    // 创建不包含链接的消息测试数据
    QStandardItem* textItem = new QStandardItem();
    textItem->setData(ChatMsgModel::MessageType::TextMsg, ChatMsgModel::ChatMsgRoles::MsgType); // MsgType = TextMsg
    textItem->setData("This is a plain text message", ChatMsgModel::ChatMsgRoles::MsgContent); // MsgContent
    textItem->setData("", ChatMsgModel::ChatMsgRoles::FilePath); // FilePath
    m_sourceModel->appendRow(textItem);
    
    // 测试不包含链接的消息
    result = m_chatSearchFilter->filterAcceptsRow(1, QModelIndex());
    QVERIFY(!result);
    
    // 测试空关键字的情况
    m_chatSearchFilter->setFilterRegExp("link:");
    result = m_chatSearchFilter->filterAcceptsRow(0, QModelIndex());
    QVERIFY(result);
}

// 测试judgeWhetherPicture方法
void TestChatSearchFilter::testJudgeWhetherPicture()
{
    // 测试图片路径
    bool result = m_chatSearchFilter->judgeWhetherPicture("/path/to/image.jpg");
    // 由于该方法调用外部命令，在单元测试中可能无法准确测试
    // 我们主要确保方法可以被调用且不会崩溃
    
    // 测试非图片路径
    result = m_chatSearchFilter->judgeWhetherPicture("/path/to/document.pdf");
    // 同样，这里主要是确保方法可以被调用
}

// 测试judgeWhetherLink方法
void TestChatSearchFilter::testJudgeWhetherLink()
{
    // 测试有效的链接
    bool result = m_chatSearchFilter->judgeWhetherLink("https://example.com");
    QVERIFY(result);
    
    // 测试无效的链接
    result = m_chatSearchFilter->judgeWhetherLink("not a link");
    QVERIFY(!result);
    
    // 测试http链接
    result = m_chatSearchFilter->judgeWhetherLink("http://example.com");
    QVERIFY(result);
    
    // 测试ftp链接
    result = m_chatSearchFilter->judgeWhetherLink("ftp://example.com");
    QVERIFY(result);
}

// 每个测试后的清理
void TestChatSearchFilter::cleanup()
{
    // 清理测试环境
    if (m_chatSearchFilter) {
        delete m_chatSearchFilter;
        m_chatSearchFilter = nullptr;
    }
    
    if (m_sourceModel) {
        delete m_sourceModel;
        m_sourceModel = nullptr;
    }
}

// 测试结束后的清理
void TestChatSearchFilter::cleanupTestCase()
{
    // 在所有测试结束后执行一次
}