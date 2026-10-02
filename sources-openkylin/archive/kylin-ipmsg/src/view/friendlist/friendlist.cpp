/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) axxny later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QDateTime>
#include <QThread>
#include "windowmanage.hpp"

#include "view/localinfo/localupdatename.h"
#include "global/utils/globaldateformat.h"
#include "view/kyview.h"
#include "friendlist.h"
#include "libkydate.h"
#include "ukuiwindowhelper.h"

FriendListView::FriendListView(QListView *parent) : QListView(parent)
{
    // 初始化组件
    setViewUi();

    // 设置组件样式
    setViewStyle();
}

FriendListView::~FriendListView()
{
    FriendInfoWid::getInstance()->deleteLater();

    QHash<int, ChatMsg *>::iterator msgWidIt = m_msgWidMap.begin();

    while (msgWidIt != m_msgWidMap.end()) {

        if (msgWidIt.value()->m_chartSearch != nullptr) {
            msgWidIt.value()->m_chartSearch->deleteLater();
        }
        msgWidIt.value()->deleteLater();
        msgWidIt++;
    }
}

// 单例，初始化返回指针
FriendListView *FriendListView::getInstance(QListView *parent)
{
    static FriendListView *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new FriendListView(parent);
            instance->setAccessibleName("kylin-ipmsg_friendlist_FriendListView_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new FriendListView(parent);
        instance->setAccessibleName("kylin-ipmsg_friendlist_FriendListView_instance");
#endif
    }

    return instance;
}

// 初始化组件
void FriendListView::setViewUi()
{
    m_friendListModel = FriendListModel::getInstance();

    m_myItemDelegate = new FriendItemDelegate();
    m_myItemDelegate->setObjectName("kylin-ipmsg_friendlist_FriendItemDelegate_mmyItemDelegate");
    this->setItemDelegate(m_myItemDelegate);

    m_sortFilterModel = new MySortFilterModel();
    m_sortFilterModel->setObjectName("kylin-ipmsg_friendlist_MySortFilterModel_msortFilterModel");
    m_sortFilterModel->setSourceModel(m_friendListModel);
    m_sortFilterModel->setDynamicSortFilter(true);

    m_sortFilterModel->sort(0);
    m_sortFilterModel->setFilterKeyColumn(0);


    this->setModel(m_sortFilterModel);
    this->setSpacing(0);
    this->setDragEnabled(false);
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", true);
    this->setFrameShape(QListView::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &FriendListView::customContextMenuRequested, this, &FriendListView::showContextMenu);
    connect(this, &FriendListView::doubleClicked, this, &FriendListView::startChat);

    // 右键菜单
    m_funcMenu = new QMenu(this);
    m_funcMenu->setAccessibleName("kylin-ipmsg_friendlist_QMenu_mfuncMenu");
    m_startChatAction = m_funcMenu->addAction(tr("Start Chat"));
    m_setTopAction = m_funcMenu->addAction(tr("Set to Top"));
    m_nicknameAction = m_funcMenu->addAction(tr("Change Nickname"));
    m_viewInfoAction = m_funcMenu->addAction(tr("View Info"));
    m_delFriendAction = m_funcMenu->addAction(tr("Delete Friend"));

    m_funcMenu->insertSeparator(m_delFriendAction);

    connect(m_startChatAction, &QAction::triggered, this, &FriendListView::startChat);
    connect(m_setTopAction, &QAction::triggered, this, &FriendListView::setTop);
    connect(m_nicknameAction, &QAction::triggered, this, &FriendListView::changeNickname);
    connect(m_viewInfoAction, &QAction::triggered, this, &FriendListView::viewInfo);
    connect(m_delFriendAction, &QAction::triggered, this, &FriendListView::delFriend);
    connect(m_friendListModel, &FriendListModel::sigUpdateFriendState, this, &FriendListView::slotFriendState);
    connect(m_friendListModel, &FriendListModel::sigDeleFriend, this, &FriendListView::slotDeleFriend);

    connect(GlobalDateFormat::getInstance(), &GlobalDateFormat::sigTimeFormatChange, this, [=](){
        m_friendListModel->updateDateTimeFormat();
    });
}

// 设置组件样式
void FriendListView::setViewStyle()
{
    //毛玻璃
    this->setProperty("useSystemStyleBlur", false);
    this->setAttribute(Qt::WA_TranslucentBackground, false);
}

void FriendListView::initListModel()
{
    m_friendListModel->getModelFromDb();
}

// 显示右键菜单
void FriendListView::showContextMenu(const QPoint &pos)
{
    QModelIndex selectIndex = this->indexAt(pos);
    if (!selectIndex.isValid()) {
        return;
    }

    this->setCurrentIndex(selectIndex);

    if (!((this->selectionModel()->selectedIndexes()).empty())) {

        // 将代理index转换为好友index
        QModelIndex friendIndex = m_sortFilterModel->mapToSource(selectIndex);
        int onlineState = friendIndex.data(FriendListModel::OnlineState).toInt();
        int priority = friendIndex.data(FriendListModel::Priority).toInt();

        if (onlineState == FriendListModel::OnlineType::Online) {
            m_delFriendAction->setVisible(false);
        } else {
            m_delFriendAction->setVisible(true);
        }

        if (priority == FriendListModel::PriorityType::PriDefault) {
            m_setTopAction->setText(tr("Set to Top"));

        } else if (priority == FriendListModel::PriorityType::PriStayTop) {
            m_setTopAction->setText(tr("Cancel the Top"));
        }

#ifndef TEST_MODE
        m_funcMenu->exec(QCursor::pos());
#endif
    }
}

// 根据好友id获取好友聊天框
ChatMsg *FriendListView::getMsgWidById(int friendId)
{
    ChatMsg *chatMsg = nullptr;

    if (m_msgWidMap.contains(friendId)) {
        chatMsg = m_msgWidMap.value(friendId);
        /* 适配kysdk的窗管 */
        UkuiWindowHelper *windowhelper = new UkuiWindowHelper(chatMsg);
        windowhelper->removeTitleBar();
    } else {
        QString friendUuid = FriendListModel::getInstance()->getUuidById(friendId);
        chatMsg = new ChatMsg(friendId, friendUuid);
        chatMsg->setAccessibleName("kylin-ipmsg_friendlist_ChatMsg_chatMsg");

        // 设置聊天窗口初始位置
        QScreen *screen = QGuiApplication::primaryScreen();

        int chatMsgXPos = (screen->geometry().width() - chatMsg->width()) / 2 + 25 * m_msgWidMap.size();
        int chatMsgYPos = (screen->geometry().height() - chatMsg->height()) / 2 + 25 * m_msgWidMap.size();

        chatMsg->move(chatMsgXPos, chatMsgYPos);

        m_msgWidMap.insert(friendId, chatMsg);

        // 发信号进行信号绑定
        emit sigNewChatMsgWid(chatMsg);
    }

    return chatMsg;
}

// 获取好友列表model
FriendListModel *FriendListView::getFriendListModel()
{
    return m_friendListModel;
}

// 根据字符串过滤好友
void FriendListView::filterFriendByReg(QString regStr)
{
    /* sdk功能点打点：主界面搜索 */
    GlobalSizeData::SDKPointMainSearch();

    MySortFilterModel *model = m_sortFilterModel;
    // m_sortFilterModel->clear();
    model->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));
    m_sortFilterModel = model;

    if (regStr == "") {
           if (this->model()->rowCount() == 0) {
               emit sigFriendsEmpty();
           }
           else {
               emit sigFriendState();
           }
       }
       else {
           SearchPage::getInstance()->FriendListSize(this->model()->rowCount());
       }
}

// 清理所有聊天框的聊天记录
void FriendListView::clearAllMessages()
{
    QHash<int, ChatMsg *>::iterator chatMsgWidIt = m_msgWidMap.begin();

    while (chatMsgWidIt != m_msgWidMap.end()) {

        ChatMsg *chatMsgWidtem = chatMsgWidIt.value();

        chatMsgWidtem->getchatMsgModel()->clear();

        chatMsgWidIt++;
    }
}

// 发起聊天
void FriendListView::startChat()
{
    // 将代理index转换为好友index
    QModelIndex friendIndex = m_sortFilterModel->mapToSource(this->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    // 清空好友未读消息数
    FriendListModel::getInstance()->clearUnreadMsgNum(friendId);

    this->slotShowMsgWidById(friendId);

    return;
}

// 设为置顶
void FriendListView::setTop()
{
    /* sdk功能点：设置置顶 */
    GlobalSizeData::SDKPointSetTop();

    // 将代理index转换为好友index
    QModelIndex friendIndex = m_sortFilterModel->mapToSource(this->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    FriendInfoData *friendInfo = new FriendInfoData();
    friendInfo->setObjectName("kylin-ipmsg_friendlist_FriendInfoData_friendInfo");
    friendInfo->m_friendId = friendId;

    if (m_friendListModel->updatePriority(friendInfo)) {
        friendInfo->deleteLater();
    }
}

// 修改好友备注
void FriendListView::changeNickname()
{
    /* SDK功能点：修改好友备注 */
    GlobalSizeData::SDKPointModifyFriendNotes();

    // 将代理index转换为好友index
    QModelIndex friendIndex = m_sortFilterModel->mapToSource(this->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    LocalUpdateName *updateNameWid = new LocalUpdateName(LocalUpdateName::FriendName, friendId, this);
    updateNameWid->setAccessibleName("kylin-ipmsg_friendlist_LocalUpdateName_updateNameWid");
    updateNameWid->show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(updateNameWid->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive))
    {
    ::kabase::WindowManage::setMiddleOfScreen(updateNameWid);
    }
    // updateNameWid->m_nameEdit->setFocus();
}

// 查看资料
void FriendListView::viewInfo()
{
    /* SDK功能点：查看资料 */
    GlobalSizeData::SDKPointViewInformation();

    // 将代理index转换为好友index
    QModelIndex friendIndex = m_sortFilterModel->mapToSource(this->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    FriendInfoWid::getInstance(this)->fillFriendInfo(friendId);
    FriendInfoWid::getInstance(this)->activateWindow();
    FriendInfoWid::getInstance(this)->show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(FriendInfoWid::getInstance(this)->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
    ::kabase::WindowManage::setMiddleOfScreen(FriendInfoWid::getInstance());
    }
}

// 删除好友
void FriendListView::delFriend()
{
    // 将代理index转换为好友index
    QModelIndex friendIndex = m_sortFilterModel->mapToSource(this->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    // this->setRowHidden(this->currentIndex().row(), true);

    m_friendListModel->removeFriend(friendId, friendIndex.row());

    // 删除好友时删除聊天框
    ChatMsg *chatMsg = nullptr;

    if (m_msgWidMap.contains(friendId)) {
        chatMsg = m_msgWidMap.value(friendId);
        m_msgWidMap.remove(friendId);

        chatMsg->deleteLater();
    }
}

void FriendListView::slotShowMsgWidById(int friendId)
{
    ChatMsg *chatMsg = nullptr;
    chatMsg = getMsgWidById(friendId);

    chatMsg->show();
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
    ::kabase::WindowManage::setMiddleOfScreen(chatMsg);
    }
    chatMsg->activateWindow();
    chatMsg->getFocus();

    return;
}

void FriendListView::slotFriendState()
{
    emit sigFriendState();
    return;
}

void FriendListView::slotDeleFriend()
{
    int friendNum = this->model()->rowCount();
    if (friendNum == 0) {
        emit sigFriendsEmpty();
    } else {
        emit sigFriendState();
    }
    return;
}
