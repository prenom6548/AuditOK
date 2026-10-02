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

#ifndef __TCP_MODULE_H__
#define __TCP_MODULE_H__

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QVector>

#include "global/declare/declare.h"
#include "global/utils/global_data.h"
#include "protocol_analysis.h"
#include "model/chatmsgmodel.h"

class TcpModule : public QObject
{
    Q_OBJECT
    friend class TestTcpModule;
    friend class TestTcpModule_ConstructorInit_Test;
    friend class TestTcpModule_TaskQueueEmptyOnInit_Test;
    friend class TestTcpModule_SlotTaskProcessCoverage_Test;
    friend class TestTcpModule_TarCommandCoverage_Test;
    friend class TestTcpModule_ConnTimeoutCoverage_Test;
    friend class TestTcpModule_TranTextAndFileCoverage_Test;
    friend class TestTcpModule_SlotReadSocketCoverage_Test;
    friend class TestTcpModule_SocketDisconnectCoverage_Test;
    friend class TestTcpModule_CoverageAdditions_Test;
    friend class TestTcpModule_CheckLinkFileWithSymlink_Test;
    friend class TestTcpModule_ConnTimeoutWithMsgQueue_Test;
    friend class TestTcpModule_ConnTimeoutWithFileQueue_Test;
    friend class TestTcpModule_ConnTimeoutRetry_Test;
    friend class TestTcpModule_DeleteTmpFileExists_Test;
    friend class TestTcpModule_DeleteTmpFileEmptyPath_Test;
    friend class TestTcpModule_RecvFileHandleDuplicateName_Test;
    friend class TestTcpModule_RecvFileDataWithInvalidFd_Test;
    friend class TestTcpModule_RecvFileCreateSavePath_Test;
    friend class TestTcpModule_RecvFileDirDecompress_Test;
    friend class TestTcpModule_RecvFileDirDecompressWithExisting_Test;
    friend class TestTcpModule_TranTextWithRealConnection_Test;
    friend class TestTcpModule_TranFileWithRealConnection_Test;
    friend class TestTcpModule_TranFileDirWithRealConnection_Test;
    friend class TestTcpModule_RecvFileWriteFail_Test;
    friend class TestTcpModule_CheckLinkFileLoopCoverage_Test;
    friend class TestTcpModule_SlotReadSocketWithData_Test;
    friend class TestTcpModule_RecvFileSavePathNoSlash_Test;
    friend class TestTcpModule_RecvFileMultipleDuplicateNames_Test;
    friend class TestTcpModule_RecvFileDuplicateNoSuffix_Test;
    friend class TestTcpModule_RecvFileDirDuplicateName_Test;
    friend class TestTcpModule_SlotReadSocketFirstReceive_Test;
    friend class TestTcpModule_SlotReadSocketUuidMismatch_Test;
    friend class TestTcpModule_TranFileSecurityCheck_Test;
    friend class TestTcpModule_RecvFileSecurityCheck_Test;
    friend class TestTcpModule_EncapsulationFileInfoDir_Test;
    friend class TestTcpModule_TranFileCancelDuringSend_Test;
    friend class TestTcpModule_TarCompressSuccess_Test;
    friend class TestTcpModule_TarDecompressSuccess_Test;

public:
    TcpModule(qintptr socketDescriptor);
    TcpModule(QString friendIp, QString friendPort, QString friendUuid = "");
    ~TcpModule();

    QQueue<ChatMsgInfo *> m_msgTaskQueue; /* 消息任务队列 */ /* 优先级高 */
    QQueue<ChatMsgInfo *> m_fileTaskQueue;                   /* 文件任务队列 */
    QVector<int> m_cancelTask;                               /* 文件取消发送任务队列 */

private:
    void recvMsg(QMap<QByteArray, QByteArray> package);
    void recvFile(QMap<QByteArray, QByteArray> package);
    void tranFile(ChatMsgInfo *send);
    void tranText(ChatMsgInfo *send);
    void establishInterrupt(void);
    QByteArray encapsulationFileInfo(ChatMsgInfo *send, int flag); /* flag : 0 ---> file , 1 ---> dir */
    QByteArray encapsulationFileEnd(ChatMsgInfo *send);
    void deleteTmpFile(char *filePath);
    void finishModuleThread();

    bool judgeCancel(ChatMsgInfo *);

    /* 代码安全规范 */
    int cmd_exec_tar_decompress(QString dstFileName, QString savePath);
    int cmd_exec_tar_compress(QString dstPath, QString detFileName, QString srcPath, QString baseName);
    int verify_file(char *const filename);
    bool checkLinkFile(QString dstFileName);

private:
    QTcpSocket *m_socket;
    ProtocolAnalysis m_moduleProtocolAnalysis;

    bool m_flag;          /* 标识 tcp 链接是否可用 */
    QString m_uuid;       /* 对端 uuid */
    QString m_friendIp;   /* 对端 ip */
    QString m_friendPort; /* 对端 port */

    /* 用于接收文件的变量 */
    bool m_recv_flag = false;   /* 判断是否接收文件数据 */
    int m_fd = -1;              /* 文件描述符 */
    char m_fileType[125] = {};  /* 文件类型 */
    char m_fileName[1024] = {}; /* 文件名 */
    int m_msgType = 0;
    long long int m_fileTotalSize = 0; /* 文件总大小 */
    long long int m_fileRecvSize = 0;  /* 接收文件总大小 */
    char m_tmpFilePath[1024 * 2] = {}; /* 缓存文件路径 */
    char m_savePath[512] = {};         /* 文件保存路径 */
    int m_i = 0;                       /* 递增变量 ， 解决文件重名问题 */
    QString m_dstFileName;             /* 接收到的文件的最终命名 */

    /* 连接超时计时器 */
    QTimer *m_connTimer;
    int m_timeoutCount; /* 超时次数 */

signals:
    void sigTaskAdd();

    void sigMaintainTcpLinkAdd(g_tcpItem item);
    void sigMaintainTcpLinkDelete(QString uuid);

    void sigSendMsgSuccess(ChatMsgInfo *send);
    void sigRecvMsgSuccess(ChatMsgInfo *recv);
    void sigSendFileSuccess(ChatMsgInfo *send);
    void sigRecvFileSuccess(ChatMsgInfo *recv);
    void sigSendFileFail(ChatMsgInfo *send);
    void sigSendFileProgress(ChatMsgInfo *send);
    void sigFriendOffline(int friendId);
    void sigFinishThread();

public slots:
    void startSocket();

private slots:
    void slotTaskProcess(void);

    void slotSocketConnect(void);
    void slotReadSocket(void);

    void slotSocketDisconnect(void);
    void slotConnTimeout();
    void slotSocketError(QAbstractSocket::SocketError);
};

#endif
