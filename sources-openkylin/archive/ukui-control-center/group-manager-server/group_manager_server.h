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

#include <ukui-service-interface/serviceobject.h>
#include <QList>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDBusContext>
#include <QDBusConnectionInterface>

#include "custom_struct.h"

class group_manager_server : public UKUI::ServiceObject,QDBusContext
{
	Q_OBJECT
    //定义Interface名称
    Q_CLASSINFO("D-Bus Interface", "org.ukui.groupmanager.interface")
    Q_PROPERTY(bool NoPwdLoginStatus READ NoPwdLoginStatus)
public:
    explicit group_manager_server();
    ~group_manager_server();

    bool authoriyEdit();
    void notifyPropertyChanged(const QString& interface, const QString &propertyName);
    bool NoPwdLoginStatus() const;

private:
    bool _addUserToGroup(QString groupName, QString userName);
    bool _delUserFromGroup(QString groupName, QString userName);

public slots:
    QVariantList getGroup();
    QVariantList getPasswd();
    bool add(QString groupName, QString groupId);
    bool set(QString groupName, QString groupId);
    bool del(QString groupName);
    bool addUserToGroup(QString groupName, QStringList userNameList);
    bool delUserFromGroup(QString groupName, QStringList userNameList);
    bool changeOtherUserPasswd(QString username, QString pwd);
    bool createUser(QString name, QString fullname, int accounttype, QString faceicon);
    bool setNoPwdLoginStatus(bool status,QString username = nullptr);
    QString getNoPwdLoginStatus();
    bool setAutomaticLogin(QString objpath, bool enabled);
    bool setIconFile(QString username, QString objpath, QString filename);
    bool setIconFileRename(QString filename, QString refilename, QString objpath);
    bool delIconFile(QStringList filenameList, QString objpath);
    bool setAccountType(QString objpath, int accountType);
    bool deleteUser(qint64 id, bool removeFiles);
    bool isPasswordMinAgeReached(const QString &username);

private:
    QList<custom_struct> value;
    qint64 _id = 0;
    bool authoriy = true;
    bool NoPwdLogin;

signals:
    void message();
};
