/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef USERINFO_H
#define USERINFO_H

#include <QFrame>
#include <QDBusObjectPath>
#include <QObject>
#include <QtPlugin>
#include <QFileSystemWatcher>
#include <QMouseEvent>
#include <QSettings>
#include <QPainterPath>
#include <QButtonGroup>

#include "changeusertype.h"
#include "changeusernickname.h"
#include "changeuserpwd.h"
#include "changeuserlogo.h"
#include "createusernew.h"
#include "deleteuserexists.h"
#include "utilsforuserinfo.h"
#include "changeusergroup.h"
#include "mulverifydialog.h"
#include "messagebar.h"
#include "qtdbus/systemdbusdispatcher.h"
#include "qtdbus/uniauthservice.h"
#include "qtdbus/pwdlessservice.h"

#include "shell/interface.h"
#include "ukcccommon.h"
using namespace ukcc;
#include "switchwidget.h"
#include "settinggroup.h"
#include "titlelabel.h"
#include "addbtn.h"
#include "addbutton.h"
#include "comboxwidget.h"

#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#ifdef ENABLEPQ
extern "C" {
#include <pwquality.h>
}
#endif

enum {
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};

typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString realname;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
}UserInfomation;

typedef struct _PwdQualityOption {
    int diff_ok;
    int min_length;
    int dig_credit;
    int up_credit;
    int low_credit;
    int oth_credit;
    int min_class;
    int max_repeat;
    int max_class_repeat;
    int max_sequence;
    int gecos_check;
    int dict_check;
    int user_check;
    int enforcing;
    int retry_times;
    int enforce_for_root;
    int local_users_only;
    int palindrome;
    int no_similar_check;
    char *bad_words;
    char *dict_path;

}PwdQualityOption;

class UkccArgEvent : public QEvent
{
private:
    QString mstr;

public:
    const static Type TYPE = static_cast<Type>(QEvent::User+0x10);
    explicit UkccArgEvent(QString s) : QEvent(TYPE)
    {
        mstr = s;
    }

    QString str()
    {
        return mstr;
    }
};

class QDBusInterface;

class UserInfo : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    UserInfo();
    ~UserInfo();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    QWidget *m_pPluginWidget = nullptr;

    QVBoxLayout *m_pMainVerLayout = nullptr;

    KLabel *m_pOthersLabel = nullptr;
    KLabel *m_pCurrentNickNameLabel = nullptr;
    KPushButton *m_pChangeNickNameBtn = nullptr;
    KLabel *m_pCurrentUserTypeLabel = nullptr;
    KLabel *m_pCurrentUserlogoBtn = nullptr;
    KPushButton *m_pChangeCurrentPwdBtn = nullptr;
    KPushButton *m_pChangeCurrentGroupsBtn = nullptr;
    SettingGroup *m_pCurrentFrame = nullptr;
    UkccFrame *m_pCurrentUserFrame = nullptr;
    SwitchWidget *m_pNopwdLoginFrame = nullptr;
    ComboxWidget *m_pAutoLoginFrame = nullptr;
    UkccFrame *m_pUserGroup = nullptr;

    SettingGroup *m_pOthersFrame = nullptr;
    UkccFrame *m_pAddUserFrame = nullptr;
    AddButton *m_pAddUserBtn = nullptr;

    KLabel *m_pMoreSettingsTitleLabel = nullptr;
    SettingGroup *m_pMultipleFactorGroup = nullptr;
    SwitchWidget *m_pMultipleFactorFrame = nullptr;
    UkccFrame *m_pOtpDynamicPasswordFrame = nullptr;
    UkccFrame *m_pSecurityKeyFrame = nullptr;
    QCheckBox *m_pAllowSShButton = nullptr;
    UkccFrame *m_pAllowSSHFrame = nullptr;
    KLabel *m_pIsBindOTPLabel = nullptr;
    KLabel *m_pIsBindSecurityLabel = nullptr;
    QButtonGroup *m_pFactoyButtonGroup = nullptr;
    QRadioButton *m_pOtpDynamicPasswordButton = nullptr;
    QRadioButton *m_pSecurityKeyButton = nullptr;

    int m_currentUserType;
    MessageBar *m_pTipMessage = nullptr;
    bool m_isExistMfa = false;
    bool polkitisShow = false;
    SystemDbusDispatcher * sysdispatcher;
    QDBusInterface *userSettingDbus;
    UniAuthService *m_pUniauthBackendDbus;
    QMap<QString, UserInfomation> allUserInfoMap;
    QList<UtilsForUserinfo*> utilsUsers;
    QString mUserName;
    QStringList m_loginedUser;
    bool mFirstLoad;

    QDBusInterface *m_pLoginMgrInterface = nullptr;
    PwdlessService *m_pMulFactorDbus = nullptr;
    MulVerifyDialog *mulDialog = nullptr;
    QString m_factorUser;
public:
    void initUI();
    void hideComponent();
    void buildAndSetupUsers();
    void setUserConnect();
    void setUserDBusPropertyConnect(const QString pObjPath);
    QString getAutomaticLogin();

    void showChangeUserTypeDialog(QString u);
    void showChangeUserNicknameDialog();
    void showChangeUserPwdDialog(QString pName);
    void showChangeUserLogoDialog(QString username, UtilsForUserinfo *utilsUser);
    void showCreateUserNewDialog();
    void showDeleteUserExistsDialog(QString pName);

    void showBindOtpDialog(QString pName);
    void showSecurityKeyDialog(QString pName);

    void buildItemForUsersAndSetConnect(UserInfomation user);

    void initMoreSettingsUI();
    void initMoreSettingsStatus();
    void initFactoryStatus();

    void initTabOrder();

private:
    void initSearchText();
    QStringList getLoginedUsers();
    void _acquireAllUsersInfo();
    UserInfomation _acquireUserInfo(QString objpath);
    QString _accountTypeIntToString(int type);
    void showChangeGroupDialog();
    bool isLastAdmin(QString uname);
    bool openAutoLoginMsg(const QString &userName);
    void showMessageBox();
    QString renameFaceName(QString fullFilename, QString username);
    qint64 getUserUid(const QString &name);
    bool getNoPwdStatus();
    bool changeOtherUserPassword(const QString &programPath, const QString &username, const QString &newPassword, int *resultCode, QString *resultMsg);


Q_SIGNALS:
    void userTypeChanged(QString n);

public slots:
    void currentUserPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList);
    void ukccPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList);
    void setSecurityControl();

    void newUserCreateDoneSlot(QDBusObjectPath op);
    void existsUserDeleteDoneSlot(QDBusObjectPath op);

    void setPasswordTips(int uid, const QString &text); 

    void onUserNew(quint32, QDBusObjectPath);
    void onUserRemove(quint32, QDBusObjectPath);

    void onSetFactoySlot(bool checked);
    void onAllowSSHChangedSlot(bool checked);
    void onChangeDualFactor(int id);
    void onSelectFactorSlot(int id);
    void onBindStatusChanged(QString user, QString method, int bind);
    void onMulEnableStatusChanged(bool isOpen, QString method);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // USERINFO_H
