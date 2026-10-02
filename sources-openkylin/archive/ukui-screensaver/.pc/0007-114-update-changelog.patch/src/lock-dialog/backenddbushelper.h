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
#ifndef BACKENDDBUSHELPER_H
#define BACKENDDBUSHELPER_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "userinfo.h"
#include "agreementinfo.h"
#include "biodefines.h"

/**
 * @brief 后端服务访问工具类
 *
 */
class BackendDbusHelper : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    /**
     * @brief 接口名称
     *
     * @return const char
     */
    static inline const char *staticInterfaceName()
    {
        return "org.ukui.ScreenSaver";
    }

public:
    /**
     * @brief 构造
     *
     * @param service dbus服务名称
     * @param path dbus路径
     * @param connection dbus连接
     * @param parent 父指针
     */
    BackendDbusHelper(
        const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    /**
     * @brief 析构
     *
     */
    ~BackendDbusHelper();

public:
    /**
     * @brief 获取用户信息列表
     *
     * @return QList<UserInfoPtr>
     */
    QList<UserInfoPtr> getUsersInfo();

    bool isSessionActive();

    QString getDefaultAuthUser();

    QString getCurrentUser();

    QList<QString> getSessionsInfo();
    QString getCurrentSession();

    AgreementInfoPtr getAgreementInfo();
    QVariant getLockScreenConf(QString strKey);
    bool setLockScreenConf(QString strKey, QVariant value);
    QVariant getScreenSaverConf(QString strKey);
    bool setScreenSaverConf(QString strKey, QVariant value);
    QVariant getPowerManagerConf(QString strKey);
    bool setPowerManagerConf(QString strKey, QVariant value);
    QVariant getMateBgConf(QString strKey);
    bool setMateBgConf(QString strKey, QVariant value);
    QVariant getUkccPluginsConf(QString strKey);
    bool setUkccPluginsConf(QString strKey, QVariant value);
    QVariant getThemeStyleConf(QString strKey);
    bool setThemeStyleConf(QString strKey, QVariant value);
    QVariant getSessionConf(QString strKey);
    bool setSessionConf(QString strKey, QVariant value);
    QVariant getKeyboardConf(QString strKey);
    bool setKeyboardConf(QString strKey, QVariant value);

    QVariant getUsdMediaKeys(QString strKey);
    QVariant getUsdMediaStateKeys(QString strKey);
    bool setUsdMediaStateKeys(QString strKey, QVariant value);

    QList<QString> getSaverThemes();

    /**
     * @brief 调用usd dbus接口实现相关快捷键功能
     *
     *@param  actionType：快捷键类型
     *
     * @return 是否调用成功
     */
    bool usdExternalDoAction(int actionType);

    bool getPowerManagerCanSwitchUser();
    bool getPowerManagerCanHibernate();
    bool getPowerManagerCanPowerOff();
    bool getPowerManagerCanReboot();
    bool getPowerManagerCanSuspend();
    bool getPowerManagerCanLockScreen();
    bool getPowerManagerCanLogout();
    bool checkSystemUpgrade();

    QStringList getBatteryArgs();

    QString getBatteryIconName();

    bool getIsBattery();

    QStringList getSleepLockcheck();

    QStringList getShutdownLockcheck();

    bool getCurTabletMode();

public Q_SLOTS:
    /**
     * @brief dbus服务信息更新处理
     *
     * @param strJson 信息json
     */
    void onUpdateInformation(const QString &strJson);

    bool setCurrentUser(QString strUserName);
    int switchToUser(QString strUserName);
    bool setCurrentSession(QString strSession);
    bool lockStateChanged(bool isVisible, bool isSessionTools);
    void startSession();

    void pamAuthenticate(QString strUserName);
    void pamRespond(QString strRespond);
    void pamAuthenticateCancel();
    bool pamIsInAuthentication();
    bool pamIsAuthenticated();
    QString pamAuthenticateUser();
    bool setPowerManager(QString strFuncName);

    QString getPublicEncrypt();
    bool sendPassword(const QString username, QByteArray password);

    void bioStartAuth(int uid, int nDevId);
    void bioStopAuth();
    QList<DeviceInfo> bioGetAvailableDevices(int nUid);
    QList<int> bioGetDisabledDevices(int nUid);
    int bioGetBioAuthState();
    DeviceInfo bioGetCurBioInfo();
    DeviceInfo bioFindDeviceById(int nUid, int nDevId);
    DeviceInfo bioFindDeviceByName(int nUid, QString strDevName);
    QString bioGetDefaultDevice(int nUid, QString strUserName, int bioType = -1);
public Q_SLOTS: // METHODS
    /**
     * @brief 获取黑色屏保状态
     *
     * @return QDBusPendingReply<bool> 是否为黑色屏保状态
     */
    inline QDBusPendingReply<bool> GetBlankState()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QStringLiteral("GetBlankState"), argumentList);
    }

    /**
     * @brief 获取信息
     *
     * @param in0 信息参数json
     * @return QDBusPendingReply<QString> 信息json
     */
    inline QDBusPendingReply<QString> GetInformation(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetInformation"), argumentList);
    }

    /**
     * @brief 获取锁定状态
     *
     * @return QDBusPendingReply<bool> 锁定状态
     */
    inline QDBusPendingReply<bool> GetLockState()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetLockState"), argumentList);
    }

    /**
     * @brief 锁定
     *
     * @return QDBusPendingReply<>
     */
    inline QDBusPendingReply<> Lock()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Lock"), argumentList);
    }

    /**
     * @brief 以黑色屏保方式锁定
     *
     * @param in0 触发锁定的类型
     * @return QDBusPendingReply<bool> 是否锁定成功
     */
    inline QDBusPendingReply<bool> LockByBlank(int in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("LockByBlank"), argumentList);
    }

    /**
     * @brief 设置信息
     *
     * @param in0 信息参数json
     * @return QDBusPendingReply<int> 设置结果 0 成功，其他失败
     */
    inline QDBusPendingReply<int> SetInformation(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetInformation"), argumentList);
    }

    /**
     * @brief 设置锁定状态
     *
     * @return QDBusPendingReply<>
     */
    inline QDBusPendingReply<> SetLockState()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("SetLockState"), argumentList);
    }

    /**
     * @brief 显示屏保
     *
     * @return QDBusPendingReply<>
     */
    inline QDBusPendingReply<> ShowScreensaver()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ShowScreensaver"), argumentList);
    }

    /**
     * @brief 解锁
     *
     * @return QDBusPendingReply<>
     */
    inline QDBusPendingReply<> UnLock()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("UnLock"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    /**
     * @brief 第二次启动参数通知
     *
     * @param param 启动参数
     */
    void SecondRunParam(const QString &param);
    /**
     * @brief 会话空闲进入屏保
     *
     */
    void SessionIdle();
    /**
     * @brief 会话空闲进入锁屏
     *
     */
    void SessionLockIdle();
    /**
     * @brief 后端服务信息更新
     *
     * @param info 信息json
     */
    void UpdateInformation(const QString &info);
    /**
     * @brief 进入锁定
     *
     */
    void lock();
    /**
     * @brief 退出锁定
     *
     */
    void unlock();

    /**
     * @brief 用户信息更新
     *
     * @param list 用户信息列表
     */
    void usersInfoChanged(QList<UserInfoPtr> list);

    /**
     * @brief 请求锁定
     *
     */
    void requestLock();
    /**
     * @brief 请求解锁
     *
     */
    void requestUnlock();
    /**
     * @brief 会话活跃状态改变
     *
     * @param isActive true 活跃，false 不活跃
     */
    void sessionActiveChanged(bool isActive);
    /**
     * @brief 准备休眠/唤醒
     *
     * @param isSleep true 休眠，false 唤醒
     */
    void PrepareForSleep(bool isSleep);

    void currentUserChanged(QString strUserName);

    void currentSessionChanged(QString strSession);

    void blockInhibitedChanged(QString lidstate);

    void batteryStatusChanged(QString iconName);

    void batteryChanged(QStringList batteryArgs);

    void screenSaverConfChanged(QString strKey, QVariant value);
    void lockScreenConfChanged(QString strKey, QVariant value);
    void powerManagerConfChanged(QString strKey, QVariant value);
    void mateBgConfChanged(QString strKey, QVariant value);
    void ukccPluginsConfChanged(QString strKey, QVariant value);
    void themeStyleConfChanged(QString strKey, QVariant value);
    void sessionConfChanged(QString strKey, QVariant value);
    void capslockConfChanged(bool state);

    void pamShowMessage(QString text, int type);
    void pamShowPrompt(QString text, int type);
    void pamAuthCompleted();

    void usdMediaKeysConfChanged(QString strKey, QString value);
    void usdMediaStateKeysConfChanged(QString strKey, int value);

    void bioServiceStatusChanged(bool bValid);
    void bioDeviceChanged();
    void bioAuthShowMessage(QString strMsg);
    void bioAuthStateChanged(int nState);
    void bioAuthFrameData(QString strData);
    void bioAuthCompleted(int nUid, bool isSuccess, int nError, int nMaxFailedTime, int nFailedTime);

    void tabletModeChanged(bool tabletMode);

private:
    /**
     * @brief 连接信号槽
     *
     */
    void initConnections();

    QList<UserInfoPtr> ParseUsersInfo(const QJsonObject &objRes);

    QList<QString> ParseSessionsInfo(const QJsonObject &objRes);

    QList<QString> ParseSaverThemes(const QJsonObject &objRes);

    QStringList ParseBatteryArgs(const QJsonObject &objRes);

    QStringList ParseSleepLockcheck(const QJsonObject &objRes);

    QStringList ParseShutdownLockcheck(const QJsonObject &objRes);

    AgreementInfoPtr ParseAgreementInfo(const QJsonObject &objRes);

    bool ParseLogin1ReqLock(const QJsonObject &objRes);

    bool ParseLogin1ReqUnLock(const QJsonObject &objRes);

    bool ParseLogin1PrepareForSleep(const QJsonObject &objRes);

    bool ParseLogin1SessionActiveChanged(const QJsonObject &objRes);

    bool ParseCurrentUser(const QJsonObject &objRes);

    bool ParseCurrentSession(const QJsonObject &objRes);

    bool ParseBlockInhibitedChanged(const QJsonObject &objRes);

    bool ParseBatteryStatusChanged(const QJsonObject &objRes);

    bool ParseBatteryChanged(const QJsonObject &objRes);

    bool ParseScreenSaverConf(const QJsonObject &objRes);
    bool ParseLockScreenConf(const QJsonObject &objRes);
    bool ParsePowerManagerConf(const QJsonObject &objRes);
    bool ParseMateBgConf(const QJsonObject &objRes);
    bool ParseUkccPluginsConf(const QJsonObject &objRes);
    bool ParseThemeStyleConf(const QJsonObject &objRes);
    bool ParseSessionConf(const QJsonObject &objRes);
    bool ParseCapslockConf(const QJsonObject &objRes);

    void ParsePamShowMessage(const QJsonObject &objRes);
    void ParsePamShowPrompt(const QJsonObject &objRes);
    void ParsePamAuthCompleted(const QJsonObject &objRes);

    bool ParseUsdMediaKeysConf(const QJsonObject &objRes);
    bool ParseUsdMediaStateKeysConf(const QJsonObject &objRes);

    void ParseBioServiceStatus(const QJsonObject &objRes);
    void ParseBioDeviceChanged(const QJsonObject &objRes);
    void ParseBioAuthShowMessage(const QJsonObject &objRes);
    void ParseBioAuthStateChanged(const QJsonObject &objRes);
    void ParseBioAuthFrameData(const QJsonObject &objRes);
    void ParseBioAuthCompleted(const QJsonObject &objRes);

    void ParseTabletModeChanged(const QJsonObject &objRes);
};

#endif // BACKENDDBUSHELPER_H
