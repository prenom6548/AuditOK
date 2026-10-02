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

#include "friendlistdb.h"

FriendInfoData::FriendInfoData(int friendId, QString friendUuid, QString friendIp, int friendPort, QString friendMac,
                               QString username, QString nickname, QString system, QString platform, QString avatarUrl,
                               QString recentMsgCont, QString recentMsgTime, int unreadMsgNum, int onlineState,
                               int priority, int display)
{
    this->m_friendId = friendId;
    this->m_friendUuid = friendUuid;
    this->m_friendIp = friendIp;
    this->m_friendPort = friendPort;
    this->m_friendMac = friendMac;
    this->m_username = username;
    this->m_nickname = nickname;
    this->m_system = system;
    this->m_platform = platform;
    this->m_avatarUrl = avatarUrl;
    this->m_recentMsgCont = recentMsgCont;
    this->m_recentMsgTime = recentMsgTime;
    this->m_unreadMsgNum = unreadMsgNum;
    this->m_onlineState = onlineState;
    this->m_priority = priority;
    this->m_display = display;
}

FriendInfoData::FriendInfoData(QMap<QString, QString> strMap, QMap<QString, int> intMap)
{
    // if (strMap.isEmpty() || intMap.isEmpty()) {
    //     return ;
    // }

    // QString类型变量赋值
    if (strMap.contains("m_friendUuid")) {
        m_friendUuid = strMap["m_friendUuid"];
    }
    if (strMap.contains("m_friendIp")) {
        m_friendIp = strMap["m_friendIp"];
    }
    if (strMap.contains("m_friendMac")) {
        m_friendMac = strMap["m_friendMac"];
    }
    if (strMap.contains("m_username")) {
        m_username = strMap["m_username"];
    }
    if (strMap.contains("m_nickname")) {
        m_nickname = strMap["m_nickname"];
    }
    if (strMap.contains("m_system")) {
        m_system = strMap["m_system"];
    }
    if (strMap.contains("m_platform")) {
        m_platform = strMap["m_platform"];
    }
    if (strMap.contains("m_avatarUrl")) {
        m_avatarUrl = strMap["m_avatarUrl"];
    }
    if (strMap.contains("m_recentMsgCont")) {
        m_recentMsgCont = strMap["m_recentMsgCont"];
    }
    if (strMap.contains("m_recentMsgTime")) {
        m_recentMsgTime = strMap["m_recentMsgTime"];
    }

    // int类型变量赋值
    if (intMap.contains("m_friendId")) {
        m_friendId = intMap["m_friendId"];
    }
    if (intMap.contains("m_friendPort")) {
        m_friendPort = intMap["m_friendPort"];
    }
    if (intMap.contains("m_unreadMsgNum")) {
        m_unreadMsgNum = intMap["m_unreadMsgNum"];
    }
    if (intMap.contains("m_priority")) {
        m_priority = intMap["m_priority"];
    }
}

FriendInfoData::~FriendInfoData() {}


FriendInfoDataBase::FriendInfoDataBase(QObject *parent) : QObject(parent)
{
    m_dbName = "kylin-ipmsg.db";
    m_dbTableName = "friend_info";

    DataBase::getInstance();

    m_db = DataBase::getInstance()->openDatabse(m_dbTableName);

    m_query = QSqlQuery(m_db);

    // 使用model
    m_model = new QSqlTableModel(this, m_db);
    m_model->setObjectName("kylin-ipmsg_friendlistdb_QSqlTableModel_model");
    m_model->setTable(m_dbTableName);
    // 提交后才修改数据库
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_model->select();
}

FriendInfoDataBase::~FriendInfoDataBase() {}

FriendInfoDataBase *FriendInfoDataBase::getInstance()
{
    static FriendInfoDataBase *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new FriendInfoDataBase();
            instance->setObjectName("kylin-ipmsg_FriendInfoDataBase_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new FriendInfoDataBase();
        instance->setObjectName("kylin-ipmsg_FriendInfoDataBase_instance");
#endif
    }

    return instance;
}

// 插入数据
// TODO 数据加密处理
int FriendInfoDataBase::insertData(FriendInfoData *friendInfo)
{
    int row = m_model->rowCount();
    int index = 0;

    m_model->insertRow(row);

    // 第一列是自增主键
    index++;

    m_model->setData(m_model->index(row, index++), friendInfo->m_friendUuid);
    m_model->setData(m_model->index(row, index++), friendInfo->m_friendIp);
    m_model->setData(m_model->index(row, index++), friendInfo->m_friendPort);
    m_model->setData(m_model->index(row, index++), friendInfo->m_friendMac);
    m_model->setData(m_model->index(row, index++), friendInfo->m_username);
    m_model->setData(m_model->index(row, index++), friendInfo->m_nickname);
    m_model->setData(m_model->index(row, index++), friendInfo->m_system);
    m_model->setData(m_model->index(row, index++), friendInfo->m_platform);
    m_model->setData(m_model->index(row, index++), friendInfo->m_avatarUrl);
    m_model->setData(m_model->index(row, index++), friendInfo->m_recentMsgCont);
    m_model->setData(m_model->index(row, index++), friendInfo->m_recentMsgTime);
    m_model->setData(m_model->index(row, index++), friendInfo->m_unreadMsgNum);
    m_model->setData(m_model->index(row, index++), friendInfo->m_priority);

    // 提交修改
    m_model->submitAll();
    m_model->select();

    int resFriendId = m_model->record(row).value(0).toInt();

    return resFriendId;
}

// 查询数据
void FriendInfoDataBase::queryData()
{
    m_model->select();
    // return m_model;
}

// 更新数据
bool FriendInfoDataBase::updateData(FriendInfoData *friendInfo)
{
    if (m_query.driver() == nullptr) {
        m_query = QSqlQuery(m_db);
    }
    // 示例
    // 使用model必须要获取到行号和列号
    int row = 0;
    int col = 0;

    m_model->setData(m_model->index(row, col), friendInfo->m_priority);

    m_model->submitAll();

    // 使用query获取行号
    QString queryIndex = QString("SELECT rowid from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(queryIndex);
    m_query.addBindValue(friendInfo->m_friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： deleteFriend
 * 函数功能： 删除好友
 * 输入参数： int
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendInfoDataBase::deleteFriend(int friendId)
{
    // 使用query
    QString deleteSql = QString("delete from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(deleteSql);
    m_query.addBindValue(friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： updateFriend
 * 函数功能： 更新好友上线信息
 * 输入参数： FriendInfoData *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendInfoDataBase::updateFriend(FriendInfoData *friendInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set friend_ip = ?, friend_port = ?, friend_mac = ?,\
                                 user_name = ?, avatar_url = ?, display = ? where friend_id = ?")
                            .arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(friendInfo->m_friendIp);
    m_query.addBindValue(friendInfo->m_friendPort);
    m_query.addBindValue(friendInfo->m_friendMac);
    m_query.addBindValue(friendInfo->m_username);
    m_query.addBindValue(friendInfo->m_avatarUrl);
    m_query.addBindValue(friendInfo->m_display);
    m_query.addBindValue(friendInfo->m_friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称：updatePriority
 * 函数功能：修改好友的优先级
 * 输入参数：FriendInfoData
 * 返回数值：bool
 * 创建人员：
 * 创建时间：2021-05-06
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendInfoDataBase::updatePriority(FriendInfoData *friendInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set priority = ? where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(friendInfo->m_priority);
    m_query.addBindValue(friendInfo->m_friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称：updateNickname
 * 函数功能：修改好友的备注名
 * 输入参数：FriendInfoData
 * 返回数值：bool
 * 创建人员：
 * 创建时间：2021-05-06
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool FriendInfoDataBase::updateNickname(FriendInfoData *friendInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set nickname = ?, avatar_url = ? where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(friendInfo->m_nickname);
    m_query.addBindValue(friendInfo->m_avatarUrl);
    m_query.addBindValue(friendInfo->m_friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： updateMessage
 * 函数功能： 更新好友聊天消息 时间 消息数
 * 输入参数： FriendInfoData
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-25
 * 修改人员：
 * 修改时间：
 **************************************************************************/
//
bool FriendInfoDataBase::updateMessage(FriendInfoData *friendInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set recent_msg_content = ?, recent_msg_time = ?,\
                                 unread_msg_num = ? where friend_id = ?")
                            .arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(friendInfo->m_recentMsgCont);
    m_query.addBindValue(friendInfo->m_recentMsgTime);
    m_query.addBindValue(friendInfo->m_unreadMsgNum);
    m_query.addBindValue(friendInfo->m_friendId);

    return m_query.exec();
}

// 清空所有好友聊天消息 时间 消息数
bool FriendInfoDataBase::clearAllMessage()
{
    // 使用query
    QString updateSql = QString("update %1 set recent_msg_content = '', recent_msg_time = '',\
                                 unread_msg_num = 0 where friend_id != -1")
                            .arg(m_dbTableName);

    m_query.prepare(updateSql);

    return m_query.exec();
}

// 更新好友显示状态
bool FriendInfoDataBase::updateDisplay(int friendId, int display)
{
    // 使用query
    QString updateSql = QString("update %1 set display = ? where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(display);
    m_query.addBindValue(friendId);

    return m_query.exec();
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
bool FriendInfoDataBase::clearUnreadMsgNum(int friendId)
{
    // 使用query
    QString updateSql = QString("update %1 set unread_msg_num = ? where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(0);
    m_query.addBindValue(friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： queryInfoById
 * 函数功能： 根据ID获取好友信息
 * 输入参数： int
 * 返回数值： FriendInfoData
 * 创建人员：
 * 创建时间： 2021-05-06
 * 修改人员：
 * 修改时间：
 **************************************************************************/
FriendInfoData *FriendInfoDataBase::queryInfoById(int friendId)
{
    // 使用query
    QString querySql = QString("select * from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(querySql);
    m_query.addBindValue(friendId);
    m_query.exec();

    FriendInfoData *friendInfo = new FriendInfoData();
    friendInfo->setObjectName("kylin-ipmsg_friendlistdb_FriendInfoData_friendInfo");

    if (m_query.next()) {
        friendInfo->m_friendId = m_query.value("friend_id").toInt();
        friendInfo->m_friendUuid = m_query.value("friend_uuid").toString();
        friendInfo->m_friendIp = m_query.value("friend_ip").toString();
        friendInfo->m_friendPort = m_query.value("friend_port").toInt();
        friendInfo->m_friendMac = m_query.value("friend_mac").toString();
        friendInfo->m_username = m_query.value("user_name").toString();
        friendInfo->m_nickname = m_query.value("nickname").toString();
        friendInfo->m_system = m_query.value("system").toString();
        friendInfo->m_platform = m_query.value("platform").toString();
        friendInfo->m_avatarUrl = m_query.value("avatar_url").toString();
        friendInfo->m_recentMsgCont = m_query.value("recent_msg_content").toString();
        friendInfo->m_recentMsgTime = m_query.value("recent_msg_time").toString();
        friendInfo->m_unreadMsgNum = m_query.value("unread_msg_num").toInt();
        friendInfo->m_priority = m_query.value("priority").toInt();
    }

    return friendInfo;
}

/**************************************************************************
 * 函数名称： queryInfoByUuid
 * 函数功能： 根据UUID获取好友信息
 * 输入参数： friendUuid
 * 返回数值： FriendInfoData
 * 创建人员：
 * 创建时间： 2021-05-06
 * 修改人员：
 * 修改时间：
 **************************************************************************/
FriendInfoData *FriendInfoDataBase::queryInfoByUuid(QString friendUuid)
{
    // 使用query
    QString querySql = QString("select * from %1 where friend_uuid = ?").arg(m_dbTableName);

    m_query.prepare(querySql);
    m_query.addBindValue(friendUuid);
    m_query.exec();

    FriendInfoData *friendInfo = new FriendInfoData();
    friendInfo->setObjectName("kylin-ipmsg_friendlistdb_FriendInfoData_friendInfo");

    if (m_query.next()) {
        friendInfo->m_friendId = m_query.value("friend_id").toInt();
        friendInfo->m_friendUuid = m_query.value("friend_uuid").toString();
        friendInfo->m_friendIp = m_query.value("friend_ip").toString();
        friendInfo->m_friendPort = m_query.value("friend_port").toInt();
        friendInfo->m_friendMac = m_query.value("friend_mac").toString();
        friendInfo->m_username = m_query.value("user_name").toString();
        friendInfo->m_nickname = m_query.value("nickname").toString();
        friendInfo->m_system = m_query.value("system").toString();
        friendInfo->m_platform = m_query.value("platform").toString();
        friendInfo->m_avatarUrl = m_query.value("avatar_url").toString();
        friendInfo->m_recentMsgCont = m_query.value("recent_msg_content").toString();
        friendInfo->m_recentMsgTime = m_query.value("recent_msg_time").toString();
        friendInfo->m_unreadMsgNum = m_query.value("unread_msg_num").toInt();
        friendInfo->m_priority = m_query.value("priority").toInt();
        friendInfo->m_display = m_query.value("display").toInt();
    }

    // qDebug() << friendInfo->m_friendId << friendInfo->m_friendUuid;

    return friendInfo;
}
