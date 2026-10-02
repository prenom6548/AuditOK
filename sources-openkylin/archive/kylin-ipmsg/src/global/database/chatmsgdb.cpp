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

#include "chatmsgdb.h"

ChatMsgInfo::ChatMsgInfo(int msgId, int friendId, QString friendUuid, int isSend, QString msgTime, QString msgContent,
                         int msgType, QString filePath, int sendState, int readState, qint64 totalSize,
                         qint64 transferSize)
{
    m_msgId = msgId;
    m_friendId = friendId;
    m_friendUuid = friendUuid;
    m_isSend = isSend;
    m_msgTime = msgTime;
    m_msgContent = msgContent;
    m_msgType = msgType;
    m_filePath = filePath;
    m_sendState = sendState;
    m_readState = readState;
    m_totalSize = totalSize;
    m_transferSize = transferSize;
    m_displayMsgTime = m_msgTime;
}

ChatMsgInfo::~ChatMsgInfo() {}


ChatMsgDB::ChatMsgDB(QObject *parent) : QObject(parent)
{
    m_dbName = "kylin-ipmsg.db";
    m_dbTableName = "chat_msg";

    m_db = DataBase::getInstance()->openDatabse(m_dbTableName);

    m_query = QSqlQuery(m_db);

    // 使用model
    m_model = new QSqlTableModel(this, m_db);
    m_model->setObjectName("kylin-ipmsg_chatmsgdb_QSqlTableModel_model");
    m_model->setTable(m_dbTableName);
    // 提交后才修改数据库
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_model->select();

    while (m_model->canFetchMore()) {
        m_model->fetchMore();
    }
}

ChatMsgDB::~ChatMsgDB() {}

/**************************************************************************
 * 函数名称： getInstance
 * 函数功能： 单例，初始化返回指针，完成数据库文件建立
 * 输入参数： 无
 * 返回数值： ChatMsgDB *
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
ChatMsgDB *ChatMsgDB::getInstance()
{
    static ChatMsgDB *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new ChatMsgDB();
            instance->setObjectName("kylin-ipmsg_ChatMsgDB_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new ChatMsgDB();
        instance->setObjectName("kylin-ipmsg_ChatMsgDB_instance");
#endif
    }

    return instance;
}

/**************************************************************************
 * 函数名称： insertData
 * 函数功能： 插入数据
 * 输入参数： ChatMsgInfo
 * 返回数值： int
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
// TODO 数据加密处理
int ChatMsgDB::insertData(ChatMsgInfo *chatMsg)
{
    m_model->select();
    int row = m_model->rowCount();
    int index = 0;

    m_model->insertRow(row);

    // 第一列是自增主键
    index++;

    m_model->setData(m_model->index(row, index++), chatMsg->m_friendId);
    m_model->setData(m_model->index(row, index++), chatMsg->m_friendUuid);
    m_model->setData(m_model->index(row, index++), chatMsg->m_isSend);
    m_model->setData(m_model->index(row, index++), chatMsg->m_msgTime);
    m_model->setData(m_model->index(row, index++), chatMsg->m_msgContent);
    m_model->setData(m_model->index(row, index++), chatMsg->m_msgType);
    m_model->setData(m_model->index(row, index++), chatMsg->m_filePath);
    m_model->setData(m_model->index(row, index++), chatMsg->m_sendState);
    m_model->setData(m_model->index(row, index++), chatMsg->m_readState);
    m_model->setData(m_model->index(row, index++), chatMsg->m_totalSize);
    m_model->setData(m_model->index(row, index++), chatMsg->m_transferSize);

    // 提交修改
    m_model->submitAll();
    m_model->select();

    while (m_model->canFetchMore()) {
        m_model->fetchMore();
    }

    int resMsgId = m_model->record(row).value(0).toInt();

    return resMsgId;
}

/**************************************************************************
 * 函数名称： queryData
 * 函数功能： 查询数据
 * 输入参数： 无
 * 返回数值： void
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
void ChatMsgDB::queryData()
{
    m_model->select();
}

/**************************************************************************
 * 函数名称： deleteMsg
 * 函数功能： 删除数据
 * 输入参数： int
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgDB::deleteMsg(int msgId)
{
    QString queryIndex = QString("delete from %1 where msg_id = ?").arg(m_dbTableName);

    m_query.prepare(queryIndex);
    m_query.addBindValue(msgId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： clearMsgByFriendId
 * 函数功能： 清空消息
 * 输入参数： int
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgDB::clearMsgByFriendId(int friendId)
{
    QString deleteIndex = QString("delete from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(deleteIndex);
    m_query.addBindValue(friendId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： clearAllMsg
 * 函数功能： 清空所有消息
 * 输入参数： 无
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-06-15
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgDB::clearAllMsg()
{
    QString deleteIndex = QString("delete from %1 where friend_id != -1").arg(m_dbTableName);

    m_query.prepare(deleteIndex);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： queryMsgById
 * 函数功能： 根据ID获取聊天信息
 * 输入参数： int
 * 返回数值： ChatMsgInfo *
 * 创建人员：
 * 创建时间： 2021-05-17
 * 修改人员：
 * 修改时间：
 **************************************************************************/
ChatMsgInfo *ChatMsgDB::queryMsgById(int msgId)
{
    // 使用query
    QString querySql = QString("select * from %1 where msg_id = ?").arg(m_dbTableName);

    m_query.prepare(querySql);
    m_query.addBindValue(msgId);
    m_query.exec();

    ChatMsgInfo *chatMsg = new ChatMsgInfo();

    if (m_query.next()) {
        chatMsg->m_msgId = m_query.value("msg_id").toInt();
        chatMsg->m_friendId = m_query.value("friend_id").toInt();
        chatMsg->m_friendUuid = m_query.value("friend_uuid").toString();
        chatMsg->m_isSend = m_query.value("is_send").toInt();
        chatMsg->m_msgTime = m_query.value("msg_time").toString();
        chatMsg->m_msgContent = m_query.value("msg_content").toString();
        chatMsg->m_msgType = m_query.value("msg_type").toInt();
        chatMsg->m_filePath = m_query.value("file_path").toString();
        chatMsg->m_sendState = m_query.value("send_state").toInt();
        chatMsg->m_readState = m_query.value("read_state").toInt();
        chatMsg->m_totalSize = m_query.value("total_size").toLongLong();
        chatMsg->m_transferSize = m_query.value("transfer_size").toLongLong();
    }

    return chatMsg;
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
bool ChatMsgDB::updateMsgState(ChatMsgInfo *chatMsgInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set send_state = ? where msg_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(chatMsgInfo->m_sendState);
    m_query.addBindValue(chatMsgInfo->m_msgId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： updateTransferSize
 * 函数功能： 更新文件传输大小
 * 输入参数： ChatMsgInfo *
 * 返回数值： bool
 * 创建人员：
 * 创建时间： 2021-06-04
 * 修改人员：
 * 修改时间：
 **************************************************************************/
bool ChatMsgDB::updateTransferSize(ChatMsgInfo *chatMsgInfo)
{
    // 使用query
    QString updateSql = QString("update %1 set total_size = ?, transfer_size = ? where msg_id = ?").arg(m_dbTableName);

    m_query.prepare(updateSql);
    m_query.addBindValue(chatMsgInfo->m_totalSize);
    m_query.addBindValue(chatMsgInfo->m_transferSize);
    m_query.addBindValue(chatMsgInfo->m_msgId);

    return m_query.exec();
}

/**************************************************************************
 * 函数名称： queryMsgByUuid
 * 函数功能： 根据uuid获取聊天信息
 * 输入参数： QString
 * 返回数值： QList<ChatMsgInfo *>
 * 创建人员：
 * 创建时间： 2021-05-18
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QList<ChatMsgInfo *> ChatMsgDB::queryMsgByUuid(QString uuid)
{
    // 使用query
    QString querySql = QString("select * from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(querySql);
    m_query.addBindValue(uuid);
    m_query.exec();

    QList<ChatMsgInfo *> resMsgList;

    while (m_query.next()) {
        ChatMsgInfo *chatMsg = new ChatMsgInfo();

        chatMsg->m_msgId = m_query.value("msg_id").toInt();
        chatMsg->m_friendId = m_query.value("friend_id").toInt();
        chatMsg->m_friendUuid = m_query.value("friend_uuid").toString();
        chatMsg->m_isSend = m_query.value("is_send").toInt();
        chatMsg->m_msgTime = m_query.value("msg_time").toString();
        chatMsg->m_msgContent = m_query.value("msg_content").toString();
        chatMsg->m_msgType = m_query.value("msg_type").toInt();
        chatMsg->m_filePath = m_query.value("file_path").toString();
        chatMsg->m_sendState = m_query.value("send_state").toInt();
        chatMsg->m_readState = m_query.value("read_state").toInt();
        chatMsg->m_totalSize = m_query.value("total_size").toLongLong();
        chatMsg->m_transferSize = m_query.value("transfer_size").toLongLong();

        resMsgList.append(chatMsg);
    }

    return resMsgList;
}

/**************************************************************************
 * 函数名称： queryMsgByFriendId
 * 函数功能： 根据friendId获取聊天信息
 * 输入参数： int
 * 返回数值： QList<ChatMsgInfo *>
 * 创建人员：
 * 创建时间： 2021-05-19
 * 修改人员：
 * 修改时间：
 **************************************************************************/
QList<ChatMsgInfo *> ChatMsgDB::queryMsgByFriendId(int friendId)
{
    // 使用query
    QString querySql = QString("select * from %1 where friend_id = ?").arg(m_dbTableName);

    m_query.prepare(querySql);
    m_query.addBindValue(friendId);
    m_query.exec();

    QList<ChatMsgInfo *> resMsgList;

    while (m_query.next()) {
        ChatMsgInfo *chatMsg = new ChatMsgInfo();

        chatMsg->m_msgId = m_query.value("msg_id").toInt();
        chatMsg->m_friendId = m_query.value("friend_id").toInt();
        chatMsg->m_friendUuid = m_query.value("friend_uuid").toString();
        chatMsg->m_isSend = m_query.value("is_send").toInt();
        chatMsg->m_msgTime = m_query.value("msg_time").toString();
        chatMsg->m_msgContent = m_query.value("msg_content").toString();
        chatMsg->m_msgType = m_query.value("msg_type").toInt();
        chatMsg->m_filePath = m_query.value("file_path").toString();
        chatMsg->m_sendState = m_query.value("send_state").toInt();
        chatMsg->m_readState = m_query.value("read_state").toInt();
        chatMsg->m_totalSize = m_query.value("total_size").toLongLong();
        chatMsg->m_transferSize = m_query.value("transfer_size").toLongLong();

        resMsgList.append(chatMsg);
    }

    // qDebug() << "resMsgList.size" << resMsgList.size();

    return resMsgList;
}
