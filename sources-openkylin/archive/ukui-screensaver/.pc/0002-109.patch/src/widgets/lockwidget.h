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
#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#include <QWidget>
#include <QObject>
#include <QFuture>
#include "userinfo.h"
#include "mediacontrolwidget.h"

class QListWidgetItem;
class MyListWidget;
class QPushButton;
class QPixmap;
class LockDialogModel;
class UserListWidget;
class SessionListWidget;
class AuthDialog;
class MyNetworkWidget;
class PowerListWidget;
class TimeWidget;
class BatteryWidget;
class VirtualKeyboardWidget;
class BlockWidget;
class StatusButton;
class NetWorkButton;

enum
{
    rootWinPicture,
    rootWinColor
};
class LockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LockWidget(LockDialogModel *model, QWidget *parent = nullptr);
    bool exitSubWidget(bool isForScreensaver = false, bool hideVirkeyboard = true);
    void stopAuth();
    void startAuth();
    void reloadRootBackground();
    void drawRootBackground();
    void updateFont();
    void updateFontSize();
    void onShowPowerListWidget(bool issessionTools = false);

    void onShowInhibitWarning(QStringList list, int type, bool iscommand = false);

    void onMulUsersLogined(int inhibitType, bool iscommand = false);

    void onSureShutDown(int inhibitType, bool iscommand = false);

    void onLockScreenClicked();

    void onSuspendClicked();

    void onSwitchuserClicked();

    void onShowUserListWidget(bool show = false);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyReleaseEvent(QKeyEvent *e);
    void changeEvent(QEvent *event);

private:
    void initUI();
    void initConnections();
    void initUserWidget();
    void initSessionWidget();
    void initPowerWidget();
    void initButtonWidget();
    void initTimeWidget();
    void initBlockWidget();
    void initSystemMonitorBtn();
    void sendNetPluginVisible(bool visible);
    void initMediaControlWidget();

    /**
     * @brief 获取usd快捷键键值：QString
     *
     */
    void initUsdMediaKeys();
    /**
     * @brief 快捷键转化：QString - QKeySequence
     * @param shortcuts   快捷键键值
     * @return QKeySequence
     */
    static const QKeySequence listFromString(QString shortcuts);

    void initUsdMediaStateKeys();

    void updateBottomButton();

    void SwitchToUser(QString strUserName);

    void refreshTranslate();

    void setUserListWidgetLocate();

    void setNetWorkWidgetLocate();

    void setSessionListWidgetLocate();

    void setBatteryWidgetLocate();

    void setRfkillState();

    void setRfkillBtnIcon();

private Q_SLOTS:
    /**
     * @brief item选中状态改变
     * @param current 前一个选中item
     *        previous 当前选中item
     * @return
     */
    void onPowerItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onButtonItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    /**
     * @brief item点击响应
     * @param item
     * @return
     */
    void onButtonItemClicked(QListWidgetItem *item);
    void onPowerItemClicked(QListWidgetItem *item);

    void onUsersInfoChanged();
    void onCurUserChanged(const QString &strUserName);
    void onSessionChanged(const QString &strSession);
    void onUserChangedByManual(const QString &userName);

    void onUsdMediaKeysChanged(const QString &keys, const QString &value);
    void onUsdMediaStateKeysChanged(const QString &keys, const int &value);

    void onShowSessionListWidget();
    void onShowBatteryWidget();
    void onShowNetworkWidget();
    void onShowVirtualKeyboard();
    void onShowVirtualKeyboard(bool tabletMode);
    void onLineEditClicked();
    void doSystemMonitor();

    void onNetWorkResetLocation();
    void onSetVirkeyboardPos();
    void onHideVirkeyboard();

    void onConfirmBtnClicked();
    void onCancelBtnClicked();
    /**
     * @brief onCustomRequestAccount 请求切换认证用户
     * @param account 用户名
     */
    void onCustomRequestAccount(QString account);
    /**
     * @brief onGetCustomPluginMsg 获取插件消息槽
     * @param strMsg 消息json
     */
    void onGetCustomPluginMsg(QString strMsg);

    void onSessionActiveChanged(bool isActive);

    void onLanguageChanged(bool isCompleted);

    void onTabletModeChanged(bool tabletMode);

    void onFontChanged(QString font);

    void onFontSizeChanged(double fontSize);

    void onAuthDialogBottomPosChanged(int nPosY);

Q_SIGNALS:
    void authSucceed(QString strUserName);

    void sessionToolsExit();

    void showBlankScreensaver(int nDelay, bool isHasLock);

private:
    void setrootWindowBackground(int type, unsigned int color, QString filename);

    void setRootWindowBgOptions(int nOption);

    bool getdrawBackgroundIsStarted();

    void setrootWindow(QList<QPair<QRect, QRect>> screenRectList);

private:
    //  日期时间
    TimeWidget *m_timeWidget = nullptr;
    //  用户列表
    UserListWidget *m_userListWidget = nullptr;
    // 会话列表
    SessionListWidget *m_sessionListWidget = nullptr;
    //  电源管理列表
    PowerListWidget *m_powerListWidget = nullptr;
    //  底部button列表
    MyListWidget *buttonListWidget = nullptr;
    //  底部button布局
    QWidget *m_pBottomWidget = nullptr;

    QPixmap background;

    LockDialogModel *m_modelLockDialog = nullptr;

    UserInfoPtr m_curUserInfo;

    AuthDialog *authDialog = nullptr;
    QTimer *m_timerChkActive = nullptr;

    // usd快捷键键值
    QString m_areaScreenShot;
    QString m_areaScreenShot2;
    QString m_screenShot;
    QString m_screenShot2;
    QString m_windowScreenshot;

    int m_rfkillState = -1;

    MyNetworkWidget *m_networkWidget = nullptr;
    BatteryWidget *batteryWidget = nullptr;
    VirtualKeyboardWidget *m_virtualKeyboardWidget = nullptr;

    // 当前是否处于阻塞状态
    bool m_isInhibitStatus = false;
    BlockWidget *m_blockWidget = nullptr;
    QString m_inhibitType;

    // 底部button
    StatusButton *m_sessionButton = nullptr;
    QListWidgetItem *m_btnItemSession = nullptr;
    StatusButton *m_batteryonButton = nullptr;
    StatusButton *m_userButton = nullptr;
    QListWidgetItem *m_btnItemUser = nullptr;
    NetWorkButton *m_networkButton = nullptr;
    StatusButton *m_virKbButton = nullptr;
    StatusButton *m_powerManagerButton = nullptr;
    StatusButton *m_pRfkillStatusButton = nullptr;
    QWidget *m_widgetMediaControl = nullptr;
    MediaControlWidget *m_mediaControlWidget = nullptr;

    // 底部系统监视器按钮
    StatusButton *m_systemMonitorBtn = nullptr;

    bool m_isCustomDefault = false; /** 是否默认使用第三方认证 */
    bool m_isShowNetwork = true;    /** 是否显示网络插件 */
    bool m_isShowUserSwitch = true; /** 是否显示用户切换 */

    QString m_rootWindowBackground;
    int m_nPicOptions = 0;
    int drawBackgroundType = 0;
    unsigned int drawBackgroundColor = 0x0;
    bool drawBackgroundIsStarted = false;
    QFuture<void> m_futureDrawBg;
    QList<QPair<QRect, QRect>> m_screenRectList;

    bool m_isTabletMode = false;
    QString m_curFont;
    double m_curFontSize;
    double m_ptToPx = 1.0;

    int m_powerBtnNum = 0;

    bool m_isSessionTools = false;
    bool m_iscommand = false;

    QString systemLang;
};

#endif // LOCKWIDGET_H
