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

#ifndef __SEARCHPAGE_H__
#define __SEARCHPAGE_H__

#include <QWidget>
#include <QListView>
#include <QScrollBar>
#include <QDebug>
#include <QCursor>
#include <QEvent>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include <QLineEdit>
#include <QMouseEvent>
#include <QListWidget>
#include <QCheckBox>
#include <QRegExp>
#include <KWindowSystem>
#include <QScrollArea>

#include "view/chatmsg/chatmsg.h"
#include "global/utils/inisetting.h"
#include "view/friendlist/friendlist.h"
#include "searchfriend_delegate.h"
#include "searchmsglist.h"
#include "view/common/globalsizedata.h"
class SearchMsgList;
class SearchPage : public QScrollArea
{
    Q_OBJECT
    friend class TestSearchPage;
public:
    SearchPage(QScrollArea *parent = nullptr);
    ~SearchPage();

    // 单例，初始化返回指针
    static SearchPage *getInstance();

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 初始化并监听gsetting
    void initGsetting();
    void changeTheme();

    QListView *m_friendView;  // 好友listview
    SearchMsgList *m_msgView; // 聊天记录listview
protected:
private:
    QWidget *m_widget;

    // 好友区域
    FriendListModel *m_friendListModel;
    SearchFriendDelegate *m_myItemDelegate;
    MySortFilterModel *m_sortFilterModel;
    QLabel *m_friendLabel;
    QPushButton *m_friendBtn;

    // 聊天记录区域
    QLabel *m_msgLabel;
    QPushButton *m_msgBtn;

    // 滚动条
    QScrollArea *m_scrollArea;
    QScrollBar *m_scrollBar;

    QScrollBar *m_scrollBar1;

    // 总体布局
    QVBoxLayout *m_mainLayout;

    // 好友区域布局
    QVBoxLayout *m_friendAreaVLayout;
    QHBoxLayout *m_friendAreaHLayout;

    // 聊天记录区域布局
    QVBoxLayout *m_msgAreaVLayout;
    QHBoxLayout *m_msgAreaHLayout;

signals:
    void sigUpdatFriList();
    void sigUpdatMsgList();
    void sigSearchEmpty();
    void sigSearchExist();

public slots:
    void FriendListSize(int num);
    void MsgListSize(int num);
    void slotCheckListView();
    void slotChangeFont();
};

#endif
