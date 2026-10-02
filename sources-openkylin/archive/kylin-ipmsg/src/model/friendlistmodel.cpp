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

#include "friendlistmodel.h"
#include "global/utils/globalutils.h"
#include "global/utils/globaldateformat.h"
#include "libkydate.h"

#include <QDateTime>

FriendListModel::FriendListModel() : QStandardItemModel(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Id] = "id";
    roleNames[Uuid] = "uuid";
    roleNames[Ip] = "ip";
    roleNames[Port] = "port";
    roleNames[Mac] = "mac";
    roleNames[Username] = "username";
    roleNames[Username] = "nickname";
    roleNames[System] = "system";
    roleNames[Platform] = "platform";
    roleNames[Avatar] = "avatar";
    roleNames[RecentMsgContent] = "recentMsgContent";
    roleNames[RecentMsgTime] = "recentMsgTime";
    roleNames[UnreadMsgNum] = "unreadMsgNum";
    roleNames[OnlineState] = "onlineState";
    roleNames[Priority] = "priority";
    roleNames[Display] = "display";
    roleNames[DisplayRecentMsgTime] = "DisplayRecentMsgTime";
    setItemRoleNames(roleNames);

    FriendInfoDataBase::getInstance();

    this->getModelFromDb();
}

FriendListModel::~FriendListModel() {}

//单例，初始化返回指针
FriendListModel *FriendListModel::getInstance()
{
    static FriendListModel *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new FriendListModel();
            instance->setObjectName("kylin-ipmsg_friendlistmodel_FriendListModel_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new FriendListModel();
        instance->setObjectName("kylin-ipmsg_friendlistmodel_FriendListModel_instance");
#endif
    }

    return instance;
}

// 通过好友uuid获取item
QStandardItem *FriendListModel::getItemByUuid(QString friendUuid)
{
    QStandardItem *resItem = nullptr;

    FriendInfoData *friendInfo = FriendInfoDataBase::getInstance()->queryInfoByUuid(friendUuid);

    if (!(friendInfo->m_friendUuid.isEmpty())) {
        resItem = new QStandardItem();
        resItem->setAccessibleText("kylin-ipmsg_friendlistmodel_QStandardItem_resItem");

        resItem->setData(friendInfo->m_friendId, FriendListModel::Id);
        resItem->setData(friendInfo->m_friendUuid, FriendListModel::Uuid);
        resItem->setData(friendInfo->m_friendIp, FriendListModel::Ip);
        resItem->setData(friendInfo->m_friendPort, FriendListModel::Port);
        resItem->setData(friendInfo->m_friendMac, FriendListModel::Mac);
        resItem->setData(friendInfo->m_username, FriendListModel::Username);
        resItem->setData(friendInfo->m_nickname, FriendListModel::Nickname);
        resItem->setData(friendInfo->m_system, FriendListModel::System);
        resItem->setData(friendInfo->m_platform, FriendListModel::Platform);
        resItem->setData(friendInfo->m_avatarUrl, FriendListModel::Avatar);
        resItem->setData(friendInfo->m_recentMsgCont, FriendListModel::RecentMsgContent);
        resItem->setData(friendInfo->m_recentMsgTime, FriendListModel::RecentMsgTime);
        resItem->setData(friendInfo->m_unreadMsgNum, FriendListModel::UnreadMsgNum);
        resItem->setData(friendInfo->m_priority, FriendListModel::Priority);
        resItem->setData(friendInfo->m_display, FriendListModel::Display);
        resItem->setData(friendInfo->m_recentMsgTime, FriendListModel::DisplayRecentMsgTime);

    }

    return resItem;
}

void FriendListModel::addFriend(FriendInfoData *friendInfo)
{
    FriendInfoData *queryInfo = nullptr;
    queryInfo = FriendInfoDataBase::getInstance()->queryInfoByUuid(friendInfo->m_friendUuid);

    // 不存在该好友
    if (queryInfo->m_friendUuid.isEmpty()) {

        // 在数据库中添加好友
        friendInfo->m_friendId = FriendInfoDataBase::getInstance()->insertData(friendInfo);
        friendInfo->m_avatarUrl = GlobalUtils::getAvatarByName(friendInfo->m_username);

        QStandardItem *newItem = new QStandardItem();
        newItem->setAccessibleText("kylin-ipmsg_friendlistmodel_QStandardItem_newItem");

        newItem->setData(friendInfo->m_friendId, FriendListModel::Id);
        newItem->setData(friendInfo->m_friendUuid, FriendListModel::Uuid);
        newItem->setData(friendInfo->m_friendIp, FriendListModel::Ip);
        newItem->setData(friendInfo->m_friendPort, FriendListModel::Port);
        newItem->setData(friendInfo->m_friendMac, FriendListModel::Mac);
        newItem->setData(friendInfo->m_username, FriendListModel::Username);
        newItem->setData(friendInfo->m_nickname, FriendListModel::Nickname);
        newItem->setData(friendInfo->m_system, FriendListModel::System);
        newItem->setData(friendInfo->m_platform, FriendListModel::Platform);
        newItem->setData(friendInfo->m_avatarUrl, FriendListModel::Avatar);
        newItem->setData(friendInfo->m_recentMsgCont, FriendListModel::RecentMsgContent);
        newItem->setData(friendInfo->m_recentMsgTime, FriendListModel::RecentMsgTime);
        newItem->setData(friendInfo->m_unreadMsgNum, FriendListModel::UnreadMsgNum);
        newItem->setData(FriendListModel::PriDefault, FriendListModel::Priority);
        newItem->setData(FriendListModel::Online, FriendListModel::OnlineState);
        newItem->setData(FriendListModel::DisShow, FriendListModel::Display);
        newItem->setData(friendInfo->m_recentMsgTime, FriendListModel::DisplayRecentMsgTime);

        appendRow(newItem);

        m_idItemsMap.insert(newItem->data(FriendListModel::Id).toInt(), newItem);
        m_uuidItemsMap.insert(newItem->data(FriendListModel::Uuid).toString(), newItem);

        emit sigUpdateFriendState();

    } else {
        updateFriend(friendInfo);
    }
    queryInfo->deleteLater();
}

// 更新好友上线信息
void FriendListModel::updateFriend(FriendInfoData *friendInfo)
{
    // 检查 friendInfo 是否为空
    if (!friendInfo) {
        qWarning() << "friendInfo is null!";
        return;
    }

    // 已加载该好友
    if (m_uuidItemsMap.contains(friendInfo->m_friendUuid)) {
        QStandardItem *friendItem = m_uuidItemsMap.value(friendInfo->m_friendUuid);

        QString nickname = friendItem->data(FriendListModel::Nickname).toString();
        int display = friendItem->data(FriendListModel::Display).toInt();

        friendInfo->m_friendId = friendItem->data(FriendListModel::Id).toInt();

        if (friendInfo->m_onlineState) {

            friendItem->setData(friendInfo->m_friendIp, FriendListModel::Ip);
            friendItem->setData(friendInfo->m_friendPort, FriendListModel::Port);
            friendItem->setData(friendInfo->m_friendMac, FriendListModel::Mac);
            friendItem->setData(friendInfo->m_username, FriendListModel::Username);

            // 判断是否需要更新头像
            if (nickname.isEmpty()) {
                friendInfo->m_avatarUrl = GlobalUtils::getAvatarByName(friendInfo->m_username);
                friendItem->setData(friendInfo->m_avatarUrl, FriendListModel::Avatar);
            } else {
                friendInfo->m_avatarUrl = friendItem->data(FriendListModel::Avatar).toString();
            }

            // 判断是否需要更新优先级
            if (display == DisplayType::DisHide) {
                friendItem->setData(DisplayType::DisShow, FriendListModel::Display);
                friendInfo->m_display = DisplayType::DisShow;

                emit sigUpdateFriendState();
            }

            FriendInfoDataBase::getInstance()->updateFriend(friendInfo);

            emit updateFriendInfo(friendInfo->m_friendId);
        }

        friendItem->setData(friendInfo->m_onlineState, FriendListModel::OnlineState);

        // 未加载该好友
    } else {
        QStandardItem *newItem = getItemByUuid(friendInfo->m_friendUuid);

        // 检查 newItem 是否为空
        if (!newItem) {
            qWarning() << "Failed to get item by UUID:" << friendInfo->m_friendUuid;
            return;
        }

        friendInfo->m_friendId = newItem->data(FriendListModel::Id).toInt();
        friendInfo->m_recentMsgCont =
            GlobalUtils::uncryptData(newItem->data(FriendListModel::RecentMsgContent).toString());

        newItem->setData(friendInfo->m_recentMsgCont, FriendListModel::RecentMsgContent);
        newItem->setData(FriendListModel::DisShow, FriendListModel::Display);
        newItem->setData(FriendListModel::Online, FriendListModel::OnlineState);

        this->appendRow(newItem);

        m_idItemsMap.insert(friendInfo->m_friendId, newItem);
        m_uuidItemsMap.insert(friendInfo->m_friendUuid, newItem);

        FriendInfoDataBase::getInstance()->updateDisplay(friendInfo->m_friendId, DisplayType::DisShow);

        emit sigUpdateFriendState();
    }
}


// 跟随ukui-control-center改变"日期短格式+时间"
void FriendListModel::updateDateTimeFormat()
{
    QHash<int, QStandardItem *>::const_iterator it = m_idItemsMap.constBegin();

    while (it != m_idItemsMap.constEnd()) {
        FriendInfoData *friendInfo = nullptr;
        friendInfo = FriendInfoDataBase::getInstance()->queryInfoById(it.key());
        QString msgTime =  friendInfo->m_recentMsgTime;
        QDateTime currentTime  = QDateTime::currentDateTime();
        QString currentTimeStr = currentTime.toString(GlobalUtils::getTimeFormat());
        // 通过改变Item中FriendListModel::DisplayMsgTime角色的值触发delegate::paint函数，更新日期格式
        it.value()->setData(currentTimeStr, FriendListModel::DisplayRecentMsgTime);
        ++it;
    }
}

// 删除好友
void FriendListModel::removeFriend(QString uuid, int row)
{
    int friendId = getIdByUuid(uuid);

    removeFriend(friendId, row);
}

// 删除好友
void FriendListModel::removeFriend(int friendId, int row)
{
    // QStandardItem *friendItem = m_idItemsMap.value(friendId);
    // friendItem->setData(DisplayType::DisHide, FriendListModel::Display);

    m_uuidItemsMap.remove(getUuidById(friendId));
    m_idItemsMap.remove(friendId);

    this->removeRow(row);

    // Q_UNUSED(friendItem);
    // delete friendItem;
    // friendItem = nullptr;

    FriendInfoDataBase::getInstance()->updateDisplay(friendId, DisplayType::DisHide);

    emit sigDeleFriend();
}

// 从数据库获取好友信息
QSqlTableModel *FriendListModel::getModelFromDb()
{
    FriendInfoDataBase::getInstance()->queryData();

    m_tableModel = FriendInfoDataBase::getInstance()->m_model;

    int rowCount = m_tableModel->rowCount();

    for (int i = 0; i < rowCount; i++) {

        QStandardItem *newItem = new QStandardItem();
        newItem->setAccessibleText("kylin-ipmsg_friendlistmodel_QStandardItem_newItem");

        // Check if the same uuid is alreay in the friend list

        int index = 0;

        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Id);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Uuid);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Ip);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Port);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Mac);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Username);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Nickname);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::System);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Platform);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Avatar);

        QString realMsgContent = GlobalUtils::uncryptData(m_tableModel->record(i).value(index++));
        newItem->setData(realMsgContent, FriendListModel::RecentMsgContent);

        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::RecentMsgTime);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::UnreadMsgNum);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Priority);
        newItem->setData(m_tableModel->record(i).value(index++), FriendListModel::Display);

        newItem->setData(0, FriendListModel::OnlineState);

        if (m_tableModel->record(i).value(index - 1).toInt() == DisplayType::DisShow) {
            appendRow(newItem);
            m_loadFriendIdList.append(newItem->data(FriendListModel::Id).toInt());
            m_idItemsMap.insert(newItem->data(FriendListModel::Id).toInt(), newItem);
            m_uuidItemsMap.insert(newItem->data(FriendListModel::Uuid).toString(), newItem);
        }
    }
    return m_tableModel;
}

/**************************************************************************
 * 函数名称： updateOnlineState
 * 函数功能： 更新好友在线状态
 * 输入参数： QString bool
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-05-08
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void FriendListModel::updateOnlineState(QString uuid, bool isOnLine)
{
    QStandardItem *friendItem = m_uuidItemsMap.value(uuid);

    if (isOnLine == true) {
        friendItem->setData(0, FriendListModel::OnlineState);
    } else {
        friendItem->setData(1, FriendListModel::OnlineState);
    }
}

/**************************************************************************
 * 函数名称： updateOnlineState
 * 函数功能： 更新好友在线状态
 * 输入参数： int bool
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-05-19
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void FriendListModel::updateOnlineState(int friendId, bool isOnLine)
{
    QStandardItem *friendItem = m_idItemsMap.value(friendId);

    if (isOnLine == true) {
        friendItem->setData(FriendListModel::OnlineType::Online, FriendListModel::OnlineState);
    } else {
        friendItem->setData(FriendListModel::OnlineType::Offline, FriendListModel::OnlineState);
    }
}

/**************************************************************************
 * 函数名称： updatePriority
 * 函数功能： 更新好友优先级
 * 输入参数： FriendInfoData *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendListModel::updatePriority(FriendInfoData *friendInfo)
{
    QStandardItem *friendItem = getFriendById(friendInfo->m_friendId);

    friendInfo->m_priority = friendItem->data(FriendListModel::Priority).toInt();
    friendInfo->m_priority = friendInfo->m_priority == 0 ? 1 : 0;

    friendItem->setData(friendInfo->m_priority, FriendListModel::Priority);

    return FriendInfoDataBase::getInstance()->updatePriority(friendInfo);
}

/**************************************************************************
 * 函数名称： updateNickname
 * 函数功能： 更新好友备注
 * 输入参数： FriendInfoData *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendListModel::updateNickname(FriendInfoData *friendInfo)
{
    if (friendInfo->m_nickname != "") {
        if (QString(friendInfo->m_nickname.at(0)) >= 'a' && QString(friendInfo->m_nickname.at(0)) <= 'z') {
            friendInfo->m_avatarUrl = QString(friendInfo->m_nickname.at(0)).toUpper();

        } else {
            friendInfo->m_avatarUrl = QString(friendInfo->m_nickname.at(0));
        }
    }

    QStandardItem *friendItem = getFriendById(friendInfo->m_friendId);

    if (friendItem != nullptr) {
        friendItem->setData(friendInfo->m_nickname, FriendListModel::Nickname);
        friendItem->setData(friendInfo->m_avatarUrl, FriendListModel::Avatar);
    }

    emit updateFriendInfo(friendInfo->m_friendId);

    return FriendInfoDataBase::getInstance()->updateNickname(friendInfo);
}

/**************************************************************************
 * 函数名称： updateMessage
 * 函数功能： 更新好友聊天消息 时间 消息数
 * 输入参数： FriendInfoData *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendListModel::updateMessage(FriendInfoData *friendInfo)
{
    QString realMsgContent = GlobalUtils::uncryptData(friendInfo->m_recentMsgCont);

    QStandardItem *friendItem = getFriendById(friendInfo->m_friendId);

    if (friendItem != nullptr) {
        friendItem->setData(realMsgContent, FriendListModel::RecentMsgContent);
        friendItem->setData(friendInfo->m_recentMsgTime, FriendListModel::RecentMsgTime);
        friendItem->setData(friendInfo->m_unreadMsgNum, FriendListModel::UnreadMsgNum);
    }

    return FriendInfoDataBase::getInstance()->updateMessage(friendInfo);
}

/**************************************************************************
 * 函数名称： clearUnreadMsg
 * 函数功能： 清空好友未读消息数
 * 输入参数： int
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-06-01
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendListModel::clearUnreadMsgNum(int friendId)
{
    QStandardItem *friendItem = getFriendById(friendId);

    if (friendItem != nullptr) {
        friendItem->setData(0, FriendListModel::UnreadMsgNum);
    }

    return FriendInfoDataBase::getInstance()->clearUnreadMsgNum(friendId);
}

// 清空所有好友聊天消息 时间 消息数
bool FriendListModel::clearAllMessage()
{
    QHash<int, QStandardItem *>::iterator friendIt = m_idItemsMap.begin();

    while (friendIt != m_idItemsMap.end()) {

        QStandardItem *friendItem = friendIt.value();

        friendItem->setData("", FriendListModel::RecentMsgContent);
        friendItem->setData("", FriendListModel::RecentMsgTime);
        friendItem->setData(0, FriendListModel::UnreadMsgNum);

        friendIt++;
    }

    return FriendInfoDataBase::getInstance()->clearAllMessage();
}

/**************************************************************************
 * 函数名称： getFriendById
 * 函数功能： 通过id获取好友item
 * 输入参数： int
 * 返回数值： QStandardItem *
 * 创建人员：
 * 创建时间： 2021-05-15
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QStandardItem *FriendListModel::getFriendById(int id)
{
    QStandardItem *resItem = nullptr;

    if (m_idItemsMap.contains(id)) {
        resItem = m_idItemsMap.value(id);
    }

    return resItem;
}

/**************************************************************************
 * 函数名称： getFriendByUuid
 * 函数功能： 通过uuid获取好友item
 * 输入参数： QString
 * 返回数值： QStandardItem *
 * 创建人员：
 * 创建时间： 2021-05-15
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QStandardItem *FriendListModel::getFriendByUuid(QString uuid)
{
    QStandardItem *resItem = nullptr;

    if (m_uuidItemsMap.contains(uuid)) {
        resItem = m_uuidItemsMap.value(uuid);
    }

    return resItem;
}

/**************************************************************************
 * 函数名称： getUuidById
 * 函数功能： 通过id获取好友uuid
 * 输入参数： int
 * 返回数值： QString
 * 创建人员：
 * 创建时间： 2021-05-21
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QString FriendListModel::getUuidById(int id)
{
    QString resUuid = "";
    QStandardItem *resItem = nullptr;

    if (m_idItemsMap.contains(id)) {
        resItem = m_idItemsMap.value(id);
        resUuid = resItem->data(FriendListModel::Uuid).toString();
    }

    return resUuid;
}

/**************************************************************************
 * 函数名称： getIdByUuid
 * 函数功能： 通过uuid获取好友id
 * 输入参数： QString
 * 返回数值： int
 * 创建人员：
 * 创建时间： 2021-05-21
 * 修改人员：
 * 修改时间：
 **************************************************************************/
// 通过uuid获取好友id
int FriendListModel::getIdByUuid(QString uuid)
{
    int resId = -1;
    QStandardItem *resItem = nullptr;

    if (m_uuidItemsMap.contains(uuid)) {
        resItem = m_uuidItemsMap.value(uuid);
        resId = resItem->data(FriendListModel::Id).toInt();
    }

    return resId;
}
