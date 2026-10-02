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
#ifndef LOCKDIALOGMODEL_H
#define LOCKDIALOGMODEL_H

#include <QObject>
#include "../../src/userinfo.h"
#include "../../src/agreementinfo.h"
#include "../../src/widgets/screensavermode.h"
#include "../../src/common/biodefines.h"
#include "../../src/common/windowdata.h"
#include <QDebug>
#include <QPair>

/**
 * @brief 锁屏模型类（管理所有功能逻辑和状态）
 *
 */
class LockDialogModel : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief
     *
     */
    struct CommandLineArgs
    {
        bool isSessionIdle = false;     /**< 是否会话空闲 */
        bool isLock = false;            /**< 是否直接锁定 */
        bool isLockStartup = false;     /**< 是否开机启动锁定 */
        bool isScreenSaver = false;     /**< 是否屏保 */
        bool isLockScreensaver = false; /**< 是否锁定与屏保 */
        bool isBlank = false;           /**< 是否黑色屏保锁定 */
        bool isBlankHasLock = true;     /**< 是否黑色屏保需要锁定 */
        int nBlankDelay = 0;            /**< 黑色屏保延迟锁定时间 */
        bool isSessionTools = false;    /**< 显示session电源管理界面*/
        int isAppBlock = -1;            /**< 显示应用阻塞关机/重启界面*/
        int isMultiUserBlock = -1;      /**< 显示多用户登录阻塞关机/重启界面*/
        int isSwitchUser = false;       /**< 显示switchuser*/
        int isShutdownAnimation = -1;   /**< 显示关机转场动画*/
        QString strSwitchToUser = "";   /**< switch to user*/
    };
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit LockDialogModel(QObject *parent = nullptr);

public:
    /**
     * @brief 解析命令行参数
     *
     * @param args 命令行参数列表
     * @param cmdArgs 解析后结果
     * @return bool 是否解析完成
     */
    bool parseCmdArguments(QStringList args, CommandLineArgs &cmdArgs);
    /**
     * @brief 更新会话状态
     *
     * @param isActive 是否激活 true 是，否则不是
     */
    void updateSessionState(bool isActive);
    /**
     * @brief 会话是否激活
     *
     * @return bool true 是，否则不是
     */
    bool sessionActive();
    /**
     * @brief 是否是wayland模式
     *
     * @return bool true 是，否则不是
     */
    inline bool isUseWayland()
    {
        return m_isUseWayland;
    }
    /**
     * @brief 更新用户信息列表
     *
     * @param list 用户信息列表
     */
    void updateUsersInfo(QList<UserInfoPtr> list);

    void updateCapslockState(bool capslockState);

    void updateUsdMediaKeys(QString keys, QString value);

    void updateUsdMediaStateKeys(QString keys, int value);

    void updatePowerButtonKey(QString keys, QString value);

    void updateBatteryArgs(QStringList batteryArgs);

    void updateBatteryIconName(QString iconName);

    void updateIsBattery(bool isBattery);

    void updateIsSessionLegacy(bool isLegacy);

    void updateLogoutLockcheck(const QVector<InhibitorInfo> &logoutLockCheck);

    void updateShutdownLockcheck(const QVector<InhibitorInfo> &shutdownLockcheck);

    void updateSleepLockcheck(const QVector<InhibitorInfo> &sleepLockcheck);

    void updateAppWindows(const QVector<WindowInfo::WindowInfo> windows);

    void updateLoggedInUsersCount();

    void updateTabletMode(bool tabletMode);
    /**
     * @brief 获取用户信息列表
     *
     * @return QList<UserInfoPtr> 用户信息列表
     */
    inline QList<UserInfoPtr> usersInfo()
    {
        return m_listUsersInfo;
    }

    inline int normalUserCount()
    {
        int nCount = 0;
        for (auto user : m_listUsersInfo) {
            if (!user->isSystemAccount()) {
                nCount++;
            }
        }
        return nCount;
    }

    void updateSessionsInfo(QStringList list);

    void updateScreensaverMode(QString value);

    void updateScreensaverTheme(QStringList value);

    void updateScreensaverImageTSEffect(int value);

    void updateScreensaverImageSwitchInterval(int value);

    void updateLockTimeout(int value);

    void updateIdleDelay(int value);

    void updateLockEnabled(bool value);

    void updateSleepActivationEnabled(bool value);

    void updateLockDatetimeEnable(bool enable);

    void updateCanSwitchUser(bool canSwitch);

    inline QStringList sessionsInfo()
    {
        return m_listSessions;
    }

    inline void updateCurSession(const QString &strSession)
    {
        m_strCurSession = strSession;
    }

    inline QString currentSession()
    {
        return m_strCurSession;
    }

    UserInfoPtr findUserByName(const QString &strName);

    UserInfoPtr findUserById(const uid_t &id);

    inline QString defaultUserName()
    {
        return m_strDefUserName;
    }

    inline void updateDefUserName(const QString &strUserName)
    {
        m_strDefUserName = strUserName;
    }

    inline QString currentUserName()
    {
        return m_strCurUserName;
    }

    inline void updateCurUserName(const QString &strUserName)
    {
        m_strCurUserName = strUserName;
    }

    void updateAgreementInfo(AgreementInfoPtr agreementInfo);

    void updateSystemFontSize(double fontSize)
    {
        m_curFontSize = fontSize;
    }

    void updateSystemFont(QString font)
    {
        m_curFont = font;
    }

    void updateSessionLogoutMusic(bool logoutMusic)
    {
        m_logoutMusic = logoutMusic;
    }

    void updateSessionPoweroffMusic(bool poweroffMusic)
    {
        m_poweroffMusic = poweroffMusic;
    }

    inline AgreementInfoPtr agreementInfo()
    {
        return m_agreementInfo;
    }

    inline bool getAgreementWindowShowLoginPrompt()
    {
        return m_agreementInfo->showLoginPrompt();
    }
    inline bool getAgreementWindowHideTitle()
    {
        return m_agreementInfo->hideTitle();
    }
    inline QString getAgreementWindowPromptTitle()
    {
        return m_agreementInfo->promptTitle();
    }
    QString getAgreementWindowText();

    inline bool getCapslockState()
    {
        return m_capslockState;
    }

    inline QString getUsdAreaScreenShotKey()
    {
        return m_areaScreenShot;
    }
    inline QString getUsdAreaScreenShot2Key()
    {
        return m_areaScreenShot2;
    }
    inline QString getUsdScreenShotKey()
    {
        return m_screenShot;
    }
    inline QString getUsdScreenShot2Key()
    {
        return m_screenShot2;
    }
    inline QString getUsdwindowScreenshotKey()
    {
        return m_windowScreenshot;
    }

    inline int getUsdMediaRfkillState()
    {
        return m_rfkillState;
    }

    inline QString getPowerButtonKey()
    {
        return m_strButtonPower;
    }

    //    inline QString getSaverMode() { return m_saverMode; }
    //    inline QString getSaverTheme() { return m_saverTheme; }
    ScreenSaver *getScreensaver();

    void updataCheckSystemUpgrade(bool);

    inline QStringList getBatteryArgs()
    {
        return m_batteryArgs;
    }

    inline QString getBatteryIconName()
    {
        return m_batteryIconName;
    }

    inline bool getIsBattery()
    {
        return m_isBattery;
    }

    inline bool getIsLegacy()
    {
        return m_isLegacy;
    }

    inline QVector<InhibitorInfo> getLogoutLockCheck()
    {
        return m_logoutLockcheck;
    }

    inline QVector<InhibitorInfo> getSleepLockcheck()
    {
        return m_sleepLockcheck;
    }

    inline QVector<InhibitorInfo> getShutdownLockcheck()
    {
        return m_shutdownLockcheck;
    }

    inline QVector<WindowInfo::WindowInfo> getWindowList()
    {
        return m_windows;
    }

    inline int getLoggedInUsersCount()
    {
        return m_loggedinUsersCount;
    }

    inline QString getCurFont()
    {
        return m_curFont;
    }

    double getCurFontSize();

    static double getPtToPx();

    inline int getLockTimeout()
    {
        return m_lockTimeout;
    }

    inline int getIdleDelay()
    {
        return m_nIdleDelay;
    }

    inline bool getLockEnabled()
    {
        return m_lockEnabled;
    }

    inline bool getTabletMode()
    {
        return m_tabletMode;
    }

    inline bool getSessionLogoutMusic()
    {
        return m_logoutMusic;
    }

    inline bool getSessionPoweroffMusic()
    {
        return m_poweroffMusic;
    }

    inline bool getSleepActivationEnabled()
    {
        return m_sleepActivationEnabled;
    }

    inline bool isSessionTools()
    {
        return m_isSessionTools;
    }

    inline void setIsSessionTools(bool isTools)
    {
        qDebug() << "isTools:"<<isTools;
        m_isSessionTools = isTools;
    }

    inline bool getLockDatetimeEnable()
    {
        return m_lockDatetimeEnable;
        qDebug() << " m_lockDatetimeEnable = " << m_lockDatetimeEnable;
    }

    inline bool getLockCanSwitchUser()
    {
        return m_canSwitchUser;
    }

    void recordLoginAuthEvent(QString strUserName, int loginResult, QString deviceType);

public Q_SLOTS:
    /**
     * @brief 会话激活状态改变
     *
     * @param isActive 是否激活 true 是，否则不是
     */
    void onSessionActiveChanged(bool isActive);
    /**
     * @brief 进程启动时再次运行传入参数
     *
     * @param strMsg 新参数
     */
    void onRunningMessage(const QString &strMsg);
    /**
     * @brief 用户信息更新
     *
     * @param list 用户信息列表
     */
    void onUsersInfoChanged(QList<UserInfoPtr> list);

    void onCurUserChanged(const QString &strUserName);

    void onCurSessionChanged(const QString &strSession);

    void onLidstateChanged(const QString &lidstate);

    void onBatteryStatusChanged(const QString &iconName);

    void onBatteryChanged(const QStringList &batteryArgs);

    void onCapslockStateChanged(const bool capslockState);

    void onUsdMediaKeysChanged(const QString &key, const QString &value);

    void onUsdMediaStateKeysChanged(const QString &key, const int &value);

    void onPowerManagerConfChanged(const QString &key, QVariant value);

    void onSecondRunParam(const QString &str);

    void onLockScreenConfChanged(const QString &key, QVariant value);

    void onThemeStyleConfChanged(const QString &key, QVariant value);

    void onTabletModeChanged(bool tabletMode);

    void onLockDatetimeEnableChanged(bool enable, QString username);

    void onSessionConfChanged(const QString &key, QVariant value);

Q_SIGNALS:
    /**
     * @brief 请求解锁会话
     *
     */
    void requestUnlockSession();
    /**
     * @brief 请求锁定会话
     *
     */
    void requestLockSession();
    /**
     * @brief 准备休眠/唤醒
     *
     * @param isSleep 休眠还是唤醒 true 休眠，否则唤醒
     */
    void prepareForSleep(bool isSleep);
    /**
     * @brief 会话激活改变
     *
     * @param isActive true 激活，否则未激活
     */
    void sessionActiveChanged(bool isActive);
    /**
     * @brief 用户信息改变
     *
     */
    void usersInfoChanged();

    /**
     * @brief 显示黑色屏保
     *
     * @param nDelay 锁定延迟时间
     * @param isHasLock 是否锁定 true 锁定，否则不
     */
    void showBlankScreensaver(int nDelay, bool isHasLock);
    /**
     * @brief 显示锁定
     *
     * @param isStartup 是否为开机启动 true 是，否则不是
     */
    void showLock(bool isStartup);
    /**
     * @brief 显示SwitchUser
     *
     */
    void showSwitchUserLock();
    /**
     * @brief
     *
     */
    void showSwitchToUser(QString strUserName);
    /**
     * @brief 显示会话空闲状态
     *
     */
    void showSessionIdle();
    /**
     * @brief 显示锁定和屏保
     *
     */
    void showLockScreensaver();
    /**
     * @brief 显示屏保
     *
     */
    void showScreensaver();
    /**
     * @brief 显示电源管理
     *
     */
    void showPowerManager();
    /**
     * @brief 显示session电源管理
     *
     */
    void showSessionTools();
    /**
     * @brief 显示应用阻塞关机界面
     *
     */
    void showAppBlockWindow(int actionType);
    /**
     * @brief 显示关机转场动画
     *
     */
    void showShutdownAnimation(int actionType);
    /**
     * @brief 显示多用户登录阻塞关机界面
     *
     */
    void showMultiUsersBlockWindow(int actionType);
    /**
     * @brief windowChanged 窗口开/闭信号
     */
    void windowChanged(const QString &operation, const QString &winId);
    /**
     * @brief setLockState 设置锁定状态
     */
    void setLockState();
    /**
     * @brief clearLockState 清除锁定状态
     */
    void clearLockState();

    void currentUserChanged(const QString &strUserName);

    void currentSessionChanged(const QString &strSession);

    void agreementInfoChanged();

    void lockStateChanged(bool isVisible, bool isSessionTools, bool isSessionIdle = false);

    void lidstateChanged(const QString &lidstate);

    void batteryStatusChanged(const QString &iconName);

    void batteryChanged(const QStringList &batteryArgs);

    void upowerServiceReady();

    void capslockStateChanged(const bool capslockState);

    void screenSaverConfChanged(const QString &key, const QString &value);

    ///
    void setCurrentUser(QString strUserName);
    int switchToUser(QString strUserName);
    void setCurrentSession(QString strSession);
    void startSession();
    void pamAuthenticate(QString strUserName);
    void pamRespond(QString strRespond);
    void pamAuthenticateCancel();
    bool pamIsInAuthentication();
    bool pamIsAuthenticated();
    QString pamAuthenticateUser();
    QString getAutologinUserHint();
    bool GetBlankState();
    void pamShowMessage(QString text, int type);
    void pamShowPrompt(QString text, int type);
    void pamShowBinaryPrompt(QVariant val, int type);
    void pamAuthCompleted();

    void bioStartAuth(int uid, int nDevId);
    void bioStopAuth();
    QList<DeviceInfo> bioGetAvailableDevices(int nUid);
    QList<int> bioGetDisabledDevices(int nUid);
    int bioGetBioAuthState();
    DeviceInfo bioGetCurBioInfo();
    DeviceInfo bioFindDeviceById(int nUid, int nDevId);
    DeviceInfo bioFindDeviceByName(int nUid, QString strDevName);
    QString bioGetDefaultDevice(int nUid, QString strUserName, int bioType = -1);

    void bioServiceStatusChanged(bool bValid);
    void bioDeviceChanged();
    void bioAuthShowMessage(QString strMsg);
    void bioAuthStateChanged(int nState);
    void bioAuthFrameData(QString strData);
    void bioAuthCompleted(int nUid, bool isSuccess, int nError, int nMaxFailedTime, int nFailedTime);

    void usdMediaKeysChanged(const QString &key, const QString &value);
    void usdMediaStateKeysChanged(const QString &key, const int &value);
    void powerButoonKeyChanged(const QString &value);
    void usdExternalDoAction(QString actionType);

    void setPowerManager(bool isFoece, QString strFuncName, bool isCloseWindow = false, bool alreadyShowMulUsers = false, bool isSessionTools = false);

    void setClosePhase(bool inCall);

    void SecondRunParam(const QString &str);

    QString getPublicEncrypt();
    bool sendPassword(const QString username, QByteArray password);

    void tabletModeChanged(bool tabletMode);

    void lockDatetimeEnableChanged(bool enable, QString username);

    void fontSizeChanged(double fontSize);

    void fontChanged(QString font);

    void setUserThemeColor(QString keyStr, QVariant themeColor);

    void setRfkillState(QString value, int rfkillState);

    /**
     * @brief getCanHibernate 是否支持休眠
     * @return true 支持，false 不支持
     */
    bool getCanHibernate();
    /**
     * @brief getCanSuspend 是否支持睡眠
     * @return true 支持，false 不支持
     */
    bool getCanSuspend();
    /**
     * @brief getCanReboot 是否支持重启
     * @return true 支持，false 不支持
     */
    bool getCanReboot();
    /**
     * @brief getCanPowerOff 是否支持关机
     * @return true 支持，false 不支持
     */
    bool getCanPowerOff();
    /**
     * @brief getCanLockScreen 是否支持锁屏
     * @return true 支持，false 不支持
     */
    bool getCanLockScreen();
    /**
     * @brief getCanSwitchUser 是否支持切换用户
     * @return true 支持，false 不支持
     */
    bool getCanSwitchUser();
    /**
     * @brief getCanLogout 是否支持注销
     * @return true 支持，false 不支持
     */
    bool getCanLogout();
    /**
     * @brief checkSystemUpgrade 系统是否有更新
     * @return true 有，false 无   QString 分钟 预计更新时间
     */
    QPair<bool, QString> checkSystemUpgrade();
    /**
     * @brief checkHideUpdate 需要更新的场景下是否隐藏重启、关机按钮
     * @return true 隐藏，false 显示
     */
    bool checkHideUpdate();
    /**
     * @brief CheckTurnOnScreen 检查并开屏
     */
    void CheckTurnOnScreen();

private:
    QString getXScreensaverPath(const QString &theme);

private:
    bool m_isSessionActive;             /**< 会话是否激活 */
    bool m_isUseWayland = false;        /**< 是否为wayland环境 */
    QList<UserInfoPtr> m_listUsersInfo; /**< 用户信息列表 */
    QString m_strDefUserName;
    QString m_strCurUserName;
    AgreementInfoPtr m_agreementInfo = nullptr;
    bool m_capslockState;
    QStringList m_listSessions; /**< 会话列表*/
    QString m_strCurSession;

    QString m_areaScreenShot;
    QString m_areaScreenShot2;
    QString m_screenShot;
    QString m_screenShot2;
    QString m_windowScreenshot;

    int m_rfkillState = -1;

    QString m_strButtonPower;

    QString m_saverMode;
    QList<QString> m_saverTheme;
    int m_imageTSEffect;
    int m_imageSwitchInterval;

    QStringList m_batteryArgs;
    QString m_batteryIconName;
    bool m_isBattery = false;

    QVector<InhibitorInfo> m_logoutLockcheck;
    QVector<InhibitorInfo> m_sleepLockcheck;
    QVector<InhibitorInfo> m_shutdownLockcheck;
    int m_loggedinUsersCount = 0;
    int m_lockTimeout = 10;
    int m_nIdleDelay = -1;
    bool m_lockEnabled = true;
    double m_curFontSize;
    QString m_curFont;
    bool m_sleepActivationEnabled = false;

    bool m_tabletMode = false;

    bool m_logoutMusic = false;
    bool m_poweroffMusic = false;
    bool m_isSessionTools = false;

    bool m_lockDatetimeEnable = false;

    bool m_canSwitchUser = true;

    QVector<WindowInfo::WindowInfo> m_windows;
    bool m_isLegacy = false;
};

#endif // LOCKDIALOGMODEL_H
