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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include "biometric.h"
#include "users.h"
#include "pam-tally.h"
#include <QPushButton>
#include <QLineEdit>
#include <qgsettings.h>
#include "bioauthwidget.h"
#include "loginoptionswidget.h"
#include "modeButton.h"
#include "kalabel.h"
#include <kysdk/applications/kdialog.h>
#include <ukuiwindowhelper.h>

using namespace kdk;

namespace Ui {
class MainWindow;
}

class MainWindow : public kdk::KDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event); //屏蔽esc按钮
    enum Mode
    {
        UNDEFINED,
        PASSWORD,
        BIOMETRIC,
        DEVICES
    };
    enum situation
    {
        TRUE,
        ERROR
    };
    void setIcon(const QString &iconName);
    void setHeader(const QString &text);
    void setUsers(const QStringList &usersList);
    void setDetails(
        const QString &subPid,
        const QString &callerPid,
        const QString &actionId,
        const QString &actionDesc,
        const QString vendorName,
        const QString vendorUrl);
    void setPrompt(const QString &text, bool echo);
    void setMessage(const QString &text, situation situat = ERROR);
    void setAuthResult(bool result, const QString &text = "");
    void clearEdit();
    void switchAuthMode(Mode mode);
    void setDoubleAuth(bool val);
    void stopDoubleAuth();
    QString check_is_pam_message(QString text);
    void switchLoginOptType(unsigned uLoginOptType);
    void setEditInputMethod(bool bEnable);
    void setCurProject(bool isMavis);
    void setUkeyTypeTip(QString text);
    void startLoadingUkey();
    void stopLoadingUkey();
    void updateLoadingPixmap();
    void updateAuthMethod();
    void resetSendPassword();
    bool checkIsAdmin(QString userName);

private:
    uid_t getUid(const QString &userName);
    void setMovie();
    void setPixmap();
    void startBioAuth(unsigned uTimeout = 200);
    void switchWidget(Mode mode);
    int enable_biometric_authentication();

    void root_unlock_countdown();
    void unlock_countdown();
    void editIcon();
    void setLoginTypeTip(QString strLoginTypeTip);
    void setMavisSheel();
    void updatePixmap();

    void setBiometricAuthDisabledStatus(bool locked);


public slots:
    void onUpdateBioAuthMsg(QString strMsg);
    void onUpdateWndSize(unsigned uLoginOptType, unsigned uLoginOptSize);
    void onShowYesPrompt();

private slots:
    /**
     * @brief onOptionSelected
     * @param uCurLoginOptType
     * @param deviceInfo
     */
    void onOptionSelected(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo);

    /**
     * @brief onNotifyOptionsChanged
     * @param uOptionsCount
     */
    void onNotifyOptionsChanged(unsigned uOptionsCount);

    /**
     * @brief onLoginOptionsAuthCompleted
     * @param uid
     * @param ret
     * @param nStatus
     */
    void onLoginOptionsAuthCompleted(uid_t uid, bool ret, int nStatus);

    void on_btnDetails_clicked();
    void on_lePassword_returnPressed();
    void on_ukeyPassword_returnPressed();
    void on_btnBioAuth_clicked();
    void on_returnButton_clicked();
    void on_cmbUsers_currentTextChanged(const QString &userName);
    void on_btnCancel_clicked();
    void on_btnAuth_clicked();
    void restart_bio_identify();
    void startBioAuthDelay();
    void onConfigurationChanged(QString key);
    void onLockStatus();
    void onUnlockStatus();
    void onRespondUkey(const QString &text);

signals:
    void accept(const QString &text);
    void canceled();
    void switchToPassword();
    void switchToBiometric();
    void userChanged(const QString &userName);
    void restartAuth();
    void yesPrompted();

private:
    Ui::MainWindow *ui;
    Users *users;
    QString userName;
    bool enableBioAuth;
    bool receiveBioPAM;
    BioDevices bioDevices;
    Mode authMode;
    bool isFirst;
    bool useDoubleAuth;
    bool isbioSuccess;
    QMap<qint32, QMap<int, int>> m_failMap;
    int maxFailedTimes;
    bool isHiddenSwitchButton;
    ModeButton *m_modeButton;
    QIcon modeIcon;
    bool pwdShow = false;
    bool isHover = false;

    // ukey优先级比密码低，该变量用于判断是否第一次认证，并选中密码
    bool isFirstAuth = true;

    QTimer *m_timer;
    // const DeviceInfo *device = NULL;
    bool isLockingFlg; //判断当前是否正在锁定倒计时
    int m_nCurLockMin; //当前锁定的分钟数
    // 登录选项
    KALabel *m_labelTip = nullptr;
    LoginOptionsWidget *m_loginOptsWidget = nullptr;
    unsigned m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD; // 当前登录验证方式
    QString m_strLoginTypeTip = "";
    QTimer *m_bioTimer = nullptr;
    DeviceInfoPtr m_deviceInfo = nullptr;
    bool m_isNetworkErr = false;
    //标题栏
    QHBoxLayout *headerLayout;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QPushButton *closeBtn;

    QGSettings *settings;
    int fontSize;

    QString app_IconName;
    int m_nLastDeviceId = -1;
    bool no_changes = false;

    bool is_Mavis = false;

    QTimer *w_timer;
    QPixmap m_waitingPixmap;

    QTimer *m_loadingTimer = nullptr;
    QPixmap m_loadingPixmap;
    bool isLoadingUkey = false;

    bool doNotNeedAuth = false;

    bool hasSendPassword = false;
    QString authMethod = "";
    
    UkuiWindowHelper *m_windowHelper = nullptr;
    bool hasReviceBiometricPam = false;
};

#endif // MAINWINDOW_H
