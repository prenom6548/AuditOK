/* loginwindow.h
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QMap>
#include <QLightDM/Greeter>
#include <QLightDM/UsersModel>

#include "biometricdeviceinfo.h"
#include "pam-tally.h"
#include "loginoptionswidget.h"
#include "pluginsloader.h"

class QTimer;
class QLabel;
class KLabel;
class QPushButton;
class QListWidget;
class Configuration;

class GreeterWrapper;
class IconEdit;
class BiometricProxy;

enum FOCUS {
    REMOVE = 0,
    IN_LIGIN,
    BIO_RIGHT,
    BIO_LEFT,
    IN_LINEEDIT,
    ON_MESSAGEBTN,
};

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(GreeterWrapper *greeter, QWidget *parent = 0);
    ~LoginWindow(){}

    bool setUserIndex(const QModelIndex& index);
    void setGreeter(GreeterWrapper *greeter);
    void setUserName(const QString& userName);
    QString getUserName();
    QString getRealName();
    void setFace(const QString& faceFile);
    void setLoggedIn(bool isLoggedIn);
    void setLoginTypeTip(QString strLoginTypeTip);
    void setUkeyTypeTip(QString text);
    void setPrompt(const QString& text);
    void setChildrenGeometry();
    QString getPassword();
    bool checkAuthOK(QString strUserName);
    void reset();
    void setFocusin(int target);
    void setClick();
    void checkPassword();
    bool isloginauth;
    void setDirLogin();
    bool getLineeditStatus();
    int getBioNum();
    bool isinput_passwd;
    void startLoadingUkey();
    void stopLoadingUkey();
    void updateLoadingPixmap();
    void setWillLoginUser(QString strUserName, bool isOneKeyLogin);
    /**
     * @brief setCustomAuthDefault 设置是否默认第三方认证
     * @param isDefault true 是，否则 否
     */
    void setCustomAuthDefault(bool isDefault);

protected:
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void keyReleaseEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
signals:
    /**
     * @brief 手动输入用户名
     * @param userName 用户名
     */
    void userChangedByManual(const QString &userName);
    void clickPassword(bool clicked);
    void setKToolTip(const QString text);
    void hideKTooltip();
    void loginOptionClicked();
    void showMessageBtn(bool is_show);
    /**
     * @brief customRequestAccount 第三方请求认证用户信号
     * @param account 用户名
     */
    void customRequestAccount(QString account);
public slots:
    void onShowMessage(QString text, QLightDM::Greeter::MessageType type);
    void onShowPrompt(QString text, QLightDM::Greeter::PromptType type);
    void onAuthenticationComplete();
    void setUserNotInView(const QString &userName);
    void pamBioSuccess();
    void switchLoginOptType(unsigned uLoginOptType);
    void onLoginOptsCount(unsigned uCount, bool is_bioBtn);
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
        
private slots:
    void onLogin(const QString &str);
    void onRespondUkey(const QString &text);
    void onBiometricButtonClicked();
    void onPasswordButtonClicked();
    void onOtherDevicesButtonClicked();
    void onRetryButtonClicked();
    void onDeviceChanged(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo, bool keyNavigation);
    void onBiometricAuthComplete(bool result, int nStatus);
    void onMessageButtonClicked();
    void onPrepareForSleep(bool sleep);
    void setQRCode(QImage& imgQRCode);
    void setFaceImg(QImage& imgFace, int nStatus = 0);
    void onM_passwordEditClicked();
    void setQRCodeMsg(QString strMsg);
    void showTooltip(QString text);
    void hideTooltip();
    void onLoadingImage();
    void onBiometricDbusChanged(bool bActive);

private:
    void initUI();
    void initBiometricWidget();
    void setBiometricWidgetGeometry();
    void startAuthentication();
    void startWaiting();
    void stopWaiting();
    void clearMessage();
    void performBiometricAuth();
    void skipBiometricAuth();
    QPixmap DrawRound(QPixmap &src, int radius);
    void show_authenticated (bool successful = true);
    void refreshTranslate();
    void startBioAuth(unsigned uTimeout = 1000);
    void restartBioAuth();
    void waitBiometricServiceStatus();

    void root_unlock_countdown();
    bool unlock_countdown();
    void updatePixmap();
    void changeEvent(QEvent *event);

    //一键开机认证函数
    void startFirstLogin();

private:
    GreeterWrapper      *m_greeter;
    QString     m_name;     //m_nameLabel显示的是全名(显示的),m_name保存的是真名(用于登录的)
    QString     m_strRealName;  /** 当前用户展示名 */
    QString     toolStr = nullptr;     //toolTip显示的全名
    QString     msgToolStr = nullptr;
    qint32      m_uid;      //用户id
    //手动输入用户标记，设置该标记的原因是判断是不是手动输入用户，
    //如果输入了无法登录的用户，就会一直输出密码错误信息
    bool        isManual = false;
    //密码错误标记，设置该标志的原因是，在有生物识别模块的情况下用户选择了密码登录，输入了错误的密码，
    //此时应该直接进入密码登录，而不是再次进入登录生物识别设备选择界面
//    bool        isPasswordError;

    enum AuthMode { PASSWORD, BIOMETRIC, UNKNOWN };

    AuthMode authMode = UNKNOWN;
    // 生物识别认证
    QString                 m_deviceName;
    DeviceInfoPtr           m_deviceInfo = nullptr;
    BiometricProxy          *m_biometricProxy = nullptr;
    QWidget                 *m_buttonsWidget = nullptr;
    QPushButton             *m_biometricButton = nullptr;
    QPushButton             *m_passwordButton = nullptr;
    QPushButton             *m_otherDeviceButton = nullptr;
    QPushButton             *m_retryButton = nullptr;
    Configuration           *m_configuration;
    double                  curFontSize;

    LoginOptionsWidget      *m_widgetLoginOpts = nullptr;

    // UI
    QWidget         *m_userWidget = nullptr;          //放置用户信息Label
    QLabel          *m_labelHeadImg = nullptr;         //头像
    KLabel          *m_nameLabel = nullptr;         //用户名
    QLabel          *m_isLoginLabel = nullptr;      //提示是否已登录

    QWidget         *m_passwdWidget;        //放置密码输入框和信息列表
    IconEdit        *m_passwordEdit;       //密码输入框
    KLabel          *m_messageLabel;         //PAM消息显示
    QString         m_messageLabelTip = "";
    QPushButton     *m_messageButton;
    bool            is_showMessageBtn = false;
    QTimer          *m_bioTimer = nullptr;

    QWidget         *m_ukeyPasswdWidget = nullptr;        //放置密码输入框和信息列表
    IconEdit        *m_ukeyPasswordEdit = nullptr;       //密码输入框
    KLabel          *m_ukeyMessageLabel = nullptr;         //PAM消息显示

    QWidget         *m_loadingWidget = nullptr;
    QPushButton     *m_loadingButton = nullptr;
    KLabel          *m_loadingText = nullptr;
    QTimer          *m_loadingTimer = nullptr;
    QPixmap         m_loadingPixmap;

    QWidget         *m_customWidget;        /** 第三方认证界面窗口 */
    QHBoxLayout     *m_layoutCustom;        /** 第三方认证界面主布局 */

    QLabel          *m_labelFace = nullptr;
    KLabel          *m_labelLoginTypeTip = nullptr;    // 登录类型提示
    QLabel          *m_labelQRCode = nullptr;   // 二维码图标
    KLabel          *m_labelQRCodeMsg = nullptr; // 二维码状态消息提示
    KLabel          *m_labelQRCodeTip = nullptr;

    bool isretry = true;
    bool prompted = false;
    bool unacknowledged_messages = false;
    bool m_name_is_login = false;
    bool isauto_switch = false;
    bool direct_login = false;
    bool isChooseDev = false;
    bool useDoubleAuth = false;
    bool isBioSuccess = false;              // 生物认证是否成功
    bool manualStopBio = false;
    bool m_isPassWdInput = false;
    bool m_isCustomDefault = false;         /** 是否默认使用第三方认证 */

    QMap<qint32,QMap<int,int>>    m_failMap;
    int                 maxFailedTimes;
    bool                isHiddenSwitchButton;
    QString             manualLoginName;
    QString             m_face;
	QString             m_preStrMessage;
    int                 m_preStrMessageType;

    bool            isLockingFlg;   //判断当前是否正在锁定倒计时
    int             m_nCurLockMin;  //当前锁定的分钟数
    QTimer *m_timer;

    QImage m_imgQRCode;
    unsigned m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD;   // 当前登录验证方式
    QString m_strLoginTypeTip = "";
    int m_nLastDeviceId = -1;
    QString m_strDefaultFace;

    QPixmap         m_waitingPixmap;
    QTimer          *w_timer;
    UniAuthService  *m_uniauthService = nullptr;
    bool           isLoadingUkey = false;
    bool isPowerup = false;
    QDBusInterface  * bioService=nullptr;
    int drvid = -1;
    bool isFirstAuth = false;
    QString m_lastUserName = "";

    // 将登录认证的用户名
    QString m_strServiceUser = "";
    // 是否为一键登录
    bool m_isOneKeyLogin = false;

    PluginsLoader *m_pluginsLoader = nullptr;       /** 插件加载器 */
};

#endif // LOGINWINDOW_H
