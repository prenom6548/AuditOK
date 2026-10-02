/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
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

#ifndef UKUI_APT_H
#define UKUI_APT_H
// Use QObject
#include <QDebug>
#include <QObject>
#include <QVariant>
// Use libqapt as apt interface
//#include <qapt/backend.h>
//#include <qapt/debfile.h>
//#include <qapt/package.h>
//#include <qapt/transaction.h>
//
#include "device_information.h"
#include <QThread>

class QTimer;

enum class ukuiAptError : int {
    UKUI_APT_SUCCESS = 0,
    UKUI_APT_CANNOT_ACCESS_SERVER,
    UKUI_APT_NO_SUCH_PACKAGE,
    UKUI_APT_INSTALL_FAIL,
};

enum class ukuiInstallStatus : int {
    UKUI_INSTALL_START = 0,
    UKUI_INSTALL_IN_PROGRESS,
    UKUI_INSTALL_SUCCESS,
    UKUI_INSTALL_FAIL,
};

//Worker Class
//TODO : 需要手动判断包是否已经安装，优化匹配速度
class AptUtilHelper : public QObject
{
    Q_OBJECT
public:
    explicit AptUtilHelper(DeviceInformation device,QObject *parent = nullptr);
    ~AptUtilHelper();
Q_SIGNALS:
    void installEnd();
    void finished();
    void succeed();
    void failed(QString err);
    void error(QString err);
//暂时还没发出来
    void alreadyInstalled();
public slots:
    void onInstalldebStatusChanged(int, QString, QString);
    void onRecvApt(bool, QStringList, QString, QString);
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
    void processPkg();
    void installPackage(QStringList packageName);
    QThread *m_thread{nullptr};
    QTimer *m_timer{nullptr};
    QString m_packageName = "";
    DeviceInformation m_device = DeviceInformation();
    QStringList m_packages = QStringList();
};

//TODO : 需要手动判断包是否已经安装，优化匹配速度
class DebUtilHelper : public QObject
{
    Q_OBJECT
public:
    explicit DebUtilHelper(QString debName,QObject *parent = nullptr);
    ~DebUtilHelper();
Q_SIGNALS:
    void installEnd();
    void finished();
    void failed(QString err);
    void succeed();
    void error(QString err);
//暂时还没发出来
    void alreadyInstalled();
public slots:
    void onInstalldebStatusChanged(int, QString, QString);
    void onRecvApt(bool, QString, QString);
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }

    void processDeb();
    void installLocalDeb(QString debFilePath);
    //工作线程
    QThread *m_thread{nullptr};
    QTimer *m_timer{nullptr};
    QString m_debName = "";
};

#endif //UKUI_APT_H
