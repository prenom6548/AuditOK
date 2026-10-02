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
#ifndef DBUSUPPERINTERFACE_H
#define DBUSUPPERINTERFACE_H

#include <QObject>
#include <QDBusContext>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QProcess>
#include <QSharedPointer>
#include <QTimer>
#include "../../src/common/utils.h"
#include "../../src/common/rsac.h"
#include "../../src/common/definetypes.h"

class LightDMHelper;
class Login1Helper;
class GSettingsHelper;
class Configuration;
class QJsonObject;
class PamAuthenticate;
class UsdHelper;
class UpowerHelper;
class AccountsHelper;
class SessionHelper;
class SystemUpgradeHelper;
class SessionWatcher;
class BioAuthenticate;
class KglobalAccelHelper;
class LibinputSwitchEvent;
class NetWorkManagerHelper;

/**
 * @brief dbus服务接口实现类
 *
 */
class DbusUpperInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SS_DBUS_SERVICE)
public:
    enum
    {
        SESSION_STATUS_SCREENSAVER = 1, // 屏保
        SESSION_STATUS_SCREENCLOSE = 2, // 关屏
        SESSION_STATUS_SLEEPED = 4,     // 休眠/睡眠
    };
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit DbusUpperInterface(QObject *parent = nullptr);

    virtual ~DbusUpperInterface();

    void init();

public:
    bool checkScreenDialogRunning();
    void emitLockState(bool val, bool isSessionTools = false);
    void LockStartupMode();
    /**
     * @brief 获取黑色屏保状态（沿用）
     *
     * @return bool true 是，false 否
     */
    bool GetBlankState();
    /**
     * @brief 获取锁屏状态
     *
     * @return bool true 锁定，false 未锁定
     */
    bool GetLockState();
    /**
     * @brief 锁屏
     *
     */
    void Lock();
    /**
     * @brief SwitchUser
     *
     */
    void SwitchUser();
    /**
     * @brief 解锁
     *
     */
    void UnLock();
    /**
     * @brief 以黑色屏保方式锁屏
     *
     * @param int 锁定方式 0 延时关屏、1 合盖、2 低电量
     * @return bool 是否执行 true 已执行，false 未执行
     */
    bool LockByBlank(int);
    /**
     * @brief 设置锁定状态
     *
     */
    void SetLockState();
    /**
     * @brief 显示屏保
     *
     */
    void ShowScreensaver();
    /**
     * @brief 显示锁屏和屏保
     *
     */
    void LockScreensaver();
    /**
     * @brief session tools
     *
     */
    bool CheckAppVersion();
    void SessionTools();
    void AppBlockWindow(QString actionType);
    void MultiUserBlockWindow(QString actionType);
    void Suspend();
    void Logout();
    void Reboot();
    void PowerOff();
    void Hibernate();
    /**
     * @brief 获取信息
     *
     * @param strJson 需要获取的信息json
     * @return QString 获取到的信息json
     */
    QString GetInformation(QString strJson);
    /**
     * @brief 设置信息
     *
     * @param strJson 需要设置的信息json
     * @return int 设置结果，0 成功，否则 失败
     */
    int SetInformation(QString strJson);

    QString getPublicEncrypt();

    bool sendPassword(const QString username, QByteArray password);

    QSharedPointer<GSettingsHelper> getGSettingsInstance();

public Q_SLOTS:
    /**
     * @brief 服务退出
     *
     * @param serviceName 服务名称
     */
    void onNameLost(const QString &serviceName);
    /**
     * @brief 用户信息改变
     *
     */
    void onUsersInfoChanged();
    /**
     * @brief 请求锁定
     *
     */
    void onLogin1ReqLock();
    /**
     * @brief 请求解锁
     *
     */
    void onLogin1ReqUnLock();
    /**
     * @brief 系统准备休眠/唤醒
     *
     * @param isSleep true 休眠,否则唤醒
     */
    void onLogin1PrepareForSleep(bool isSleep);
    /**
     * @brief 用户会话活跃状态改变
     *
     * @param isActive true 活跃，否则不活跃
     */
    void onLogin1SessionActiveChanged(bool isActive);
    /**
     * @brief 程序阻塞关机/睡眠状态改变
     *
     * @param blockInhibited 阻塞类型
     */
    void onBlockInhibitedChanged(QString blockInhibited);

    void onBatteryStatusChanged(QString iconName);

    void onBatteryChanged(QStringList batteryArgs);

    void onLidStateChanged(bool isClosed);

    void onLockScreenConfigChanged(QString strKey, QVariant value);
    void onScreenSaverConfigChanged(QString strKey, QVariant value);
    void onPowerManagerConfigChanged(QString strKey, QVariant value);
    void onMateBgConfigChanged(QString strKey, QVariant value);
    void onUkccPluginsConfigChanged(QString strKey, QVariant value);
    void onThemeStyleConfigChanged(QString strKey, QVariant value);
    void onSessionConfigChanged(QString strKey, QVariant value);
    void onKeyboardConfigChanged(QString strKey, QVariant value);

    void onCurrentSessionChanged(QString strSession);
    void onPamShowMessage(QString strMsg, int type);
    void onPamShowPrompt(QString strPrompt, int type);
    void onPamAuthCompleted();

    void onUsdMediaKeysConfigChanged(QString strKey, QVariant value);
    void onUsdMediaStateKeysConfigChanged(QString strKey, QVariant value);

    // 空闲锁屏
    void onSessionIdleReceived();
    void onSessionIdleExit();
    void stopDelayLockScreen();

    void onBioAuthShowMessage(QString strMsg);
    void onBioAuthStateChanged(int nState);
    void onBioServiceStatusChanged(bool bValid);
    void onBioDeviceChanged();
    void onBioAuthFrameData(QString strData);
    void onBioAuthCompleted(int nUid, bool isSuccess, int nError, int nMaxFailedTime, int nFailedTime);

    // 平板模式切换
    void onTabletModeChanged(bool tabletMode);

    void onNetWorkConnectStateChanged(int state);
private Q_SLOTS:
    void onLockDialogProcExit(int exitCode, QProcess::ExitStatus exitStatus);

Q_SIGNALS:
    /**
     * @brief 信息更新
     *
     * @param info 信息内容json
     */
    void UpdateInformation(const QString &info);

private:
    /**
     * @brief 生成用户信息json数组
     *
     * @return QJsonArray 信息数组
     */
    QJsonArray GenerateUserInfoList();

    QString GetDefaultAuthUser();

    QString GetCurrentUser();

    int SetCurrentUser(const QJsonObject &objInfo);

    int switchToUser(const QJsonObject &objInfo);

    QJsonArray GenerateSessionInfoList();

    QString GetCurrentSession();

    int SetCurrentSession(const QJsonObject &objInfo);

    int StartSession(const QJsonObject &objInfo);

    /**
     * @brief 生成开机特别提示json字段
     *
     * @return QJsonObject 信息数组
     */
    QJsonObject GenerateAgreementInfo();

    void GetScreenSaverConf(QJsonObject &reqObj, QJsonObject &retObj);

    QJsonArray getSaverPhotoPath(QString key);

    int SetScreenSaverConf(const QJsonObject &objInfo);

    void GetLockScreenConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetLockScreenConf(const QJsonObject &objInfo);

    void GetPowerManagerConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetPowerManagerConf(const QJsonObject &objInfo);

    void GetMateBgConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetMateBgConf(const QJsonObject &objInfo);

    void GetUkccPluginsConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetUkccPluginsConf(const QJsonObject &objInfo);

    void GetThemeStyleConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetThemeStyleConf(const QJsonObject &objInfo);

    void GetSessionConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetSessionConf(const QJsonObject &objInfo);

    void GetKeyboardConf(QJsonObject &reqObj, QJsonObject &retObj);

    int SetKeyboardConf(const QJsonObject &objInfo);

    void inhibit();

    void uninhibit();

    bool checkStatus(int nStatus);

    bool GetSlpState();

    void onShowBlankScreensaver();

    void IsPamInAuthentication(QJsonObject &reqObj, QJsonObject &retObj);

    void IsPamAuthenticated(QJsonObject &reqObj, QJsonObject &retObj);

    void PamAuthenticationUser(QJsonObject &reqObj, QJsonObject &retObj);

    int PamAuthenticateUser(const QJsonObject &objInfo);

    int PamAuthenticateCancel(const QJsonObject &objInfo);

    int PamRespond(const QJsonObject &objInfo);

    void GetUsdMediaKeys(QJsonObject &reqObj, QJsonObject &retObj);

    void GetUsdMediaStateKeys(QJsonObject &reqObj, QJsonObject &retObj);

    int SetUsdMediaStateKeys(const QJsonObject &objInfo);

    void GetWeatherKeys(QJsonObject &reqObj, QJsonObject &retObj);

    bool usdExternalDoAction(const QJsonObject &objInfo);

    bool blockShortcut(const QJsonObject &objInfo);

    int SetPowerManager(const QJsonObject &objInfo);

    bool lockStateChanged(const QJsonObject &objInfo);

    void GetCanPowerManager(QJsonObject &reqObj, QJsonObject &retObj);

    QJsonArray GenerateBatteryArgsList();

    QString getBatteryIconName();

    bool getIsBattery();

    QJsonArray getSleepLockCheck();

    QJsonArray getShutdownLockcheck();

    QJsonArray getSaverTheme();

    /**
     * @brief delayLockScreen 延迟锁屏
     */
    void delayLockScreen();

    /**
     * @brief onLockScreenTimeout 延迟锁屏处理过程
     */
    void onLockScreenTimeout();

    void BioGetAvailableDevices(QJsonObject &reqObj, QJsonObject &retObj);

    void BioGetDisabledDevices(QJsonObject &reqObj, QJsonObject &retObj);

    void BioGetBioAuthState(QJsonObject &reqObj, QJsonObject &retObj);

    void BioGetCurBioInfo(QJsonObject &reqObj, QJsonObject &retObj);

    void BioGetDefaultDevice(QJsonObject &reqObj, QJsonObject &retObj);

    void BioFindDeviceById(QJsonObject &reqObj, QJsonObject &retObj);

    void BioFindDeviceByName(QJsonObject &reqObj, QJsonObject &retObj);

    int BioStartAuth(const QJsonObject &objInfo);

    int BioStopAuth(const QJsonObject &objInfo);

    bool getCurTabletMode();

    int getNetworkConnectState();

private:
    /**
     * @brief 初始化数据
     *
     */
    void initData();
    /**
     * @brief 初始化连接
     *
     */
    void initConnections();
    /**
     * @brief SendUpdateInfoSig 发送更新信息信号
     * @param strJson 信息内容json
     * @return true 发送成功，否则失败
     */
    bool SendUpdateInfoSig(QString strJson);

private:
    QSharedPointer<LightDMHelper> m_lightDmHelper = nullptr; /**< lightdm工具实例 */
    QSharedPointer<Login1Helper> m_login1Helper = nullptr;   /**< login1工具实例 */
    Configuration *m_config = nullptr;
    QSharedPointer<GSettingsHelper> m_gsettingsHelper = nullptr;
    QProcess m_procLockDialog;
    bool m_bLockState;
    bool m_bBlankState = false;
    bool m_bSlpState;
    int m_nStatus = 0;         // 当前状态
    QTimer *m_timer = nullptr; // check lockdialog is show
    int m_timerCount = 0;
    QTimer *m_timerLock = nullptr;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    QSharedPointer<PamAuthenticate> m_pamAuth = nullptr;
    QSharedPointer<UsdHelper> m_usdHelper = nullptr;
    QSharedPointer<UpowerHelper> m_upowerHelper = nullptr;
    QSharedPointer<AccountsHelper> m_accountsHelper = nullptr;
    bool lockState = false;
    QSharedPointer<SessionHelper> m_sessionHelper = nullptr;
    QSharedPointer<SystemUpgradeHelper> m_systemsUpgradeHelper = nullptr;
    QSharedPointer<SessionWatcher> m_sessionWatcher = nullptr;
    QSharedPointer<BioAuthenticate> m_bioAuth = nullptr;
    RSAC rsac;
    QByteArray pubKey, priKey;
    QSharedPointer<KglobalAccelHelper> m_kglobalHelper = nullptr;
    QSharedPointer<LibinputSwitchEvent> m_libinputSwitchEvent = nullptr;
    bool m_bBlockShortcutState = false;
    QSharedPointer<NetWorkManagerHelper> m_networkManagerHelper = nullptr;
};

#endif // DBUSUPPERINTERFACE_H
