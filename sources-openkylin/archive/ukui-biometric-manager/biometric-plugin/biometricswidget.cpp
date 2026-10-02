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
#include "biometricswidget.h"
#include "ui_biometricswidget.h"
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QToolButton>
#include <QMenu>
#include <QDebug>
#include <QDBusInterface>
#include <QLineEdit>
#include <QMessageBox>
#include <unistd.h>
#include "namelabel.h"
#include "changefeaturename.h"
#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"
#include "changeuserpwd.h"
#include "servicemanager.h"
#include "securitykeysetdlg.h"
#include "binddialog.h"
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <glib.h>
#include <kysdk/kysdk-system/libkysysinfo.h>
#define DEFAULTFACE "/usr/share/ukui/faces/default.png"

#define SECURITY_ANWSER_NUMBER (3)
#define ITEMHEIGH 61
BiometricsWidget::BiometricsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BiometricsWidget)
    , m_strBoundWechat("")
    , m_uniAuthService(new UniAuthService(this))
    , m_pSecurityQuestionAnswer(new SecurityQuestionAnswer(this))
{
    registerCustomTypes();
    
    ui->setupUi(this);
    initBioComonent();
}

BiometricsWidget::~BiometricsWidget()
{
    delete ui;
}

void BiometricsWidget::registerCustomTypes()
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QList<DeviceInfo>>();
    qDBusRegisterMetaType<QList<QDBusVariant>>();
}

void BiometricsWidget::initBioComonent()
{
    m_biometricProxy = new BiometricProxy(this);
    initSearchText();
    ui->enableBiometricFrame->setFrameShape(QFrame::NoFrame);
    ui->biometricTypeFrame->setFrameShape(QFrame::NoFrame);
    ui->biometricDeviceFrame->setFrameShape(QFrame::NoFrame);
    ui->enableBiometricFrame->setObjectName("enableBiometricFrame");
    serviceInterface = new QDBusInterface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647); /* 微秒 */

    m_pwdlessProxy = new PwdlessProxy(this);
    connect(m_pwdlessProxy, &PwdlessProxy::mfaStatusChanged, this, &BiometricsWidget::onMfaStatusChanged);

    ui->titleLabel->setContentsMargins(16, 0, 0, 0);

    addBioFeatureBtn = new EnrollBtn;
    KDK_EXTEND_ALL_INFO_FORMAT(
        addBioFeatureBtn, "LoginOptions", "BiometricsWidget", "add biometric feature button of LoginOptions dailog");
    addUserHorLayout = new QHBoxLayout();
    addUserHorLayout->setSpacing(8);
    addUserHorLayout->setContentsMargins(0, 0, 0, 0);
    addUserHorLayout->addWidget(addBioFeatureBtn);

    addUserFrame = new QFrame();
    addUserFrame->setObjectName("continue");
    addUserFrame->setMinimumSize(QSize(550, 60));
    addUserFrame->setMaximumSize(QSize(16777215, 60));
    addUserFrame->setFrameShape(QFrame::NoFrame);
    addUserFrame->setLayout(addUserHorLayout);

    //    addBioFeatureBtn->setProperty("useIconHighlightEffect", true);
    //    addBioFeatureBtn->setProperty("useIconHighlightEffect", 0x2);
    connect(addBioFeatureBtn, &EnrollBtn::clicked, [=] { showEnrollDialog(); });
    ui->addFeatureLayout->addWidget(addUserFrame);

    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->bioFeatureListWidget,
        "LoginOptions",
        "BiometricsWidget",
        "biometric feature listWidget of LoginOptions dailog");
    ui->bioFeatureListWidget->setStyleSheet("QListWidget{border-radius:0px;};\
                                            QListWidget::Item:hover{background:palette(base);border-radius:0px;}");
    ui->bioFeatureListWidget->setSpacing(0);

    if (biometricFeatureMap.count() < 3) {
        ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count() * ITEMHEIGH + 3);
    } else {
        ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count() * ITEMHEIGH + biometricFeatureMap.count());
    }

    connect(
        ui->biometrictypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onbiometricTypeBoxCurrentIndexChanged(int)));
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometrictypeBox, "LoginOptions", "BiometricsWidget", "biometric type combox of LoginOptions dailog");

    connect(
        ui->biometricDeviceBox_2,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(onbiometricDeviceBoxCurrentIndexChanged(int)));
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricDeviceBox_2, "LoginOptions", "BiometricsWidget", "biometric device combox of LoginOptions dailog");

    //    ui->toolButton->setProperty("isWindowButton", 0x1);
    //    ui->toolButton->setProperty("useIconHighlightEffect", 0x2);
    //    ui->toolButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    //     ui->toolButton->setObjectName("advancedSettingsBtn");
    //    menu = new QMenu();
    //    QAction *installAction = new QAction(tr("Advanced Settings"), this);
    //    connect(installAction, &QAction::triggered, this, [&]{
    //        QDBusInterface ifc("com.kylin.AppManager",
    //                           "/com/kylin/AppManager",
    //                           "com.kylin.AppManager",
    //                           QDBusConnection::sessionBus());
    //        if(ifc.isValid())
    //            ifc.call("LaunchApp", "/usr/share/applications/biometric-manager.desktop");
    //        else {
    //            QProcess process(this);
    //            process.startDetached("/usr/bin/biometric-manager");
    //        }
    //    });
    //    menu->addActions({installAction});
    //    ui->toolButton->setPopupMode(QToolButton::InstantPopup);
    //    ui->toolButton->setMenu(menu);
    connect(ui->advancedSettingsBtn, &QPushButton::clicked, this, [&] {
        QDBusInterface ifc(
            "com.kylin.AppManager", "/com/kylin/AppManager", "com.kylin.AppManager", QDBusConnection::sessionBus());
        if (ifc.isValid())
            ifc.call("LaunchApp", "/usr/share/applications/biometric-manager.desktop");
        else {
            QProcess process(this);
            process.startDetached("/usr/bin/biometric-manager");
        }
    });
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->advancedSettingsBtn,
        "LoginOptions",
        "BiometricsWidget",
        "login options advance button of LoginOptions dailog");

    updateDevice();
    onbiometricDeviceBoxCurrentIndexChanged(ui->biometricDeviceBox_2->currentIndex());
    if (getProductNameN90F3()) {
        getDeviceInfo();
        updateEnableTypeList();
        ui->settingsLabel->setContentsMargins(16, 0, 0, 0);
        connect(
            ui->authTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAuthTypeBoxCurrentIndexChanged(int)));
    } else {
        ui->defaultAuthTypeFrame->hide();
        ui->settingsLabel->hide();
    }

    if (m_biometricProxy && m_biometricProxy->isValid()) {
        connect(
            m_biometricProxy, &BiometricProxy::USBDeviceHotPlug, this, &BiometricsWidget::onBiometricUSBDeviceHotPlug);

        connect(m_biometricProxy, &BiometricProxy::FeatureChanged, this, &BiometricsWidget::onFeatureChanged);
    }
    ServiceManager *sm = ServiceManager::instance();
    connect(sm, &ServiceManager::serviceStatusChanged, this, &BiometricsWidget::onBiometricServiceStatus);

    QPalette paltte;
    paltte.setColor(QPalette::Text, Qt::gray);
    ui->qrCodeLoginlabel2->setPalette(paltte);
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->qrCodeLoginlabel2, "LoginOptions", "BiometricsWidget", "qrcode login tip label of LoginOptions dailog");
    ui->enableBiometricLabel2->setPalette(paltte);
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->enableBiometricLabel2,
        "LoginOptions",
        "BiometricsWidget",
        "enable biometric tip label of LoginOptions dailog");

    //    QString strDisplay1=ui->enableBiometricLabel2->text();
    //    QString text1 = ui->enableBiometricLabel2->text();
    //    QFontMetrics font1(strDisplay1);
    //    int font_size1 = font1.width(strDisplay1);
    //    if(font_size1 > 200)
    //    {
    //        //返回字符串末尾带省略号的字符串
    //        ui->enableBiometricLabel2->setToolTip(strDisplay1);
    //        strDisplay1 = font1.elidedText(text1, Qt::ElideRight, 195);
    //    }
    //    ui->enableBiometricLabel2->setText(strDisplay1);

    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->enableBiometricLabel, "LoginOptions", "BiometricsWidget", "enable biometric label of LoginOptions dailog");
    enableBiometricBtn = new KSwitchButton(ui->enableBiometricFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(
        enableBiometricBtn, "LoginOptions", "BiometricsWidget", "enable biometric button of LoginOptions dailog");
    enableBiometricBtn->setObjectName("enableBiometricBtn");

    mBiometricWatcher = nullptr;
    initDashboardBioAuthSection();
    ui->enableBiometricLayout->addWidget(enableBiometricBtn);
    connect(enableBiometricBtn, &KSwitchButton::stateChanged, [=](bool checked) {
        UkccCommon::buriedSettings(
            "Biometrics",
            enableBiometricBtn->objectName(),
            QString("clicked"),
            enableBiometricBtn->isChecked() ? "true" : "false");
        if (checked) {
            m_uniAuthService->setBioAuthStatus(0, true);
        } else {
            m_uniAuthService->setBioAuthStatus(0, false);
        }
    });

    ui->securityIssuePB->setObjectName("securityIssuePB");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->securityIssuePB, "LoginOptions", "BiometricsWidget", "set security issue button of LoginOptions dailog");
    connect(ui->securityIssuePB, &QPushButton::clicked, this, [=](bool checked) {
        UkccCommon::buriedSettings("Biometrics", ui->securityIssuePB->objectName(), QString("clicked"));
        Q_UNUSED(checked)
        if (polkitSecurityQuestions())
            showSecurityQuestionDialog();
    });

    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->passwordLabel, "LoginOptions", "BiometricsWidget", "change password label of LoginOptions dailog");
    ui->changePasswordPB->setObjectName("changePasswordPB");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->changePasswordPB, "LoginOptions", "BiometricsWidget", "change password button of LoginOptions dailog");
    connect(ui->changePasswordPB, &QPushButton::clicked, this, [=](bool checked) {
        UkccCommon::buriedSettings("Biometrics", ui->changePasswordPB->objectName(), QString("clicked"));
        Q_UNUSED(checked)
        showChangePwdDialog();
    });
    initUserInfo();

    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->qrCodeLoginlabel, "LoginOptions", "BiometricsWidget", "qrCode login label of LoginOptions dailog");
    ui->bindOrUnbindBtn->setObjectName("bindOrUnbindBtn");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->bindOrUnbindBtn, "LoginOptions", "BiometricsWidget", "bind wechat button of LoginOptions dailog");
    connect(ui->bindOrUnbindBtn, &QToolButton::clicked, this, [=]() {
        UkccCommon::buriedSettings(
            "Biometrics",
            ui->bindOrUnbindBtn->objectName(),
            QString("clicked"),
            ui->bindOrUnbindBtn->text().contains(tr("Binding")) ? "Binding WeChat" : "Unbind");
        QString strDrivName;
        int nDrivId;
        if (getCurUserQRCodeAcount(strDrivName, nDrivId).isEmpty()) {
            if (authoriyUserForQRCode(true)) {
                showQRCodeScanDialog();
            }
        } else {
            m_biometricProxy->deleteFeature(nDrivId, getuid(), 0, -1);
            QTimer::singleShot(0, this, [&, this]() { Q_EMIT this->updateQRCodeInfo(); });
        }
    });
    connect(this, &BiometricsWidget::updateQRCodeInfo, this, &BiometricsWidget::onUpdateQRCodeInfo);
    connect(this, &BiometricsWidget::updateFunctionsWidget, this, &BiometricsWidget::onUpdateFunctionsWidget);
    QTimer::singleShot(0, this, [&, this]() { Q_EMIT this->updateQRCodeInfo(); });
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->boundWechatKey, "LoginOptions", "BiometricsWidget", "bind wechat username label of LoginOptions dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->labelSecurityKey, "LoginOptions", "BiometricsWidget", "security key label of LoginOptions dailog");
    ui->pbSetSK->setObjectName("setSetSecurityKeyBtn");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->pbSetSK, "LoginOptions", "BiometricsWidget", "set securitykey button of LoginOptions dailog");
    connect(ui->pbSetSK, &QPushButton::clicked, this, &BiometricsWidget::onSetSecurityKey);

    ui->pbSetOtpKey->setObjectName("setSetOtpKeyBtn");
    connect(ui->pbSetOtpKey, &QPushButton::clicked, this, &BiometricsWidget::onSetOtpKey);

    m_mfaMessage = new MessageBar;
    m_mfaMessage->setMinimumSize(550, 36);
    m_mfaMessage->setMaximumSize(16777215, 36);
    m_mfaMessage->setContentsMargins(0,0,0,0);
    KLabel *tipLabel = new KLabel;
    tipLabel->setPixmap(QIcon::fromTheme("dialog-warning"), 16, 16);
    tipLabel->setText(tr("Two-factor authentication has been enabled for the system. If you want to use other login methods, please disable two-factor authentication first."));
    KBorderlessButton *borderlessBtn = new KBorderlessButton;
    borderlessBtn->setText(tr("Set"));
    connect(borderlessBtn, &KBorderlessButton::clicked, this, [=]() {
        QProcess process;
        process.startDetached("/usr/bin/ukui-control-center -m UserInfo");
    });

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(16,0,16,0);
    hlayout->addWidget(tipLabel);
    hlayout->addWidget(borderlessBtn);
    m_mfaMessage->setLayout(hlayout);
    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout*>(ui->biometricWidget->layout());
    if(vLayout){
        vLayout->insertWidget(1,m_mfaMessage);
    }

    if(m_pwdlessProxy && m_pwdlessProxy->mfaStatus()!= ""){
        m_mfaMessage->show();
    } else{
        m_mfaMessage->hide();
    }


    connect(this, &BiometricsWidget::updateOtpInfo, this, &BiometricsWidget::onUpdateOtpInfo);
    QTimer::singleShot(0, this, [&, this]() { Q_EMIT this->updateOtpInfo(); });

    readConfigures();
}

/**
 * @brief 获取设备列表并存储起来备用
 */
void BiometricsWidget::getDeviceInfo()
{
    // 检查 serviceInterface 是否存在
    if (!serviceInterface) {
        qWarning() << "getDeviceInfo: serviceInterface is null";
        deviceCount = 0;
        return;
    }
    
    // 检查 serviceInterface 是否有效（DBus 连接是否正常）
    if (!serviceInterface->isValid()) {
        qWarning() << "getDeviceInfo: serviceInterface is invalid, error:" << serviceInterface->lastError();
        deviceCount = 0;
        return;
    }
    
    QVariant variant;
    QDBusArgument argument;
    QList<QDBusVariant> qlist;
    QDBusVariant item;
    DeviceInfo *deviceInfo;

    /* 返回值为 i -- int 和 av -- array of variant */
    QDBusPendingReply<int, QList<QDBusVariant>> reply = serviceInterface->call("GetDrvList");
    
    // 等待 DBus 调用完成
    reply.waitForFinished();
    
    if (reply.isError()) {
        qWarning() << "getDeviceInfo: DBus error:" << reply.error();
        deviceCount = 0;
        return;
    }

    /* 解析 DBus 返回值，reply 有两个返回值，都是 QVariant 类型 */
    variant = reply.argumentAt(0);             /* 得到第一个返回值 */
    deviceCount = variant.value<int>();        /* 解封装得到设备个数 */
    variant = reply.argumentAt(1);             /* 得到第二个返回值 */
    argument = variant.value<QDBusArgument>(); /* 解封装，获取QDBusArgument对象 */
    argument >> qlist; /* 使用运算符重载提取 argument 对象里面存储的列表对象 */

    for (int i = 0; i < __MAX_NR_BIOTYPES; i++)
        deviceInfosMap[i].clear();
    ;

    for (int i = 0; i < deviceCount; i++) {
        item = qlist[i];          /* 取出一个元素 */
        variant = item.variant(); /* 转为普通QVariant对象 */
        /* 解封装得到 QDBusArgument 对象 */
        argument = variant.value<QDBusArgument>();
        deviceInfo = new DeviceInfo();
        argument >> *deviceInfo; /* 提取最终的 DeviceInfo 结构体 */

        deviceInfosMap[DeviceType::bioTypeToIndex(deviceInfo->deviceType)].append(deviceInfo);
    }
}

void BiometricsWidget::initDashboardBioAuthSection()
{
    // if (m_uniAuthService && m_uniAuthService->isActivatable()) {
    connect(
        m_uniAuthService, &UniAuthService::bioAuthStatusChanged, this, &BiometricsWidget::setBiometricDeviceVisible);
    setBiometricDeviceVisible(getenv("USER"), 0, m_uniAuthService->getBioAuthStatus(getenv("USER"), 0));
    //}
}

void BiometricsWidget::initUserInfo()
{
    QDBusInterface iface(
        "org.freedesktop.Accounts",
        "/org/freedesktop/Accounts",
        "org.freedesktop.Accounts",
        QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)getuid());
    if (!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        m_user.current = true;
        m_user.logined = false;
        m_user.autologin = false;

        QString path = userPath.value().path();
        QDBusInterface iproperty(
            "org.freedesktop.Accounts", path, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QMap<QString, QVariant>> reply = iproperty.call("GetAll", "org.freedesktop.Accounts.User");
        if (reply.isValid()) {
            QMap<QString, QVariant> propertyMap;
            propertyMap = reply.value();
            m_user.username = propertyMap.find("UserName").value().toString();
            m_user.realname = propertyMap.find("RealName").value().toString();

            if (m_user.realname.isEmpty()) {
                m_user.realname = propertyMap.find("UserName").value().toString();
            }

            if (m_user.username == QString(g_get_user_name())) {
                m_user.current = true;
                m_user.logined = true;
                m_user.noPwdLogin = false;
            }
            m_user.accounttype = propertyMap.find("AccountType").value().toInt();
            m_user.iconfile = propertyMap.find("IconFile").value().toString();
            if (m_user.iconfile.isEmpty()) {
                m_user.iconfile = DEFAULTFACE;
            }
            char *iconpath = m_user.iconfile.toLatin1().data();
            if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)) {
                m_user.iconfile = DEFAULTFACE;
            }

            m_user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
            m_user.uid = propertyMap.find("Uid").value().toInt();
            m_user.autologin = false;
            m_user.objpath = path;

        } else
            qDebug() << "reply failed";
    }
}

QString BiometricsWidget::_accountTypeIntToString(int type)
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

void BiometricsWidget::showChangePwdDialog()
{
    ChangeUserPwd dialog(m_user.username, this);
    dialog.exec();
}

bool BiometricsWidget::polkitSecurityQuestions()
{
    PolkitQt1::Authority::Result result;
    // PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
        "org.ukui.biometric.manager.security.questions",
        PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
        PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) { // 认证通过
        qDebug() << QString("operation authorized");
        return true;
    } else {
        qDebug() << QString("not authorized");
        return false;
    }
}

void BiometricsWidget::showSecurityQuestionDialog()
{
    if (!m_pSecQueDig) {
        m_pSecQueDig = new KSecurityQuestionDialog(this);
        m_pSecQueDig->setFixedSize(424, 476);
        m_pSecQueDig->addSecurityQuestionItem(SECURITY_ANWSER_NUMBER, true);
        QList<QuestionInfo> listQuestionInfo
            = m_pSecurityQuestionAnswer->GetPresetSecurityQuestions(getenv("LANGUAGE"));

        QStringList listQuestion;
        for (auto questionInfo : listQuestionInfo) {
            qDebug() << questionInfo.nQuestionId << questionInfo.strQuestionText;
            listQuestion << questionInfo.strQuestionText;
        }

        m_pSecQueDig->initQustionCombox(listQuestion);

        for (int i = 0; i < SECURITY_ANWSER_NUMBER; ++i) {
            if (m_pSecQueDig->questionCombox(i))
                m_pSecQueDig->questionCombox(i)->setFocusPolicy(Qt::NoFocus);
        }
        m_pSecQueDig->confirmButton()->setFocusPolicy(Qt::NoFocus);
        m_pSecQueDig->cancelButton()->setFocusPolicy(Qt::NoFocus);
        m_pSecQueDig->closeButton()->setFocusPolicy(Qt::NoFocus);
        if (m_pSecQueDig->questionCombox(0))
            m_pSecQueDig->setFocusProxy(m_pSecQueDig->questionCombox(0));

        connect(m_pSecQueDig->confirmButton(), &KPushButton::clicked, this, [=] {
            QList<AnswerInfo> listAnswerInfo;
            for (int i = 0; i < SECURITY_ANWSER_NUMBER; ++i) {
                AnswerInfo answerInfo;
                if (m_pSecQueDig->questionCombox(i))
                    answerInfo.nQuestionId = m_pSecQueDig->questionCombox(i)->currentIndex();
                if (m_pSecQueDig->answerLineedit(i)) {
                    answerInfo.strAnswerText
                        = m_pSecurityQuestionAnswer->EncodeSecurityInfo(m_pSecQueDig->answerLineedit(i)->text());
                }
                listAnswerInfo.append(answerInfo);
            }
            int nRet = m_pSecurityQuestionAnswer->BindUserSecurityAnswers(getuid(), listAnswerInfo);
            qDebug() << "Bind ret:" << nRet;
            m_pSecQueDig->close();
        });
        connect(m_pSecQueDig->cancelButton(), &KPushButton::clicked, this, [=]() { m_pSecQueDig->close(); });
        connect(m_pSecQueDig, &KSecurityQuestionDialog::finished, this, [=]() {
            for (int i = 0; i < SECURITY_ANWSER_NUMBER; ++i) {
                if (m_pSecQueDig->answerLineedit(i))
                    m_pSecQueDig->answerLineedit(i)->clear();
                if (m_pSecQueDig->tipsLabel(i))
                    m_pSecQueDig->tipsLabel(i)->clear();
                if (m_pSecQueDig->questionCombox(i))
                    m_pSecQueDig->questionCombox(i)->setCurrentIndex(i);
            }
        });
    }
    m_pSecQueDig->exec();
}

void BiometricsWidget::onBiometricUSBDeviceHotPlug(int drvid, int action, int deviceNum)
{
    switch (action) {
        case ACTION_ATTACHED: {
            // 插入设备后，需要更新设备列表
            QTimer::singleShot(500, this, [&, this]() {
                int savedDeviceId = -1;
                if (this->currentDevice)
                    savedDeviceId = this->currentDevice->id;
                this->updateDevice();
                if (savedDeviceId >= 0)
                    this->setCurrentDevice(savedDeviceId);
                this->onbiometricDeviceBoxCurrentIndexChanged(this->ui->biometricDeviceBox_2->currentIndex());
                DeviceList devList;
                DeviceMap::iterator itDevMap = deviceMap.begin();
                for (; itDevMap != deviceMap.end(); itDevMap++) {
                    devList.append(itDevMap.value());
                }
                Q_EMIT driverAttachedChanged(true, devList);
            });
        } break;
        case ACTION_DETACHED: {
            updateDevice();
            DeviceList devList;
            DeviceMap::iterator itDevMap = deviceMap.begin();
            for (; itDevMap != deviceMap.end(); itDevMap++) {
                devList.append(itDevMap.value());
            }
            Q_EMIT driverAttachedChanged(false, devList);
        } break;
        default:
            break;
    }
    getDeviceInfo();
    updateEnableTypeList();
}

void BiometricsWidget::onBiometricServiceStatus(bool isActive)
{
    if (isActive) {
        QTimer::singleShot(500, this, [&, this]() {
            int savedDeviceId = -1;
            if (this->currentDevice)
                savedDeviceId = this->currentDevice->id;
            this->updateDevice();
            if (savedDeviceId >= 0)
                this->setCurrentDevice(savedDeviceId);
            this->onbiometricDeviceBoxCurrentIndexChanged(this->ui->biometricDeviceBox_2->currentIndex());
            DeviceList devList;
            DeviceMap::iterator itDevMap = deviceMap.begin();
            for (; itDevMap != deviceMap.end(); itDevMap++) {
                devList.append(itDevMap.value());
            }
            Q_EMIT driverAttachedChanged(true, devList);
            getDeviceInfo();
            updateEnableTypeList();
        });
    } else {
        Q_EMIT driverAttachedChanged(false, DeviceList());
    }
}

void BiometricsWidget::setBiometricDeviceVisible(QString userName, int authType, bool visible)
{
    if (userName != getenv("USER") || authType != 0)
        return;
    enableBiometricBtn->setChecked(visible);
    enableAuth = visible;
    if (visible) {
        ui->biometricTypeFrame->show();
        ui->biometricDeviceFrame->show();
        ui->bioFeatureListWidget->show();
        addBioFeatureBtn->show();
        addUserFrame->show();
        ui->line1->show();
        ui->line2->show();
        ui->line3->show();
    } else {
        ui->biometricTypeFrame->hide();
        ui->biometricDeviceFrame->hide();
        ui->bioFeatureListWidget->hide();
        addBioFeatureBtn->hide();
        addUserFrame->hide();
        ui->line1->hide();
        ui->line2->hide();
        ui->line3->hide();
    }
    updateFeatureList();
}

void BiometricsWidget::updateDevice()
{
    deviceMap.clear();
    DeviceList deviceList = m_biometricProxy->GetDevList();

    for (auto pDeviceInfo : deviceList) {
        deviceMap[pDeviceInfo->deviceType].push_back(pDeviceInfo);
    }
    ui->biometrictypeBox->clear();
    for (int type = 0; type <= DeviceType::VoicePrint; type++) {
        ui->biometrictypeBox->addItem(DeviceType::getDeviceType_tr(type), type);
    }
    if (deviceMap.size() > 0) {
        int index = deviceMap.keys().at(0);
        if (deviceMap[index].size() > 0) {
            setCurrentDevice(deviceMap[index].at(0));
        }
    }
}

void BiometricsWidget::setCurrentDevice(int drvid)
{
    DeviceInfoPtr pDeviceInfo = findDeviceById(drvid);
    if (pDeviceInfo) {
        setCurrentDevice(pDeviceInfo);
    }
}

void BiometricsWidget::setCurrentDevice(const QString &deviceName)
{
    DeviceInfoPtr pDeviceInfo = findDeviceByName(deviceName);
    if (pDeviceInfo) {
        setCurrentDevice(pDeviceInfo);
    }
}

void BiometricsWidget::setCurrentDevice(const DeviceInfoPtr &pDeviceInfo)
{
    this->currentDevice = pDeviceInfo;
    ui->biometrictypeBox->setCurrentText(DeviceType::getDeviceType_tr(pDeviceInfo->deviceType));
    ui->biometricDeviceBox_2->setCurrentText(pDeviceInfo->shortName);
}

DeviceInfoPtr BiometricsWidget::findDeviceById(int drvid)
{
    for (int type : deviceMap.keys()) {
        DeviceList &deviceList = deviceMap[type];
        auto iter
            = std::find_if(deviceList.begin(), deviceList.end(), [&](DeviceInfoPtr ptr) { return ptr->id == drvid; });
        if (iter != deviceList.end()) {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

DeviceInfoPtr BiometricsWidget::findDeviceByName(const QString &name)
{
    for (int type : deviceMap.keys()) {
        DeviceList &deviceList = deviceMap[type];
        auto iter = std::find_if(
            deviceList.begin(), deviceList.end(), [&](DeviceInfoPtr ptr) { return ptr->shortName == name; });
        if (iter != deviceList.end()) {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

bool BiometricsWidget::deviceExists(int drvid)
{
    return (findDeviceById(drvid) != nullptr);
}

bool BiometricsWidget::deviceExists(const QString &deviceName)
{
    return (findDeviceByName(deviceName) != nullptr);
}

void BiometricsWidget::onbiometricTypeBoxCurrentIndexChanged(int index)
{
    if (index < 0 /*|| index >= deviceMap.keys().size()*/) {
        return;
    }

    if (!m_isFeatureChanged) {
        UkccCommon::buriedSettings(
            "Biometrics", ui->biometrictypeBox->objectName(), QString("select"), ui->biometrictypeBox->currentText());
    }
    int type = ui->biometrictypeBox->itemData(index).toInt();
    ui->biometricDeviceBox_2->clear();
    int count = 0;
    for (auto &deviceInfo : deviceMap.value(type)) {
        count++;
        QString default_name = m_uniAuthService->getDefaultDevice(getenv("USER"), deviceInfo->deviceType);
        if (default_name == deviceInfo->shortName) {
            QString defaultText = tr("(default)");
            defaultText = deviceInfo->shortName + "   " + defaultText;
            ui->biometricDeviceBox_2->addItem(defaultText);
        } else if (default_name == "") {
            // 如果当前设备类型没有设置默认设备，设该设备为默认设备。
            m_uniAuthService->setDefaultDevice(deviceInfo->deviceType, deviceInfo->shortName);
            QString defaultText = tr("(default)");
            defaultText = deviceInfo->shortName + "   " + defaultText;
            ui->biometricDeviceBox_2->addItem(defaultText);
        } else {
            ui->biometricDeviceBox_2->addItem(deviceInfo->shortName);
        }
    }
    QString addText = tr("Add ") + DeviceType::getDeviceType_tr(type);
    addBioFeatureBtn->setName(addText);
    addBioFeatureBtn->setObjectName(addText);
    if (count == 0) {
        ui->biometricDeviceBox_2->blockSignals(true);
        ui->biometricDeviceBox_2->addItem(tr("No available device was detected"));
        ui->biometricDeviceBox_2->blockSignals(false);
        ui->biometricDeviceBox_2->setDisabled(true);
        addBioFeatureBtn->setDisabled(true);
    } else {
        int index = ui->biometricDeviceBox_2->findText(tr("No available device was detected"));
        if (index != -1) {
            ui->biometricDeviceBox_2->blockSignals(true);
            ui->biometricDeviceBox_2->removeItem(index);
            ui->biometricDeviceBox_2->blockSignals(false);
        }
        ui->biometricDeviceBox_2->setDisabled(false);
        addBioFeatureBtn->setDisabled(false);
    }
    biometricFeatureMap.clear();
    updateFeatureList();
}

void BiometricsWidget::onbiometricDeviceBoxCurrentIndexChanged(int index)
{
    if (index < 0) {
        return;
    }

    int type = ui->biometrictypeBox->currentData().toInt();

    if (!deviceMap.contains(type) || index >= deviceMap.value(type).size())
        return;
    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);
    QList<QVariant> args;

    currentDevice = deviceInfo;
    args << QVariant((int)getuid()) << QVariant(0) << QVariant(-1);
    serviceInterface->callWithCallback(
        "GetAllFeatureList",
        args,
        this,
        SLOT(updateFeatureListCallback(QDBusMessage)),
        SLOT(errorCallback(QDBusError)));
}

void BiometricsWidget::updateEnableTypeList()
{
    if (!getProductNameN90F3()) {
        return ;
    }
    QList<QVariant> args;

    args << QVariant((int)getuid()) << QVariant(0) << QVariant(-1);
    serviceInterface->callWithCallback(
        "GetAllFeatureList",
        args,
        this,
        SLOT(updateEnableTypeListCallback(QDBusMessage)),
        SLOT(errorCallback(QDBusError)));
}

bool BiometricsWidget::getProductNameN90F3()
{
    QFile file("/sys/class/dmi/id/product_name");

    if (!file.exists()) {
        qWarning() << "File does not exist:" << file.fileName();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << file.errorString();
        return false;
    }

    QString productName = file.readAll().trimmed();
    file.close();

    qDebug() << "Product name:" << productName;


    if (productName.contains("N90F3", Qt::CaseInsensitive) || productName.contains("N80F3", Qt::CaseInsensitive) || productName.contains("90KQA01YCD", Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

void BiometricsWidget::onFeatureChanged(int drvid, int uid, int cType)
{
    updateEnableTypeList();
    if (isShowDialog || isShowEnrollDialog) {
        isShowDialog = false;
        return;
    }

    QTimer::singleShot(200, [&] {
        int savedDeviceId = -1;
        if (currentDevice)
            savedDeviceId = currentDevice->id;
        m_isFeatureChanged = true;
        updateDevice();
        m_isFeatureChanged = false;
        if (savedDeviceId >= 0)
            setCurrentDevice(savedDeviceId);
    });
}

bool compareBarData(const QDBusVariant &feature1, const QDBusVariant &feature2)
{
    FeatureInfo *featureInfo1 = new FeatureInfo;
    FeatureInfo *featureInfo2 = new FeatureInfo;

    feature1.variant().value<QDBusArgument>() >> *featureInfo1;
    feature2.variant().value<QDBusArgument>() >> *featureInfo2;

    QString item1Data = featureInfo1->index_name;
    QString item2Data = featureInfo2->index_name;

    //  使用QCollator进行排序
    QCollator collator(QLocale(QLocale::Chinese, QLocale::China));
    //  将数字作为数字而不是字符串进行比较
    collator.setNumericMode(true);
    //  区分大小写
    collator.setCaseSensitivity(Qt::CaseSensitivity::CaseSensitive);
    //  不忽略标点符号
    collator.setIgnorePunctuation(false);

    return collator.compare(item1Data, item2Data) < 0;
}

void BiometricsWidget::updateFeatureListCallback(QDBusMessage callbackReply)
{
    QList<QDBusVariant> qlist;
    int listsize;
    QList<QVariant> variantList = callbackReply.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    qSort(qlist.begin(), qlist.end(), compareBarData);
    if (qlist.size() > 0) {
        // 当前设备存在时
        if (currentDevice) {
            // 清除生物特征列表
            ui->bioFeatureListWidget->clear();
            biometricFeatureMap.clear();
            // 重新添加生物特征
            for (int i = 0; i < listsize; i++) {
                FeatureInfoPtr featureInfo = std::make_shared<FeatureInfo>();
                qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
                if (currentDevice->deviceType == featureInfo->biotype
                    && currentDevice->shortName == featureInfo->device_shortname) {
                    addFeature(featureInfo);
                }
            }
        } else {
            ui->bioFeatureListWidget->clear();
            biometricFeatureMap.clear();
            for (int i = 0; i < listsize; i++) {
                FeatureInfoPtr featureInfo = std::make_shared<FeatureInfo>();
                qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
                addFeature(featureInfo);
            }
        }

        for (int i = 0; i < m_enableTypeList.size(); i++) {
            qDebug() << " m_enableTypeList biotype = " << m_enableTypeList.at(i);
        }
    } else if (qlist.size() == 0) {
        ui->bioFeatureListWidget->clear();
        biometricFeatureMap.clear();
    }
    updateFeatureList();
}

void BiometricsWidget::updateEnableTypeListCallback(QDBusMessage callbackReply)
{
    if (!getProductNameN90F3()) {
        return ;
    }
    QList<QDBusVariant> qlist;
    int listsize;
    QList<QVariant> variantList = callbackReply.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    if (qlist.size() > 0) {
        m_enableTypeList.clear();
        for (int i = 0; i < listsize; i++) {
            FeatureInfoPtr featureInfo = std::make_shared<FeatureInfo>();
            qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
            bool deviceAvailable = false;
            qDebug() << " contain featureInfo->biotype = " << featureInfo->biotype;
            for (auto deviceInfo : deviceInfosMap[DeviceType::bioTypeToIndex(featureInfo->biotype)]) {
                if (deviceInfo->shortName == featureInfo->device_shortname && deviceInfo->deviceNum > 0) {
                    deviceAvailable = true;
                    break;
                }
            }
            if (!m_enableTypeList.contains(featureInfo->biotype) && deviceAvailable) {
                qDebug() << " featureInfo->biotype = " << featureInfo->biotype;
                m_enableTypeList.append(featureInfo->biotype);
            }
        }
        ui->authTypeComboBox->clear();
        ui->authTypeComboBox->blockSignals(true);
        ui->authTypeComboBox->addItem(tr("User Password"), -1);
        for (int i = 0; i < m_enableTypeList.size(); i++) {
            ui->authTypeComboBox->addItem(DeviceType::getDeviceType_tr(m_enableTypeList.at(i)), m_enableTypeList.at(i));
        }
        int type = m_uniAuthService->getDefaultAuthType(getenv("USER"));
        int index = ui->authTypeComboBox->findData(type);
        if (index != -1) {
            ui->authTypeComboBox->setCurrentIndex(index);
        } else {
            m_uniAuthService->setDefaultAuthType(getenv("USER"), -1);
        }
        ui->authTypeComboBox->blockSignals(false);
    }
}

void BiometricsWidget::onAuthTypeBoxCurrentIndexChanged(int index)
{
    if (!getProductNameN90F3()) {
        return ;
    }
    if (index < 0) {
        return;
    }
    int type = ui->authTypeComboBox->itemData(index).toInt();
    qDebug() << " type = " << type ;
    m_uniAuthService->setDefaultAuthType(getenv("USER"), type);
}

void BiometricsWidget::updateFeatureList()
{
    if (biometricFeatureMap.count() < 3) {
        ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count() * ITEMHEIGH + 3);
    } else {
        ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count() * ITEMHEIGH + biometricFeatureMap.count());
    }
    // 不在手动设置frame的高度，直接通过布局来调整frame的高度
    //    if(enableAuth)
    //        ui->frame->setFixedHeight(71 + 61*2 + biometricFeatureMap.count()*ITEMHEIGH  + biometricFeatureMap.count()
    //        + 60);
    //    else
    //        ui->frame->setFixedHeight(60);
}

void BiometricsWidget::errorCallback(QDBusError error) {}

void BiometricsWidget::showEnrollDialog()
{
    if (ui->biometricDeviceBox_2->count() <= 0 || ui->biometrictypeBox->count() <= 0)
        return;

    int index = ui->biometricDeviceBox_2->currentIndex();
    int type = ui->biometrictypeBox->currentData().toInt();

    if (index < 0 || type < 0)
        return;

    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);

    if (!deviceInfo)
        return;

    UkccCommon::buriedSettings("Biometrics", addBioFeatureBtn->objectName(), QString("clicked"));

    if (deviceInfo->deviceType == BIOTYPE_FACE) {
        QMessageBox riskTip(this);
        //        riskTip.setWindowTitle(tr(""));
        riskTip.setText(tr("'Face recognition' does not support live body detection, and the verification method is "
                           "risky. Do you want to continue?"));
        riskTip.setIcon(QMessageBox::Warning);
        QPushButton *btnContinue = riskTip.addButton(tr("Continue"), QMessageBox::AcceptRole);
        btnContinue->setProperty("isImportant", true);
        btnContinue->setObjectName("msgContinueBtn");
        QPushButton *btnCancel = riskTip.addButton(tr("Cancel"), QMessageBox::RejectRole);
        btnCancel->setProperty("useButtonPalette", true);
        btnCancel->setObjectName("msgCancelBtn");
        riskTip.exec();
        if (riskTip.clickedButton() != btnContinue) {
            return;
        }
    }
    isShowEnrollDialog = true;
    BiometricEnrollDialog *dialog
        = new BiometricEnrollDialog(serviceInterface, deviceInfo->deviceType, deviceInfo->id, getuid(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &BiometricEnrollDialog::enrollFinished, this, [=]() {
        onbiometricDeviceBoxCurrentIndexChanged(ui->biometricDeviceBox_2->currentIndex());
    });
    // gdxfp显示指纹图片
    if (deviceInfo->shortName == "gdxfp")
        dialog->setProcessed(true);

    if (deviceInfo->deviceType == BIOTYPE_FACE)
        dialog->setIsFace(true);

    QDBusReply<int> reply = serviceInterface->call("StopOps", QVariant(deviceInfo->id), QVariant(3000));
    int num = 1;
    QList<int> idxList;
    QStringList strList = m_biometricProxy->getAllFeatureinfolist(getuid(), 0, -1, deviceInfo->deviceType, idxList);

    QString featurename;
    int freeIndex = strList.count();
    while (1) {
        featurename = DeviceType::getDeviceType_tr(deviceInfo->deviceType) + QString::number(num);
        if (!strList.contains(featurename))
            break;
        num++;
    }
    num = 0;
    while (1) {
        if (!idxList.contains(num)) {
            freeIndex = num;
            break;
        }
        num++;
    }

    qDebug() << "freeIndex = " << freeIndex;
    dialog->enroll(deviceInfo->id, getuid(), freeIndex, featurename);

    onbiometricDeviceBoxCurrentIndexChanged(ui->biometricDeviceBox_2->currentIndex());
    isShowEnrollDialog = false;
}

void BiometricsWidget::showVerifyDialog(FeatureInfoPtr featureinfo)
{
    DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
    if (!deviceInfoPtr)
        return;

    BiometricEnrollDialog *dialog
        = new BiometricEnrollDialog(serviceInterface, deviceInfoPtr->deviceType, deviceInfoPtr->id, getuid());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (deviceInfoPtr->shortName == "huawei")
        dialog->setProcessed(true);

    if (deviceInfoPtr->deviceType == BIOTYPE_FACE)
        dialog->setIsFace(true);

    dialog->verify(deviceInfoPtr->id, getuid(), featureinfo->index);
}

void BiometricsWidget::biometricShowMoreInfoDialog()
{
    if (ui->biometricDeviceBox_2->count() <= 0 || ui->biometrictypeBox->count() <= 0)
        return;

    int index = ui->biometricDeviceBox_2->currentIndex();
    int type = ui->biometrictypeBox->currentData().toInt();

    if (index < 0 || type < 0)
        return;

    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);

    if (!deviceInfo)
        return;

    BiometricMoreInfoDialog *dialog = new BiometricMoreInfoDialog(deviceInfo);
    dialog->exec();
}

void BiometricsWidget::renameFeaturedone(FeatureInfoPtr featureinfo, QString newname)
{
    QListWidgetItem *item = biometricFeatureMap.value(featureinfo->index_name);
    ui->bioFeatureListWidget->takeItem(ui->bioFeatureListWidget->row(item));
    biometricFeatureMap.remove(featureinfo->index_name);

    featureinfo->index_name = newname;
    addFeature(featureinfo);
}

void BiometricsWidget::deleteFeaturedone(FeatureInfoPtr featureinfo)
{

    QListWidgetItem *item = biometricFeatureMap.value(featureinfo->index_name);

    ui->bioFeatureListWidget->takeItem(ui->bioFeatureListWidget->row(item));
    biometricFeatureMap.remove(featureinfo->index_name);

    updateFeatureList();
}

void BiometricsWidget::addFeature(FeatureInfoPtr &featureinfo)
{
    HoverWidget *baseWidget = new HoverWidget(featureinfo->index_name);
    KDK_EXTEND_ALL_INFO_FORMAT(baseWidget, "LoginOptions", "BiometricsWidget", "feature widget of LoginOptions dailog");
    baseWidget->setMinimumSize(550, 60);
    baseWidget->setMaximumSize(16777215, 60);
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);
    //    baseWidget->setStyleSheet("HoverWidget{background: palette(base);}\
//                              HoverWidget:hover:!pressed{background: palette(base);}");
    // ui->currentUserFrame->setContentsMargins(16,0,16,0);

    QVBoxLayout *baseVerLayout = new QVBoxLayout(baseWidget);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *baseHorLayout = new QHBoxLayout();
    baseHorLayout->setSpacing(16);
    baseHorLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *widget = new QWidget(baseWidget);
    //    widget->setFrameShape(QFrame::Shape::Box);
    widget->setFixedHeight(60);

    QHBoxLayout *mainHorLayout = new QHBoxLayout(widget);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(10, 0, 16, 0);

    NameLabel *nameLabel = new NameLabel(widget);
    KDK_EXTEND_ALL_INFO_FORMAT(
        nameLabel, "LoginOptions", "BiometricsWidget", "feature name label of LoginOptions dailog");
    nameLabel->setText(featureinfo->index_name);

    connect(nameLabel, &NameLabel::clicked, this, [&, this, featureinfo]() {
        DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
        if (!deviceInfoPtr)
            return;
        QStringList names = m_biometricProxy->getAllFeaturelist(getuid());
        // QStringList names = m_biometricProxy->getFeaturelist(deviceInfoPtr->id,getuid());
        ChangeFeatureName *dialog = new ChangeFeatureName(names, this);
        KDK_EXTEND_ALL_INFO_FORMAT(
            dialog, "LoginOptions", "BiometricsWidget", "changeFeatureName dialog of LoginOptions dailog");

        dialog->setTitle(ui->biometrictypeBox->currentText());
        connect(dialog, &ChangeFeatureName::sendNewName, [&, this, featureinfo](QString rename) {
            if (rename == "" || rename == featureinfo->index_name)
                return;

            DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
            if (!deviceInfoPtr)
                return;
            if (rename.contains("\n")) { // 按照设计师要求，当复制过来的文本中含有换行符时，过滤掉换行符
                rename = rename.remove("\n");
            }
            bool res = m_biometricProxy->renameFeature(deviceInfoPtr->id, getuid(), featureinfo->index, rename);
            renameFeaturedone(featureinfo, rename);
        });
        dialog->exec();
    });

    mainHorLayout->addWidget(nameLabel);
    //    mainHorLayout->addWidget(renameEdit);
    mainHorLayout->addStretch();
    //    mainHorLayout->addWidget(renameBtn);
    //    mainHorLayout->addWidget(verifyBtn);

    widget->setLayout(mainHorLayout);

    QPushButton *delBtn = new QPushButton(baseWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(
        delBtn, "LoginOptions", "BiometricsWidget", "delete feature button of LoginOptions dailog");
    delBtn->setFixedSize(30, 30);
    delBtn->setIconSize(QSize(16, 16));
    delBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    delBtn->setObjectName(featureinfo->index_name);
    delBtn->setProperty("isWindowButton", 0x1);
    delBtn->setProperty("useIconHighlightEffect", 0x2);
    delBtn->hide();
    connect(delBtn, &QPushButton::clicked, this, [&, this, featureinfo](bool checked) {
        Q_UNUSED(checked)
        DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
        if (!deviceInfoPtr)
            return;
        isShowDialog = true;
        bool res = m_biometricProxy->deleteFeature(deviceInfoPtr->id, getuid(), featureinfo->index, featureinfo->index);
        if (!res) {
            deleteFeaturedone(featureinfo);
        }
    });

    connect(baseWidget, &HoverWidget::enterWidget, this, [=](QString name) {
        Q_UNUSED(name)
        //        renameBtn->show();
        // 驱动gdxfp不支持验证
        //        if(featureinfo->device_shortname != "gdxfp")
        //        verifyBtn->show();
        delBtn->show();
    });
    connect(baseWidget, &HoverWidget::leaveWidget, this, [=](QString name) {
        Q_UNUSED(name)
        //        renameBtn->hide();
        //        verifyBtn->hide();
        delBtn->hide();
    });

    baseHorLayout->addWidget(widget);
    baseHorLayout->addWidget(delBtn, Qt::AlignVCenter);
    baseHorLayout->addSpacing(16);

    baseVerLayout->addLayout(baseHorLayout);
    KHLineFrame *line = new KHLineFrame(baseWidget);
    baseWidget->setLayout(baseVerLayout);
    baseVerLayout->addWidget(line);

    QListWidgetItem *item = new QListWidgetItem(ui->bioFeatureListWidget);
    item->setSizeHint(QSize(QSizePolicy::Expanding, ITEMHEIGH));
    item->setData(Qt::UserRole, QVariant(featureinfo->index_name));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    ui->bioFeatureListWidget->setItemWidget(item, baseWidget);
    ui->bioFeatureListWidget->setStyleSheet("QListWidget::Item:hover{background-color:palette(base)}");

    biometricFeatureMap.insert(featureinfo->index_name, item);
}

QString BiometricsWidget::btnHoverColor(QString styleName, bool hoverFlag)
{
    QColor color1 = palette().color(QPalette::Active, QPalette::Button);
    QColor color2 = palette().color(QPalette::Active, QPalette::BrightText);
    QColor color;
    qreal r, g, b, a;
    QString hoverColor;
    if (((styleName.contains("dark") || styleName.contains("black")) && hoverFlag)
        || ((!styleName.contains("dark") && !styleName.contains("black")) && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor
        = QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    return hoverColor;
}

void BiometricsWidget::showQRCodeScanDialog()
{
    // 判断生物识别库是否支持扫码
    DeviceList listDeviceInfo;
    DeviceMap::iterator itDeviceMap = deviceMap.begin();
    for (; itDeviceMap != deviceMap.end(); itDeviceMap++) {
        if (itDeviceMap.key() == BIOTYPE_REMOTE) {
            listDeviceInfo = itDeviceMap.value();
            break;
        }
    }
    if (listDeviceInfo.isEmpty()) {
        return;
    }

    // TODO
    QRCodeEnrollDialog *dialog = new QRCodeEnrollDialog(serviceInterface, getuid(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->enrollBegin(listDeviceInfo);
    dialog->exec();

    QTimer::singleShot(50, this, [&, this]() {
        this->ui->qrCodeLoginlabel->setMaximumWidth(180);
        this->ui->qrCodeLoginlabel->setText(tr("Scan code login"));
        int maxWidth;
        maxWidth = this->ui->qrCodeFrame->width() - 76 - this->ui->qrCodeLoginlabel->width()
                   - ui->boundWechatKey->width() - this->ui->bindOrUnbindBtn->width()
                   - this->ui->boundWechatLabel->width();
        this->ui->qrCodeLoginlabel2->setFixedWidth(maxWidth);
        this->ui->qrCodeLoginlabel2->setText(
            tr("(Can be used to log in, unlock the system, and authorize authentication)"));
        ui->enableBiometricLabel2->setFixedWidth(
            ui->enableBiometricFrame->width()
            - (enableBiometricBtn->width() + ui->enableBiometricLabel->width() + ui->advancedSettingsBtn->width()
               + 64));
        ui->enableBiometricLabel2->setText(tr("(Fingerprint, face recognition, etc)"));
    });

    QTimer::singleShot(0, this, [&, this]() { Q_EMIT this->updateQRCodeInfo(); });
}

bool BiometricsWidget::isQRCodeModuleExist()
{
    bool bExistQrCodeModule = false;
    DeviceMap::iterator itDeviceMap = deviceMap.begin();
    for (; itDeviceMap != deviceMap.end(); itDeviceMap++) {
        if (itDeviceMap.key() == BIOTYPE_REMOTE) {
            bExistQrCodeModule = true;
            break;
        }
    }
    return bExistQrCodeModule;
}

bool BiometricsWidget::isUKeyModuleExist()
{
    if (!m_biometricProxy)
        return false;
    DeviceList devList = m_biometricProxy->GetDrvList();
    bool bExistUKeyModule = false;
    for (auto devInfo : devList) {
        if (devInfo->deviceType == BIOTYPE_GENERALUKEY) {
            bExistUKeyModule = true;
            break;
        }
    }
    return bExistUKeyModule;
}

QString BiometricsWidget::getCurUserQRCodeAcount(QString &strDrivName, int &drivId)
{
    FeatureList listFeature = m_biometricProxy->getAllFeatureList(getuid(), 0, -1);
    for (auto feature : listFeature) {
        if (feature->biotype == BIOTYPE_REMOTE) {
            // get driv id from feature
            DeviceList listDevice;
            if (deviceMap.contains(BIOTYPE_REMOTE)) {
                listDevice = deviceMap[BIOTYPE_REMOTE];
            }
            for (auto device : listDevice) {
                if (device->shortName == feature->device_shortname) {
                    drivId = device->id;
                    break;
                }
            }
            // get driver name from feature
            if (feature->device_shortname == "wechat_driver") {
                strDrivName = tr("wechat");
            } else {
                strDrivName = feature->device_shortname;
            }
            return feature->index_name;
        }
    }
    return "";
}

void BiometricsWidget::onUpdateOtpInfo()
{
    hasBindOtp = false;
    if (m_pwdlessProxy && m_pwdlessProxy->isExists()) {
        QStringList authMethors = m_pwdlessProxy->userAuthMethods(QString(g_get_user_name()));
        if (authMethors.contains("otp")) {
            ui->pbSetOtpKey->setText(tr("Unbind"));
            hasBindOtp = true;
        } else {
            ui->pbSetOtpKey->setText(tr("Go to Bind"));
            hasBindOtp = false;
        }
    }
}

void BiometricsWidget::onUpdateQRCodeInfo()
{
    QString strDrivName;
    int nDrivId;
    m_strBoundWechat = getCurUserQRCodeAcount(strDrivName, nDrivId);
    if (m_strBoundWechat.isEmpty()) {
        this->ui->boundWechatKey->hide();
        this->ui->boundWechatKey->setText("");
        this->ui->boundWechatLabel->setText(tr(""));
        this->ui->bindOrUnbindBtn->setText(tr("Binding"));
    } else {
        this->ui->boundWechatKey->show();
        this->ui->boundWechatKey->setText(tr("Bound %1 account:").arg(strDrivName));
        this->ui->boundWechatLabel->setText(m_strBoundWechat);
        this->ui->bindOrUnbindBtn->setText(tr("Unbind"));
    }
}

void BiometricsWidget::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(50, this, [&, this]() {
        int maxWidth;
        maxWidth = this->ui->qrCodeFrame->width() - 76 - this->ui->qrCodeLoginlabel->width()
                   - ui->boundWechatKey->width() - this->ui->bindOrUnbindBtn->width()
                   - this->ui->boundWechatLabel->width();
        this->ui->qrCodeLoginlabel2->setFixedWidth(maxWidth);
        this->ui->qrCodeLoginlabel2->setText(
            tr("(Can be used to log in, unlock the system, and authorize authentication)"));
    });
}

void BiometricsWidget::readConfigures()
{
    if (m_uniAuthService) {
        m_isEnabelQRCode = m_uniAuthService->getQRCodeEnable();
    }
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    sysSettings.beginGroup("Functions");
    if (sysSettings.allKeys().contains("EnableUKey")) {
        m_isEnableUKey = sysSettings.value("EnableUKey").toBool();
    }
    sysSettings.endGroup();
    Q_EMIT updateFunctionsWidget();
}

void BiometricsWidget::onUpdateFunctionsWidget()
{
    if (m_isEnabelQRCode && isQRCodeModuleExist()) {
        ui->qrCodeFrame->show();
    } else {
        ui->qrCodeFrame->hide();
    }
    if (m_isEnableUKey && (isUKeyModuleExist() || (m_pwdlessProxy && m_pwdlessProxy->isExists()))) {
        ui->frameSecurityKey->show();
    } else {
        ui->frameSecurityKey->hide();
    }

    if (m_isEnableOtp && (m_pwdlessProxy && m_pwdlessProxy->isExists())) {
        ui->frameOtpKey->show();
    } else {
        ui->frameOtpKey->hide();
    }
}

/* 添加搜索索引 */
void BiometricsWidget::initSearchText()
{
    //~ contents_path /Login Options/Login Options
    ui->titleLabel->setText(tr("Login Options"));
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->titleLabel, "LoginOptions", "BiometricsWidget", "biometric title label of LoginOptions dailog");
    //~ contents_path /Login Options/Password
    ui->passwordLabel->setText(tr("Password"));
    //~ contents_path /Login Options/Scan code login
    ui->qrCodeLoginlabel->setText(tr("Scan code login"));
    //~ contents_path /Login Options/Biometric
    ui->enableBiometricLabel->setText(tr("Biometric"));
    //~ contents_path /Login Options/Type
    ui->biometrictypeLabel->setText(tr("Type"));
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometrictypeLabel, "LoginOptions", "BiometricsWidget", "biometric type label of LoginOptions dailog");
    //~ contents_path /Login Options/Device
    ui->biometricDeviceLabel->setText(tr("Device"));
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricDeviceLabel, "LoginOptions", "BiometricsWidget", "biometric device label of LoginOptions dailog");
    //~ contents_path /Login Options/Security Key
    ui->labelSecurityKey->setText(tr("Security Key"));
    //~ contents_path /Login Options/Change password
    ui->changePasswordPB->setText(tr("Change password"));
    //~ contents_path /Login Options/Binding
    ui->bindOrUnbindBtn->setText(tr("Binding"));
    //~ contents_path /Login Options/Setup
    ui->pbSetSK->setText(tr("Setup"));
    //~ contents_path /Login Options/Security issues
    ui->securityIssuePB->setText(tr("Security issues"));
}

void BiometricsWidget::onSetSecurityKey()
{
    UkccCommon::buriedSettings("Biometrics", ui->pbSetSK->objectName(), QString("clicked"));
    if (!m_isEnableUKey)
        return;

    if (m_pwdlessProxy && isPwdlessKey()) {
        if (!serviceServiceInterface) {
            serviceServiceInterface = new QDBusInterface(
                PWDLESS_DBUS_SERVICE, PWDLESS_DBUS_PATH, PWDLESS_DBUS_INTERFACE, QDBusConnection::systemBus());
        }
        SecurityKeySetDlg *dlg = new SecurityKeySetDlg(serviceServiceInterface, nullptr, getuid(), this);
        dlg->setServiceProxy(m_pwdlessProxy);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->exec();
        return;
    }
    // 判断生物识别是否有ukey设备接入
    DeviceInfoPtr deviceInfo = nullptr;
    DeviceMap::iterator itDeviceMap = deviceMap.begin();
    for (; itDeviceMap != deviceMap.end(); itDeviceMap++) {
        if (itDeviceMap.key() == BIOTYPE_GENERALUKEY) {
            for (auto dev : itDeviceMap.value()) {
                if (dev) {
                    deviceInfo = dev;
                    break;
                }
            }
        }
    }
    SecurityKeySetDlg *dlg = new SecurityKeySetDlg(serviceInterface, deviceInfo, getuid(), this);
    connect(this, &BiometricsWidget::driverAttachedChanged, dlg, &SecurityKeySetDlg::onUKeyDevStateChanged);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}

bool BiometricsWidget::isPwdlessKey()
{
    if (m_pwdlessProxy) {
        return (m_pwdlessProxy->ukeyStatus() == 1);
    }
    return false;
}

void BiometricsWidget::onSetOtpKey()
{
    if (hasBindOtp) {
        if (m_pwdlessProxy) {
            if (m_pwdlessProxy->mfaStatus() == "otp") {
                BindDialog *dlg = new BindDialog(m_pwdlessProxy, 0, this);
                dlg->setPromptLabelText(tr("The system has enabled two-factor authentication for this login method. "
                                           "Please disable the two-factor authentication first before unbinding."));
                dlg->setAttribute(Qt::WA_DeleteOnClose);
                dlg->exec();
            } else {
                QString errMsg;
                int ret = m_pwdlessProxy->otpRemove(QString(g_get_user_name()), errMsg);
                if (ret != 0) {
                    BindDialog *dlg = new BindDialog(m_pwdlessProxy, 0, this);
                    dlg->setPromptLabelText(errMsg);
                    dlg->setAttribute(Qt::WA_DeleteOnClose);
                    dlg->exec();
                }
            }
        }
    } else {
        BindDialog *dlg = new BindDialog(m_pwdlessProxy, 1, this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->exec();
    }
    QTimer::singleShot(0, this, [&, this]() { Q_EMIT this->updateOtpInfo(); });
    return;
}

void BiometricsWidget::onMfaStatusChanged(bool status, QString method)
{
    qDebug()<<"onMfaStstusChanged  "<<status<<method;

    if (!m_mfaMessage) {
        return;
    }

    if (status) {
        m_mfaMessage->show();
    } else {
        m_mfaMessage->hide();
    }
}

bool BiometricsWidget::authoriyUserForQRCode(bool isEnroll)
{
    PolkitQt1::Authority::Result result;

    if (isEnroll) {
        result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
            "org.ukui.biometric.enroll-wechat-data",
            PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
            PolkitQt1::Authority::AllowUserInteraction);
    } else {
        result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
            "org.ukui.biometric.clean-wechat-data",
            PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
            PolkitQt1::Authority::AllowUserInteraction);
    }

    if (result == PolkitQt1::Authority::Yes) { //认证通过
        qDebug() << QString("operation authorized") << result;
        return true;
    } else {
        qDebug() << QString("not authorized") << result;
        return false;
    }
}
