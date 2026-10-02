/*
* Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "group_manager_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include <shadow.h>
#include <QDateTime>
#include <QString>

#include <QtConcurrent/QtConcurrent>
#include <QDBusMessage>
#include <QDBusInterface>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include "groupmanager_adaptor.h"

group_manager_server::group_manager_server()
    : UKUI::ServiceObject(nullptr), QDBusContext()
{
    qRegisterMetaType<custom_struct>("custom_struct");
    qDBusRegisterMetaType<custom_struct>();
    new InterfaceAdaptor(this);
}

group_manager_server::~group_manager_server()
{

}

//解析组文件
QVariantList group_manager_server::getGroup()
{
    const QString fileName = "/etc/group";
    QFile groupFile(fileName);

    QVariantList value;
    QVariant cnt;

    if(!groupFile.exists()){
        printf("/etc/group file not exist \n");
    }
    if(!groupFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        printf("open /etc/group fail \n");
    }

    QTextStream in(&groupFile);
    QString line = in.readLine();
    QVector<custom_struct> demo;

    while(!line.isNull()){
        QStringList lineList = line.split(":");
        line = in.readLine();
        custom_struct entry;
        entry.groupname   = lineList.at(0);
        entry.passphrase  = lineList.at(1);
        entry.groupid     = lineList.at(2);
        entry.usergroup   = lineList.at(3);
        demo.append(entry);
        cnt = QVariant::fromValue(demo.last());
        value << cnt;
    }
    return value;
}

// 解析passwd文件
QVariantList group_manager_server::getPasswd()
{
    const QString fileName = "/etc/passwd";
    QFile passwdFile(fileName);

    QVariantList value;
    QVariant cnt;

    if(!passwdFile.exists()){
        printf("/etc/passwd file not exist \n");
    }
    if(!passwdFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        printf("open /etc/passwd fail \n");
    }

    QTextStream in(&passwdFile);
    QString line = in.readLine();
    // 使用动态容器替代固定大小栈数组，消除缓冲区溢出风险
    QVector<custom_struct> demo;

    while(!line.isNull()){
        QStringList lineList = line.split(":");
        line = in.readLine();
        custom_struct item;
        item.groupname   = lineList.at(0);
        item.passphrase  = lineList.at(1);
        item.groupid     = lineList.at(3);
        item.usergroup   = "";
        demo.append(item);
        cnt = QVariant::fromValue(demo.last());
        value << cnt;
    }
    return value;
}

// 添加组
bool group_manager_server::add(QString groupName, QString groupId)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QString groupadd = "/usr/sbin/groupadd";
    QString addgroup = "/usr/sbin/addgroup";
    QString command;
    QFile groupaddFile("/usr/sbin/addgroup");
    QFile addgroupFile("/usr/sbin/groupadd");

    QProcess p(0);
    QStringList args;

    if(!addgroupFile.exists()){
        printf("/usr/sbin/addgroup file not exist \n");
        if(!groupaddFile.exists()){
            return false;
        }
        command = groupadd;
        args.append("-g");
        args.append(groupId);
        args.append(groupName);
    }else{
        command = addgroup;
        args.append("-gid");
        args.append(groupId);
        args.append(groupName);
    }


    p.execute(command,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);
//    qDebug()<<QString::fromLocal8Bit(p.readAllStandardError());
    return true;
}

// 修改组
bool group_manager_server::set(QString groupName, QString groupId)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QString groupmod = "/usr/sbin/groupmod";
    QFile groupmodFile(groupmod);
    QProcess p(0);
    QStringList args;

    if(!groupmodFile.exists()){
        printf("/usr/sbin/groupmod file not exist \n");
        return false;
    }
    args.append("-g");
    args.append(groupId);
    //args.append("-n");
    args.append(groupName);

    p.execute(groupmod,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);
//    qDebug()<<QString::fromLocal8Bit(p.readAllStandardError());
    return true;
}

// 删除组
bool group_manager_server::del(QString groupName)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QString groupdel = "/usr/sbin/groupdel";
    QFile groupdelFile(groupdel);
    QProcess p(0);
    QStringList args;

    if(!groupdelFile.exists()){
        printf("/usr/sbin/groupdel file not exist \n");
        return false;
    }
    args.append(groupName);

    p.execute(groupdel,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);
//    qDebug()<<QString::fromLocal8Bit(p.readAllStandardError());
    return true;
}

// 添加用户到组
bool group_manager_server::_addUserToGroup(QString groupName, QString userName)
{
    QString usermod = "/usr/sbin/usermod";
    QString gpasswd = "/usr/bin/gpasswd";
    QString command;

    QFile usermodFile(usermod);
    QFile gpasswdFile(gpasswd);

    QProcess p(0);
    QStringList args;

    if(!usermodFile.exists()){
        printf("/usr/sbin/usermod file not exist \n");
        if(!gpasswdFile.exists()){
            printf("/usr/sbin/gpasswd file not exist \n");
            return false;
        }
        command = gpasswd;
        args.append("-a");
        args.append(userName);
        args.append(groupName);
    } else {
        command = usermod;
        args.append("-a");
        args.append("-G");
        args.append(groupName);
        args.append(userName);
    }
    p.execute(command,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);
//    qDebug()<<QString::fromLocal8Bit(p.readAllStandardError());
    return true;
}

bool group_manager_server::addUserToGroup(QString groupName, QStringList userNameList)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    foreach (QString userName,userNameList) {
        bool ret = _addUserToGroup(groupName, userName);
        if (!ret) {
            qDebug() << "adduser " << userName << " to " << groupName << " failed!";
        }
    }
    return true;
}

bool group_manager_server::authoriyEdit()
{
    QDBusConnection conn = connection();
    QDBusMessage msg = QDBusContext::message();

    _id = conn.interface()->servicePid(msg.service()).value();

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.ukui.groupmanager.action.edit",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool group_manager_server::_delUserFromGroup(QString groupName, QString userName)
{
    QString gpasswd = "/usr/bin/gpasswd";
    QString command;

    QFile gpasswdFile(gpasswd);

    QProcess p(0);
    QStringList args;

    if(!gpasswdFile.exists()){
        printf("/usr/sbin/gpasswd file not exist \n");
        return false;
    }
    command = gpasswd;
    args.append("-d");
    args.append(userName);
    args.append(groupName);

    p.execute(command,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);
//    qDebug() << QString::fromLocal8Bit(p.readAllStandardError());
    return true;
}

// 删除用户从组
bool group_manager_server::delUserFromGroup(QString groupName, QStringList userNameList)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    foreach (QString userName,userNameList) {
        bool ret = _delUserFromGroup(groupName, userName);
        if (!ret) {
            qDebug() << "deluser " << userName << " to " << groupName << " failed!";
        }
    }
    return true;
}

// 修改其他用户密码
bool group_manager_server::changeOtherUserPasswd(QString username, QString pwd)
{
    return true;
}

// 创建新用户
bool group_manager_server::createUser(QString name, QString fullname, int accounttype, QString faceicon)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> reply = iface.call("CreateUser", name, fullname, accounttype);

    if (reply.isValid()){
        QString op = reply.value().path();
        if (!op.isEmpty()){

            QDBusInterface ifaceUser("org.freedesktop.Accounts",
                                     op,
                                     "org.freedesktop.Accounts.User",
                                     QDBusConnection::systemBus());
            // 设置头像
            ifaceUser.call("SetIconFile", faceicon);
        }
    }

    return true;

}

//获取免密登录状态
QString group_manager_server::getNoPwdLoginStatus()
{
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/group |grep nopasswdlogin");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    }else{
        qDebug()<<"popen文件打开失败"<<endl;
    }
    return QString(ba);
}

//设置免密登录状态
bool group_manager_server::setNoPwdLoginStatus(bool status,QString username)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    if (username == nullptr) {
        if (status == false) {
            QString noPwdLoginUser = getNoPwdLoginStatus();
            qDebug() << "noPwdLoginUser:" << noPwdLoginUser;
            QStringList tmp = noPwdLoginUser.split(":", QString::SkipEmptyParts);

            QString noPasswdUsers = tmp.at(tmp.count()-1);
            QStringList noPasswdUsersList = noPasswdUsers.split(",", QString::SkipEmptyParts);
            foreach (QString noPasswdUser, noPasswdUsersList) {
                noPasswdUser.remove(QChar('\n'), Qt::CaseInsensitive);
                qDebug() << "nopasswduser:" << noPasswdUser;
                QString cmd = QString("gpasswd  -d %1 nopasswdlogin").arg(noPasswdUser);;
                QProcess::execute(cmd);
            }
        }
    } else {
        QString cmd;
        if(true == status){
             cmd = QString("gpasswd  -a %1 nopasswdlogin").arg(username);
        } else{
            cmd = QString("gpasswd  -d %1 nopasswdlogin").arg(username);
        }
        QProcess::execute(cmd);
    }

    NoPwdLogin = status;
    notifyPropertyChanged("org.ukui.groupmanager", "NoPwdLoginStatus");
    return true;
}

// 修改自动登录
bool group_manager_server::setAutomaticLogin(QString objpath, bool enabled)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                          objpath,
                                                          "org.freedesktop.Accounts.User",
                                                          "SetAutomaticLogin");
    message << enabled;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage){
        return false;
    }
    return true;
}

// 设置用户头像
bool group_manager_server::setIconFile(QString username, QString objpath, QString filename)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                          objpath,
                                                          "org.freedesktop.Accounts.User",
                                                          "SetIconFile");
    message << filename;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage){
        return false;
    }
    return true;
}

// 设置用户头像并重命名为 refilename
bool group_manager_server::setIconFileRename(QString filename, QString refilename, QString objpath)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                          objpath,
                                                          "org.freedesktop.Accounts.User",
                                                          "SetIconFileRename");
    message << filename << refilename;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage){
        return false;
    }
    return true;
}

// 删除自定义历史头像
bool group_manager_server::delIconFile(QStringList filenameList, QString objpath)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    foreach (QString filename, filenameList) {
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                              objpath,
                                                              "org.freedesktop.Accounts.User",
                                                              "DelIconFile");
        message << filename;
        QDBusMessage response = QDBusConnection::systemBus().call(message);
        if (response.type() == QDBusMessage::ErrorMessage){
            return false;
        }
    }

    return true;
}

// 修改账户类型
bool group_manager_server::setAccountType(QString objpath, int accountType)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                          objpath,
                                                          "org.freedesktop.Accounts.User",
                                                          "SetAccountType");
    message << accountType;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage){
        return false;
    }
    return true;
}

// 删除用户
bool group_manager_server::deleteUser(qint64 id, bool removeFiles)
{
    //密码校验
    if (!authoriyEdit()){
        return false;
    }

    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());

    QDBusMessage response = iface.call("DeleteUser", id, removeFiles);
    if (response.type() == QDBusMessage::ErrorMessage){
        qDebug() << "====" << response.errorMessage();
        return false;
    }
    return true;
}


void group_manager_server::notifyPropertyChanged( const QString& interface,
                            const QString& propertyName)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        "/",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, property(propertyName.toLatin1().data()));
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::systemBus().send(signal);
}

bool group_manager_server::NoPwdLoginStatus() const
{
    return NoPwdLogin;
}

/**
 * @brief 判断密码是否已到最短使用期限
 * @param username 用户名
 * @return true 已到最短使用期限（允许修改密码）
 *         false 未到最短使用期限，或读取失败
 * @note 需要 root 权限
 */
bool group_manager_server::isPasswordMinAgeReached(const QString &username)
{
    QByteArray ba = username.toUtf8();
    struct spwd *sp = getspnam(ba.constData());
    if (!sp) {
        qDebug() << "sp is null " << ba.constData();
        return false;
    }

    // 无限制或从未修改，允许修改
    if (sp->sp_min <= 0 || sp->sp_lstchg <= 0) {
        qDebug() << "sp->sp_min " << sp->sp_min  << "sp->sp_lstchg " << sp->sp_lstchg;
        return true;
    }

    // Qt 接口获取当前 UTC 日期距 1970-01-01 的天数（与 sp_lstchg 定义一致）
    qint64 today = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() / 86400;
    qint64 daysPassed = today - sp->sp_lstchg;

    qDebug() << "daysPassed = " << daysPassed  << "today = " << today << "sp->sp_lstchg = " << sp->sp_lstchg << "sp->sp_min = " << sp->sp_min;

    return daysPassed >= sp->sp_min;
}
