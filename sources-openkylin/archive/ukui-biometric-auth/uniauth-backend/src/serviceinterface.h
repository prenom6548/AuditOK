/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include "biodeviceinfo.h"
#include <QCoreApplication>
#include <QDBusContext>
#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>
#include <QDBusInterface>
#include "rsac.h"
#include "greeterconfiginterface.h"

#define DBUS_SERVICE "org.ukui.Biometric"
#define DBUS_PATH "/org/ukui/Biometric"
#define DBUS_INTERFACE "org.ukui.Biometric"

enum authEnableType
{
    ENABLETYPE_BIO,     // 全局总使能
    ENABLETYPE_SAVER,   // 锁屏
    ENABLETYPE_GREETER, // 登录
    ENABLETYPE_POLKIT,  // 授权
    ENABLETYPE_SU,      // 暂保留
    ENABLETYPE_SUDO,    // 暂保留
    ENABLETYPE_LOGIN,   // 暂保留
};

/**
 * @brief 用户信息
 */
struct WillLoginUserInfo
{
    QString strUserName = "";
    bool isOneKeyLogin = false;
};

QDBusArgument &operator<<(QDBusArgument &arg, const WillLoginUserInfo &willLoginUserInfo);
const QDBusArgument &operator>>(const QDBusArgument &arg, WillLoginUserInfo &willLoginUserInfo);
QDebug operator<<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo);

Q_DECLARE_METATYPE(WillLoginUserInfo)

class ServiceInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.UniauthBackend")
public:
    ServiceInterface();

public slots:
    // 设置默认设备
    void setDefaultDevice(int bioDevType, QString deviceName);
    // 获取默认设备
    QString getDefaultDevice(QString userName, int bioDevType);
    // 获取所有默认设备
    QStringList getAllDefaultDevice(QString userName);
    // 生物特征开关接口
    bool getBioAuthStatus(QString userName, int bioAuthType);
    void setBioAuthStatus(int bioAuthType, bool status);
    // 获取最大失败次数
    int getMaxFailedTimes();
    // 获取人脸识别超时停用次数
    int getFTimeoutTimes();
    // 设置人脸识别超时停用次数
    void setFTimeoutTimes(int times);
    // 获取是否使能微信扫码登录
    bool getQRCodeEnable();
    // 获取是否双认证
    bool getDoubleAuth();
    // 获取用户绑定
    bool getUserBind();
    // 获取是否在控制面板显示
    bool getIsShownInControlCenter();
    // 获取是否使用第一个设备
    bool getUseFirstDevice();
    // 获取是否隐藏切换按钮
    bool getHiddenSwitchButton();
    // 记录当前正在进行生物认证的设备
    void recordAuthDrive(QString appName, int drvid, bool insert);
    // 获取当前正在进行生物认证的应用
    QStringList getAllAuthApp();
    // 获取当前应用在认证的驱动
    int getAppAuthDrive(QString appName);
    // 获取用户配置
    QString GetUserInformation(QString username);
    // 同步登录界面配置
    void SetLoginSynInformation(QString val);
    // 获取公钥
    QString getPublicEncrypt();
    // 发送用户名和密码
    bool sendPassword(QString username, QByteArray password);
    // 前端注册到后端接口
    bool registerLoginApp();
    // 发送认证结果
    bool sendSignalLoginFinished(QString username, bool res);
    /**
     * @brief 一键切换并登录用户
     *
     * @param strUserName 用户名
     * @return int 0 成功，否则失败
     */
    int SwitchToUser(QString strUserName);
    /**
     * @brief 切换到登录用户
     *
     * @param strUserName 用户名
     * @return int 0 成功，否则失败
     */
    int SwitchToGreeterUser(QString strUserName);
    /**
     * @brief 获取将要登录的用户名
     *
     * @return WillLoginUserInfo 用户信息
     */
    WillLoginUserInfo getWillSwitchUser();
    /**
     * @brief SaveLastLoginUser 记录上一次登录成功的用户
     * @param strUserName 用户名
     * @return 0 成功，其他失败
     */
    int SaveLastLoginUser(QString strUserName);
    /**
     * @brief GetLastLoginUser 获取上一次登录的用户
     * @return 用户名
     */
    QString GetLastLoginUser();
    /**
     * @brief SaveQuickLoginUser 保存下次快速登录的用户
     * @param strUserName 用户名
     * @return 0 成功，其他失败
     */
    int SaveQuickLoginUser(QString strUserName);
    /**
     * @brief GetQuickLoginUser 获取快速登录的用户名
     * @return 用户名
     */
    QString GetQuickLoginUser();

    // 设置优先级最高的认证类型（默认为密码）
    void setDefaultAuthType(QString userName, int bioDevType);
    // 获取优先级最高的认证类型（默认为密码）
    int getDefaultAuthType(QString userName);

private slots:
    void onUSBDeviceHotPlug(int drvid, int action, int deviceNum);
    void onBiometricDbusChanged(bool bActive);
    void onDbusNameOwnerChanged(QString name, QString oldOwner, QString newOwner);

signals:
    // 默认设备改变
    void defaultDeviceChanged(QString userName, int bioDevType, QString deviceName);
    // 开关状态改变
    void bioAuthStatusChanged(QString userName, int type, bool status);
    // 用户配置改变
    void updateUserInformation(QString jsonstring);
    // 单点登录完成信号
    void onSignalLoginFinished(QString username, bool res);
    // 登录服务以启动信号
    void onLoginServicdRegisted();
    /**
     * @brief 将要登录的用户名改变
     *
     * @param willLoginUser 新用户信息（需对空用户名异常处理）
     */
    void userChanged(WillLoginUserInfo willLoginUser);
    // 默认认证类型改变
    void defaultAuthTypeChanged(QString userName, int bioDevType);

private:
    // 设置默认设备
    void setDefaultDevice(QString userName, int bioDevType, QString deviceName);
    // 设置通用默认设备
    void setCommDefaultDevice(int bioDevType, QString deviceName);
    // 获取通用默认设备
    QString getCommDefaultDevice(int bioDevType);
    // 获取旧版app使能值
    int getOldAppStatus();
    // 初始化数据
    void initData();
    // 更新通用默认设备
    void updateCommDefaultDevice(int nDriId);
    // 等待生物识别服务
    void waitBiometricServiceStatus();
    // 除 SSO 外接口：仅 root 或白名单进程
    bool ensureTrustedCaller();
    // SSO 相关接口：仅 root
    bool ensureRootCaller();
    // 判断系统是否为990
    void checkIs990();
    // 获取系统是否为990
    bool getIs990();
    // 根据pid获取进程名称
    QString getProcessNameFromPid(int pid);
    // 根据uid获取用户名
    QString getUserNameFromUid(int uid);
    // 获取登录锁屏界面的公钥
    QString getLoginPubKey(QString service, QString path, QString interface);
    // 发送密码到登录锁屏界面
    bool sendLoginPassword(QString service, QString path, QString interface, QString username, QByteArray array);
    // 处理首次登录逻辑
    bool handleFirstSingleLogn(QString username, QByteArray decryptText);

private:
    QDBusInterface *m_serviceInterface = nullptr;
    DeviceList m_listDeviceInfos;
    int deviceCount = 0;
    bool is990 = false;
    bool m_isDataInited = false;
    QMap<QString, int> m_AuthingDriveList;

    QMap<QString, QString> loginAppList;
    RSAC rsac;
    QByteArray priKey, pubKey;
    bool isFirstLogin = true;
    bool hasSaveLogin = false;
    QString savedUsername;
    QByteArray savedPassword;
    GreeterConfigInterface *m_greeterConfigInterface = nullptr;

    WillLoginUserInfo m_willLoginUser; /** 将登录的用户信息 */
};

#endif // SERVICEINTERFACE_H
