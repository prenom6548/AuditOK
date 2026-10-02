/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include "mylistwidget.h"
#include <QWidget>
#include "userinfo.h"
#include "../dbusifs/biometrichelper.h"
#include "definetypes.h"
#include "biodefines.h"
#include "loginoptionswidget.h"
#include "pluginsloader.h"
#include <QDateTime>

class QLabel;
class CustomLabel;
class QPushButton;
class IconEdit;
class LockDialogModel;
class BioButtonListWidget;
class StatusButton;
class QTimer;
class SecurityQuestionAnswer;

extern float scale;
class AuthDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AuthDialog(LockDialogModel *model, UserInfoPtr userInfo, QWidget *parent = nullptr);

    void initUI();
    void switchLoginOptType(unsigned uLoginOptType, bool faceBtnClicked = false);

public:
    void startAuth();
    void stopAuth();
    void onCurUserChanged(UserInfoPtr userInfo);
    void onCurUserInfoChanged(UserInfoPtr userInfo);

    void updateUI();
    void updateAuthSize();
    void setLoginTypeTip(QString strLoginTypeTip);
    void setUkeyTypeTip(QString text);
    void setDirLogin();
    void startLoadingUkey();
    void stopLoadingUkey();
    void updateLoadingPixmap();
    void setWillLoginUser(QString strUserName, bool isOneKeyLogin);
    /**
     * @brief setCustomAuthDefault 设置是否默认第三方认证
     * @param isDefault true 是，否则 否
     */
    void setCustomAuthDefault(bool isDefault);

    void updateAuthFont(QString fontFamily);

    void updateAuthFontSize(double fontSize);

    void inhibitShow(bool isInhibit);

    QString getCurAuthRealName();
    qint32 getCurAuthUserUid();
    QPixmap getCurAuthUserFace();
    void clearMessage();
    int getUserAuthFailedTimes();

private Q_SLOTS:
    void onPamShowMessage(QString strMsg, int nType);
    void onPamShowPrompt(QString strPrompt, int nType);
    void onPamAuthCompleted();
    void onRespond(const QString &strRes);
    void onMessageButtonClicked();

    void onLoginOptsCount(unsigned uCount);
    void onLoginOptImage(QImage img);
    void setLoginMsg(QString strLoginMsg);
    void onFirstLoginIdentifyComplete(QDBusPendingCallWatcher *watcher);

    /**
     * @brief onCustomRequestAccount 第三方请求认证用户槽
     * @param strName 用户名
     */
    void onCustomRequestAccount(QString strName);
    /**
     * @brief onCustomAuthResult 第三方认证结果
     * @param nResult 结果值，0 成功，其他失败
     * @param strMsg 结果消息
     */
    void onCustomAuthResult(int nResult, QString strMsg);
    /**
     * @brief onCustomRequest 第三方请求响应槽
     * @param strReqJson 请求json
     * @return 请求结果
     */
    QString onCustomRequest(QString strReqJson);
    /**
     * @brief onCustomPlugEnv 插件所处环境
     * @return 登录或锁屏等
     */
    int onCustomPlugEnv();

private Q_SLOTS:
    void onBiometricButtonClicked();
    void onDeviceChanged(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo);
    void onBiometricAuthComplete(bool result, int nStatus);
    void onPrepareForSleep(bool sleep);
    void setQRCode(QImage &imgQRCode);
    void setFaceImg(QImage &imgFace, int nStatus = 0);
    void setQRCodeMsg(QString strMsg);
    void onLoadingImage();
    void onBiometricDbusChanged(bool bActive);
    void onFRetryButtonClicked();
    void pamBioSuccess();
    void onRespondUkey(const QString &text);

Q_SIGNALS:
    void authSucceed(QString strUserName);
    /**
     * @brief 手动输入用户名
     * @param userName 用户名
     */
    void userChangedByManual(const QString &userName);
    /**
     * @brief customRequestAccount 第三方请求认证用户信号
     * @param account 用户名
     */
    void customRequestAccount(QString account);

    void lineEditClicked();

    void bottomPositionChanged(int nPosY);

    void showResetPasswdBtn();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void initTipWidget();
    void initHeaderWidget();
    void initNameWidget();
    void initEditWidget();
    void initMessagerWidget();
    void initUkeyPasswordWidget();
    void initLoginoptionWidget();
    void initCustomAuthWidget();

    void showAuthenticated(bool successful = true);
    void initConnections();
    bool unlockUserTimer();

    void initBiometricWidget();
    void setBiometricWidgetGeometry();
    void performBiometricAuth();
    void skipBiometricAuth();

    void startBioAuth(unsigned uTimeout = 1000);
    void restartBioAuth();
    void waitBiometricServiceStatus();
    QString getDeviceType_tr(int deviceType);
    QString getDeviceType_lowerTr(int deviceType);

    void updatePixmap();

    //一键开机认证函数
    void startFirstLogin();

    void startBioWaiting(unsigned uCurLoginOptType);
    void stopBioWaiting(bool isDisable = false);

    void showPwdTipBtn();

private:
    LockDialogModel *m_modelLockDialog = nullptr;

    QWidget *m_tipWidget = nullptr;
    QWidget *m_headerWidget = nullptr;
    QWidget *m_nameWidget = nullptr;
    QLabel *m_labelHeadImg = nullptr; //头像
    CustomLabel *m_nameLabel = nullptr;    //用户名

    QWidget *m_editWidget = nullptr;
    IconEdit *m_passwordEdit = nullptr;     //密码输入框
    StatusButton *m_pPwdTipMessage = nullptr;
    QHBoxLayout *m_pEditLayout;
    CustomLabel *m_messageLabel = nullptr;       // PAM消息显示
    QPushButton *m_messageButton = nullptr; //免密登录按钮

    QWidget *m_ukeyPasswdWidget = nullptr;  //放置密码输入框和信息列表
    IconEdit *m_ukeyPasswordEdit = nullptr; // ukey密码输入框
    CustomLabel *m_ukeyMessageLabel = nullptr;   // PAM消息显示

    QWidget *m_ukeyLoadingWidget = nullptr; // ukey loading窗口
    QPushButton *m_loadingButton = nullptr;
    CustomLabel *m_ukeyLoadingText = nullptr;
    QTimer *m_loadingTimer = nullptr;
    QPixmap m_ukeyLoadingPixmap;

    QLabel *m_labelFace = nullptr;
    CustomLabel *m_labelLoginTypeTip = nullptr; // 登录类型提示
    QLabel *m_labelQRCode = nullptr;       // 二维码图标
    CustomLabel *m_labelQRCodeMsg = nullptr;    // 二维码状态消息提示
    QLabel *m_labelQRCodeTip = nullptr;
    QPushButton *m_fRetryButton = nullptr; //人脸识别重试按钮
    // 指纹、指静脉、虹膜、声纹识别动图
    QLabel *m_labelBioWaiting = nullptr; // 指纹、指静脉、虹膜、声纹识别动图提示
    QTimer *m_timerBioWaiting = nullptr; // 指纹、指静脉、虹膜、声纹识别动图切换定时器
    int m_nBioWaitingFrame = 0;             // 当前显示的动图帧
    int m_nCurBioWaitingType = -1;          // 当前显示的动图类型
    QWidget *m_widgetPlaceholder = nullptr; // 占位控件

    QWidget *m_widgetLoginOpts = nullptr; // 登录选项
    CustomLabel *m_optionTip = nullptr;

    BioButtonListWidget *bottomListWidget = nullptr;

    UserInfoPtr m_curUserInfo;
    bool m_bRecvPrompt = false;
    bool m_bHasUnacknowledgedMsg = false;
    bool m_bDirectLogin = false;
    bool m_bNeedRetry = false;
    QString m_preStrMessage = "";
    int m_preStrMessageType = -1;
    bool m_isNameLogin = false;
    bool m_isAutoSwitch = false;
    bool m_isPassWdInput = false;
    //手动输入用户标记，设置该标记的原因是判断是不是手动输入用户，
    //如果输入了无法登录的用户，就会一直输出密码错误信息
    bool m_isManual = false;
    QString m_strManualLoginName;
    bool m_isInputPasswd;

    enum AuthMode
    {
        PASSWORD,
        BIOMETRIC,
        UNKNOWN
    };

    AuthMode authMode = UNKNOWN;
    // 生物识别认证
    QString m_deviceName;
    DeviceInfoPtr m_deviceInfo = nullptr;
    BiometricHelper *m_biometricProxy = nullptr;
    LoginOptionsWidget *m_loginOpts = nullptr;

    QWidget *m_customWidget;     /** 第三方认证界面窗口 */
    QHBoxLayout *m_layoutCustom; /** 第三方认证界面主布局 */

    bool isBioSuccess = false; // 生物认证是否成功
    bool manualStopBio = false;

    bool m_isCustomDefault = false; /** 是否默认使用第三方认证 */
    QMap<qint32, QMap<int, int>> m_failMap;
    int maxFailedTimes;
    QString m_face;
    QTimer *m_bioTimer = nullptr;
    QImage m_imgQRCode;
    unsigned m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD; // 当前登录验证方式
    QString m_strLoginTypeTip = "";
    int m_nLastDeviceId = -1;
    QString m_strDefaultFace;
    int m_fTimeoutTimes = 0;

    QPixmap m_waitingPixmap;
    QTimer *w_timer = nullptr;
    bool isLoadingUkey = false;
    bool isPowerup = false;
    QDBusInterface *bioService = nullptr;
    int drvid = -1;
    bool isFirstAuth = false;
    QString m_lastUserName = "";

    // 将登录认证的用户名
    QString m_strServiceUser = "";
    // 是否为一键登录
    bool m_isOneKeyLogin = false;

    PluginsLoader *m_pluginsLoader = nullptr; /** 插件加载器 */

    bool m_isLockingFlg; //判断当前是否正在锁定倒计时
    int m_nCurLockMin;   //当前锁定的分钟数
    QTimer *m_timerUserUnlock = nullptr;

    double m_curFontSize;
    double m_ptToPx = 1.0;

    QString m_strLastPamAuthName = "";
    QDateTime m_lastPamAuthTime;
    bool m_isInhibitShow = false;

    SecurityQuestionAnswer *m_pSecurityQuestionAnswer = nullptr;
    QMap<qint32, int> m_userFailedTimesMap;
};
#endif // AUTHDIALOG_H
