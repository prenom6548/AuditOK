/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <ukui-service-interface/serviceobject.h>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QSettings>
#include <QVector>
#include <QDBusContext>
#include <QDBusConnectionInterface>
#include <QRegularExpression>
#include <QStringList>

class SysdbusRegister : public UKUI::ServiceObject,QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.control.center.interface")
    Q_PROPERTY(bool NoPwdLoginStatus READ NoPwdLoginStatus)

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

public:
    bool checkAuthorization(qint64 id, QString action = "org.control.center.qt.systemdbus.action");
    bool authoriySetPid(qint64 id);
    bool authoriyGrub();
    bool authoriyAutoLogin(qint64 id);
    bool authoriySetHostName(qint64 id);
    void notifyPropertyChanged(const QString& interface, const QString &propertyName);
    bool NoPwdLoginStatus() const;

private slots:
    void prepareForSleepSlot(bool start);

private:
    QProcess *m_proberProcess = nullptr;
    QString mHibernateFile;
    QSettings *mHibernateSet;
    volatile bool exitFlag;
    volatile bool toGetDisplayInfo;
    QSettings  *aptSettings;
    qint64 _id;
    bool NoPwdLogin;
    bool forceI2c = false;
    bool m_isWayland = false;
    QString m_diskInfo;
    bool m_isRequesting = false;

private:
    bool isSudoGroupNumber(QString uname);

    QString getCpuInfo();

    bool callerPathCompare(QString path);

    void getDiskInfofromDbus();

    void onDiskInfoReceivedSlot(QDBusPendingCallWatcher *watcher);

    void serviceStatusChangedSlot(const QString &name, bool noEmpty);

    void asynGetDiskInfo();

    bool authoriySetNtpServer(qint64 id);
    // 校验软件包安装权限
    bool authoriyInstallPackage(qint64 id);
    // 校验软件包卸载权限
    bool authoriyUninstallPackage(qint64 id);

    bool m_packageInstallRunning = false;
    bool m_packageUninstallRunning = false;

signals:
    Q_SCRIPTABLE void nameChanged(QString);
    Q_SCRIPTABLE void computerinfo(QString);
    Q_SCRIPTABLE void changed(QString);
    Q_SCRIPTABLE void diskInfoReady(QString);
    Q_SCRIPTABLE void ntpServerChangedDefault();
    // 软件包安装进度信号
    Q_SCRIPTABLE void packageInstallProgress(QString requestId,
                                             QString packageName,
                                             int progress,
                                             int state,
                                             QString logInfo);
    // 软件包安装完成信号
    Q_SCRIPTABLE void packageInstallFinished(QString requestId,
                                             QString packageName,
                                             bool success,
                                             QString errorText);
    // 软件包卸载进度信号
    Q_SCRIPTABLE void packageUninstallProgress(QString requestId,
                                               QString packageName,
                                               int progress,
                                               int state,
                                               QString logInfo);
    // 软件包卸载完成信号
    Q_SCRIPTABLE void packageUninstallFinished(QString requestId,
                                               QString packageName,
                                               bool success,
                                               QString errorText);

public slots:

    Q_SCRIPTABLE void exitService();
    Q_SCRIPTABLE QString GetComputerInfo();
    // 设置进程id
    Q_SCRIPTABLE int setPid(qint64 id);

    // 设置免密登录状态
    Q_SCRIPTABLE bool setNoPwdLoginStatus(bool status,QString username = nullptr);

    // 获取免密登录状态
    Q_SCRIPTABLE QString getNoPwdLoginStatus();

    // 设置自动登录状态
    Q_SCRIPTABLE int setAutoLoginStatus(QString username);

    // 获取挂起到休眠时间
    Q_SCRIPTABLE QString getSuspendThenHibernate();

    // 设置挂起到休眠时间
    Q_SCRIPTABLE void setSuspendThenHibernate(QString time);

    // 提权修改其他用户密码
    Q_SCRIPTABLE int changeOtherUserPasswd(QString username, QString pwd);

    // 提权创建用户，避免两次验证弹窗
    Q_SCRIPTABLE int createUser(QString name, QString fullname, int accounttype, QString faceicon, QString pwd);

    // 修改硬件时间
    Q_SCRIPTABLE int changeRTC();

    // 设置NTP授时服务器
    Q_SCRIPTABLE bool setNtpSerAddress(QString serverAddress);

    //新亮度相关的接口
    Q_SCRIPTABLE void getDisplayInfo();
    Q_SCRIPTABLE QString showDisplayInfo();
    Q_SCRIPTABLE void setDisplayBrightness(QString brightness, QString edidHash, QString i2cBus = "-1");
    Q_SCRIPTABLE int getDisplayBrightness(QString edidHash, QString i2cBus = "-1");

    //设置apt代理
    Q_SCRIPTABLE bool setaptproxy(QString ip , QString port ,bool open);

    //设置apt代理
    Q_SCRIPTABLE bool setaptproxy(QString httpIp, QString httpPort, QString httpsIp, QString httpsPort, bool open);


    //获取apt代理信息
    Q_SCRIPTABLE QHash<QString , QVariant> getaptproxy();

    // 在线批量安装软件包
    Q_SCRIPTABLE bool installPackagesOnline(QStringList packageNames, QString requestId);

    // 批量卸载软件包
    Q_SCRIPTABLE bool uninstallPackages(QStringList packageNames, QString requestId);

    //修改/etc/hosts文件内的主机名
    Q_SCRIPTABLE bool sethostname(QString hostname);

    //获取内存大小
    Q_SCRIPTABLE QString getMemory();

    //获取设备类型
    Q_SCRIPTABLE QString getDmidecodeType();

    //获取dmidecode解析结果
    Q_SCRIPTABLE QString getDmiDecodeRes(QString args);

    //获取系统时区
    Q_SCRIPTABLE QString getShowTimezone();

    //设置系统时区
    Q_SCRIPTABLE bool setShowTimezone(QString timezone);

    // 设置 grub 密码
    Q_SCRIPTABLE bool setGrupPasswd(QString username, QString passwd, QString lang, bool status);

    // 设置 grub 启动时间
    Q_SCRIPTABLE bool setGrupBootTime(int time, QString lang);

    // 获取 grub 启动时间
    Q_SCRIPTABLE int getGrubBootTime();

    // 获取系统启动失败后是否进入救援模式
    Q_SCRIPTABLE bool getRescueModeStatus();

    // 设置系统启动失败后是否进入救援模式
    Q_SCRIPTABLE bool setRescueMode(bool status, QString lang);

    // 获取系统启动失败次数
    Q_SCRIPTABLE int getFailuresTimes();

    // 设置系统启动失败次数
    Q_SCRIPTABLE bool setFailuresTimes(int index, QString lang);

    // 设置默认启动项
    Q_SCRIPTABLE bool setDefaultBootItem(int index, QString lang);
    Q_SCRIPTABLE int getDefaultBootItem();

    // 通过os-prober 获取当前存在的系统个数
    Q_SCRIPTABLE int getActualSystem();

    // 获取 grub 密码是否开启
    Q_SCRIPTABLE bool getGrupPasswdStatus();

    // 手动更新 grub
    Q_SCRIPTABLE bool updateGrub(const QString &lang);

    // 获取系统安装时间
    Q_SCRIPTABLE QString getSysInstallTime(const QString &part);

    //设置xrdp服务
    Q_SCRIPTABLE void setXrdpService(bool status);

    // 获取硬盘容量大小
    Q_SCRIPTABLE QString getDiskInfo();
};

#endif // SYSDBUSREGISTER_H
