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

#ifndef TEST_CHAT_SEARCH_DELEGATE_H
#define TEST_CHAT_SEARCH_DELEGATE_H

// Qt includes
#include <QObject>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QHelpEvent>
#include <QAbstractItemView>
#include <QtTest/QtTest>

// 前向声明
class ChatMsg;
class ChatSearch;

// 包含被测类的头文件
#include "../../../../../../src/view/chatsearch/chat_search_delegate.h"

// 创建一个可测试的ChatSearchDelegate子类
class TestableChatSearchDelegate : public ChatSearchDelegate
{
    Q_OBJECT

public:
    TestableChatSearchDelegate(ChatMsg *chatMsg) : ChatSearchDelegate(chatMsg) {}
    TestableChatSearchDelegate(ChatMsg *chatMsg, ChatSearch *chatSearch) : ChatSearchDelegate(chatMsg, chatSearch) {}
    
    // 提供公共访问方法来测试受保护的成员
    QSize sizeHintPublic(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return sizeHint(option, index);
    }
    
    void paintPublic(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return paint(painter, option, index);
    }
    
    bool helpEventPublic(QHelpEvent *event, QAbstractItemView *view,
                        const QStyleOptionViewItem &option, const QModelIndex &index)
    {
        return helpEvent(event, view, option, index);
    }
};

// 测试类
class TestChatSearchDelegate : public QObject
{
    Q_OBJECT

public:
    explicit TestChatSearchDelegate(QObject *parent = nullptr) 
        : QObject(parent)
        , m_chatSearchDelegate(nullptr)
        , m_model(nullptr)
        , m_chatMsg(nullptr)
        , m_chatSearch(nullptr)
    {}

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数和析构函数
    void testConstructorAndDestructor();
    
    // 测试sizeHint方法
    void testSizeHint();
    
    // 测试构造函数覆盖
    void testConstructorCoverage();
    
    // 测试paint方法 - 异常情况
    void testPaintExceptions();
    
    // 测试paint方法 - 文本消息
    void testPaintTextMessage();
    
    // 测试paint方法 - 文件消息
    void testPaintFileMessage();
    
    // 测试paint方法 - 目录消息
    void testPaintDirMessage();
    
    // 测试paint方法 - 鼠标悬停状态
    void testPaintMouseOver();
    
    // 测试paint方法 - 选中状态
    void testPaintHasFocus();
    
    // 测试paint方法 - 批量删除状态
    void testPaintDeleteState();

    // 测试paint方法 - 批量删除状态下的文件/目录消息
    void testPaintDeleteState_FileDirMessage();
    
    // 测试helpEvent方法
    void testHelpEvent();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    TestableChatSearchDelegate* m_chatSearchDelegate;
    QStandardItemModel* m_model;
    ChatMsg* m_chatMsg;
    ChatSearch* m_chatSearch;
};

#endif // TEST_CHAT_SEARCH_DELEGATE_H