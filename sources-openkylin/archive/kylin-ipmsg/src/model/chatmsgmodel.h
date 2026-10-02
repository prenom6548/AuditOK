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

#ifndef CHATMSGMODEL_H
#define CHATMSGMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDebug>

#include "global/database/chatmsgdb.h"
#include "model/friendlistmodel.h"

class ChatMsgModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit ChatMsgModel(int friendId = -1, QString friendUuid = "");
    ~ChatMsgModel();

    // 通过msgId获取item
    QStandardItem *getMsgById(int msgId);

    // 测试添加消息
    void testAddChatMsg(int iSend);

    // 添加消息
    bool addChatMsg(ChatMsgInfo *chatMsgInfo);

    // 删除消息（聊天窗口）
    void delChatMsg(int msgId, int row);

    // 删除消息(搜索记录窗口)
    void delSearchMsg(int msgId);

    // 清空消息
    void clearChatMsg();

    // 更新消息发送状态
    bool updateMsgState(ChatMsgInfo *chatMsgInfo);

    // 更新文件传输大小
    void updateTransferSize(ChatMsgInfo *chatMsgInfo);

    // 更新好友列表信息
    void updateListInfo(int msgId = -1);

    // 更新日期短格式和时间的格式
    void updateDateTimeFormat(QString);

    // 从数据库获取好友信息
    QSqlTableModel *getModelFromDb();

    // 获取时间item
    QStandardItem *addTimeItem(QString fristTimeStr, QString secondTimeStr = "");

    // 设置当前信息是否被选中
    void setMsgChooseState(ChatMsgInfo *chatMsgInfo);

    enum ChatMsgRoles {
        MsgId = Qt::UserRole + 1,
        FriendId,
        FriendUuid,
        IsSend,
        MsgTime,
        MsgContent,
        MsgType,
        FilePath,
        SendState,
        ReadState,
        TotalSize,
        TransferSize,
        MsgCount,
        ChooseStateType,
        DisplayMsgTime, /* 用于"日期短格式+时间"的更新，无其它作用 */
    };

    enum MessageType {
        TextMsg = 0,
        FileMsg,
        DirMsg,
        TimeMsg,
        ImageMsg,
        VideoMsg,
        LinkMsg,
    };

    enum SendType {
        RecvMsg = 0,
        SendMsg,
    };

    enum MesssageState {
        SendDefault = 0,
        SendFailed,
        SendSucceed,
    };

    enum ChooseMsgState {
        ChooseCancel = 0,
        ChooseConfirm,
    };

    // 数据库model
    QSqlTableModel *m_tableModel;

    /*<"msgId" , QStandardItem *>*/
    QHash<int, QStandardItem *> m_msgIdItemsMap;

private:
    int m_friendId;
    QString m_friendUuid;

    QList<ChatMsgInfo *> m_msgList;
};





#endif // CHATMSGMODEL_H
