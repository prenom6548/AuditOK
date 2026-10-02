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

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>

#include <QFileInfo>

#include "tcp_module.h"
#include "tcp_client.h"
#include "global/utils/inisetting.h"
#include "global/utils/globalutils.h"
#include <QDir>

#define PACKAGE_SIZE (1024 * 100)
#define READ_SIZE (1024 * 100 - 1024)
#define RECV_FILE_SAVE_PATH "./"

/* 用于tcp server */
TcpModule::TcpModule(qintptr socketDescriptor)
{
    if (!socketDescriptor) {
        qDebug() << "Error : TcpModule , call server , socket point is NULL";
    }

    this->m_flag = false;
    this->m_uuid = "";

    this->m_socket = new QTcpSocket(this);
    this->m_socket->setObjectName("kylin-ipmsg_tcpmodule_QTcpSocket_socket");
    this->m_socket->setSocketDescriptor(socketDescriptor);

    this->m_msgTaskQueue.clear();
    this->m_fileTaskQueue.clear();
    this->m_cancelTask.clear();

    establishInterrupt();
}

/* 重载构造 ， 用于tcp client */
TcpModule::TcpModule(QString friendIp, QString friendPort, QString friendUuid)
{
    if (friendUuid.isEmpty()) {
        this->m_flag = false;
        this->m_uuid = "";
    } else {
        this->m_flag = true;
        this->m_uuid = friendUuid;
    }

    this->m_friendIp = friendIp;
    this->m_friendPort = friendPort;

    this->m_msgTaskQueue.clear();
    this->m_fileTaskQueue.clear();
    this->m_cancelTask.clear();
}

TcpModule::~TcpModule()
{
    if (this->m_socket != NULL) {
        delete m_socket;
        m_socket = NULL;
    }
}

void TcpModule::startSocket()
{
    m_socket = new QTcpSocket(this);
    m_socket->setObjectName("kylin-ipmsg_tcpmodule_QTcpSocket_socket");

    establishInterrupt();

    this->m_socket->connectToHost(QHostAddress(m_friendIp), m_friendPort.toUShort(), QTcpSocket::ReadWrite);

    /* 连接超时计时器 */
    m_timeoutCount = 0;
    m_connTimer = new QTimer(this);
    m_connTimer->setObjectName("kylin-ipmsg_tcpmodule_QTimer_connTimer");
    connect(m_connTimer, &QTimer::timeout, this, &TcpModule::slotConnTimeout);
    m_connTimer->start(3000);
}

void TcpModule::establishInterrupt(void)
{
    connect(this->m_socket, &QTcpSocket::connected, this, &TcpModule::slotSocketConnect);
    connect(this->m_socket, &QTcpSocket::readyRead, this, &TcpModule::slotReadSocket);
    connect(this->m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(slotSocketError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(this, &TcpModule::sigTaskAdd, this, &TcpModule::slotTaskProcess);
    connect(this, &TcpModule::sigFinishThread, this, &TcpModule::finishModuleThread);

    return;
}

void TcpModule::slotTaskProcess(void)
{
    while (this->m_msgTaskQueue.count() > 0) {
        ChatMsgInfo *tmp = this->m_msgTaskQueue.dequeue();

        if (!judgeCancel(tmp)) {
            this->tranText(tmp);
        }
    }

    while (this->m_fileTaskQueue.count() > 0) {
        ChatMsgInfo *tmp = this->m_fileTaskQueue.dequeue();

        if (!judgeCancel(tmp)) {
            this->tranFile(tmp);
        }
    }

    return;
}

bool TcpModule::judgeCancel(ChatMsgInfo *task)
{
    for (int i = 0; i < this->m_cancelTask.count(); i++) {
        if (this->m_cancelTask.at(i) == task->m_msgId) {
            this->m_cancelTask.remove(i);

            return true;
        }
    }

    return false;
}

int TcpModule::cmd_exec_tar_decompress(QString dstFileName, QString savePath)
{
    if(checkLinkFile(dstFileName)) {
        qDebug() << "( TcpModule::cmd_exec_tar_decompress )  contains linkfile";
        return -1;
    } else {
        /* 代码安全规范 ：使用 fork+evecve执行解压缩操作： tar -zxf \"%1\" -C \"%2\ */

//        qDebug() << "( TcpModule::cmd_exec_tar_decompress ) fork+evecve tart -zxf " << dstFileName << "-C" << savePath;
//        pid_t pid = -1;
//        int status = -1;
//        const char *args[6];
//        const char *des;
//        const char *save;

//        QByteArray des_arr = dstFileName.toLocal8Bit();
//        QByteArray save_arr = savePath.toLocal8Bit();
//        des = des_arr.data();
//        save = save_arr.data();

//        args[0] = "tar";
//        args[1] = "-zxf";
//        args[2] = des;
//        args[3] = "-C";
//        args[4] = save;
//        args[5] = NULL;
//        char **env = NULL;

        qDebug() << "( TcpModule::cmd_exec_tar_decompress ) fork+evecve tart -zxf "
                 << dstFileName
                 << "-C" << savePath;
        int status = -1;
        QProcess tarProcess;
        QStringList arguments;
        arguments << "-zxf"
                  << dstFileName
                  << "-C"
                  << savePath;

        qDebug() << "Executing: tar" << arguments.join(" ");
        tarProcess.start("/usr/bin/tar", arguments);
//        pid = fork();
//        if (pid == (pid_t)0) {
//            /* Child side */
//            (void)execve("/usr/bin/tar",(char *const*)args, env);
//            qWarning() << "( TcpModule::cmd_exec_tar_decompress) pid = " << pid << " execve failed";
//            _exit(127);
//        } else if (pid < (pid_t)0) {
//            /* the fork failed */
//            qDebug() << "( TcpModule::cmd_exec_tar_decompress ) fork failed";
//            status = -1;
//        } else {
//            /* Parent side */

//            int n;
//            do{
//                n = waitpid(pid, &status, 0);
//            }while(n ==-1 && errno == EINTR);

//            if (n != pid) {
//                status = -1;
//            }
//            qDebug() << "( TcpModule::cmd_exec_tar_decompress ) fork parent side end";
//        }

        if (!tarProcess.waitForStarted()) {
            qWarning() << "Failed to start tar process";
            return status;
        }

        if (!tarProcess.waitForFinished()) {
            qWarning() << "Tar process failed or timed out";
            return status;
        }

        if (tarProcess.exitStatus() != QProcess::NormalExit || tarProcess.exitCode() != 0) {
            qWarning() << "Tar process exited abnormally with code" << tarProcess.exitCode()
                       << "Error:" << tarProcess.readAllStandardError();
            status = -1;
        } else {
            qDebug() << "Tar decompression completed successfully";
            status = 0;
        }
        return status;
    }
}

int TcpModule::cmd_exec_tar_compress(QString dstPath, QString detFileName, QString srcPath, QString baseName)
{
    /* 代码安全规范 ：使用 fork+evecve 执行压缩操作： tar -hzcf \"%1\" -C \"%2\  \"%3\*/
//    qDebug() << "( TcpModule::cmd_exec_tar_compress ) fork+evecve tart -hzcf " << dstPath << "/"
//             << detFileName << ".tar.gz" <<"-C" << srcPath << " " << baseName;
//    pid_t pid = -1;
//    int status = -1;
//    const char *args[7];
//    const char *des_path;
//    const char *src_path;
//    const char *file_name;


//    QByteArray des_path_arr = (dstPath + '/' + detFileName + ".tar.gz").toLocal8Bit();
//    QByteArray src_path_arr = srcPath.toLocal8Bit();
//    QByteArray file_name_arr = baseName.toLocal8Bit();
//    des_path = des_path_arr.data();
//    src_path = src_path_arr.data();
//    file_name =file_name_arr.data();

//    qDebug() << "des_path = " << des_path;
//    qDebug() << "src_path = " << src_path;
//    qDebug() << "file_name = " << file_name;

//    args[0] = "tar";
//    args[1] = "-hzcf";
//    args[2] = des_path;
//    args[3] = "-C";
//    args[4] = src_path;
//    args[5] = file_name;
//    args[6] = NULL;
//    char **env = NULL;
    qDebug() << "( TcpModule::cmd_exec_tar_compress ) QProcess tar -hzcf"
             << dstPath << "/" << detFileName << ".tar.gz"
             << "-C" << srcPath << baseName;
    int ret = -1;
    QProcess tarProcess;
    QStringList arguments;
    arguments << "-hzcf"
              << QDir(dstPath).filePath(detFileName + ".tar.gz")
              << "-C"
              << srcPath
              << baseName;

    qDebug() << "Executing: tar" << arguments.join(" ");

    tarProcess.start("/usr/bin/tar", arguments);
    if (!tarProcess.waitForStarted()) {
        qWarning() << "Failed to start tar process:" << tarProcess.errorString();
        return ret;
    }

    if (!tarProcess.waitForFinished()) {
        qWarning() << "Tar process failed:" << tarProcess.errorString();
        return ret;
    }

    if (tarProcess.exitStatus() != QProcess::NormalExit || tarProcess.exitCode() != 0) {
        qWarning() << "Tar command failed with code" << tarProcess.exitCode()
                  << "Error:" << tarProcess.readAllStandardError();
        ret = -1;
    } else {
        qDebug() << "Tar compression completed successfully";
        ret = 0;
    }

//    pid = fork();
//    if (pid == (pid_t)0) {
//        /* Child side */
//        (void)execve("/usr/bin/tar",(char *const*)args, env);
//        qDebug() << "( TcpModule::cmd_exec_tar_compress ) pid = " << pid << "execve failed";
//        _exit(127);
//    } else if (pid < (pid_t)0) {
//        /* the fork failed */
//        qDebug() << "( TcpModule::cmd_exec_tar_compress ) fork failed";
//        status = -1;
//    } else {
//        /* Parent side */
//        int n;
//        do{
//            n = waitpid(pid, &status, 0);
//        }while(n ==-1 && errno == EINTR);

//        if (n != pid) {
//            status = -1;
//        }
//    }
    qDebug() << "( TcpModule::cmd_exec_tar_compress ) cmd tar exit";
    return ret;
}

bool TcpModule::checkLinkFile(QString dstFileName)
{
    //解压前查看是否包含软链接:tar -tvf \"%1\"'
    QProcess tarProcess;
    QStringList tarArguments;
    tarArguments << "-tvf" << dstFileName;
    qDebug() << "( TcpModule::checkLinkFile ) tar -tvf "<< dstFileName;
    tarProcess.start("/usr/bin/tar", tarArguments);
    tarProcess.waitForFinished();
    bool isLink = false;
    while(tarProcess.canReadLine()) {
            QString str = tarProcess.readLine();
            if (str.begin()[0] == 'l') {
                isLink = true;
                break;
            }
    }
    return isLink;
}

/* 主动连接超时处理 */
void TcpModule::slotConnTimeout()
{
    m_timeoutCount++;

    qDebug() << "Info : TcpModule , conn timeout [" << m_timeoutCount << "] times";

    /* 判断超时次数 */
    if (m_timeoutCount >= 3) {
        disconnect(m_connTimer, &QTimer::timeout, this, &TcpModule::slotConnTimeout);

        m_connTimer->stop();

        if (m_connTimer != nullptr) {
            delete m_connTimer;
            m_connTimer = nullptr;
        }

        /* 发送好友下线信号 */
        /* PRO : 好友 id 为前端数据 , 应改为 uuid */
        int friendId = 0;
        if (this->m_msgTaskQueue.count() > 0 || this->m_fileTaskQueue.count() > 0) {
            friendId = this->m_msgTaskQueue.at(0)->m_friendId;
            if (!friendId) {
                friendId = this->m_fileTaskQueue.at(0)->m_friendId;
            }
        }

        while (this->m_msgTaskQueue.count() > 0) {
            emit sigSendFileFail(this->m_msgTaskQueue.dequeue());
        }

        while (this->m_fileTaskQueue.count() > 0) {
            emit sigSendFileFail(this->m_fileTaskQueue.dequeue());
        }

        emit sigFriendOffline(friendId);

        /* PRO : 此处只是移除了 tcp 的信息 , 此处意思是好友已经下线 , 应该移除整条记录 , 有待确认 */
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        qDebug() << "Waring : TcpModule slotConnTimeout conn times more , conn fail ...";

    } else {
        /* 再次链接 */
        this->m_socket->abort();
        this->m_socket->connectToHost(QHostAddress(m_friendIp), m_friendPort.toUShort(), QTcpSocket::ReadWrite);
    }

    return;
}

void TcpModule::slotSocketConnect(void)
{
    /* 取消超时计时器 */
    disconnect(m_connTimer, &QTimer::timeout, this, &TcpModule::slotConnTimeout);

    m_connTimer->stop();
    if (m_connTimer != nullptr) {
        m_connTimer->deleteLater();
    }

    emit this->sigTaskAdd();

    return;
}

/* socket标准错误处理 */
void TcpModule::slotSocketError(QAbstractSocket::SocketError se)
{
    qDebug() << "Waring : TcpModule socketError uuid : " << this->m_uuid << " [ " << se << " ]";

    /* PRO : 此处 tcp 链接已经中断 , 正常逻辑应该发出失败信号 , 释放该模块 */
    emit sigMaintainTcpLinkDelete(this->m_uuid);

    return;
}

void TcpModule::tranText(ChatMsgInfo *send)
{
    if (send->m_friendUuid != m_uuid) {
        return;
    }

    this->m_socket->waitForConnected();

    /* 协议封装 */
    /* PRO 所有数据应从网络模块取 , 不应依赖外模块 */
    QMap<QByteArray, QByteArray> map;
    map.clear();

    map.insert(QByteArray("uuid"), IniSettings::getInstance()->getLocalUuid().toLocal8Bit());
    map.insert(QByteArray("type"), QString::number(send->m_msgType).toLocal8Bit());
    map.insert(QByteArray("body_size"), QByteArray::number(send->m_msgContent.size()));
    map.insert(QByteArray("body"), send->m_msgContent.toLocal8Bit());

    QByteArray encapsulationtData = m_moduleProtocolAnalysis.encapsulationHeadBody(map);
    if (encapsulationtData.isEmpty()) {
        qDebug() << "Waring : TcpModule , tranText , encapsulation msg fail";
        return;
    }

    /* 发送 */
    if (this->m_socket->error() == -1 && m_socket->isWritable()) {
        this->m_socket->write(encapsulationtData);
        this->m_socket->flush();
        this->m_socket->waitForBytesWritten();
        usleep(50000);

        /* 发送消息发送成功信号 */
        emit sigSendMsgSuccess(send);
    } else {
        /* PRO : tcp 异常 , 好友下线需确认 , 是否应删除整个 tcp 模块 */
        emit sigFriendOffline(send->m_friendId);
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        qDebug() << "Waring :TcpModule , tranText , fail , tcp link abnormal";

        return;
    }

    return;
}

void TcpModule::tranFile(ChatMsgInfo *send)
{
    if (send->m_friendUuid != m_uuid) {
        return;
    }

    this->m_socket->waitForConnected();

    int ret = -1;
    int sendPackageSum = 0;
    double dirCommpressRate = 0.00;

    /* 最终文件路径 */
    char dstFilePath[1024];
    memset(dstFilePath, 0x00, sizeof(dstFilePath));

    /* 获取原文件路径 */
    QString filePath = send->m_filePath;
    std::string stdFilePath = filePath.toStdString();
    char *cFilePath = const_cast<char *>(stdFilePath.c_str());

    /* 获取文件基础路径和基础文件名 */
    QFileInfo fileInfo(send->m_filePath);
    QString baseName = fileInfo.fileName();
    filePath = send->m_filePath;
    QString basePath = fileInfo.absolutePath();

    /* 修复安全漏洞KVE-2024-0202 */
    if (baseName.contains("../")){
        qDebug() << "There is security threat";
        return;
    }

    std::string stdBaseName = baseName.toStdString();
    char *cBaseName = const_cast<char *>(stdBaseName.c_str());

    std::string stdBasePath = basePath.toStdString();
    char *cBasePath = const_cast<char *>(stdBasePath.c_str());

    /* 判断发送文件还是文件夹 */
    struct stat st;
    memset(&st, 0x00, sizeof(struct stat));

    ret = stat(cFilePath, &st);
    if (ret == -1) {
        /* PRO : 只是获取文件属性失败 , 并不代表 tcp 异常 */
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        qDebug() << "Error : TcpModule , tranFile , get file stat fail" << errno << strerror(errno);

        return;
    }

    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        /* 目录 , 压缩传输*/
        qDebug() << "Info : TcpModule , tranFile , this is a dir";
        int ret = -1;

        /* 获取家目录 */
        char *homePath = getenv("HOME");
        if (homePath == NULL) {
            qDebug() << "Error : TcpModule , tranFile , get home path fail";
            return;
        }

        /* 获取缓存路径 */
        // char cachePath[1024];
        // memset(cachePath, 0x00, sizeof(cachePath));
        // sprintf(cachePath, "%s/.kylin-ipmsg", homePath);
        /* 代码安全规范进行替换（字符串拼接） */
        QString strHpmePath = QString::fromUtf8(homePath);
        QString strCachePath = QString("%1/.kylin-ipmsg").arg(strHpmePath);
        std::string str = strCachePath.toStdString();
        const char* cachePath = str.c_str();

        if (access(cachePath, F_OK)) {
            char *tmpCachePath = (char*)malloc((PATH_MAX) * sizeof(char));
            /* 添加安全代码规范 */
            realpath(cachePath, tmpCachePath);
            if (!tmpCachePath) {
                qDebug() << "Error : TcpModule , recvFile , realpath is null !!!!";
            }
            if (verify_file(tmpCachePath)) {
                qDebug() << "Error : TcpModule , recvFile , verify_file is 0 !!!!";
                assert(verify_file(tmpCachePath));
            }

            if (mkdir(tmpCachePath, 0777)) {
                free(tmpCachePath);
                qDebug() << "Error : TcpModule , tranFile , create cache path fail";
                return;
            } else {
                qDebug() << "Info : TcpModule , tranFile , create cache path success!!";
            }
            free(tmpCachePath);
        }

        /* 压缩 */
        // char command[1024 * 5];
        // memset(command, 0x00, sizeof(command));
        // sprintf(command, "tar -zcf \"%s/%s.tar.gz\" -C %s \"%s\"", cachePath, cBaseName, cBasePath, cBaseName);
        /* 代码安全规范进行替换（字符串拼接） */
        //QString qstrCommand = QString("tar -zcf \"%1/%2.tar.gz\" -C %3 \"%4\"").arg(strCachePath).arg(baseName).arg(basePath).arg(baseName);
        //std::string strCommand = qstrCommand.toStdString();
        //const char* command = strCommand.c_str();
        //
        //ret = system(command);
        ret = cmd_exec_tar_compress(strCachePath, baseName, basePath, baseName);
        qDebug() << __func__ << __LINE__ << "( TcpModule::recvFile )  cmd_exec_tar_compress return value:ret = " << ret;
        if (ret != 0 && ret !=256) {
            /* PRO : 只是压缩失败 , 并不代表 tcp 异常 */
            emit sigSendFileFail(send);
            emit sigMaintainTcpLinkDelete(this->m_uuid);

            qDebug() << "Error : TcpModule , tranFile , tar dir fail";
            return;
        }

        /* 修改发送结构路径 */
        // char modifyPath[1024 * 3];
        // memset(modifyPath, 0x00, sizeof(modifyPath));
        // sprintf(modifyPath, "%s/%s.tar.gz", cachePath, cBaseName);
        /* 代码安全规范进行替换（字符串拼接） */
        QString qstrModifyPath = QString("%1/%2.tar.gz").arg(strCachePath).arg(baseName);
        std::string strModifyPath = qstrModifyPath.toStdString();
        const char* modifyPath = strModifyPath.c_str();

        /* 获取压缩前文件夹的大小 , 获取后修改路径为压缩文件路径 */
        send->m_totalSize = GlobalUtils::getDirSize(send->m_filePath);
        send->m_filePath = QString(modifyPath);

        strcpy(dstFilePath, modifyPath);

        /* 计算压缩比例 */
        struct stat comSt;
        memset(&comSt, 0x00, sizeof(struct stat));

        ret = stat(modifyPath, &comSt);
        if (ret == -1) {
            /* PRO : 命令失败 , 并不等于 tcp 链接有误 */
            emit sigSendFileFail(send);
            emit sigMaintainTcpLinkDelete(this->m_uuid);

            qDebug() << "Error : TcpModule , tranFile , get file stat fail" << errno << strerror(errno);
            return;
        }

        dirCommpressRate = send->m_totalSize * 1.0 / comSt.st_size;

    } else {
        /* 文件 */
        strcpy(dstFilePath, cFilePath);
        send->m_totalSize = (long long int)st.st_size;
    }

    /* 开始发送 */
    QByteArray encapsulationtData;

    /* 发送文件消息包 */
    encapsulationtData.clear();

    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        encapsulationtData = this->encapsulationFileInfo(send, 1);
    } else {
        encapsulationtData = this->encapsulationFileInfo(send, 0);
    }

    if (encapsulationtData.isEmpty()) {
        /* PRO */
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        qDebug() << "Error : TcpModule , tranFile , encapsulation file info fail";
        return;
    }

    if (this->m_socket->error() == -1 && m_socket->isWritable()) {
        this->m_socket->write(encapsulationtData);
        this->m_socket->flush();
        this->m_socket->waitForBytesWritten();
        usleep(50000);
        sendPackageSum++;
    } else {
        /* PRO */
        emit sigFriendOffline(send->m_friendId);
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        return;
    }

    /* 发送文件数据 */
    int readSize = -1;
    char buff[READ_SIZE];
    memset(buff, 0x00, sizeof(buff));

    int fd = open(dstFilePath, O_RDONLY);
    if (fd == -1) {
        /* PRO */
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        qDebug() << "Error : TcpModule , tranFile , get file fd fail";
        return;
    }

    while ((readSize = read(fd, buff, sizeof(buff))) > 0) {
        /* 处理优先级高的任务队列 */
        while (this->m_msgTaskQueue.count() > 0) {
            ChatMsgInfo *tmp = this->m_msgTaskQueue.dequeue();

            if (!judgeCancel(tmp)) {
                this->tranText(tmp);
            }
        }

        /* 判断当前文件是否取消发送 */
        if (judgeCancel(send)) {
            close(fd);
            return;
        }

        /* 封装协议 */
        QMap<QByteArray, QByteArray> map;
        map.clear();

        map.insert(QByteArray("uuid"), IniSettings::getInstance()->getLocalUuid().toLocal8Bit());
        map.insert(QByteArray("type"), QByteArray::number(send->m_msgType));
        map.insert(QByteArray("body_size"), QByteArray::number(readSize));
        map.insert(QByteArray("file_stat"), QByteArray("data"));
        map.insert(QByteArray("body"), QByteArray(buff, readSize));

        encapsulationtData.clear();
        encapsulationtData = m_moduleProtocolAnalysis.encapsulationHeadBody(map);

        if (encapsulationtData.isEmpty()) {
            /* PRO */
            emit sigSendFileFail(send);
            emit sigMaintainTcpLinkDelete(this->m_uuid);

            qDebug() << "Error : TcpModule , tranFile , encapsulation file data fail";
            close(fd);
            return;
        }

        /* 发送 */
        if (this->m_socket->error() == -1 && m_socket->isWritable()) {
            this->m_socket->write(encapsulationtData);
            this->m_socket->flush();
            this->m_socket->waitForBytesWritten(5000);
        } else {
            /* PRO */
            emit sigFriendOffline(send->m_friendId);
            emit sigSendFileFail(send);
            emit sigMaintainTcpLinkDelete(this->m_uuid);

            qDebug() << "Error : TcpModule , tranFile , encapsulation file data fail";
            close(fd);
            return;
        }

        /* 发送传输进度信号 */
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            send->m_transferSize += (long long int)(readSize * dirCommpressRate);
        } else {
            send->m_transferSize += readSize;
        }
        emit sigSendFileProgress(send);

        memset(buff, 0x00, sizeof(buff));
        readSize = -1;
        sendPackageSum++;
        usleep(50000);
    }

    /* 发送文件结束 */
    encapsulationtData.clear();
    encapsulationtData = this->encapsulationFileEnd(send);
    if (encapsulationtData.isEmpty()) {
        /* PRO */
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);
        qDebug() << "Error : TcpModule , tranFile , encapsulation file end fail";

        close(fd);
        return;
    }

    if (this->m_socket->error() == -1 && m_socket->isWritable()) {
        this->m_socket->write(encapsulationtData);
        this->m_socket->flush();
        this->m_socket->waitForBytesWritten();
        sendPackageSum++;
        usleep(50000);
    } else {
        /* PRO */
        emit sigFriendOffline(send->m_friendId);
        emit sigSendFileFail(send);
        emit sigMaintainTcpLinkDelete(this->m_uuid);

        close(fd);
        return;
    }
    close(fd);

    /* 文件发送成功 */
    emit sigSendFileSuccess(send);

    /* 如果是目录 , 删除压缩文件 */
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        ret = unlink(dstFilePath);
        if (ret == -1) {
            qDebug() << "Waring : TcpModule , tranFile , delete tar file fail";
        }
    }

    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : TcpModule , tranFile , file [ " << QString(cFilePath) << " ] send success !!";
        qDebug() << "Info : TcpModule , tranFile , ------send data package sum is [ " << sendPackageSum << " ]";
    }

    return;
}

QByteArray TcpModule::encapsulationFileInfo(ChatMsgInfo *send, int flag)
{
    int ret = -1;
    QByteArray dst;
    dst.clear();

    /* 获取文件名 */
    QString fileName;
    fileName.clear();

    QStringList fileNameList = send->m_filePath.split('/');
    if (!fileNameList.isEmpty()) {
        fileName = fileNameList.last();
    } else {
        qDebug() << "Error : TcpModule , encapsulationFileInfo , analysys file name is empty";
        return dst;
    }

    /* 获取文件路径 */
    std::string stdFilePath = send->m_filePath.toStdString();
    char *cFilePath = const_cast<char *>(stdFilePath.c_str());

    struct stat st;
    memset(&st, 0x00, sizeof(struct stat));

    ret = stat(cFilePath, &st);
    if (ret == -1) {
        qDebug() << "Error : TcpModule , encapsulationFileInfo , get file stat fail";
        return dst;
    }

    /* 封装协议 */
    QMap<QByteArray, QByteArray> map;
    map.clear();

    map.insert(QByteArray("uuid"), IniSettings::getInstance()->getLocalUuid().toLocal8Bit());
    map.insert(QByteArray("type"), QByteArray::number(send->m_msgType));
    map.insert(QByteArray("body_size"), QByteArray("0"));

    map.insert(QByteArray("file_stat"), QByteArray("info"));
    if (flag == 1) {
        map.insert(QByteArray("file_type"), QByteArray("dir"));
    } else {
        map.insert(QByteArray("file_type"), QByteArray("file"));
    }
    map.insert(QByteArray("file_name"), fileName.toLocal8Bit());
    map.insert(QByteArray("file_total_size"), QByteArray::number((long long int)(st.st_size)));

    dst = m_moduleProtocolAnalysis.encapsulationHeadBody(map);

    return dst;
}

QByteArray TcpModule::encapsulationFileEnd(ChatMsgInfo *send)
{
    QByteArray dst;
    dst.clear();

    /* 封装协议 */
    QMap<QByteArray, QByteArray> map;
    map.clear();

    map.insert(QByteArray("uuid"), IniSettings::getInstance()->getLocalUuid().toLocal8Bit());
    map.insert(QByteArray("type"), QByteArray::number(send->m_msgType));
    map.insert(QByteArray("body_size"), QByteArray("0"));
    map.insert(QByteArray("file_stat"), QByteArray("end"));

    dst = m_moduleProtocolAnalysis.encapsulationHeadBody(map);

    return dst;
}

void TcpModule::slotReadSocket(void)
{
    qDebug() << "xxxxxxxxxxxr: into slotreadsocket";
    /* 接收数据包 , 一次接受一个包 */
    qint64 availableData;
    if (this->m_socket->error() == -1 && m_socket->isReadable()) {
        availableData = this->m_socket->bytesAvailable();
    } else {
        return;
    }

    if (availableData < PACKAGE_SIZE) {
        if (GlobalUtils::DEBUG_MODE) {
            qDebug() << "Waring : TcpModule , slotReadSocket , available data less than a package";
        }
        return;
    }

    QByteArray recvData;
    if (this->m_socket->error() == -1 && m_socket->isReadable()) {
        recvData = this->m_socket->read(PACKAGE_SIZE);
    } else {
        return;
    }

    /* 解析协议 */
    QMap<QByteArray, QByteArray> map = m_moduleProtocolAnalysis.analysisHeadBody(recvData);
    if (map.isEmpty()) {
        qDebug() << "Error : TcpModule , slotReadSocket , analysis package fail";
        return;
    }

    /* 判断tcp module 是否可用 , 为了维护tcp链表 , 因为在服务端获取不到数据包中uuid ,
     * 所以在modle第一次接收消息时进行维护 */
    if (this->m_flag == false) {
        this->m_uuid = map.value(QByteArray("uuid"));
        if (this->m_uuid.isEmpty()) {
            qDebug() << "Error : TcpModule , slotReadSocket , recv data uuid is empty , recv data discard";
            return;
        }

        /* 维护tcp链接表 */
        g_tcpItem item;
        item.uuid = this->m_uuid;
        item.selfIp = GlobalData::getInstance()->m_tcpListenIP;
        item.selfPort = GlobalData::getInstance()->m_tcpListenPort;
        item.peerIp = this->m_socket->peerAddress().toString();
        item.peerPort = QString::number(this->m_socket->peerPort());
        item.module = this;

        /* 同步调用槽 , 接收返回值 */
        emit sigMaintainTcpLinkAdd(item);

        this->m_flag = true;
    }

    /* 判断数据包是否对应 */
    if (map.value(QByteArray("uuid")) != this->m_uuid) {
        qDebug() << "Error : TcpModule , slotReadSocket , recv uuid differ save uuid , recv data discard";
        return;
    }

    /* 判断数据包种类 */
    if (map.value(QByteArray("type")) == QString::number(ChatMsgModel::MessageType::TextMsg)) {
        /* 接收信息 */
        this->recvMsg(map);

    } else {
        /* 接收文件 */
        this->recvFile(map);
    }

    // 处理栈内存可能溢出的问题
    recvData.clear();
    map.clear();

    return;
}

/* 处理消息数据包 */
void TcpModule::recvMsg(QMap<QByteArray, QByteArray> package)
{
    ChatMsgInfo *recv = new ChatMsgInfo();

    recv->m_friendUuid = this->m_uuid;
    recv->m_msgType = ChatMsgModel::MessageType::TextMsg;
    recv->m_msgContent = package.value(QByteArray("body"));

    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : TcpModule , recvMsg , recv msg  ---> " << recv->m_msgContent;
    }

    emit sigRecvMsgSuccess(recv);

    return;
}

/* 处理文件数据包 */
/* 当前只支持同一时间传输一个文件 , 在此基础上可以增加同时传输多个文件功能 */
void TcpModule::recvFile(QMap<QByteArray, QByteArray> package)
{
    int ret = -1;

    /* 传送文件的第一个包 */
    /* 后续可扩展同时传输多个文件 */
    if (package.value(QByteArray("file_stat")) == QByteArray("info")) {
        /* 开始接收文件 , 初始化成员变量 */
        memset(this->m_fileType, 0x00, sizeof(this->m_fileType));
        memset(this->m_fileName, 0x00, sizeof(this->m_fileName));
        memset(this->m_tmpFilePath, 0x00, sizeof(this->m_tmpFilePath));
        memset(this->m_savePath, 0x00, sizeof(this->m_savePath));
        this->m_fileTotalSize = -1;
        this->m_fileRecvSize = -1;
        this->m_recv_flag = false;
        this->m_fd = -1;
        this->m_i = 0;

        /* 记录文件信息 */
        char *cFileName = const_cast<char *>(package.value(QByteArray("file_name")).data());
        strncpy(this->m_fileName, cFileName, sizeof(this->m_fileName) - 1);

        char *cFileType = const_cast<char *>(package.value(QByteArray("file_type")).data());
        strncpy(this->m_fileType, cFileType, sizeof(this->m_fileType) - 1);

        /* 修复安全漏洞KVE-2024-0202 */
        QString filename = this->m_fileName;
        if (filename.contains("../")){
            qDebug() << "There is security threat";
            return;
        }

        this->m_fileTotalSize = package.value(QByteArray("file_total_size")).toLongLong();

        this->m_msgType = package.value(QByteArray("type")).toInt();

        QString savePath = IniSettings::getInstance()->getFilePath();
        // 如果路径不存在，创建目录而不是回退到默认路径
        if (!QFileInfo::exists(savePath)) {
            QDir dir;
            if (!dir.mkpath(savePath)) {
                // 如果创建失败，才回退到默认路径
                qDebug() << "Warning: TcpModule, recvFile, failed to create save path, fallback to default path";
                savePath = GlobalUtils::getDefaultPath();
            } else {
                //qDebug() << "Info: TcpModule, recvFile, created save path:" << savePath;
            }
        }

        if (savePath.back() != "/") {
            savePath += "/";
        }
        std::string stdSavePath = savePath.toStdString();
        char *cSavePath = const_cast<char *>(stdSavePath.c_str());

        strcpy(this->m_savePath, cSavePath);

        /* 创建缓存文件 , 生成一个uuid用来命名文件名 */
        char buff[1024];
        memset(buff, 0x00, sizeof(buff));

        FILE *file = NULL;
        file = popen("uuidgen", "r");
        if (file == NULL) {
            qDebug() << "Error : TcpModule , recvFile , get cache file uuid fail";
            return;
        }

        if (fgets(buff, 1024, file) == NULL) {
            qDebug() << "Error : TcpModule , recvFile , read uuid pipe fail";
            return;
        }

        /* 去除\n字符 */
        char *tmp = strchr(buff, '\n');
        if (tmp != NULL) {
            *tmp = '\0';
        }

        pclose(file);

        /* 生成临时文件 , 获取文件描述符 */
        sprintf(this->m_tmpFilePath, "%s%s", this->m_savePath, buff);
        char *tmpFilePath = (char*)malloc((PATH_MAX) * sizeof(char));

        /* 添加安全代码规范 */
        realpath(this->m_tmpFilePath, tmpFilePath);
        if (!tmpFilePath) {
            qDebug() << "Error : TcpModule , recvFile , realpath is null";
        }
        if (verify_file(tmpFilePath)) {
            qDebug() << "Error : TcpModule , recvFile , verify_file is 0";
            assert(verify_file(tmpFilePath));
        }

        this->m_fd = open(tmpFilePath, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        free(tmpFilePath);
        if (this->m_fd == -1) {
            qDebug() << "Error : TcpModule , recvFile , get tmp file fd fail";
            return;
        }

        this->m_recv_flag = true;

        qDebug() << "Info : TcpModule , recvFile , -------file first package recv success , init success !!-------";

        return;
    }
    qDebug() << "xxxxxxxxxxxxr: finish first package trans";

    /* 如果初始化失败 , 后续的数据包直接丢弃 , 不在处理 */
    if (this->m_recv_flag == false) {
        deleteTmpFile(this->m_tmpFilePath);
        return;
    }

    /* 文件数据包 */
    if (package.value(QByteArray("file_stat")) == QByteArray("data")) {
        if (this->m_fd == -1) {
            this->m_recv_flag = false;
            deleteTmpFile(this->m_tmpFilePath);

            qDebug() << "Error : TcpModule , recvFile , recv file fd is error";
            return;
        }

        /* 将数据写入文件 */
        QByteArray data = package.value(QByteArray("body"));

        ret = write(this->m_fd, data.data(), data.size());
        if (ret == -1) {
            this->m_recv_flag = false;
            deleteTmpFile(this->m_tmpFilePath);
            qDebug() << "Error : TcpModule , recvFile , write file fail";
            return;
        }

        /* 记录接收文件大小 , 用于显示进度 , 服务端暂时不处理 */
        this->m_fileRecvSize += package.value(QByteArray("body_size")).toInt();

        if (GlobalUtils::DEBUG_MODE) {
            qDebug() << "Info : TcpModule , recvFile , -------file data package recv success !!-------";
        }

        return;
    }
    qDebug() << "xxxxxxxxxxxxxxr: finish data package trans";

    /* 文件传输结束数据包 */
    if (package.value(QByteArray("file_stat")) == QByteArray("end")) {
        this->m_recv_flag = false;

        /* 关闭文件描述符 */
        close(this->m_fd);
        this->m_fd = -1;

        /* 修改临时文件名 */
        /* 判断是否有重名现象 */
        // char dstFileName[1024 * 2];
        // memset(dstFileName, 0x00, sizeof(dstFileName));
        // sprintf(dstFileName, "%s%s", this->m_savePath, this->m_fileName);

        /********************** 代码安全规范进行替换（字符串拼接） **********************/
        QString qstrSavePath = QString::fromUtf8(this->m_savePath);
        QString qstrFileName = QString::fromUtf8(this->m_fileName);
        QString qstrDstFileName = QString("%1%2").arg(qstrSavePath).arg(qstrFileName);
        std::string strDstFileName = qstrDstFileName.toStdString();
        const char* chardstFileName = strDstFileName.c_str();
        /********************** 代码安全规范进行替换（字符串拼接） **********************/


        QFileInfo file(QString::fromLocal8Bit(chardstFileName));
        QString name;
        for (;;) {
            ret = access(chardstFileName, F_OK);
            if (!ret) {

                if (file.completeSuffix().isEmpty()) {
                    name = file.baseName() + " (" + QString::number(++m_i) + ")";
                } else {
                    name = file.baseName() + " (" + QString::number(++m_i) + ")." + file.completeSuffix();
                }

                // std::string stdBaseName = name.toStdString();
                // char *cBaseName = const_cast<char *>(stdBaseName.c_str());
                // sprintf(dstFileName, "%s%s", this->m_savePath, cBaseName);

                qstrDstFileName = QString("%1%2").arg(qstrSavePath).arg(name);
                strDstFileName = qstrDstFileName.toStdString();
                chardstFileName = strDstFileName.c_str();

            } else {
                break;
            }
        }
        /********************** 添加安全代码规范 **********************/
        char * dstFileName= (char*)malloc((PATH_MAX) * sizeof(char));
        realpath(chardstFileName, dstFileName);
        if (!dstFileName) {
            qDebug() << "Error : TcpModule , recvFile , realpath is null !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        }
        if (verify_file(dstFileName)) {
            qDebug() << "Error : TcpModule , recvFile , verify_file is 0 !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            assert(verify_file(dstFileName));
        }
        /********************** 添加安全代码规范 **********************/

        ret = rename(this->m_tmpFilePath, dstFileName);
        if (ret == -1) {
            qDebug() << "Error : TcpModule , recvFile , rename file fail" << errno << strerror(errno);
        }

        /* 如果为目录需要解压 , 并删除压缩包 */
        if (!strcmp(this->m_fileType, "dir")) {
            /* 判断解压后的文件名是否存在重名问题 , 若重名 , 则重新生成一个 */
            char decomDirName[1024 * 2];
            char originFileName[1024];
            memset(decomDirName, 0x00, sizeof(decomDirName));
            memset(originFileName, 0x00, sizeof(originFileName));

            strncpy(originFileName, this->m_fileName, sizeof(originFileName));
            char *var = strstr(originFileName, ".tar.gz");
            *var = '\0';

            sprintf(decomDirName, "%s%s", this->m_savePath, originFileName);

            QFileInfo file(decomDirName);
            QString name;
            int i = 0;
            ret = QFileInfo(decomDirName).exists();
            if (ret) {
                /* 生成文件名 */
                for (;;) {
                    if (file.completeSuffix().isEmpty()) {
                        name = file.baseName() + " (" + QString::number(++i) + ")";
                    } else {
                        name = file.baseName() + " (" + QString::number(++i) + ")." + file.completeSuffix();
                    }

                    std::string stddecomDirName = name.toStdString();
                    char *cDecomDirName = const_cast<char *>(stddecomDirName.c_str());

                    sprintf(decomDirName, "%s%s", this->m_savePath, cDecomDirName);

                    if (access(decomDirName, F_OK)) {
                        break;
                    }
                }

                /* 创建临时解压目录 */
                // char tmpDecomDir[1024];
                // memset(tmpDecomDir, 0x00, sizeof(tmpDecomDir));
                // sprintf(tmpDecomDir, "%s%s", this->m_savePath, ".kylin-ipmsg/");
                /********************** 代码安全规范进行替换（字符串拼接） **********************/
                // QString qstrSavePath = QString(QLatin1String(this->m_savePath));
                QString qstrDecomDir = QString("%1.kylin-ipmsg/").arg(qstrSavePath);
                std::string strTmpDecomDir = qstrDecomDir.toStdString();
                const char* tmpCharDecomDir = strTmpDecomDir.c_str();
                /********************** 代码安全规范进行替换（字符串拼接） **********************/

                /********************** 添加安全代码规范 **********************/
                char * tmpDecomDir= (char*)malloc((PATH_MAX) * sizeof(char));
                realpath(tmpCharDecomDir, tmpDecomDir);
                if (!tmpDecomDir) {
                    qDebug() << "Error : TcpModule , recvFile , realpath is null !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                }
                if (verify_file(tmpDecomDir)) {
                    qDebug() << "Error : TcpModule , recvFile , verify_file is 0 !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    assert(verify_file(tmpDecomDir));
                }
                /********************** 添加安全代码规范 **********************/

                ret = QFileInfo(tmpDecomDir).exists();
                if (!ret) {
                    ret = mkdir(tmpDecomDir, 0777);
                    if (ret) {
                        qDebug() << "Error : create tmp decom path fail";
                        free(tmpDecomDir);
                        free(dstFileName);
                        return;
                    }
                }

                /* 解压到临时目录 */
                // char command[1024 * 4];
                // memset(command, 0x00, sizeof(command));
                // sprintf(command, "tar -zxf \"%s\" -C \"%s\"", dstFileName, tmpDecomDir);
                //ret = system(QString("tar -zxf \"%1\" -C \"%2\"").arg(QString(dstFileName)).arg(QString(tmpDecomDir)).toStdString().c_str());
                ret  = cmd_exec_tar_decompress(dstFileName, QString(tmpDecomDir));
                qDebug() << __func__ << __LINE__ << "( TcpModule::recvFile ) cmd_exec_tar_decompress return value:ret =  " << ret;
                if (ret == -1) {
                    qDebug() << "Error : TcpModule , recvFile , tar command if fail";
                }

                /* 重命名 */
                char oldPath[1024 * 2];
                memset(oldPath, 0x00, sizeof(oldPath));
                sprintf(oldPath, "%s/%s", tmpDecomDir, originFileName); // TODO:将“/”进行char*判断是否包含

                ret = rename(oldPath, decomDirName);
                if (ret) {
                    qDebug() << "Error : TcpModule , recvFile , rename file fail";
                }

                /* 删除临时目录和压缩包 */
                ret = remove(tmpDecomDir);
                if (ret) {
                    qDebug() << "Error : TcpModule , recvFile , delete tmp decom dir fail";
                }

                ret = remove(dstFileName);
                if (ret) {
                    qDebug() << "Error : TcpModule , recvFile , delete decom file fail";
                }

                free(tmpDecomDir);
            } else {
                // char command[1024 * 3];
                // memset(command, 0x00, sizeof(command));
                // sprintf(command, "tar -zxf \"%s\" -C \"%s\"", dstFileName, this->m_savePath);
                /********************** 添加安全代码规范 **********************/
                char *savePath = (char*)malloc((PATH_MAX) * sizeof(char));
                realpath(this->m_savePath, savePath);
                if (!savePath) {
                    qDebug() << "Error : TcpModule , recvFile , realpath is null !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                }
                if (verify_file(savePath)) {
                    qDebug() << "Error : TcpModule , recvFile , verify_file is 0 !!!!" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
                    assert(verify_file(savePath));
                }

                //ret = system(QString("tar -zxf \"%1\" -C \"%2\"").arg(QString(dstFileName)).arg(QString(savePath)).toStdString().c_str());
                ret = cmd_exec_tar_decompress(dstFileName, QString(savePath));
                qDebug() << __func__ << __LINE__ << "( TcpModule::recvFile ) cmd_exec_tar_decompress return value:ret =  " << ret;
                if (ret == -1) {
                     qDebug() << "Waring : TcpModule , recvFile , tar command is fail";
                 }
                /********************** 添加安全代码规范 **********************/

                ret = remove(dstFileName);
                if (ret == -1) {
                    qDebug() << "Waring : TcpModule , recvFile , delete file fail";
                }
                free(savePath);
            }

            this->m_dstFileName = QString(decomDirName);
        } else {
            this->m_dstFileName = QString(dstFileName);
        }

        if (GlobalUtils::DEBUG_MODE) {
            qDebug() << "Info : TcpModule , recvFile , file rename success !!";
        }

        qDebug() << "Info : TcpModule , recvFile , -------file third package recv success-------";

        /* 文件接收成功 , 发送信号 */
        ChatMsgInfo *recv = new ChatMsgInfo();
        recv->m_friendUuid = this->m_uuid;
        recv->m_msgType = this->m_msgType;
        recv->m_filePath = this->m_dstFileName;
        recv->m_totalSize = this->m_fileTotalSize;
        recv->m_transferSize = this->m_fileTotalSize;

        free(dstFileName);

        emit sigRecvFileSuccess(recv);

        return;
    }

    qDebug() << "xxxxxxxxxxxxxxxxxr: finish third package trans";
    return;
}

void TcpModule::deleteTmpFile(char *filePath)
{
    int ret = -1;

    if (filePath == NULL || !strlen(filePath)) {
        printf("Waring : TcpModule , deleteTmpFile , path is error\n");
        return;
    }

    ret = access(filePath, F_OK);
    if (ret == -1) {
        return;
    }

    ret = unlink(filePath);
    if (ret == -1) {
        printf("Error : TcpModule , deleteTmpFile , delete file fail\n");
        return;
    }

    return;
}

void TcpModule::slotSocketDisconnect(void)
{
    /* 释放socket */
    if (this->m_socket != NULL) {
        delete this->m_socket;
        this->m_socket = NULL;
    }

    /* 维护tcp链接表 */
    emit sigMaintainTcpLinkDelete(this->m_uuid);

    return;
}

/* 断开连接时的结束操作 */
void TcpModule::finishModuleThread()
{
    this->deleteLater();
}

int TcpModule::verify_file(char *filename)
{
	/* Get /etc/passwd entry for current user */
	struct passwd *pwd = getpwuid(getuid());
	if (pwd == NULL) {
		/* Handle error */
		return 0;
	}
	const size_t len = strlen( pwd->pw_dir);
	if (strncmp( filename, pwd->pw_dir, len) != 0) {
		return 0;
	}
	/* Make sure there is only one '/', immediately after homedir */
	if (strrchr( filename, '/') == filename + len) {
		return 1;
	}
	return 0;
}
