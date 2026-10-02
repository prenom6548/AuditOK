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
#ifndef UNIAUTH_SERVICE_H
#define UNIAUTH_SERVICE_H

#include <QtDBus>
#include <QDBusAbstractInterface>

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

QDBusArgument &operator <<(QDBusArgument &arg, const WillLoginUserInfo &willLoginUserInfo);
const QDBusArgument &operator >>(const QDBusArgument &arg, WillLoginUserInfo &willLoginUserInfo);
QDebug operator <<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo);

Q_DECLARE_METATYPE(WillLoginUserInfo)

class UniAuthService : public QDBusAbstractInterface
{
    Q_OBJECT
private:
    explicit UniAuthService(QObject *parent = nullptr);

public:
    /**
     * @brief 单实例
     *
     * @param parent 父指针
     * @return UniAuthService 单实例对象指针
     */
    static UniAuthService *instance(QObject *parent = nullptr);

public Q_SLOTS:
    // 设置默认设备
    void setDefaultDevice(int bioDevType, QString deviceName);
    // 获取默认设备
    QString getDefaultDevice(QString userName, int bioDevType);
    // 获取所有默认设备
    QStringList getAllDefaultDevice(QString userName);
    //生物特征开关接口
    bool getBioAuthStatus(QString userName, int bioAuthType);
    void setBioAuthStatus(int bioAuthType, bool status);
    // 获取最大失败次数
    int getMaxFailedTimes();
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
    // 获取人脸超时停用次数
    int getFTimeoutTimes();
    /**
     * @brief 用户名是否有效
     *
     * @param strUserName 用户名
     * @return bool true 有效，否则 无效
     */
    bool isUserNameValid(QString strUserName);
    /**
     * @brief 一键切换并登录用户
     *
     * @param strUserName 用户名
     */
    void SwitchToUser(QString strUserName);
    /**
     * @brief 切换到登录用户
     *
     * @param strUserName 用户名
     */
    void SwitchToGreeterUser(QString strUserName);
    /**
     * @brief 将要登录的用户信息
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

public:
    bool isActivatable();

Q_SIGNALS:
    //默认设备改变
    void defaultDeviceChanged(QString userName, int bioDevType, QString deviceName);
    //开关状态改变
    void bioAuthStatusChanged(QString userName, int type, bool status);
    /**
     * @brief 将要登录的用户名改变
     *
     * @param strUserName 新用户名（需对空串异常处理）
     * @param isOneKeyLogin 是否需要一键登录
     */
    void userChanged(WillLoginUserInfo willLoginUser);

private:
    class MemCollector{
    public:
        ~MemCollector(){
            if (UniAuthService::instance()) {
                delete UniAuthService::instance();
                UniAuthService::m_instance = nullptr;
            }
        }
    };
    static UniAuthService *m_instance; /** 统一认证配置服务单实例指针 */
    static MemCollector m_memCollector;

private:
    bool m_isActivatable;
};

#endif // UNIAUTH_SERVICE_H
