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
#include "userinfo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>

#include <polkit-qt5-1/polkitqt1-authority.h>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/applications/accessinfohelper.h>

#include "otpbinddialog.h"
#include "securitykeysetdlg.h"

#include "passwdcheckutil.h"
#include "loginedusers.h"

#include "ukcccommon.h"
using namespace ukcc;

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif
#define DEFAULTFACECOMMUNITY "/usr/share/ukui/faces/01-default-community.png"
#define DEFAULTFACECOMMERCIAL "/usr/share/ukui/faces/01-default-commercial.png"
#define DEFAULTFACE (UkccCommon::isCommunity() || UkccCommon::isOpenkylin())?DEFAULTFACECOMMUNITY:DEFAULTFACECOMMERCIAL
#define ITEMHEIGH 60

const static QString login1Service = QStringLiteral("org.freedesktop.login1");
const static QString login1Path = QStringLiteral("/org/freedesktop/login1");
const static QString login1ManagerInterface = QStringLiteral("org.freedesktop.login1.Manager");

UserInfo::UserInfo() : mFirstLoad(true)
{
    pluginName = QObject::tr("User Info");
    pluginType = ACCOUNT;
}

UserInfo::~UserInfo()
{
    if (!mFirstLoad) {
    }
}

QString UserInfo::plugini18nName()
{
    return pluginName;
}

int UserInfo::pluginTypes()
{
    return pluginType;
}

QWidget *UserInfo::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        /**/
        m_pPluginWidget = new QWidget;
        m_pPluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        m_pMulFactorDbus = new PwdlessService(this);

        initUI();

        // 构建System dbus调度对象
        sysdispatcher = new SystemDbusDispatcher(this);

        userSettingDbus =  new QDBusInterface("org.ukui.groupmanager",
                                              "/org/ukui/groupmanager",
                                              "org.ukui.groupmanager.interface",
                                              QDBusConnection::systemBus());
        // 将以后所有DBus调用的超时设置为 milliseconds
        userSettingDbus->setTimeout(2147483647); // -1 为默认的25s超时

        m_pUniauthBackendDbus = new UniAuthService(this);
        // 将以后所有DBus调用的超时设置为 milliseconds
        m_pUniauthBackendDbus->setTimeout(2147483647); // -1 为默认的25s超时

        m_pLoginMgrInterface = new QDBusInterface(login1Service,
                                                 login1Path,
                                                 login1ManagerInterface,
                                                 QDBusConnection::systemBus(),
                                                 this);

        // 获取系统全部用户信息，用户Uid大于等于1000的
        _acquireAllUsersInfo();

        initSearchText();

        buildAndSetupUsers();
        setUserConnect();
        qApp->installEventFilter(this);
        initTabOrder();
    }
    return m_pPluginWidget;
}

const QString UserInfo::name() const
{

    return QStringLiteral("Userinfo");
}

bool UserInfo::isShowOnHomePage() const
{
    return !UkccCommon::isTablet();
}

QIcon UserInfo::icon() const
{
    return QIcon::fromTheme("user-available-symbolic");
}

bool UserInfo::isEnable() const
{
    return !UkccCommon::isTablet();
}

void UserInfo::initSearchText()
{
    //~ contents_path /Userinfo/Add
    tr("Add");
}

/**3.1****begin************/

void UserInfo::initUI()
{
    //标题
    KLabel *currentLabel = new KLabel();
    currentLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Userinfo/CurrentUser
    currentLabel->setText(tr("CurrentUser"));
    m_pOthersLabel = new KLabel();
    m_pOthersLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Userinfo/OthersUser
    m_pOthersLabel->setText(tr("OthersUser"));

    //当前用户区域
    m_pCurrentFrame = new SettingGroup();

    //当前用户
    m_pCurrentUserFrame = new UkccFrame();

    m_pCurrentUserlogoBtn = new KLabel();
    m_pCurrentUserlogoBtn->setObjectName("currentUserlogo");
    m_pCurrentUserlogoBtn->setFixedSize(QSize(96, 96));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCurrentUserlogoBtn, "Userinfo", "", "");
    m_pCurrentUserlogoBtn->setAccessibleName(ChangeUserLogo::tr("User Logo"));
    m_pCurrentUserlogoBtn->setAccessibleDescription(QString());

    m_pCurrentNickNameLabel = new KLabel();
    QFont font = m_pCurrentNickNameLabel->font();
    qDebug() << "init font size : " << font.pointSize() << font.pixelSize();
    font.setPointSize(font.pointSize() + 2);
    m_pCurrentNickNameLabel->setFont(font);

    m_pChangeNickNameBtn = new KPushButton();
    //~ contents_path /Userinfo/Change Username
    m_pChangeNickNameBtn->setText(tr("Change Username"));
    m_pChangeNickNameBtn->setObjectName("currentNickNameChange");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pChangeNickNameBtn, "Userinfo", "", "");
    m_pChangeNickNameBtn->setAccessibleName(tr("Change Username"));

    m_pChangeCurrentPwdBtn = new KPushButton();
    //~ contents_path /Userinfo/Password
    m_pChangeCurrentPwdBtn->setText(tr("Password"));
    m_pChangeCurrentPwdBtn->setObjectName("Password");
    m_pChangeCurrentPwdBtn->setAccessibleName(tr("Password"));

    m_pCurrentUserTypeLabel = new KLabel();
    m_pCurrentUserTypeLabel->setAlignment(Qt::AlignTop);
    m_pCurrentUserTypeLabel->setFontColor(Qt::gray);
    m_pCurrentUserTypeLabel->setFixedHeight(24);
    m_pCurrentUserTypeLabel->setFixedWidth(100);

    QVBoxLayout *currentUserinfoVerLayout = new QVBoxLayout();
    currentUserinfoVerLayout->setSpacing(0);
    currentUserinfoVerLayout->setContentsMargins(0, 0, 0, 0);
    currentUserinfoVerLayout->addStretch();
    currentUserinfoVerLayout->addWidget(m_pCurrentNickNameLabel);
    currentUserinfoVerLayout->addWidget(m_pCurrentUserTypeLabel, Qt::AlignHCenter);
    currentUserinfoVerLayout->addStretch();

    QHBoxLayout *currentUserHorLayout = new QHBoxLayout();
    currentUserHorLayout->setSpacing(8);
    currentUserHorLayout->setContentsMargins(16, 0, 16, 0);
    currentUserHorLayout->addWidget(m_pCurrentUserlogoBtn);
    currentUserHorLayout->addLayout(currentUserinfoVerLayout);
    currentUserHorLayout->addStretch();
    currentUserHorLayout->addWidget(m_pChangeNickNameBtn);
    currentUserHorLayout->addWidget(m_pChangeCurrentPwdBtn);

    m_pCurrentUserFrame->setMinimumSize(QSize(550, 127));
    m_pCurrentUserFrame->setMaximumSize(QSize(16777215, 127));
    m_pCurrentUserFrame->setFrameShape(QFrame::WinPanel);
    m_pCurrentUserFrame->setLayout(currentUserHorLayout);

    //免密登录
    //~ contents_path /Userinfo/LoginWithoutPwd
    m_pNopwdLoginFrame = new SwitchWidget(tr("LoginWithoutPwd"));
    m_pNopwdLoginFrame->setObjectName("LoginWithoutPwd");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNopwdLoginFrame->switchButton(), "Userinfo", "", "");
    m_pNopwdLoginFrame->switchButton()->setAccessibleName(tr("LoginWithoutPwd"));

    m_pCurrentFrame->addWidget(m_pCurrentUserFrame);
    m_pCurrentFrame->addWidget(m_pNopwdLoginFrame);

    // 开启了双因子认证的提示
    m_pTipMessage =  new MessageBar;
    m_pTipMessage->setMinimumSize(550, 36);
    m_pTipMessage->setMaximumSize(16777215, 36);
    m_pTipMessage->setContentsMargins(0,0,0,0);
    KLabel *tipLabel = new KLabel;
    tipLabel->setPixmap(QIcon::fromTheme("dialog-warning"), 16, 16);
    tipLabel->setText(tr("Two factor authentication has been enabled, but the \"password free login\" and \"automatic login upon startup\" functions cannot be activated."));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(16,0,16,0);
    hlayout->addWidget(tipLabel);
    m_pTipMessage->setLayout(hlayout);
    m_pTipMessage->hide();

    //其他用户区域
    m_pAddUserBtn = new AddButton(m_pPluginWidget);
    m_pAddUserBtn->setObjectName("addUser");
    m_pAddUserBtn->setRadiusType(UkccFrame::Around);

    QHBoxLayout *addUserHorLayout = new QHBoxLayout();
    addUserHorLayout->setContentsMargins(0, 0, 0, 0);
    addUserHorLayout->addWidget(m_pAddUserBtn);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pAddUserBtn, "Userinfo", "", "");
    m_pAddUserBtn->setAccessibleName(tr("Add"));

    m_pAddUserFrame = new UkccFrame(m_pPluginWidget, UkccFrame::BorderRadiusStyle::Around, true);
    m_pAddUserFrame->setContentsMargins(0,0,0,0);
    m_pAddUserFrame->setObjectName("m_pAddUserFrame");
    m_pAddUserFrame->setLayout(addUserHorLayout);

    m_pOthersFrame = new SettingGroup(m_pPluginWidget);
    m_pOthersFrame->insertWidget(0, m_pAddUserFrame);

    //界面主布局
    m_pMainVerLayout = new QVBoxLayout(m_pPluginWidget);
    m_pMainVerLayout->setSpacing(8);
    m_pMainVerLayout->setContentsMargins(0, 0, 0, 0);

    m_pMainVerLayout->addWidget(currentLabel);
    m_pMainVerLayout->addWidget(m_pCurrentFrame);
    m_pMainVerLayout->addWidget(m_pTipMessage);

    m_pMainVerLayout->addSpacing(40);

    m_pMainVerLayout->addWidget(m_pOthersLabel);
    m_pMainVerLayout->addWidget(m_pOthersFrame);

    //开机自动登录
    //~ contents_path /Userinfo/Automatically log in this user
    m_pAutoLoginFrame = new ComboxWidget(tr("Automatically log in this user"), m_pPluginWidget, UkccFrame::Bottom);
    m_pAutoLoginFrame->setObjectName("Automatically log in this user");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pAutoLoginFrame->comboBox(), "Userinfo", "", "");
    m_pAutoLoginFrame->label()->setAccessibleName(tr("Automatically log in this user"));
    m_pAutoLoginFrame->label()->setBuddy(m_pAutoLoginFrame->comboBox());
    m_pAutoLoginFrame->comboBox()->setAccessibleName(tr("Automatically log in this user"));
    m_pAutoLoginFrame->comboBox()->addItem(tr("Open"), 0);
    m_pAutoLoginFrame->comboBox()->addItem(tr("Close"), 1);

    // 服务器 && root用户
    m_isExistMfa = UkccCommon::isExistMfa();
    if (m_isExistMfa) {
        if (UkccCommon::isServerVersion()) {
            if (!getuid()) {
                initMoreSettingsUI();
                initMoreSettingsStatus();
            }
        } else {
            initMoreSettingsUI();
            initMoreSettingsStatus();
        }
    }

    KLabel *settingName = new KLabel();
    //~ contents_path /Userinfo/User Group
    settingName->setText(tr("User Group"));
    KLabel *hintText = new KLabel();
    hintText->setFontColor(ThemeController::getCustomColorFromDT("kfont-secondary"));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "styleName" || key == "themeColor" || key == "widgetThemeName") {
            hintText->setFontColor(ThemeController::getCustomColorFromDT("kfont-secondary"));
            qDebug() << Q_FUNC_INFO << "ukui style key changed" << key;
        }
    });
    hintText->setText(tr("Add user groups, modify user group permissions, etc"));
    m_pChangeCurrentGroupsBtn = new KPushButton();
    m_pChangeCurrentGroupsBtn->setText(tr("Edit"));
    m_pChangeCurrentGroupsBtn->setObjectName("Edit");
    m_pChangeCurrentGroupsBtn->setAccessibleName(tr("User Group"));
    m_pUserGroup = new UkccFrame();
    m_pUserGroup->setMinimumSize(QSize(550, 0));
    m_pUserGroup->setMaximumSize(QSize(16777215, 16777215));
    QHBoxLayout *userGroupLayout = new QHBoxLayout();
    userGroupLayout->setContentsMargins(16, 0, 16, 0);
    QVBoxLayout *vLayout  = new QVBoxLayout();
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(settingName);
    vLayout->addWidget(hintText);
    userGroupLayout->setContentsMargins(16, 16, 16, 16);
    userGroupLayout->addLayout(vLayout);
    userGroupLayout->addStretch();
    userGroupLayout->addWidget(m_pChangeCurrentGroupsBtn);
    m_pUserGroup->setLayout(userGroupLayout);

    m_pMainVerLayout->addSpacing(40);
    m_pMainVerLayout->addWidget(m_pMoreSettingsTitleLabel);
    m_pMainVerLayout->addWidget(m_pAutoLoginFrame);
    m_pMainVerLayout->addWidget(m_pMultipleFactorGroup);
    m_pMainVerLayout->addWidget(m_pUserGroup);

    m_pMainVerLayout->addStretch();
    m_pPluginWidget->setLayout(m_pMainVerLayout);

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
        m_pAddUserBtn->setEnabled(false);
    }
#endif

    //root需要屏蔽部分功能
    if (!getuid()){
        m_pChangeNickNameBtn->setEnabled(false);
        m_pCurrentNickNameLabel->setEnabled(false);
        m_pCurrentUserlogoBtn->setEnabled(false);
        m_pAutoLoginFrame->hide();
        m_pNopwdLoginFrame->hide();
    }
    hideComponent();
}

void UserInfo::initTabOrder()
{
    // 头像可点改头像，须可 Tab；用户类型标签不可聚焦，不得写入 setTabOrder
    // （Qt 对 NoFocus 直接 return，会把「修改用户名→修改密码」等链打断，导致「添加」成为侧栏后首项）
    if (m_pCurrentUserlogoBtn) {
        m_pCurrentUserlogoBtn->setFocusPolicy(Qt::StrongFocus);
    }

    QList<QWidget *> order;
    auto appendFocusable = [&order](QWidget *w) {
        if (!w) {
            return;
        }
        // 初始化时插件可能尚未 show，勿用 isVisibleTo 判断
        if (!(w->focusPolicy() & Qt::TabFocus)) {
            return;
        }
        if (order.contains(w)) {
            return;
        }
        order.append(w);
    };

    // 自上而下：当前用户 → 免密 → 其他用户 → 添加 → 更多设置 → 用户组
    appendFocusable(m_pCurrentUserlogoBtn);
    appendFocusable(m_pChangeNickNameBtn);
    appendFocusable(m_pChangeCurrentPwdBtn);
    if (m_pNopwdLoginFrame) {
        appendFocusable(m_pNopwdLoginFrame->switchButton());
    }
    foreach (UtilsForUserinfo *utils, utilsUsers) {
        if (!utils) {
            continue;
        }
        appendFocusable(utils->changePwdBtn);
        appendFocusable(utils->changeTypeBtn);
        appendFocusable(utils->delUserBtn);
        appendFocusable(utils->getUtilsToolBtn());
    }
    // 「添加」创建较早，须显式排在其他用户之后，避免默认焦点链把它放到最前
    appendFocusable(m_pAddUserBtn);
    if (m_pAutoLoginFrame) {
        appendFocusable(m_pAutoLoginFrame->comboBox());
    }
    if (m_pMultipleFactorFrame) {
        appendFocusable(m_pMultipleFactorFrame->switchButton());
        appendFocusable(m_pOtpDynamicPasswordButton);
        appendFocusable(m_pSecurityKeyButton);
        appendFocusable(m_pAllowSShButton);
    }
    appendFocusable(m_pChangeCurrentGroupsBtn);

    for (int i = 0; i + 1 < order.size(); ++i) {
        QWidget::setTabOrder(order.at(i), order.at(i + 1));
    }

    // 侧栏 ListView 之后进入本页首个可 Tab 控件
    if (!order.isEmpty() && m_pPluginWidget) {
        if (QWidget *win = m_pPluginWidget->window()) {
            const QList<QListView *> views = win->findChildren<QListView *>();
            for (QListView *view : views) {
                if (!view || m_pPluginWidget->isAncestorOf(view)) {
                    continue;
                }
                if (view->focusPolicy() & Qt::TabFocus) {
                    QWidget::setTabOrder(view, order.first());
                    break;
                }
            }
        }
    }
}

void UserInfo::initMoreSettingsUI()
{
    m_pMoreSettingsTitleLabel = new KLabel();
    m_pMoreSettingsTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Userinfo/More Settings
    m_pMoreSettingsTitleLabel->setText(tr("More Settings"));

    //~ contents_path /Userinfo/Enable two factor authentication
    m_pMultipleFactorFrame = new SwitchWidget(tr("Enable two factor authentication"), m_pPluginWidget,
                                            UkccFrame::None, tr("After activation, secondary verification is required when logging in with a password."));
    m_pMultipleFactorFrame->setHeightAdaptive(false);
    m_pMultipleFactorFrame->setMinimumSize(QSize(550, 0));
    m_pMultipleFactorFrame->setMaximumSize(QSize(16777215, 16777215));
    m_pMultipleFactorFrame->setObjectName("Enable two factor authentication");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pMultipleFactorFrame, "Userinfo", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pMultipleFactorFrame->switchButton(), "Userinfo", "", "");
    m_pMultipleFactorFrame->switchButton()->setAccessibleName(tr("Enable two factor authentication"));

    m_pOtpDynamicPasswordFrame = new UkccFrame();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pOtpDynamicPasswordFrame, "Userinfo", "", "");
    m_pOtpDynamicPasswordButton = new QRadioButton(tr("OTP Dynamic Password"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pOtpDynamicPasswordButton, "Userinfo", "", "");
    m_pOtpDynamicPasswordButton->setAccessibleName(tr("OTP Dynamic Password"));
    m_pIsBindOTPLabel = new KLabel();
    QHBoxLayout *optHLayout = new QHBoxLayout;
    optHLayout->addWidget(m_pOtpDynamicPasswordButton);
    optHLayout->addWidget(m_pIsBindOTPLabel);
    optHLayout->addStretch();
    m_pOtpDynamicPasswordFrame->setLayout(optHLayout);

    m_pSecurityKeyFrame = new UkccFrame();
    m_pSecurityKeyButton = new QRadioButton(tr("Security Key"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityKeyFrame, "Userinfo", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityKeyButton, "Userinfo", "", "");
    m_pSecurityKeyButton->setAccessibleName(tr("Security Key"));
    m_pIsBindSecurityLabel = new KLabel();
    QHBoxLayout *securityKeyHLayout = new QHBoxLayout;
    securityKeyHLayout->addWidget(m_pSecurityKeyButton);
    securityKeyHLayout->addWidget(m_pIsBindSecurityLabel);
    securityKeyHLayout->addStretch();
    m_pSecurityKeyFrame->setLayout(securityKeyHLayout);

    m_pAllowSSHFrame = new UkccFrame();
    //~ contents_path /Userinfo/Allow SSH to log in through two factor login
    m_pAllowSShButton = new QCheckBox(tr("Allow SSH to log in through two factor login"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityKeyFrame, "Userinfo", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pAllowSShButton, "Userinfo", "", "Allow SSH to log in through two factor login");
    m_pAllowSShButton->setAccessibleName(tr("Allow SSH to log in through two factor login"));
    QHBoxLayout *allowSSHHLayout = new QHBoxLayout;
    allowSSHHLayout->addWidget(m_pAllowSShButton);
    m_pAllowSSHFrame->setLayout(allowSSHHLayout);

    m_pFactoyButtonGroup = new QButtonGroup;
    m_pFactoyButtonGroup->addButton(m_pOtpDynamicPasswordButton, 0);
    m_pFactoyButtonGroup->addButton(m_pSecurityKeyButton, 1);
    m_pMultipleFactorGroup = new SettingGroup;
    m_pMultipleFactorGroup->addWidget(m_pMultipleFactorFrame);
    m_pMultipleFactorGroup->addWidget(m_pOtpDynamicPasswordFrame);
    m_pMultipleFactorGroup->addWidget(m_pSecurityKeyFrame);
    m_pMultipleFactorGroup->addWidget(m_pAllowSSHFrame);
}

void UserInfo::initFactoryStatus()
{
    // 判断是否绑定OTP
    QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(g_get_user_name());
    if (authMethodList.contains("otp")) {
        m_pIsBindOTPLabel->setText(tr("Bound"));
        m_pIsBindOTPLabel->setFontColor(Qt::green);
    } else {
        m_pIsBindOTPLabel->setText(tr("Unbound"));
        m_pIsBindOTPLabel->setFontColor(Qt::red);
    }

    // 判断是否绑定安全密钥
    if (authMethodList.contains("ukey")) {
        m_pIsBindSecurityLabel->setText(tr("Bound"));
        m_pIsBindSecurityLabel->setFontColor(Qt::green);
    } else {
        m_pIsBindSecurityLabel->setText(tr("Unbound"));
        m_pIsBindSecurityLabel->setFontColor(Qt::red);
    }

    QString method;
    int result = m_pMulFactorDbus->GetDualFactorStatus(method);
    bool isOpen = (result == 1 ? true : false);

    m_pMultipleFactorFrame->switchButton()->blockSignals(true);
    m_pMultipleFactorFrame->switchButton()->setChecked(isOpen);
    m_pMultipleFactorFrame->switchButton()->blockSignals(false);
    m_pOtpDynamicPasswordFrame->setVisible(isOpen);
    m_pSecurityKeyFrame->setVisible(isOpen);

    // 402637要求 ukey 不支持 ssh，进行隐藏
    m_pAllowSSHFrame->setVisible(isOpen && !(method.compare("ukey") == 0) && UkccCommon::isExistOpensshServer());

    if (isOpen) {
        m_pNopwdLoginFrame->blockSignals(true);
        m_pNopwdLoginFrame->setChecked(false);
        m_pNopwdLoginFrame->blockSignals(false);
        m_pAutoLoginFrame->blockSignals(true);
        m_pAutoLoginFrame->setCurrentIndex(1);
        m_pAutoLoginFrame->blockSignals(false);
        m_pAutoLoginFrame->setEnabled(false);
        m_pNopwdLoginFrame->setEnabled(false);
        m_pTipMessage->setVisible(true);
    } else {
        m_pAutoLoginFrame->setEnabled(true);
        m_pNopwdLoginFrame->setEnabled(true);
        m_pTipMessage->setVisible(false);
    }

    m_pFactoyButtonGroup->blockSignals(true);
    m_pFactoyButtonGroup->button(0)->setChecked(method.compare("otp") == 0);
    m_pFactoyButtonGroup->button(1)->setChecked(method.compare("ukey") == 0);
    m_pFactoyButtonGroup->blockSignals(false);

    m_pAllowSShButton->blockSignals(true);
    m_pAllowSShButton->setChecked((m_pMulFactorDbus->GetSshAuthStatus("otp")
                                   || m_pMulFactorDbus->GetSshAuthStatus("ukey")));
    m_pAllowSShButton->blockSignals(false);
}

void UserInfo::initMoreSettingsStatus()
{
    initFactoryStatus();

    connect(m_pMultipleFactorFrame->switchButton(), &KSwitchButton::stateChanged, this, &UserInfo::onSetFactoySlot);

    connect(m_pAllowSShButton, &QCheckBox::stateChanged, this, &UserInfo::onAllowSSHChangedSlot);

    connect(m_pFactoyButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &UserInfo::onChangeDualFactor);


    QDBusConnection::systemBus().connect("com.kylin.pwdless.daemon",
                                         "/",
                                         "com.kylin.pwdless.daemon",
                                         "userBindStatusChanged",
                                         this,
                                         SLOT(onBindStatusChanged(QString,QString,int)));


    QDBusConnection::systemBus().connect("com.kylin.pwdless.daemon",
                                         "/",
                                         "com.kylin.pwdless.daemon",
                                         "mfaStatusChanged",
                                         this,
                                         SLOT(onMulEnableStatusChanged(bool, QString)));
}

void UserInfo::onMulEnableStatusChanged(bool isOpen, QString method)
{
    m_pMultipleFactorFrame->switchButton()->blockSignals(true);
    m_pMultipleFactorFrame->switchButton()->setChecked(isOpen);
    m_pMultipleFactorFrame->switchButton()->blockSignals(false);
    m_pOtpDynamicPasswordFrame->setVisible(isOpen);
    m_pSecurityKeyFrame->setVisible(isOpen);
    m_pAllowSSHFrame->setVisible(isOpen && !(method.compare("ukey") == 0) && UkccCommon::isExistOpensshServer());
    m_pFactoyButtonGroup->blockSignals(true);
    m_pFactoyButtonGroup->button(0)->setChecked(method.compare("otp") == 0);
    m_pFactoyButtonGroup->button(1)->setChecked(method.compare("ukey") == 0);
    m_pFactoyButtonGroup->blockSignals(false);

    if (isOpen) {
        m_pNopwdLoginFrame->blockSignals(true);
        m_pNopwdLoginFrame->setChecked(false);
        m_pNopwdLoginFrame->blockSignals(false);
        m_pAutoLoginFrame->blockSignals(true);
        m_pAutoLoginFrame->setCurrentIndex(1);
        m_pAutoLoginFrame->blockSignals(false);
        m_pAutoLoginFrame->setEnabled(false);
        m_pNopwdLoginFrame->setEnabled(false);
        m_pTipMessage->setVisible(true);
    } else {
        m_pAutoLoginFrame->setEnabled(true);
        m_pNopwdLoginFrame->setEnabled(true);
        m_pTipMessage->setVisible(false);
    }
}

void UserInfo::onBindStatusChanged(QString user, QString method, int bind)
{
    if (user == mUserName) {
        QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(mUserName);
        if (authMethodList.contains("otp")) {
            m_pIsBindOTPLabel->setText(tr("Bound"));
            m_pIsBindOTPLabel->setFontColor(Qt::green);
        } else {
            m_pIsBindOTPLabel->setText(tr("Unbound"));
            m_pIsBindOTPLabel->setFontColor(Qt::red);
        }

        // 判断是否绑定安全密钥
        if (authMethodList.contains("ukey")) {
            m_pIsBindSecurityLabel->setText(tr("Bound"));
            m_pIsBindSecurityLabel->setFontColor(Qt::green);
        } else {
            m_pIsBindSecurityLabel->setText(tr("Unbound"));
            m_pIsBindSecurityLabel->setFontColor(Qt::red);
        }
    }
}

void UserInfo::onChangeDualFactor(int id)
{
    qDebug() << __FUNCTION__ << id;

    // 当前已开启 点击的第二因子为选中状态，不调用接口，直接返回
    QString method;
    int result = m_pMulFactorDbus->GetDualFactorStatus(method);
    if ((id == 0 && method == "otp") || (id == 1 && method == "ukey")) {
        return;
    }

    UkccCommon::buriedSettings(name(), id == 0 ? QString("OTP Dynamic Password") : QString("Security Key"), QString("settings"), QString::number(id));

    m_factorUser = mUserName;

    // 判断选中是否是已绑定的，未绑定则先弹窗绑定
    QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(m_factorUser);
    switch (id) {
    case 0:
        if (authMethodList.contains("otp")) {
            m_pMulFactorDbus->EnableDualFactor("otp");
        } else {
            showBindOtpDialog(m_factorUser);
        }
        break;
    case 1:
        if (authMethodList.contains("ukey")) {
            m_pMulFactorDbus->EnableDualFactor("ukey");
        } else {
            showSecurityKeyDialog(m_factorUser);
        }
        break;
    default:
        break;
    }

    // 绑定完成后，设置为第二因子
    QStringList authMethodList1 = m_pMulFactorDbus->GetBoundFactorMethods(m_factorUser);
    if (id == 0 && !authMethodList.contains("otp") && authMethodList1.contains("otp")) {
        m_pMulFactorDbus->EnableDualFactor("otp");
    }

    if (id == 1 && !authMethodList.contains("ukey") && authMethodList1.contains("ukey")) {
        m_pMulFactorDbus->EnableDualFactor("ukey");
    }

    initFactoryStatus();
}

void UserInfo::onSetFactoySlot(bool checked)
{
    UkccCommon::buriedSettings(name(), m_pMultipleFactorFrame->objectName(), QString("settings"), checked ? "true":"false");
    if (checked) {
        // 开启多因子认证，需要关闭所有用户的免密登录与自动登录，弹窗提示用户
        QMessageBox msg(this->m_pPluginWidget);
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("Enabling two factor authentication will disable all user password free login and automatic login settings."));
        QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
        msg.setEscapeButton(cancelBtn);
        QTimer::singleShot(0, &msg, [&]() {
            cancelBtn->setVisible(false);
            qDebug() << "Cancel button status：" << (cancelBtn->isVisible() ? "visible" : "hidden");
        });
        QPushButton *confirmBtn = msg.addButton(tr("Confirm"), QMessageBox::AcceptRole);
        msg.setDefaultButton(confirmBtn);
        int ret = msg.exec();
        qDebug() << "ret = " << ret << msg.clickedButton() << confirmBtn;
        if (ret != QMessageBox::Accepted) {
            m_pMultipleFactorFrame->switchButton()->blockSignals(true);
            m_pMultipleFactorFrame->switchButton()->setChecked(false);
            m_pMultipleFactorFrame->switchButton()->blockSignals(false);
            return;
        }

        // 开启多因子认证，先弹窗选择第二因子
        m_factorUser = mUserName;
        mulDialog = new MulVerifyDialog(false, m_pPluginWidget);
        QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(m_factorUser);
        qDebug() << authMethodList;
        mulDialog->refreshBindStatus("otp", authMethodList.contains("otp"));
        mulDialog->refreshBindStatus("ukey", authMethodList.contains("ukey"));
        // 根据选择的第二因子是否已绑定，进行绑定设置
        connect(mulDialog, &MulVerifyDialog::onSelectFactorySignal, this, [=](int id){
            onSelectFactorSlot(id);
            QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(m_factorUser);
            mulDialog->refreshBindStatus("otp", authMethodList.contains("otp"));
            mulDialog->refreshBindStatus("ukey", authMethodList.contains("ukey"));
        });
        if (mulDialog->exec() != QDialog::Accepted) {
            // 选择弹窗未进行选择，直接关闭，恢复账户界面ui
            m_pMultipleFactorFrame->switchButton()->blockSignals(true);
            m_pMultipleFactorFrame->switchButton()->setChecked(false);
            m_pMultipleFactorFrame->switchButton()->blockSignals(false);
        } else {

            initFactoryStatus();
        }
    } else {
        // 关闭多因子认证
        int reply = m_pMulFactorDbus->DisableDualFactor();
        if (reply == 0) {
            m_pOtpDynamicPasswordFrame->setVisible(false);
            m_pSecurityKeyFrame->setVisible(false);
            m_pAllowSSHFrame->setVisible(false);
            m_pNopwdLoginFrame->setEnabled(true);
            m_pAutoLoginFrame->setEnabled(true);
            m_pTipMessage->setVisible(false);
        } else {
            m_pMultipleFactorFrame->switchButton()->blockSignals(true);
            m_pMultipleFactorFrame->switchButton()->setChecked(!checked);
            m_pMultipleFactorFrame->switchButton()->blockSignals(false);
        }
    }
}

void UserInfo::onSelectFactorSlot(int id)
{
    QStringList authMethodList = m_pMulFactorDbus->GetBoundFactorMethods(m_factorUser);
    qDebug() << id << authMethodList;
    switch (id) {
    case 0:
        // otp 若未绑定则先进行绑定
        if (!authMethodList.contains("otp")) {
            // 弹窗绑定otp
            if (mulDialog != nullptr) {
                OtpBindDialog *otpBindDialog = new OtpBindDialog(m_factorUser, m_pMulFactorDbus, mulDialog);
                otpBindDialog->setQrCodePixmap(m_pMulFactorDbus->GetOtpQrCode(m_factorUser, 0));
                otpBindDialog->exec();
            } else {
                OtpBindDialog *otpBindDialog = new OtpBindDialog(m_factorUser, m_pMulFactorDbus, m_pPluginWidget);
                otpBindDialog->setQrCodePixmap(m_pMulFactorDbus->GetOtpQrCode(m_factorUser, 0));
                otpBindDialog->exec();
            }

        } else {
            // 设置otp为第二因子
            m_pMulFactorDbus->EnableDualFactor("otp");
        }
        break;
    case 1:
        // 安全密钥
        if (!authMethodList.contains("ukey")) {
            // 弹窗 绑定安全密钥
            if (mulDialog != nullptr) {
                SecurityKeySetDlg *securityKeySetDlg = new SecurityKeySetDlg(m_factorUser, m_pMulFactorDbus, mulDialog);
                securityKeySetDlg->setAttribute(Qt::WA_DeleteOnClose);
                int ret = securityKeySetDlg->exec();
            } else {
                SecurityKeySetDlg *securityKeySetDlg = new SecurityKeySetDlg(m_factorUser, m_pMulFactorDbus, m_pPluginWidget);
                securityKeySetDlg->setAttribute(Qt::WA_DeleteOnClose);
                int ret = securityKeySetDlg->exec();
            }
        } else {
            // 设置安全密钥为第二因子
            m_pMulFactorDbus->EnableDualFactor("ukey");
        }
        break;
    default:
        break;
    }
}

void UserInfo::onAllowSSHChangedSlot(bool checked)
{
    // 设置允许ssh连接
    UkccCommon::buriedSettings(name(), m_pAllowSShButton->objectName(), QString("settings"), checked ? "true":"false");

    int ret = m_pMulFactorDbus->SetSshAuthStatus(m_pFactoyButtonGroup->checkedId()==0?"otp":"ukey", checked);
    if (ret != 0) {
        m_pAllowSShButton->blockSignals(true);
        m_pAllowSShButton->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
        m_pAllowSShButton->blockSignals(false);
    }
}

void UserInfo::hideComponent()
{
    if (QString(QLatin1String(kdk_system_get_hostCloudPlatform())) == "huawei") {
        m_pChangeCurrentPwdBtn->setVisible(false);
        m_pAutoLoginFrame->setVisible(false);
        m_pNopwdLoginFrame->setVisible(false);
    }

    // ABC 项目要求隐藏
    kdk_custom_info info = kdk_system_get_custom_version();
    qDebug() << "custom_info = " << info.custom_info;
    if (QString(info.custom_info).compare("ABC", Qt::CaseInsensitive) == 0) {
        m_pAutoLoginFrame->setVisible(false);
        m_pNopwdLoginFrame->setVisible(false);
    }

    // 香港警察要求本地用户与域用户修改密码按钮均置灰
#ifdef XGJC
    m_pChangeCurrentPwdBtn->setEnabled(false);
#endif
}

void UserInfo::setSecurityControl()
{
    // 安全管控 设置项的显示与隐藏
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value(name().toLower() + "Settings").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << __FUNCTION__ << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0) << item.at(1);
        if (item.at(0) == "noPwdLoginFrame") {
            m_pNopwdLoginFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "autoLoginFrame") {
            if (item.at(1) == "true") {
                m_pNopwdLoginFrame->setRadiusType(UkccFrame::None);
            } else {
                m_pNopwdLoginFrame->setRadiusType(UkccFrame::Bottom);
            }
            m_pAutoLoginFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "changeCurrentPwdBtn") {
            m_pChangeCurrentPwdBtn->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "changeCurrentGroupsBtn") {
            m_pUserGroup->setVisible(item.at(1) == "true");
        }
        // 当前用户为管理员用户，才可修改其他用户密码、类型、删除用户，非管理员用户隐藏入口
        if (item.at(0) == "UtilsChangePwdBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->setPwdVisible(item.at(1) == "true" && m_currentUserType != STANDARDUSER);
            }
        }

        if (item.at(0) == "UtilsChangeTypeBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->setTypeVisible(item.at(1) == "true" && m_currentUserType != STANDARDUSER);
            }
        }

        if (item.at(0) == "UtilsDeleteBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->setDelVisible(item.at(1) == "true" && m_currentUserType != STANDARDUSER);
            }
        }

        // 当前用户为管理员用户，才可添加用户，非管理员用户隐藏入口
        if (item.at(0) == "addUserBtn") {
            m_pAddUserFrame->setVisible(item.at(1) == "true" && m_currentUserType != STANDARDUSER);
        }

        if (item.at(0) == "currentNickNameChangeLabel") {
            m_pChangeNickNameBtn->setVisible(item.at(1) == "true");
        }

        if (item.at(0) == "UtilsUsersAll") {
            m_pOthersFrame->setVisible(item.at(1) == "true");
            m_pOthersLabel->setVisible(item.at(1) == "true");
        }
    }

    // 安全管控 设置项的启用与禁用
    QString moduleEnableSettings = ModuleMap.value(name().toLower() + "Enable").toString();
    QStringList enableSetItems = moduleEnableSettings.split(",");

    foreach (QString setItem, enableSetItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0) << item.at(1);
        if (item.at(0) == "noPwdLoginFrame") {
            m_pNopwdLoginFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "autoLoginFrame") {
            m_pAutoLoginFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "changeCurrentPwdBtn") {
            m_pChangeCurrentPwdBtn->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "changeCurrentGroupsBtn") {
            m_pChangeCurrentGroupsBtn->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "UtilsChangePwdBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->refreshPwdStatus(item.at(1) == "true");
            }
        }
        if (item.at(0) == "UtilsChangeTypeBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->refreshTypeStatus(item.at(1) == "true");
            }
        }

        if (item.at(0) == "UtilsDeleteBtn") {
            foreach (UtilsForUserinfo *utils, utilsUsers) {
                utils->refreshDelStatus(item.at(1) == "true");
            }
        }

        if (item.at(0) == "addUserBtn") {
            m_pAddUserBtn->setEnabled(item.at(1) == "true");
        }

        if (item.at(0) == "currentNickNameChangeLabel") {
            m_pChangeNickNameBtn->setEnabled(item.at(1) == "true");
        }

        if (item.at(0) == "UtilsUsersAll") {
            m_pOthersFrame->setEnabled(item.at(1) == "true");
        }
    }

}

void UserInfo::buildAndSetupUsers()
{
    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        //当前用户
        if (user.username == QString(g_get_user_name())) {
            //设置用户头像
            QPixmap iconfile = UkccCommon::makeRoundLogo(user.iconfile, m_pCurrentUserlogoBtn->width(), m_pCurrentUserlogoBtn->height(), m_pCurrentUserlogoBtn->width()/2, m_pCurrentUserlogoBtn->devicePixelRatioF());
            m_pCurrentUserlogoBtn->setPixmap(iconfile, m_pCurrentUserlogoBtn->width(), m_pCurrentUserlogoBtn->height());

            //设置用户昵称
            m_pCurrentNickNameLabel->setText(user.realname);

            //用户类型
            QString cType = _accountTypeIntToString(user.accounttype);
            m_pCurrentUserTypeLabel->setText(cType);

            m_currentUserType = user.accounttype;
            if (m_currentUserType == STANDARDUSER) {
                m_pAddUserFrame->hide();
                if (m_pMoreSettingsTitleLabel != nullptr) {
                    m_pMoreSettingsTitleLabel->hide();
                }
                if (m_pMultipleFactorGroup != nullptr) {
                    m_pMultipleFactorGroup->hide();
                }
            }

            QProcess *process = new QProcess;
            process->start("dpkg -l | grep kim-client");
            process->waitForFinished();

            QByteArray ba = process->readAllStandardOutput();
            delete process;
            QString mOutput = QString(ba.data());
            // 域用户用户信息不可设置
            if (UkccCommon::isDomainUser(user.username.toLatin1().data())) {
                // 工行修改密码按钮置灰
                if (mOutput.contains("icbc")) {
                    m_pChangeCurrentPwdBtn->setEnabled(false);
                }
                m_pChangeNickNameBtn->setEnabled(false);
                m_pCurrentNickNameLabel->setEnabled(false);
                m_pChangeCurrentGroupsBtn->setEnabled(false);
                m_pNopwdLoginFrame->setEnabled(false);
                m_pAutoLoginFrame->setEnabled(false);
                m_pAddUserBtn->setEnabled(false);
            }

            //设置自动登录状态
            m_pAutoLoginFrame->blockSignals(true);
            if (user.autologin) {
                m_pAutoLoginFrame->setCurrentIndex(0);
            } else {
                m_pAutoLoginFrame->setCurrentIndex(1);
            }

            m_pAutoLoginFrame->blockSignals(false);

            //设置免密登录状态
            m_pNopwdLoginFrame->blockSignals(true);
            m_pNopwdLoginFrame->setChecked(user.noPwdLogin);
            m_pNopwdLoginFrame->blockSignals(false);

            //绑定当前用户的属性改变回调
            setUserDBusPropertyConnect(user.objpath);

        } else {

            buildItemForUsersAndSetConnect(user);
        }
    }

    setSecurityControl();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setSecurityControl()));

    QDBusConnection::systemBus().connect(QString(), QString(), "org.freedesktop.Accounts", "UserAdded", this, SLOT(newUserCreateDoneSlot(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(QString(), QString(), "org.freedesktop.Accounts", "UserDeleted", this, SLOT(existsUserDeleteDoneSlot(QDBusObjectPath)));
}

void UserInfo::buildItemForUsersAndSetConnect(UserInfomation user)
{
    UtilsForUserinfo * utils = new UtilsForUserinfo(m_isExistMfa, this);
    utilsUsers << utils;
    utils->m_username = user.username;
    utils->refreshUserLogo(user.iconfile);
    utils->refreshUserNickname(user.realname);
    utils->refreshUserType(user.accounttype);
    utils->setObjectPathData(user.objpath);
    // 香港警察要求所有用户修改密码按钮均置灰
#ifdef XGJC
    utils->refreshPwdStatus(false);
#endif

    if (m_currentUserType == STANDARDUSER) {
        utils->setDelVisible(false);
        utils->setPwdVisible(false);
        utils->setTypeVisible(false);
        utils->setMoreBtnVisible(false);
    }

    if (user.accounttype){
        utils->refreshDelStatus(!isLastAdmin(user.username));
    }
    utils->refreshTypeStatus(!isLastAdmin(user.username) && !user.logined);

    // 域用户按钮不可设置
    UserInfomation curruser = allUserInfoMap.value(g_get_user_name());
    if (UkccCommon::isDomainUser(curruser.username.toLatin1().data())) {
        utils->refreshDelStatus(false);
        utils->refreshPwdStatus(false);
        utils->refreshTypeStatus(false);
    }

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status()){
        if (user.username == "secadm" || user.username == "auditadm"){
            utils->refreshDelStatus(false);
            utils->refreshPwdStatus(false);
            utils->refreshTypeStatus(false);
        }
    }
#endif

    connect(utils, &UtilsForUserinfo::changePwdBtnPressed, this, [=]{
        showChangeUserPwdDialog(user.username);
    });
    connect(utils, &UtilsForUserinfo::changeTypeBtnPressed, this, [=]{
        showChangeUserTypeDialog(user.username);
    });

    connect(utils, &UtilsForUserinfo::deleteUserBtnPressed, this, [=]{
        showDeleteUserExistsDialog(user.username);
    });

    connect(utils, &UtilsForUserinfo::bindOtpActPressed, this, [=]{
        showBindOtpDialog(user.username);
    });

    connect(utils, &UtilsForUserinfo::bindUkeyActPressed, this, [=]{
        showSecurityKeyDialog(user.username);
    });

    //用户类型发生变化，重置状态
    connect(this, &UserInfo::userTypeChanged, utils, [=](QString n){
        QStringList loginedUsers = getLoginedUsers();
        qDebug() << "update userType"<< user.username << loginedUsers.contains(user.username);
        utils->setDelVisible(m_currentUserType == STANDARDUSER ? false : true);
        utils->setPwdVisible(m_currentUserType == STANDARDUSER ? false : true);
        utils->setTypeVisible(m_currentUserType == STANDARDUSER ? false : true);
        m_pAddUserFrame->setVisible(m_currentUserType == STANDARDUSER ? false : true);
        utils->setMoreBtnVisible(m_currentUserType == STANDARDUSER ? false : true);

        utils->refreshDelStatus(!isLastAdmin(user.username));
        utils->refreshTypeStatus(!isLastAdmin(user.username) && !loginedUsers.contains(user.username));
#ifdef WITHKYSEC
        if (!kysec_is_disabled() && kysec_get_3adm_status()){
            if (user.username == "secadm" || user.username == "auditadm"){
                utils->refreshDelStatus(false);
                utils->refreshPwdStatus(false);
                utils->refreshTypeStatus(false);
            }
        }
#endif
    });

    QDBusInterface tmpProperty("org.freedesktop.Accounts",
                             user.objpath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());

    tmpProperty.connection().connect("org.freedesktop.Accounts", user.objpath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    utils, SLOT(userPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));

    UkccFrame *newUserFrame = utils->buildItemForUsers();

    m_pOthersFrame->insertWidget(0, newUserFrame);
    m_pAddUserBtn->setRadiusType(UkccFrame::Bottom);
}

void UserInfo::showCreateUserNewDialog()
{
    //获取系统所有用户名列表，创建时判断重名
    QStringList usersStringList;
    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();
        usersStringList.append(user.realname);
    }

    CreateUserNew *dialog = new CreateUserNew(usersStringList, m_pPluginWidget);

    QList<AnswerInfo> securityQuestion;
    connect(dialog, &CreateUserNew::bindSecurityQuestion, this, [&](QString name, QList<AnswerInfo> answerInfo){
        securityQuestion = answerInfo;
    });
    QString passwordTips = "";
    connect(dialog, &CreateUserNew::bindPasswordTips, this, [&](const QString &text){
        passwordTips = text;
    });

    connect(dialog, &CreateUserNew::confirmCreated, this, [&](QString name, QString fullname, int accounttype, QString pwd){
        QDBusPendingCall call = userSettingDbus->asyncCall("createUser", name, fullname, accounttype, DEFAULTFACE);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [&, name, pwd, securityQuestion, passwordTips](QDBusPendingCallWatcher *call){
            const qint64 uid = getUserUid(name);
            if (!securityQuestion.isEmpty()) {
                int ret = m_pUniauthBackendDbus->BindUserSecurityAnswers(uid, securityQuestion);
                qDebug() << "ret of bind security question = " << ret;
            }

            QString programPath = QString("/usr/bin/changeotheruserpwd");
            int resultCode = -1;
            QString resultMsg;
            if (changeOtherUserPassword(programPath, name, pwd, &resultCode, &resultMsg)) {
                qDebug() << "Result code:" << resultCode;
                qDebug() << "Message:" << resultMsg;
            }

            if (!passwordTips.isEmpty()) {
                setPasswordTips(uid, passwordTips);
            }
        });
    });

    dialog->exec();
}

qint64 UserInfo::getUserUid(const QString &name)
{
    QStringList objectpaths = sysdispatcher->list_cached_users();
    qint64 uid = 0;
    for (QString objectpath : objectpaths){
        UserInfomation user;
        user = _acquireUserInfo(objectpath);
        if (user.username == name) {
            uid = user.uid;
        }
        allUserInfoMap.insert(user.username, user);
    }
    return uid;
}

void UserInfo::setPasswordTips(int uid, const QString &text)
{
    if (uid != 0) {
        int ret = m_pUniauthBackendDbus->SetUserPasswordHint(uid, text);
        qDebug() << "reply of SetUserPasswordHint = " << ret;
    }
}

void UserInfo::showDeleteUserExistsDialog(QString pName)
{
    QStringList loginedusers = getLoginedUsers();
    if (loginedusers.contains(pName)) {
        QMessageBox::warning(m_pPluginWidget, tr("Warning"), tr("The user is logged in, please delete the user after logging out"));
        return;
    }

    if (allUserInfoMap.keys().contains(pName)){
        UserInfomation user = allUserInfoMap.value(pName);

        DeleteUserExists *dialog = new DeleteUserExists(user.username, user.realname, m_pPluginWidget);
        connect(dialog, &DeleteUserExists::confirmChanged, this, [=](bool removeFiles){
            userSettingDbus->call("deleteUser", user.uid, removeFiles);
        });
        dialog->exec();
    }
}

void UserInfo::showBindOtpDialog(QString pName)
{
    m_factorUser = pName;
    OtpBindDialog *otpBindDialog = new OtpBindDialog(m_factorUser, m_pMulFactorDbus, m_pPluginWidget);
    otpBindDialog->setQrCodePixmap(m_pMulFactorDbus->GetOtpQrCode(m_factorUser, 0));
    otpBindDialog->exec();
}

void UserInfo::showSecurityKeyDialog(QString pName)
{
    m_factorUser = pName;
    SecurityKeySetDlg *securityKeySetDlg = new SecurityKeySetDlg(m_factorUser, m_pMulFactorDbus, m_pPluginWidget);
    securityKeySetDlg->setAttribute(Qt::WA_DeleteOnClose);
    int ret = securityKeySetDlg->exec();
}

void UserInfo::showChangeUserNicknameDialog()
{
    if (allUserInfoMap.keys().contains(g_get_user_name())){
        QStringList names;
        QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
        for (; it != allUserInfoMap.end(); it++){
            UserInfomation user = it.value();

            if (QString::compare(user.username, g_get_user_name()) == 0)
                continue;

            names.append(user.username);
            names.append(user.realname);
        }

        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        ChangeUserNickname dialog(user.realname, names, user.objpath , m_pPluginWidget);
        dialog.exec();

    } else {
        qWarning() << "User Data Error When Change User Type";
    }

    _acquireAllUsersInfo();
}

void UserInfo::showChangeUserLogoDialog(QString username, UtilsForUserinfo *utilsUser)
{
    if (allUserInfoMap.keys().contains(username)) {
        UserInfomation user = allUserInfoMap.value(username);
        ChangeUserLogo *dialog = new ChangeUserLogo(user.realname, user.username, user.iconfile, m_pPluginWidget);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        //窗体显示在中间
        int move_x = m_pPluginWidget->topLevelWidget()->x() + (m_pPluginWidget->topLevelWidget()->width() - dialog->width())/2;
        int move_y = m_pPluginWidget->topLevelWidget()->y() + (m_pPluginWidget->topLevelWidget()->height() - dialog->height())/2;
        move_x = move_x > 0 ? move_x : 0;
        move_y = move_y > 0 ? move_y : 0;
        dialog->move(move_x, move_y);

        qDebug() << user.iconfile << ";" << __LINE__;
        connect(dialog, &ChangeUserLogo::confirmChanged, this, [=](QString filename, bool systemFaces) {

            QString reFilename = renameFaceName(filename, user.username);
            // 系统头像或历史头像不进行重命名，直接拷贝
            if (systemFaces || filename.contains(CUSTOMFACEPATH)) {
                reFilename = filename.split("/").at(filename.split("/").count() - 1);
            }

            qDebug() << "renamed filename = " << reFilename;
            if (g_get_user_name() == user.username) {
                QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                                      user.objpath,
                                                                      "org.freedesktop.Accounts.User",
                                                                      "SetIconFileRename");
                message << filename << reFilename;
                QDBusMessage response = QDBusConnection::systemBus().call(message);
                if (response.type() == QDBusMessage::ErrorMessage){
                    qDebug() << "setIconFileRename failed" << filename;
                }
            }
        });

        connect(dialog, &ChangeUserLogo::deleteLogos, this, [=](QStringList filenameList){
            foreach (QString filename, filenameList) {
                QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                                      user.objpath,
                                                                      "org.freedesktop.Accounts.User",
                                                                      "DelIconFile");
                message << filename;
                QDBusMessage response = QDBusConnection::systemBus().call(message);
                if (response.type() == QDBusMessage::ErrorMessage){
                    qDebug() << "call DelIconFile failed" << response.errorMessage();
                }
            }
        });
        dialog->show();
    } else {
        qWarning() << "User Info Data Error When Change User Pwd";
    }

    _acquireAllUsersInfo();
}

QString UserInfo::renameFaceName(QString fullFilename, QString username)
{
    // 拷贝文件名 原名-用户名-时间戳
    QDateTime time = QDateTime::currentDateTime();
    QString filename = fullFilename.split("/").at(fullFilename.split("/").count()-1);
    QString suffix = "." + filename.split(".").at(filename.split(".").count() - 1);
    filename = filename.left(filename.count() - suffix.count());
    qDebug() << filename;

    return filename + "-" + username + "-" + time.toString("yyyyMMddhhmmss") + suffix;
}

bool UserInfo::changeOtherUserPassword(const QString &programPath, const QString &username, const QString &newPassword, int *resultCode, QString *resultMsg)
{
    QProcess process;

    /* 启动程序，不传递任何命令行参数 */
    QString program = "/usr/bin/pkexec";
    QStringList arguments;
    arguments << "--disable-internal-agent" << programPath;

    process.start(program, arguments);
    if (!process.waitForStarted(5000)) {
        qWarning() << "Failed to start process:" << process.errorString();
        return false;
    }

    /* 编码密码为 UTF-8 */
    QByteArray usernameBytes = username.toUtf8();
    QByteArray newPwdBytes = newPassword.toUtf8();

    /* 构造二进制数据包 */
    QByteArray packet;
    packet.reserve(8 + usernameBytes.size() + newPwdBytes.size());

    /* 旧密码长度（4字节，小端序） */
    uint32_t oldLen = static_cast<uint32_t>(usernameBytes.size());
    packet.append(reinterpret_cast<const char*>(&oldLen), sizeof(oldLen));
    packet.append(usernameBytes);

    /* 新密码长度（4字节，小端序） */
    uint32_t newLen = static_cast<uint32_t>(newPwdBytes.size());
    packet.append(reinterpret_cast<const char*>(&newLen), sizeof(newLen));
    packet.append(newPwdBytes);

    /* 写入 stdin */
    process.write(packet);
    process.closeWriteChannel();  /* 关闭写端，通知子进程数据发送完毕 */

    /* 等待子进程完成 */
    if (!process.waitForFinished(30000)) {
        qWarning() << "Process timeout or failed:" << process.errorString();
        process.kill();
        return false;
    }

    /* 读取 stdout 返回的结果 */
    QByteArray output = process.readAllStandardOutput();
    if (output.size() < 8) {
        qWarning() << "Invalid response from pipe, too short:" << output.size();
        return false;
    }

    /* 解析结果码 */
    uint32_t code = 0;
    memcpy(&code, output.constData(), sizeof(code));
    *resultCode = static_cast<int>(code);

    /* 解析消息长度 */
    uint32_t msgLen = 0;
    memcpy(&msgLen, output.constData() + 4, sizeof(msgLen));

    /* 解析消息 */
    if (msgLen > 0 && output.size() >= static_cast<int>(8 + msgLen)) {
        *resultMsg = QString::fromUtf8(output.constData() + 8, msgLen);
    } else {
        *resultMsg = QString();
    }

    return true;
}

void UserInfo::showChangeUserPwdDialog(QString pName)
{
    if (allUserInfoMap.keys().contains(pName)) {
        UserInfomation user = allUserInfoMap.value(pName);
        ChangeUserPwd *dialog = new ChangeUserPwd(pName, m_pPluginWidget);
        QStringList objectpaths = sysdispatcher->list_cached_users();
        qint64 uid = 0;
        for (QString objectpath : objectpaths){
            UserInfomation user;
            user = _acquireUserInfo(objectpath);
            if (user.username == pName) {
                uid = user.uid;
            }
        }

        connect(dialog, &ChangeUserPwd::changeOtherUserPwd, this, [=](QString pwd) {
            QString programPath = QString("/usr/bin/changeotheruserpwd");
            int resultCode = -1;
            QString resultMsg;

            if (changeOtherUserPassword(programPath, pName, pwd, &resultCode, &resultMsg)) {
                qDebug() << "Result code:" << resultCode;
                qDebug() << "Message:" << resultMsg;
            }
        });

        connect(dialog, &ChangeUserPwd::changeOtherUserPrompt, this, [=](const QString &prompt) {
            setPasswordTips(uid, prompt);
        });

        dialog->exec();

    } else {
        qWarning() << "User Info Data Error When Change User Pwd";
    }
}

void UserInfo::showChangeUserTypeDialog(QString u)
{
    if (allUserInfoMap.keys().contains(u)){
        UserInfomation user = allUserInfoMap.value(u);

        ChangeUserType *dialog = new ChangeUserType(m_pPluginWidget);
        dialog->requireUserInfo(user.iconfile, user.realname, user.accounttype, _accountTypeIntToString(user.accounttype));
        connect(dialog, &ChangeUserType::confirmChanged, this, [=](int id){
            QDBusReply<bool> reply = userSettingDbus->call("setAccountType", user.objpath, id);
            if (reply) {
                //告知有用户的类型发生变化
                emit userTypeChanged(u);
            }
        });
        dialog->exec();
    } else {
        qWarning() << "User Data Error When Change User Nickname";
    }

    _acquireAllUsersInfo();
}

void UserInfo::setUserConnect()
{
    connect(m_pCurrentUserlogoBtn, &KLabel::clicked, this, [=]{
        UkccCommon::buriedSettings(name(), m_pCurrentUserlogoBtn->objectName(), QString("clicked"));

        showChangeUserLogoDialog(QString(g_get_user_name()), nullptr);
    });

    connect(m_pChangeNickNameBtn, &KPushButton::clicked, this, [=]{
        UkccCommon::buriedSettings(name(), m_pChangeNickNameBtn->objectName(), QString("clicked"));
        showChangeUserNicknameDialog();
    });

    connect(m_pChangeCurrentPwdBtn, &QPushButton::clicked, this, [=]{
        UkccCommon::buriedSettings(name(), m_pChangeCurrentPwdBtn->objectName(), QString("clicked"));

        showChangeUserPwdDialog(QString(g_get_user_name()));
    });

    connect(m_pChangeCurrentGroupsBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        UkccCommon::buriedSettings(name(), m_pChangeCurrentGroupsBtn->objectName(), QString("clicked"));

        showChangeGroupDialog();

    });
    //自动登录登录
    connect(m_pAutoLoginFrame, &ComboxWidget::currentIndexChanged, this, [=](int index) {
        UkccCommon::buriedSettings(name(), m_pAutoLoginFrame->objectName(), QString("settings"), m_pAutoLoginFrame->comboBox()->currentText());

        UserInfomation user = allUserInfoMap.value(g_get_user_name());
        QString autoUser = getAutomaticLogin();
        qDebug() << "Current Auto User:" << autoUser;
        //冲突，弹出提示窗口由用户选择
        if (index == 0 && !autoUser.isEmpty()){
            if (!openAutoLoginMsg(user.username)){
                m_pAutoLoginFrame->blockSignals(true);
                m_pAutoLoginFrame->setCurrentIndex(1);
                m_pAutoLoginFrame->blockSignals(false);
                return;
            }
        }
        polkitisShow =true;
        QDBusPendingCall call = userSettingDbus->asyncCall("setAutomaticLogin", user.objpath, index == 0 ? true : false);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call,this);
        connect(watcher,&QDBusPendingCallWatcher::finished,this, [=](QDBusPendingCallWatcher *call){
            QDBusPendingReply<bool> reply = *call;
            if (!reply.value()){
                m_pAutoLoginFrame->blockSignals(true);
                if (index == 0) {
                    m_pAutoLoginFrame->setCurrentIndex(1);
                } else {
                    m_pAutoLoginFrame->setCurrentIndex(0);
                }

                m_pAutoLoginFrame->blockSignals(false);
            }
            polkitisShow = false;
        });
    });

    //免密登录
    connect(m_pNopwdLoginFrame, &SwitchWidget::stateChanged, this, [=](bool checked) {
        polkitisShow = true;
        UkccCommon::buriedSettings(name(), m_pNopwdLoginFrame->objectName(), QString("settings"), checked ? "true":"false");

        UserInfomation user = allUserInfoMap.value(g_get_user_name());
        QDBusPendingCall call = userSettingDbus->asyncCall("setNoPwdLoginStatus", checked, user.username);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher,&QDBusPendingCallWatcher::finished,this, [=](QDBusPendingCallWatcher *call){
            QDBusPendingReply<bool> reply = *call;
            if (reply.value() == false) {
                m_pNopwdLoginFrame->blockSignals(true);
                m_pNopwdLoginFrame->setChecked(!checked);
                m_pNopwdLoginFrame->blockSignals(false);
            }
            polkitisShow = false;
        });
    });

    connect(m_pAddUserBtn, &AddButton::clicked, [=]{
        UkccCommon::buriedSettings(name(), m_pAddUserBtn->objectName(), QString("settings"));

        showCreateUserNewDialog();
    });

    connect(m_pLoginMgrInterface, SIGNAL(UserNew(quint32,QDBusObjectPath)), this, SLOT(onUserNew(quint32,QDBusObjectPath)));
    connect(m_pLoginMgrInterface, SIGNAL(UserRemoved(quint32,QDBusObjectPath)), this, SLOT(onUserRemove(quint32,QDBusObjectPath)));
}

void UserInfo::setUserDBusPropertyConnect(const QString pObjPath)
{
    QDBusInterface iproperty("org.freedesktop.Accounts",
                             pObjPath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());

    iproperty.connection().connect("org.freedesktop.Accounts", pObjPath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    this, SLOT(currentUserPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));

    QDBusInterface ukccProperty("org.ukui.groupmanager",
                                "/org/ukui/groupmanager",
                                "org.ukui.groupmanager.interface",
                                QDBusConnection::systemBus());
    ukccProperty.connection().connect("org.ukui.groupmanager", "/", "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                       this, SLOT(ukccPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));
}

void UserInfo::ukccPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList)
{
    qDebug() << "------------key:" << propertyMap.keys();
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("NoPwdLoginStatus")) {
        m_pNopwdLoginFrame->blockSignals(true);
        m_pNopwdLoginFrame->setChecked(getNoPwdStatus());
        m_pNopwdLoginFrame->blockSignals(false);
    }
}

void UserInfo::currentUserPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList)
{
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("AutomaticLogin") && getuid()){
        bool current = propertyMap.value("AutomaticLogin").toBool();

        if (m_pAutoLoginFrame->comboBox()->currentIndex() == 0 && !current) {
            m_pAutoLoginFrame->blockSignals(true);
            m_pAutoLoginFrame->setCurrentIndex(1);
            m_pAutoLoginFrame->blockSignals(false);
        } else if (m_pAutoLoginFrame->comboBox()->currentIndex() == 1 && current) {
            m_pAutoLoginFrame->blockSignals(true);
            m_pAutoLoginFrame->setCurrentIndex(0);
            m_pAutoLoginFrame->blockSignals(false);
        }
    }

    if (propertyMap.keys().contains("RealName") && getuid()) {
        QString current = propertyMap.value("RealName").toString();
        if (QString::compare(current, m_pCurrentNickNameLabel->text()) != 0) {
            //更新用户昵称
            m_pCurrentNickNameLabel->setText(current);
        }
    }

    if (propertyMap.keys().contains("IconFile") && getuid()){
        QString current = propertyMap.value("IconFile").toString();
        QPixmap iconfile = UkccCommon::makeRoundLogo(current, m_pCurrentUserlogoBtn->width(), m_pCurrentUserlogoBtn->height(), m_pCurrentUserlogoBtn->width()/2, m_pCurrentUserlogoBtn->devicePixelRatioF());
        m_pCurrentUserlogoBtn->setPixmap(iconfile, m_pCurrentUserlogoBtn->width(), m_pCurrentUserlogoBtn->height());
    }

    if (propertyMap.keys().contains("AccountType") && getuid()){
        bool current = propertyMap.value("AccountType").toBool();
        //刷新当前用户类型
        QString cType = _accountTypeIntToString(current);
        m_pCurrentUserTypeLabel->setText(cType);
        showMessageBox();
    }
    //刷新用户数据
    _acquireAllUsersInfo();

}

void UserInfo::showMessageBox()
{
    QMessageBox msg(qApp->activeWindow());
    msg.setIcon(QMessageBox::Warning);

    msg.setText(tr("The account type of “%1” has been modified, will take effect after logout, whether to logout?").arg(m_pCurrentNickNameLabel->text()));
    msg.addButton(tr("logout later"), QMessageBox::NoRole);
    msg.addButton(tr("logout now"), QMessageBox::ApplyRole);

    int ret = msg.exec();

    if (ret == 1) {
        system("ukui-session-tools --logout");
    }
    return;
}

void UserInfo::newUserCreateDoneSlot(QDBusObjectPath op)
{
    qDebug() << "newUserCreateDoneSlot " << op.path();
    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            op.path(),
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        QString username = propertyMap.find("UserName").value().toString();
        bool systemAccount = propertyMap.find("SystemAccount").value().toBool();
        qDebug() << op.path() << "username = " << username << "systemAccount = " << systemAccount;
        if (username.compare("lightdm") == 0) {
            return;
        }
    }
    //刷新用户数据
    _acquireAllUsersInfo();

    UserInfomation user;
    user = _acquireUserInfo(op.path());

    buildItemForUsersAndSetConnect(user);

    //
    emit userTypeChanged(user.username);
}

void UserInfo::existsUserDeleteDoneSlot(QDBusObjectPath op)
{
    //刷新用户数据
    _acquireAllUsersInfo();

    QObjectList list = m_pOthersFrame->children();
    foreach (QObject * obj, list) {
        if (obj->objectName() == op.path()){
            QFrame * f = qobject_cast<QFrame *>(obj);
            f->setParent(NULL);
            m_pOthersFrame->removeWidget(f);

            f->hide();
        }
    }

    if (m_pOthersFrame->layout()->count() == 1) {
        m_pAddUserBtn->setRadiusType(UkccFrame::Around);
    } else {
        m_pAddUserBtn->setRadiusType(UkccFrame::Bottom);
    }

    emit userTypeChanged("");
}

bool UserInfo::isLastAdmin(QString uname)
{
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return false;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");
    int num = users.length();

    if (users.contains(uname)){
        if (num > 1){
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

QString UserInfo::getAutomaticLogin()
{
    QString filename = "/etc/lightdm/lightdm.conf";
    QSettings *autoSettings = new QSettings(filename, QSettings::IniFormat, this);
    autoSettings->beginGroup("SeatDefaults");

    QString autoUser = autoSettings->value("autologin-user", "").toString();

    autoSettings->endGroup();

    return autoUser;
}

bool UserInfo::openAutoLoginMsg(const QString &userName)
{
    QString autoLoginedUser = this->getAutomaticLogin();
    bool res = true;
    int  ret;
    if (!autoLoginedUser.isEmpty() && userName != autoLoginedUser) {
        QMessageBox msg(this->m_pPluginWidget);
        msg.setWindowTitle(tr("Hint"));
        msg.setText(tr("The system only allows one user to log in automatically."
                       "After it is turned on, the automatic login of other users will be turned off."
                       "Is it turned on?"));
        QPushButton *confirmBtn = msg.addButton(tr("Trun on"), QMessageBox::AcceptRole);
        QPushButton *cancelBtn =msg.addButton(tr("Close on"), QMessageBox::RejectRole);
        msg.setDefaultButton(confirmBtn);

        ret = msg.exec();

        switch (ret) {
          case QMessageBox::AcceptRole:
              res = true;
              break;
          case QMessageBox::RejectRole:
              res = false;
              break;
        }
    }
    return res;
}


/**3.1****end**************/


QString UserInfo::_accountTypeIntToString(int type)
{
    QString atype;
    if (type == STANDARDUSER)
        atype = tr("Standard");
    else if (type == ADMINISTRATOR)
        atype = tr("Admin");
    else if (type == ROOT)
        atype = tr("root");

    return atype;
}

void UserInfo::_acquireAllUsersInfo()
{

    mUserName = qgetenv("USER");
    if (mUserName.isEmpty()) {
        mUserName = qgetenv("USERNAME");
    }

    QStringList objectpaths = sysdispatcher->list_cached_users();

    //初始化用户信息QMap
    allUserInfoMap.clear();

    //root
    if (!getuid()) {
        UserInfomation root;
        root.username = g_get_user_name();
        root.realname = g_get_real_name();
        root.current = true;
        root.logined = true;
        root.autologin = false;
        root.uid = 0;
        root.accounttype = ADMINISTRATOR;
        root.iconfile = DEFAULTFACE;
        allUserInfoMap.insert(root.username, root);
    }

    for (QString objectpath : objectpaths){
        UserInfomation user;
        user = _acquireUserInfo(objectpath);
        allUserInfoMap.insert(user.username, user);
    }

    if (allUserInfoMap.isEmpty()) {
        m_pCurrentFrame->setVisible(false);
        m_pAutoLoginFrame->setVisible(false);
    } else {
        m_pCurrentFrame->setVisible(true);
        m_pAutoLoginFrame->setVisible(true);
    }
}

void UserInfo::onUserNew(quint32 uid, QDBusObjectPath opUser)
{
    qDebug() << "logind userNew " << uid << opUser.path();
    QStringList loginedusers = getLoginedUsers();
    foreach (UtilsForUserinfo *utils, utilsUsers) {
        utils->refreshTypeStatus(!isLastAdmin(utils->m_username) && !loginedusers.contains(utils->m_username));
    }
}

void UserInfo::onUserRemove(quint32 uid, QDBusObjectPath opUser)
{
    qDebug() << "logind userRemove " << uid << opUser.path();
    QStringList loginedusers = getLoginedUsers();
    foreach (UtilsForUserinfo *utils, utilsUsers) {
        utils->refreshTypeStatus(!isLastAdmin(utils->m_username) && !loginedusers.contains(utils->m_username));
    }
}

UserInfomation UserInfo::_acquireUserInfo(QString objpath)
{
    UserInfomation user;

    //默认值
    user.current = false;
    user.logined = false;
    user.autologin = false;
    user.objpath = objpath;

    QStringList loginedusers = getLoginedUsers();

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){

        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        user.realname = propertyMap.find("RealName").value().toString();
        user.accounttype = propertyMap.find("AccountType").value().toInt();

        if (user.realname.isEmpty()){
            user.realname = propertyMap.find("UserName").value().toString();
        }

        if (user.username == QString(g_get_user_name())) {
            user.current = true;
            user.logined = true;
            user.noPwdLogin = getNoPwdStatus();
            m_currentUserType = user.accounttype;
            qDebug() << m_currentUserType << user.username << user.accounttype;
        }

        user.logined = loginedusers.contains(user.username);

        user.iconfile = propertyMap.find("IconFile").value().toString();
        user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
        user.autologin = propertyMap.find("AutomaticLogin").value().toBool();
        user.objpath = objpath;

        //用户头像文件不存在 使用DEFAULTFACE
        std::string iconpathString = user.iconfile.toStdString();
        const char *iconpath = iconpathString.c_str();
        if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
            user.iconfile = DEFAULTFACE;
        }
    }
    else
        qDebug() << "reply failed";

    delete iproperty;
    iproperty = nullptr;

    return user;
}

QStringList UserInfo::getLoginedUsers()
{
    m_loginedUser.clear();
    qRegisterMetaType<LoginedUsers>("LoginedUsers");
    qDBusRegisterMetaType<LoginedUsers>();
    QDBusInterface loginInterface("org.freedesktop.login1",
                                  "/org/freedesktop/login1",
                                  "org.freedesktop.login1.Manager",
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
//        qDebug() << "create interface sucess";
    }

    QDBusMessage result = loginInterface.call("ListUsers");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusArgument dbvFirst = first.value<QDBusArgument>();
    QVariant vFirst = dbvFirst.asVariant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    QVector<LoginedUsers> loginedUsers;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        LoginedUsers user;
        dbusArgs >> user;
        loginedUsers.push_back(user);
    }
    dbusArgs.endArray();

    for (LoginedUsers user : loginedUsers) {

        QDBusInterface userPertyInterface("org.freedesktop.login1",
                                          user.objpath.path(),
                                          "org.freedesktop.DBus.Properties",
                                          QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = userPertyInterface.call("Get", "org.freedesktop.login1.User", "State");
        if (reply.isValid()) {
            QString status = reply.value().toString();
            if ("closing" != status) {
                m_loginedUser.append(user.userName);
            }
        }
    }
    return m_loginedUser;
}

void UserInfo::showChangeGroupDialog()
{
    changeUserGroup *dialog = new changeUserGroup(m_pPluginWidget);
    connect(dialog, &changeUserGroup::showDialog, this, [=](){
        qDebug() << "user type = " << m_currentUserType << __FUNCTION__;
        if (m_currentUserType == STANDARDUSER) {
            dialog->setEnableStatus(false);
        }
        dialog->exec();
    });
}

bool UserInfo::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()== UkccArgEvent::TYPE) {
        UkccArgEvent *str = dynamic_cast<UkccArgEvent* >(event);
        if (str != nullptr) {
            qDebug() << "event string = " << str->str();
            // 修改用户头像
            if (str->str() == "userLogo") {
                if (m_pCurrentUserlogoBtn->isEnabled()) {
                    showChangeUserLogoDialog(QString(g_get_user_name()), nullptr);
                } else {
                    QMessageBox::warning(m_pPluginWidget, tr("Warning"), tr("This function can not be edit!"));
                }
            }

            // 修改用户昵称
            if (str->str() == "userNickname") {
                if (m_pChangeNickNameBtn->isEnabled()) {
                    showChangeUserNicknameDialog();
                } else {
                    QMessageBox::warning(m_pPluginWidget, tr("Warning"), tr("This function can not be edit!"));
                }
            }

            // 新建用户
            if (str->str() == "createUser") {
                if (m_pAddUserBtn->isEnabled()) {
                    showCreateUserNewDialog();
                } else {
                    QMessageBox::warning(m_pPluginWidget, tr("Warning"), tr("This function can not be edit!"));
                }
            }

            // 修改密码
            if (str->str() == "password") {
                if (m_pChangeCurrentPwdBtn->isEnabled()) {
                    showChangeUserPwdDialog(QString(g_get_user_name()));
                } else {
                    QMessageBox::warning(m_pPluginWidget, tr("Warning"), tr("This function can not be edit!"));
                }
            }
        }
    }

    if (polkitisShow){
        if (event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseButtonPress
                || event->type() == QEvent::MouseButtonRelease) {
            return true;
        }
    }

    // KLabel 仅在鼠标松开时发 clicked；空格/回车需补一次等效释放才能打开改头像
    if (watched == m_pCurrentUserlogoBtn && event->type() == QEvent::KeyPress
            && m_pCurrentUserlogoBtn && m_pCurrentUserlogoBtn->isEnabled()) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();
        if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
            if (!keyEvent->isAutoRepeat()) {
                QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                                         m_pCurrentUserlogoBtn->rect().center(),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);
                QApplication::sendEvent(m_pCurrentUserlogoBtn, &releaseEvent);
            }
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

bool UserInfo::getNoPwdStatus()
{
    // 获取当前用户免密登录属性
    // 获取免密登录状态
    QDBusReply<QString> noPwdres;
    if (!userSettingDbus->isValid()) {
        qDebug() << "Create dbus error: " << QDBusConnection::systemBus().lastError();
        return false;
    }

    noPwdres  = userSettingDbus->call("getNoPwdLoginStatus");
    if (!noPwdres.isValid() || noPwdres.value().isEmpty()) {
        qDebug() << "noPwdres.error() = " << noPwdres.error() << "; noPwdres.value() = " << noPwdres.value();
        return false;
    }

    QStringList tmp = noPwdres.value().split(":", QString::SkipEmptyParts);

    QString noPasswdUsers = tmp.at(tmp.count()-1);
    QStringList noPasswdUsersList = noPasswdUsers.split(",", QString::SkipEmptyParts);
    foreach (QString noPasswdUser, noPasswdUsersList) {
        noPasswdUser.remove(QChar('\n'), Qt::CaseInsensitive);
        qDebug() << "nopasswduser:" << noPasswdUser;
        if (noPasswdUser.compare(mUserName) == 0) {
            return true;
        }
    }

    return false;
}
