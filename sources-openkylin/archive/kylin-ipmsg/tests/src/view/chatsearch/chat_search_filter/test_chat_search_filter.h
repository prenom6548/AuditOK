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

#ifndef TEST_CHAT_SEARCH_FILTER_H
#define TEST_CHAT_SEARCH_FILTER_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include "../../../../../../src/view/chatsearch/chat_search_filter.h"
#include "../../../../../../src/model/chatmsgmodel.h"

// 创建一个可测试的ChatSearchFilter子类
class TestableChatSearchFilter : public ChatSearchFilter
{
    Q_OBJECT

public:
    // 提供公共访问方法来测试受保护的成员
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
        return ChatSearchFilter::lessThan(source_left, source_right);
    }
    
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        return ChatSearchFilter::filterAcceptsRow(source_row, source_parent);
    }
    
    bool judgeWhetherPicture(const QString& filePath) const
    {
        return ChatSearchFilter::judgeWhetherPicture(filePath);
    }
    
    bool judgeWhetherLink(const QString& content) const
    {
        return ChatSearchFilter::judgeWhetherLink(content);
    }
};

class TestChatSearchFilter : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化和清理
    void initTestCase();    // 在所有测试开始前执行一次
    void init();            // 在每个测试前执行
    void cleanup();         // 在每个测试后执行
    void cleanupTestCase(); // 在所有测试结束后执行一次
    
    // 测试用例
    void testConstructorAndDestructor();
    void testLessThan();
    void testFilterAcceptsRowEmptyFilter();
    void testFilterAcceptsRowAllFilter();
    void testFilterAcceptsRowFileFilter();
    void testFilterAcceptsRowImageFilter();
    void testFilterAcceptsRowLinkFilter();
    void testJudgeWhetherPicture();
    void testJudgeWhetherLink();

private:
    TestableChatSearchFilter* m_chatSearchFilter;
    QStandardItemModel* m_sourceModel;
};

#endif // TEST_CHAT_SEARCH_FILTER_H