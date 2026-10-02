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

#include "chatmsgmodel.h"
#include "global/utils/globalutils.h"
#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "libkydate.h"

ChatMsgModel::ChatMsgModel(int friendId, QString friendUuid) : QStandardItemModel(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[MsgId] = "msgId";
    roleNames[FriendId] = "friendId";
    roleNames[FriendUuid] = "friendUuid";
    roleNames[IsSend] = "isSend";
    roleNames[MsgTime] = "msgTime";
    roleNames[MsgContent] = "msgContent";
    roleNames[MsgType] = "msgType";
    roleNames[FilePath] = "filePath";
    roleNames[SendState] = "sendState";
    roleNames[ReadState] = "readState";
    roleNames[TotalSize] = "totalSize";
    roleNames[TransferSize] = "transferSize";
    roleNames[ChooseStateType] = "chooseStateType";
    roleNames[DisplayMsgTime] = "DisplayMsgTime";
    setItemRoleNames(roleNames);

    if (friendId != -1) {
        m_friendId = friendId;
    }

    if (friendUuid != "") {
        m_friendUuid = friendUuid;
    }

    ChatMsgDB::getInstance();
}

ChatMsgModel::~ChatMsgModel() {}

/**************************************************************************
 * 函数名称： getMsgById
 * 函数功能： 通过msgId获取item
 * 输入参数： int
 * 返回数值： QStandardItem *
 * 创建人员：
 * 创建时间： 2021-05-15
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QStandardItem *ChatMsgModel::getMsgById(int msgId)
{
    QStandardItem *resItem = nullptr;

    if (m_msgIdItemsMap.contains(msgId)) {
        resItem = m_msgIdItemsMap.value(msgId);
    }

    return resItem;
}

/**************************************************************************
 * 函数名称： testAddChatMsg
 * 函数功能： 测试添加消息
 * 输入参数： int
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-05-15
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgModel::testAddChatMsg(int iSend)
{
    QStandardItem *newItem = NULL;

    int friendId = 1;
    QString msgTime = "2021-02-02 11:44";
    QString msgContent = "lalalalalalala lalallalalalalalalalalallalllalalalalalalalalallalllalalalalalalalalallal";
    QString filePath = "/home/path";
    int msgId = 1;
    int isSend = iSend;
    int msgType = 0;
    int sendState = 1;
    int readState = 1;
    int totalSize = 1;
    int transferSize = 1;

    newItem = new QStandardItem();
    newItem->setAccessibleText("kylin-ipmsg_chatmsgmodel_QStandardItem_newItem");
    newItem->setData(friendId, ChatMsgModel::FriendId);
    newItem->setData(isSend, ChatMsgModel::IsSend);
    newItem->setData(msgTime, ChatMsgModel::MsgTime);
    newItem->setData(msgContent, ChatMsgModel::MsgContent);
    newItem->setData(msgType, ChatMsgModel::MsgType);
    newItem->setData(filePath, ChatMsgModel::FilePath);
    newItem->setData(sendState, ChatMsgModel::SendState);
    newItem->setData(readState, ChatMsgModel::ReadState);
    newItem->setData(totalSize, ChatMsgModel::TotalSize);
    newItem->setData(transferSize, ChatMsgModel::TransferSize);
    newItem->setData(msgTime, ChatMsgModel::DisplayMsgTime);

    appendRow(newItem);

    ChatMsgInfo *msgInfo = new ChatMsgInfo();
    msgInfo->m_msgId = msgId;
    msgInfo->m_friendId = friendId;
    msgInfo->m_isSend = isSend;
    msgInfo->m_msgTime = msgTime;
    msgInfo->m_msgContent = msgContent;
    msgInfo->m_msgType = msgType;
    msgInfo->m_filePath = filePath;
    msgInfo->m_sendState = sendState;
    msgInfo->m_readState = readState;
    msgInfo->m_totalSize = totalSize;
    msgInfo->m_transferSize = transferSize;
    msgInfo->m_displayMsgTime = msgTime;
    ChatMsgDB::getInstance()->insertData(msgInfo);
}

/**************************************************************************
 * 函数名称： addChatMsg
 * 函数功能： 添加消息
 * 输入参数： ChatMsgInfo *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-21
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgModel::addChatMsg(ChatMsgInfo *chatMsgInfo)
{
    // qDebug() << "addChatMsg" << chatMsgInfo->m_friendUuid <<
    //          chatMsgInfo->m_msgContent << chatMsgInfo->m_msgTime;

    ChatMsg *currentMsgWid = FriendListView::getInstance()->getMsgWidById(m_friendId);

    bool isOnBottom = currentMsgWid->isScrollOnBottom();

    QStandardItem *timeItem = nullptr;

    // 当前消息为第一条消息
    if (this->rowCount() == 0) {
        timeItem = this->addTimeItem(chatMsgInfo->m_msgTime);

    } else {
        // 判断与上一条记录相差的时间
        QModelIndex endIndex = this->index(this->rowCount() - 1, 0);
        QString endTimeStr = endIndex.data(ChatMsgModel::MsgTime).toString();

        timeItem = this->addTimeItem(endTimeStr, chatMsgInfo->m_msgTime);
    }

    if (timeItem != nullptr) {
        this->appendRow(timeItem);
    }

    chatMsgInfo->m_msgId = ChatMsgDB::getInstance()->insertData(chatMsgInfo);

    QString realMsgContent = GlobalUtils::uncryptData(chatMsgInfo->m_msgContent);

    QStandardItem *newItem = NULL;

    newItem = new QStandardItem();
    newItem->setAccessibleText("kylin-ipmsg_chatmsgmodel_QStandardItem_newItem");
    newItem->setData(chatMsgInfo->m_msgId, ChatMsgModel::MsgId);
    newItem->setData(chatMsgInfo->m_friendId, ChatMsgModel::FriendId);
    newItem->setData(chatMsgInfo->m_friendUuid, ChatMsgModel::FriendUuid);
    newItem->setData(chatMsgInfo->m_isSend, ChatMsgModel::IsSend);
    newItem->setData(chatMsgInfo->m_msgTime, ChatMsgModel::MsgTime);
    newItem->setData(realMsgContent, ChatMsgModel::MsgContent);
    newItem->setData(chatMsgInfo->m_msgType, ChatMsgModel::MsgType);
    newItem->setData(chatMsgInfo->m_filePath, ChatMsgModel::FilePath);
    newItem->setData(chatMsgInfo->m_sendState, ChatMsgModel::SendState);
    newItem->setData(chatMsgInfo->m_readState, ChatMsgModel::ReadState);
    newItem->setData(chatMsgInfo->m_totalSize, ChatMsgModel::TotalSize);
    newItem->setData(chatMsgInfo->m_transferSize, ChatMsgModel::TransferSize);
    newItem->setData(chatMsgInfo->m_chooseMsgState, ChatMsgModel::ChooseStateType);
    newItem->setData(chatMsgInfo->m_displayMsgTime, ChatMsgModel::DisplayMsgTime);

    appendRow(newItem);

    m_msgIdItemsMap.insert(chatMsgInfo->m_msgId, newItem);

    // 更新好友列表信息
    QStandardItem *friendItem = FriendListModel::getInstance()->getFriendById(chatMsgInfo->m_friendId);
    int unreadNum = 0;
    if (friendItem != nullptr) {
        unreadNum = friendItem->data(FriendListModel::UnreadMsgNum).toInt();
    }

    // 接收消息 未读消息数增加
    if (chatMsgInfo->m_isSend == ChatMsgModel::SendType::RecvMsg
        && !(FriendListView::getInstance()->getMsgWidById(m_friendId)->isActiveWindow())) {
        unreadNum++;

        emit TrayIconWid::getInstance()->newUnreadMsg();
    }

    FriendInfoData *friendInfo = new FriendInfoData();
    friendInfo->setObjectName("kylin-ipmsg_chatmsgmodel_FriendInfoData_friendInfo");
    friendInfo->m_friendId = chatMsgInfo->m_friendId;
    friendInfo->m_recentMsgCont = chatMsgInfo->m_msgContent;
    friendInfo->m_recentMsgTime = chatMsgInfo->m_msgTime;
    friendInfo->m_unreadMsgNum = unreadNum;
    friendInfo->m_recentDispalyMsgTime = chatMsgInfo->m_displayMsgTime;

    if (FriendListModel::getInstance()->updateMessage(friendInfo)) {
        friendInfo->deleteLater();
    }

    if (isOnBottom) {
        currentMsgWid->setScrollToBottom();
    }

    return true;
}

/**************************************************************************
 * 函数名称： delChatMsg
 * 函数功能： 删除消息
 * 输入参数： int, int
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-06-07
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgModel::delChatMsg(int msgId, int row)
{
    this->removeRow(row);

    // 删除消息的上一条消息
    QModelIndex prevIndex = this->index(row - 1, 0);
    int msgType = prevIndex.data(ChatMsgModel::MsgType).toInt();

    if (msgType == ChatMsgModel::MessageType::TimeMsg) {
        this->removeRow(row - 1);
        row = row - 1;
    }

    m_msgIdItemsMap.remove(msgId);

    ChatMsgDB::getInstance()->deleteMsg(msgId);

    // 删除的是最后一条消息
    if (row == this->rowCount()) {

        if (row == 0) {
            this->updateListInfo();
        } else {
            int lastMsgId = this->index(row - 1, 0).data(ChatMsgModel::MsgId).toInt();
            this->updateListInfo(lastMsgId);
        }
        return;
    }

    // 删除消息的下一条消息
    int nextMsgType = this->index(row, 0).data(ChatMsgModel::MsgType).toInt();

    if (nextMsgType == ChatMsgModel::MessageType::TimeMsg) {
        return;
    }

    // 添加时间item
    QStandardItem *timeItem = nullptr;

    if (row == 0) {
        QString firstMsgTime = this->index(row, 0).data(ChatMsgModel::MsgTime).toString();
        timeItem = this->addTimeItem(firstMsgTime);
    } else {
        QString firstMsgTime = this->index(row - 1, 0).data(ChatMsgModel::MsgTime).toString();
        QString secondMsgTime = this->index(row, 0).data(ChatMsgModel::MsgTime).toString();
        timeItem = this->addTimeItem(firstMsgTime, secondMsgTime);
    }

    if (timeItem != nullptr) {
        this->insertRow(row, timeItem);
    }
}

//删除搜索聊天记录信息
void ChatMsgModel::delSearchMsg(int msgId)
{
    QStandardItem *item = m_msgIdItemsMap.value(msgId);
    if (item == nullptr) {
        return;
    }
    int row = item->row();
    this->delChatMsg(msgId, row);

    return;
}

/**************************************************************************
 * 函数名称： clearChatMsg
 * 函数功能： 清空消息
 * 输入参数： void
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-06-07
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgModel::clearChatMsg()
{
    this->clear();
    m_msgIdItemsMap.clear();

    ChatMsgDB::getInstance()->clearMsgByFriendId(m_friendId);

    this->updateListInfo();
}

/**************************************************************************
 * 函数名称： updateMsgState
 * 函数功能： 更新消息发送状态
 * 输入参数： ChatMsgInfo *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-06-04
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgModel::updateMsgState(ChatMsgInfo *chatMsgInfo)
{
    QStandardItem *msgItem = getMsgById(chatMsgInfo->m_msgId);

    if (msgItem != nullptr) {
        msgItem->setData(chatMsgInfo->m_sendState, ChatMsgModel::SendState);

        return ChatMsgDB::getInstance()->updateMsgState(chatMsgInfo);
    }

    return true;
}

/**************************************************************************
 * 函数名称： updateTransferSize
 * 函数功能： 更新文件传输大小
 * 输入参数： ChatMsgInfo *
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-06-04
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgModel::updateTransferSize(ChatMsgInfo *chatMsgInfo)
{
    QStandardItem *msgItem = getMsgById(chatMsgInfo->m_msgId);

    if (msgItem != nullptr) {
        msgItem->setData(chatMsgInfo->m_totalSize, ChatMsgModel::TotalSize);
        msgItem->setData(chatMsgInfo->m_transferSize, ChatMsgModel::TransferSize);

        // TODO : 可能会造成频繁访问数据库
        ChatMsgDB::getInstance()->updateTransferSize(chatMsgInfo);
    }
}

/**************************************************************************
 * 函数名称： updateListInfo
 * 函数功能： 更新好友列表信息
 * 输入参数： int
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-06-07
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgModel::updateListInfo(int msgId)
{
    FriendInfoData *friendInfo = new FriendInfoData();
    friendInfo->setObjectName("kylin-ipmsg_chatmsgmodel_FriendInfoData_friendInfo");
    friendInfo->m_friendId = m_friendId;

    if (msgId != -1) {
        QStandardItem *msgItem = getMsgById(msgId);

        if (msgItem != nullptr) {
            friendInfo->m_recentMsgCont = GlobalUtils::encryptData(msgItem->data(ChatMsgModel::MsgContent).toString());
            friendInfo->m_recentMsgTime = msgItem->data(ChatMsgModel::MsgTime).toString();
        }
    }

    if (FriendListModel::getInstance()->updateMessage(friendInfo)) {
        friendInfo->deleteLater();
    }
}

/**
 * @brief 跟随ukui-control-center改变"日期短格式+时间"
 * @param timeFormat 时间制， 如"12小时制"或"24小时制"
 */
void ChatMsgModel::updateDateTimeFormat(QString timeFormat)
{
    QHash<int, QStandardItem *>::const_iterator it = m_msgIdItemsMap.constBegin();
    while (it != m_msgIdItemsMap.constEnd()) {
        QDateTime currentTime  = QDateTime::currentDateTime();
        QString currentTimeStr = currentTime.toString(GlobalUtils::getTimeFormat());
        // 通过改变Item中ChatMsgModel::DisplayMsgTime角色的值触发delegate::paint函数，更新日期格式
        it.value()->setData(currentTimeStr, ChatMsgModel::DisplayMsgTime);
        ++it;
    }
}

/**************************************************************************
 * 函数名称： getModelFromDb
 * 函数功能： 从数据库获取好友信息
 * 输入参数： 无
 * 返回数值： QSqlTableModel *
 * 创建人员：
 * 创建时间： 2021-05-18
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QSqlTableModel *ChatMsgModel::getModelFromDb()
{
    m_msgList = ChatMsgDB::getInstance()->queryMsgByFriendId(m_friendId);

    m_tableModel = ChatMsgDB::getInstance()->m_model;

    for (int i = 0; i < m_msgList.size(); i++) {

        QStandardItem *timeItem = nullptr;

        // 第一条消息
        if (i == 0) {
            timeItem = this->addTimeItem(m_msgList[i]->m_msgTime);
        } else {
            timeItem = this->addTimeItem(m_msgList[i - 1]->m_msgTime, m_msgList[i]->m_msgTime);
        }

        if (timeItem != nullptr) {
            this->appendRow(timeItem);
        }

        QString realMsgContent = GlobalUtils::uncryptData(m_msgList[i]->m_msgContent);

        QStandardItem *newItem = new QStandardItem();
        newItem->setAccessibleText("kylin-ipmsg_chatmsgmodel_QStandardItem_newItem");

        // qDebug() << "getModelFromDb" << m_msgList[i]->m_msgContent << m_msgList[i]->m_msgTime;

        newItem->setData(m_msgList[i]->m_msgId, ChatMsgModel::MsgId);
        newItem->setData(m_msgList[i]->m_friendId, ChatMsgModel::FriendId);
        newItem->setData(m_msgList[i]->m_isSend, ChatMsgModel::IsSend);
        newItem->setData(m_msgList[i]->m_msgTime, ChatMsgModel::MsgTime);
        newItem->setData(realMsgContent, ChatMsgModel::MsgContent);
        newItem->setData(m_msgList[i]->m_msgType, ChatMsgModel::MsgType);
        newItem->setData(m_msgList[i]->m_filePath, ChatMsgModel::FilePath);
        newItem->setData(m_msgList[i]->m_sendState, ChatMsgModel::SendState);
        newItem->setData(m_msgList[i]->m_readState, ChatMsgModel::ReadState);
        newItem->setData(m_msgList[i]->m_totalSize, ChatMsgModel::TotalSize);
        newItem->setData(m_msgList[i]->m_transferSize, ChatMsgModel::TransferSize);
        newItem->setData(m_msgList[i]->m_chooseMsgState, ChatMsgModel::ChooseStateType);
        newItem->setData(m_msgList[i]->m_displayMsgTime, ChatMsgModel::DisplayMsgTime);

        if (m_msgList[i]->m_isSend == ChatMsgModel::SendType::SendMsg
            && m_msgList[i]->m_sendState != ChatMsgModel::MesssageState::SendSucceed) {
            m_msgList[i]->m_sendState = ChatMsgModel::MesssageState::SendFailed;

            newItem->setData(ChatMsgModel::MesssageState::SendFailed, ChatMsgModel::SendState);

            ChatMsgDB::getInstance()->updateMsgState(m_msgList[i]);
        }

        appendRow(newItem);

        m_msgIdItemsMap.insert(newItem->data(ChatMsgModel::MsgId).toInt(), newItem);
    }

    return m_tableModel;
}

// 获取时间item
QStandardItem *ChatMsgModel::addTimeItem(QString fristTimeStr, QString secondTimeStr)
{
    QStandardItem *timeItem = nullptr;

    // 第一条消息
    if (secondTimeStr.isEmpty()) {
        timeItem = new QStandardItem();
        timeItem->setAccessibleText("kylin-ipmsg_chatmsgmodel_QStandardItem_timeItem");
        timeItem->setData(fristTimeStr, ChatMsgModel::MsgTime);
        timeItem->setData(MessageType::TimeMsg, ChatMsgModel::MsgType);

    } else {
        // 判断与上一条记录相差的时间
        QDateTime fristTime = QDateTime::fromString(fristTimeStr, GlobalUtils::getTimeFormat());
        QDateTime secondTime = QDateTime::fromString(secondTimeStr, GlobalUtils::getTimeFormat());

        // 上一条和这一条消息相差的秒数
        qint64 secDiff = fristTime.secsTo(secondTime);

        // 相差两分钟
        if (secDiff > 120) {
            timeItem = new QStandardItem();
            timeItem->setAccessibleText("kylin-ipmsg_chatmsgmodel_QStandardItem_timeItem");
            timeItem->setData(secondTime, ChatMsgModel::MsgTime);
            timeItem->setData(MessageType::TimeMsg, ChatMsgModel::MsgType);
        }
    }

    return timeItem;
}

void ChatMsgModel::setMsgChooseState(ChatMsgInfo *chatMsgInfo)
{
    QStandardItem *msgItem = getMsgById(chatMsgInfo->m_msgId);
    if (!msgItem) {
        return;
    }
    chatMsgInfo->m_chooseMsgState = msgItem->data(ChatMsgModel::ChatMsgRoles::ChooseStateType).toInt();

    // if (chatMsgInfo->m_chooseMsgState == ChooseMsgState::ChooseCancel) {
    //     chatMsgInfo->m_chooseMsgState = ChooseMsgState::ChooseConfirm;
    // } else {
    //     chatMsgInfo->m_chooseMsgState = ChooseMsgState::ChooseCancel;
    // }

    chatMsgInfo->m_chooseMsgState = chatMsgInfo->m_chooseMsgState == ChooseMsgState::ChooseCancel
                                        ? ChooseMsgState::ChooseConfirm
                                        : ChooseMsgState::ChooseCancel;


    msgItem->setData(chatMsgInfo->m_chooseMsgState, ChatMsgModel::ChooseStateType);

    return;
}
