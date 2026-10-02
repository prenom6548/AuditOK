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

#include "globalutils.h"
#include "inisetting.h"

#include <QDir>

// debug日志打印标识
bool GlobalUtils::DEBUG_MODE = false;

GlobalUtils::GlobalUtils() {}

GlobalUtils::~GlobalUtils() {}

/**************************************************************************
 * 函数名称： getAvailTcpPort
 * 函数功能： 获取可用的TCP端口号 39901~39999
 * 输入参数： 无
 * 返回数值： int
 * 创建人员：
 * 创建时间： 2021-05-19
 * 修改人员：
 * 修改时间：
 **************************************************************************/
int GlobalUtils::getAvailTcpPort()
{
    static int resPort = 0;

    if (resPort != 0) {
        return resPort;
    }

    int tmpPort = DEFAULT_TCP_PORT - 1;

    // 判断端口是否被占用

    QString proRes = "";

    // 从9696端口开始判断
    while (!proRes.contains("0")) {
        QProcess process(0);
        tmpPort++;
        process.start("sh", QStringList()
                                << "-c"
                                << QString("/usr/bin/netstat -pan|grep -w %1 |wc -l").arg(QString::number(tmpPort)));

        process.waitForFinished();
        proRes = process.readAllStandardOutput();

        // qDebug() << "proRes" << proRes;

        // process.terminate();
        // process.kill();
        // process.deleteLater();
    }

    resPort = tmpPort;

    return resPort;
}

// 获取系统用户名
QString GlobalUtils::getUsername()
{
    static QString username = "";

    if (username != "") {
        return username;
    }

    // 环境变量获取用户名
    QString uname(getenv("USERNAME"));
    if (uname == "") {
        uname = getenv("USER");
    }
    if (uname == "") {
        uname = "Unknown";
    }
    uname = uname.left(1).toUpper() + uname.mid(1);
    username = uname;

    return uname;
}

// 获取昵称
QString GlobalUtils::getNickname()
{
    QString nickname = IniSettings::getInstance()->getLocalNickname();

    if (nickname.isEmpty()) {
        nickname = getUsername();
    }

    return nickname;
}

// 获取系统平台名
QString GlobalUtils::getPlatformName()
{
#if defined(Q_WS_WIN)
    return "Windows";
#elif defined(Q_WS_MAC)
    return "Macintosh";
#elif defined(Q_OS_UNIX)
    return "Linux";
#elif defined(Q_WS_S60)
    return "Symbian";
#else
    return "Unknown";
#endif
}

// 生成用户UUID
QString GlobalUtils::getUserUuid()
{
    QString resUuid = QUuid::createUuid().toString().remove("{").remove("}");
    return resUuid;
}

// 默认文件保存目录
QString GlobalUtils::getDefaultPath(bool isSendPath)
{
    QString defaultPath = QString(getenv("HOME")) + "/ipmsgReceivedFiles/";
    QString sendFilePath = QString(getenv("HOME")) + "/ipmsgReceivedFiles/sendFiles/";

    QDir *folder = new QDir;

    if (!(folder->exists(defaultPath))) {
        folder->mkdir(defaultPath);
        folder->mkdir(sendFilePath);

    } else if (!(folder->exists(sendFilePath))) {
        folder->mkdir(sendFilePath);
    }

    delete folder;

    QString resPath = isSendPath ? sendFilePath : defaultPath;
    return resPath;
}

// 时间戳格式
QString GlobalUtils::getTimeFormat(bool isMsec)
{
    QString timeFormat;

    if (isMsec) {
        timeFormat = "yyyy-MM-dd HH:mm:ss.zzz";
    } else {
        timeFormat = "yyyy-MM-dd HH:mm:ss";
    }

    return timeFormat;
}

// 根据用户名或备注获取头像
QString GlobalUtils::getAvatarByName(QString name)
{
    QString avatar = QString("");

    if (name != "") {
        if (QString(name.at(0)) >= 'a' && QString(name.at(0)) <= 'z') {
            avatar = QString(name.at(0)).toUpper();
        } else {
            avatar = QString(name.at(0));
        }
    }

    return avatar;
}

// 将数据进行加密
QString GlobalUtils::encryptData(QVariant data)
{
    QString dataStr = "";
    QString resStr = "";

    if (data.canConvert(QVariant::String)) {
        dataStr = data.toString();
    }

    QByteArray text = dataStr.toLocal8Bit();
    QByteArray by = text.toBase64();

    resStr = QString(by);

    return resStr;
}

// 将数据进行解密
QString GlobalUtils::uncryptData(QVariant data)
{
    QString dataStr = "";
    QString resStr = "";

    if (data.canConvert(QVariant::String)) {
        dataStr = data.toString();
    }

    QByteArray text = dataStr.toLocal8Bit();
    QByteArray by = text.fromBase64(text);
    resStr = QString::fromLocal8Bit(by);

    return resStr;
}

// 获取文件夹大小
quint64 GlobalUtils::getDirSize(const QString filePath)
{
    QDir tmpDir(filePath);
    quint64 size = 0;

    /*获取文件列表  统计文件大小*/
    foreach (QFileInfo fileInfo, tmpDir.entryInfoList(QDir::Files)) {
        size += fileInfo.size();
    }

    /*获取文件夹  并且过滤掉.和..文件夹 统计各个文件夹的文件大小 */
    foreach (QString subDir, tmpDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        size += getDirSize(filePath + QDir::separator() + subDir); //递归进行  统计所有子目录
    }

    return size;
}
