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
#include "sharemain.h"

#include <QProcess>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QtConcurrent>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "ukcccommon.h"
using namespace ukcc;

ShareMain::ShareMain(QWidget *parent) :
    QWidget(parent)
{
    initUI();

    m_pServiceDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                     "/",
                                     "com.control.center.interface",
                                     QDBusConnection::systemBus(), this);

    m_pKrd = new ComKylinRemoteDesktopInterface("com.kylin.RemoteDesktop",
                                             "/com/kylin/RemoteDesktop",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (!m_pKrd->isValid()) {
        qDebug() << "start com.kylin.RemoteDesktop service failed";
    }

    QByteArray idd(kUkccSchmas);
    if (QGSettings::isSchemaInstalled(idd)) {
        m_pUkccGsettings = new QGSettings(kUkccSchmas, QByteArray(), this);
    }

    if (UkccCommon::isWayland()) {
        initWaylandData();
    }

    initConnection();
}

ShareMain::~ShareMain()
{
}

void ShareMain::initUI()
{
    TitleLabel *vinoTitleLabel = new TitleLabel(this);
    //~ contents_path /Vino/Remote Desktop
    vinoTitleLabel->setText(tr("Remote Desktop"));
    KDK_EXTEND_ALL_INFO_FORMAT(vinoTitleLabel, "Vino", "", "");

    //~ contents_path /Vino/Connect to your desktop remotely
    m_pEnableFrame = new SwitchWidget(tr("Allow remote access to this machine"), this, UkccFrame::BorderRadiusStyle::Around, tr("Allow other computers to remotely access the local computer and view the desktop"));
    m_pEnableFrame->setMinimumSize(QSize(550, 0));
    m_pEnableFrame->setMaximumSize(QSize(16777215, 16777215));
    m_pEnableFrame->setHeightAdaptive(false);
    m_pEnableFrame->switchButton()->setObjectName("enable");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pEnableFrame->switchButton(), "Vino", "", "Connect to your desktop remotely");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pEnableFrame, "Vino", "", "");
    m_pEnableFrame->switchButton()->setAccessibleName(tr("Allow remote access to this machine"));

    //~ contents_path /Vino/Allow others to connect to your desktop remotely using RDP
    m_pRdpEnableFrame = new SwitchWidget(tr("Allow others to connect to your desktop remotely using RDP"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pRdpEnableFrame->switchButton(), "Vino", "", "rdp switchbutton");
    m_pRdpEnableFrame->switchButton()->setObjectName("rdp-enable");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pRdpEnableFrame, "Vino", "", "rdp label");
    m_pRdpEnableFrame->switchButton()->setAccessibleName(tr("Allow others to connect to your desktop remotely using RDP"));

    //~ contents_path /Vino/Remote Connection Protocol
    m_pProtocolFrame = new RadioButtonWidget(tr("Remote Connection Protocol"));
    QRadioButton *vncProtocolRadio = new QRadioButton(tr("VNC Protocol"), this);
    QRadioButton *rdpProtocolRadio = new QRadioButton(tr("RDP Protocol"), this);
    m_pProtocolFrame->mainHLayout()->insertStretch(1);
    m_pProtocolFrame->setButtonSpacing(45);
    m_pProtocolFrame->addButton(vncProtocolRadio, 0);
    m_pProtocolFrame->addButton(rdpProtocolRadio, 1);
    vncProtocolRadio->setAccessibleName(tr("VNC Protocol"));
    rdpProtocolRadio->setAccessibleName(tr("RDP Protocol"));

    m_pVncFrameGroup = new SettingGroup(this);
    //~ contents_path /Vino/Allow others to connect to your desktop remotely using VNC
    m_pVncEnableFrame = new SwitchWidget(tr("Allow others to connect to your desktop remotely using VNC"),this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pVncEnableFrame->switchButton(), "Vino", "", "vnc enable switchbutton");
    m_pVncEnableFrame->switchButton()->setObjectName("vnc-enable");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pVncEnableFrame, "Vino", "", "vnc enable label");
    m_pVncEnableFrame->switchButton()->setAccessibleName(tr("Allow others to connect to your desktop remotely using VNC"));

    //~ contents_path /Vino/Allow connection to control screen
    m_pViewFrame = new SwitchWidget(tr("Allow connection to control screen"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pViewFrame->switchButton(), "Vino", "", "view only switchbutton");
    m_pViewFrame->switchButton()->setObjectName("vnc-control");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pViewFrame, "Vino", "", "view only label");
    m_pViewFrame->switchButton()->setAccessibleName(tr("Allow connection to control screen"));

    //~ contents_path /Vino/You must confirm every visit for this machine
    m_pSecurityFrame = new SwitchWidget(tr("You must confirm every visit for this machine"),this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityFrame->switchButton(), "Vino", "", "need confirm switchbutton");
    m_pSecurityFrame->switchButton()->setObjectName("vnc-confirm");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityFrame, "Vino", "", "need confirm label");
    m_pSecurityFrame->switchButton()->setAccessibleName(tr("You must confirm every visit for this machine"));

    //~ contents_path /Vino/Require user to enter this password while using VNC
    m_pSecurityPwdFrame = new SwitchWidget(tr("Require user to enter this password while using VNC"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityPwdFrame->switchButton(), "Vino", "", "password switchbutton");
    m_pSecurityPwdFrame->switchButton()->setObjectName("vnc-pwd");
    m_pSecurityPwdFrame->switchButton()->setAccessibleName(tr("Require user to enter this password while using VNC"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityPwdFrame, "Vino", "", "password label");
    m_pPwdEditBtn =  new QPushButton(tr("Change Password"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPwdEditBtn, "Vino", "", "");
    m_pPwdEditBtn->setObjectName("vnc-pwdsettings");
    m_pSecurityPwdFrame->insertWidget(2, m_pPwdEditBtn, 8, Qt::AlignRight);
    m_pSecurityPwdFrame->nameLabel()->setAccessibleName(tr("Require user to enter this password while using VNC"));
    m_pSecurityPwdFrame->nameLabel()->setBuddy(m_pPwdEditBtn);
    m_pPwdEditBtn->setAccessibleName(tr("Change Password"));

    m_pVncFrameGroup->addWidget(m_pProtocolFrame);
    m_pVncFrameGroup->addWidget(m_pVncEnableFrame);
    m_pVncFrameGroup->addWidget(m_pViewFrame);
    m_pVncFrameGroup->addWidget(m_pSecurityFrame);
    m_pVncFrameGroup->addWidget(m_pSecurityPwdFrame);

    //~ contents_path /Vino/Displays that allow remote access
    m_pOutputDisplayFrame = new ComboxWidget(tr("Displays that allow remote access"), this);
    m_pOutputDisplayFrame->setTitleMaximumWidth(402);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pOutputDisplayFrame->comboBox(), "Vino", "", "display combox");
    m_pOutputDisplayFrame->comboBox()->setObjectName("display");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pOutputDisplayFrame, "Vino", "", "display label");
    m_pOutputDisplayFrame->label()->setAccessibleName(tr("Displays that allow remote access"));
    m_pOutputDisplayFrame->label()->setBuddy(m_pOutputDisplayFrame->comboBox());
    m_pOutputDisplayFrame->comboBox()->setAccessibleName(tr("Displays that allow remote access"));

    //~ contents_path /Vino/The number of remote clients allowed to connect
    m_pClientsFrame = new ComboxWidget(tr("The number of remote clients allowed to connect"), this);
    m_pClientsFrame->setTitleMaximumWidth(402);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pClientsFrame->comboBox(), "Vino", "", "client combox");
    m_pClientsFrame->comboBox()->setObjectName("display");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pClientsFrame, "Vino", "", "client label");
    m_pClientsFrame->label()->setAccessibleName(tr("The number of remote clients allowed to connect"));
    m_pClientsFrame->label()->setBuddy(m_pClientsFrame->comboBox());
    m_pClientsFrame->comboBox()->setAccessibleName(tr("The number of remote clients allowed to connect"));

    m_pDisplayWithClientsFrameGroup = new SettingGroup(this);
    m_pDisplayWithClientsFrameGroup->addWidget(m_pOutputDisplayFrame);
    m_pDisplayWithClientsFrameGroup->addWidget(m_pClientsFrame);

    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->addWidget(vinoTitleLabel);
    mainVLayout->setSpacing(8);
    mainVLayout->addWidget(m_pEnableFrame);
    mainVLayout->addWidget(m_pRdpEnableFrame);
    mainVLayout->addWidget(m_pVncFrameGroup);
    mainVLayout->addWidget(m_pDisplayWithClientsFrameGroup);
    mainVLayout->addStretch();

    m_pRdpEnableFrame->hide();

    if (UkccCommon::isWayland()) {
        m_pRdpEnableFrame->setVisible(false);
        m_pVncEnableFrame->setVisible(false);
    } else {
        m_pProtocolFrame->setVisible(false);
        m_pDisplayWithClientsFrameGroup->setVisible(false);
    }

    // 协议单选：Tab 只进当前选中项（FocusPolicy 由 RadioButtonWidget 维护），两项均接到后续控件
    QWidget::setTabOrder(m_pEnableFrame->switchButton(), vncProtocolRadio);
    QWidget::setTabOrder(vncProtocolRadio, m_pViewFrame->switchButton());
    QWidget::setTabOrder(rdpProtocolRadio, m_pViewFrame->switchButton());
    QWidget::setTabOrder(m_pViewFrame->switchButton(), m_pSecurityFrame->switchButton());
    QWidget::setTabOrder(m_pSecurityFrame->switchButton(), m_pPwdEditBtn);
    QWidget::setTabOrder(m_pPwdEditBtn, m_pSecurityPwdFrame->switchButton());
    QWidget::setTabOrder(m_pSecurityPwdFrame->switchButton(), m_pOutputDisplayFrame->comboBox());
    QWidget::setTabOrder(m_pOutputDisplayFrame->comboBox(), m_pClientsFrame->comboBox());
}

void ShareMain::initWaylandData()
{
    bool isEnabled = m_pKrd->GetStatus();
    m_pEnableFrame->setChecked(isEnabled);

    int ret = m_pKrd->GetServerType();
    bool isOpenVNC = ret == 0 ? true : false;
    bool isOpenRDP = ret == 1 ? true : false;
    qDebug() << isOpenVNC << isOpenRDP;
    m_pProtocolFrame->buttonGroup()->button(ret)->setChecked(true);

    bool needPwd = m_pKrd->GetAuthMethod();
    QString password = m_pKrd->GetPassword();

    bool isAllowControl = m_pKrd->GetControlPermissionStatus();
    bool needConfirm = m_pKrd->GetVisitConfirmStatus();
    m_pViewFrame->setChecked(isAllowControl);
    m_pSecurityFrame->setChecked(needConfirm);

    setFrameVisible(isEnabled);

    m_pSecurityPwdFrame->setVisible(isOpenVNC);

    m_pSecurityPwdFrame->switchButton()->setChecked(needPwd);
    m_pPwdEditBtn->setVisible(needPwd);

    initOutputs();
    initMaxClientValue();
}

void ShareMain::initOutputs()
{
    QStringList outputs = m_pKrd->GetSupportedOutputDevice();
    qDebug() << "GetSupportedOutputDevice" << outputs;

    m_pOutputDisplayFrame->comboBox()->blockSignals(true);
    /* first delete output that is removed */
    for(int i = 0; i< m_pOutputDisplayFrame->comboBox()->count(); i++) {
        if (!outputs.contains(m_pOutputDisplayFrame->comboBox()->itemText(i))) {
            qDebug() << "remove output" << m_pOutputDisplayFrame->comboBox()->itemText(i);
            m_pOutputDisplayFrame->comboBox()->removeItem(i);
        }
    }

    /* add output that is new */
    foreach (const QString &output, outputs) {
        if (m_pOutputDisplayFrame->comboBox()->findText(output) == -1) {
            qDebug() << "add output" << output;
            m_pOutputDisplayFrame->comboBox()->addItem(output);
        }
    }
    if (m_pOutputDisplayFrame->comboBox()->count() == 1) {
        m_pOutputDisplayFrame->comboBox()->setEnabled(false);
    } else {
        m_pOutputDisplayFrame->comboBox()->setEnabled(true);
    }
    m_pOutputDisplayFrame->comboBox()->blockSignals(false);

    QString output = m_pKrd->GetCurrentDevice();
    qDebug() << "GetCurrentDevice" << output;
    if (!output.isEmpty() && m_pOutputDisplayFrame->comboBox()->findText(output) != -1) {
        m_pOutputDisplayFrame->comboBox()->setCurrentText(output);
    } else {
        m_pOutputDisplayFrame->comboBox()->setCurrentIndex(0);
        // 以开启远程连接的情况下使用 start 接口切换输出屏幕是否生效，是否需要断开上一次设置的output；
        currentDisplayChanged(m_pOutputDisplayFrame->comboBox()->currentText());
    }
}

void ShareMain::currentDisplayChanged(QString output)
{
    qDebug() << "selected output" << output;

    if (output.isEmpty()) {
        QMessageBox::warning(NULL, tr("Warning"), tr("please select an output"));
        m_pEnableFrame->switchButton()->setChecked(false);
        return;
    }

    if (m_pProtocolFrame->buttonGroup()->checkedId()== 0) {
        m_pKrd->Start(output);
    } else {
        m_pKrd->StartRDP(output);
    }

    UkccCommon::buriedSettings(QStringLiteral("Vino"), m_pOutputDisplayFrame->title(), QString("selected"), output);

    qDebug() << "ZDEBUG " << "start " << output << __LINE__ ;
}

void ShareMain::initMaxClientValue()
{
    int maxClient = m_pKrd->GetAllowedMaxClient();
    ClientInfo client = m_pKrd->clientsInfo();
    qDebug() << Q_FUNC_INFO << "maxClient = " << maxClient << client.count();
    m_pClientsFrame->comboBox()->clear();
    QStringList clientCountList;
    // 最小应为已连接的客户端数量
    for(int i = client.count() > 0 ? client.count() : 1; i <= 10; i++) {
        clientCountList << QString::number(i);
    }

    m_pClientsFrame->comboBox()->addItems(clientCountList);

    // 当连接数量大于0且，大于maxClient，SetAllowedMaxClient为当前连接数量
    if (client.count() > 0 && client.count() > maxClient) {
        maxClient = client.count();
    }

    // 当maxClient=0,SetAllowedMaxClient最小为1
    if (maxClient == 0 ) {
        maxClient = 1;
    }

    m_pClientsFrame->comboBox()->setCurrentText(QString::number(maxClient));
    m_pKrd->SetAllowedMaxClient(maxClient);
}

void ShareMain::maxClientValueChangedSlot(QString cNum)
{
    m_pKrd->SetAllowedMaxClient(cNum.toInt());
    UkccCommon::buriedSettings(QStringLiteral("Vino"), m_pClientsFrame->title(), QString("selected"), cNum);
}

void ShareMain::onChanged(int type)
{
    bool isEnabled = m_pKrd->GetStatus();
    if (!isEnabled) {
        return;
    }
    if (type == 1) {
        initOutputs();
    } else if (type == 2) {
        // 调整m_pClientsCombox中可供设置的值
        int allowMax = m_pKrd->GetAllowedMaxClient();
        ClientInfo client = m_pKrd->clientsInfo();
        qDebug() << "client active number == " << client.count();

        if(client.count() > 0) {
            // 将小于已连接数的移除
            for(int i = 1; i < client.count(); i++) {
                if (m_pClientsFrame->comboBox()->findText(QString::number(i)) != -1) {
                    m_pClientsFrame->comboBox()->removeItem(m_pClientsFrame->comboBox()->findText(QString::number(i)));
                }
            }
            // 将大于等于的数添加
            for(int i = client.count(); i <= 10; i++) {
                if (m_pClientsFrame->comboBox()->findText(QString::number(i)) == -1) {
                    qDebug() << "m_pClientsCombox add number" << i;;
                    m_pClientsFrame->comboBox()->insertItem(i - 1, QString::number(i));
                }
            }
        }
        // 已连接数量大于当前设置的最大允许连接数，设置最大允许连接为已连接数? 无已连接大于允许的情况
        if(client.count() > 0 && client.count() > allowMax) {
            allowMax = client.count();
        }
        m_pClientsFrame->comboBox()->setCurrentText(QString::number(allowMax));
    }
}

void ShareMain::setSecurityControl()
{
    // 深信服控件禁用
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("vinoSettings").toString();
    QStringList setItems = moduleSettings.split(",");

    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "mEnableFrame") {
            m_pEnableFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mRdpEnableFrame") {
            m_pRdpEnableFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mVncEnableFrame") {
            m_pVncEnableFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mViewFrame") {
            m_pViewFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mSecurityFrame") {
            m_pSecurityFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mSecurityPwdFrame") {
            m_pSecurityPwdFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "m_pOutputDisplayFrame") {
             m_pOutputDisplayFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "m_pClientsFrame") {
             m_pClientsFrame->setVisible(item.at(1) == "true");
        }
    }

    // 安全管控 禁用与启用
    QString moduleEnable = ModuleMap.value("vinoEnable").toString();
    QStringList enableItems = moduleEnable.split(",");

    foreach (QString setItem, enableItems) {
        QStringList item = setItem.split(":");
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "mEnableFrame") {
            m_pEnableFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mRdpEnableFrame") {
            m_pRdpEnableFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mVncEnableFrame") {
            m_pVncEnableFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mViewFrame") {
            m_pViewFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mSecurityFrame") {
            m_pSecurityFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mSecurityPwdFrame") {
            m_pSecurityPwdFrame->setEnabled(item.at(1) == "true");
        }
        // 允许远程访问的显示器数量只有一个时，combox禁用不可选，管控策略不可调整原有逻辑
        if (item.at(0) == "m_pOutputDisplayFrame" && m_pOutputDisplayFrame->comboBox()->count() != 1) {
            m_pOutputDisplayFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "m_pClientsFrame") {
             m_pClientsFrame->setEnabled(item.at(1) == "true");
        }
    }
}

void ShareMain::initConnection()
{
    QByteArray id(kVinoSchemas);
    if (QGSettings::isSchemaInstalled(id) && !UkccCommon::isWayland()) {
        m_pVinoGsetting = new QGSettings(kVinoSchemas, QByteArray(), this);

        initEnableStatus();
        initServiceStatus();

        connect(m_pEnableFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::enableSlot);

        connect(m_pRdpEnableFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::xrdpEnabledSlot);

        connect(m_pVncEnableFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::vinoEnableSlot);

    }

    connect(m_pViewFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::viewBoxSlot);

    connect(m_pSecurityFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::accessSlot);

    connect(m_pSecurityPwdFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::pwdEnableSlot);

    connect(m_pPwdEditBtn, &QPushButton::clicked, this, &ShareMain::pwdInputSlot);

    if (UkccCommon::isWayland()) {
        connect(m_pKrd, &ComKylinRemoteDesktopInterface::Changed, this, &ShareMain::onChanged);

        connect(m_pEnableFrame->switchButton(), &KSwitchButton::stateChanged, this, &ShareMain::waylandEnableSlot);

        connect(m_pProtocolFrame, &RadioButtonWidget::buttonClicked, this, &ShareMain::protocolChangedSlot);

        connect(m_pOutputDisplayFrame->comboBox(), &QComboBox::currentTextChanged, this, &ShareMain::currentDisplayChanged);

        connect(m_pClientsFrame->comboBox(), &QComboBox::currentTextChanged, this, &ShareMain::maxClientValueChangedSlot);
    }

    setSecurityControl();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setSecurityControl()));
}

void ShareMain::waylandEnableSlot(bool status)
{
    m_pVncFrameGroup->setVisible(status);
    m_pDisplayWithClientsFrameGroup->setVisible(status);

    if (status) {
        QString output = m_pKrd->GetCurrentDevice();
        qDebug() << "GetCurrentDevice" << output;
        int id = m_pProtocolFrame->buttonGroup()->checkedId();
        if (id == 0) {
            m_pKrd->Start(output);
        } else {
            m_pKrd->StartRDP(output);
        }
    } else {
        m_pKrd->Exit();
    }
}

void ShareMain::protocolChangedSlot(int id)
{
    QString output = m_pKrd->GetCurrentDevice();
    qDebug() << "GetCurrentDevice" << output;
    if (id == 0) {
        if (!output.isEmpty()) {
            m_pKrd->Start(m_pKrd->GetCurrentDevice());
        } else {
            m_pKrd->Start(m_pOutputDisplayFrame->comboBox()->itemText(0));
        }
        m_pSecurityPwdFrame->setVisible(true);
    } else {
        if (!output.isEmpty()) {
            m_pKrd->StartRDP(m_pKrd->GetCurrentDevice());
        } else {
            m_pKrd->StartRDP(m_pOutputDisplayFrame->comboBox()->itemText(0));
        }
        m_pSecurityPwdFrame->setVisible(false);
    }
    UkccCommon::buriedSettings(QStringLiteral("Vino"), m_pProtocolFrame->title(), QString("settings"), QString::number(id));
}

void ShareMain::initEnableStatus()
{
    bool isShared = m_pVinoGsetting->get(kVinoViewOnlyKey).toBool();
    bool need = m_pVinoGsetting->get(kVinoPromptKey).toBool();
    QString pwd = m_pVinoGsetting->get(kAuthenticationKey).toString();
    m_secpwd = m_pVinoGsetting->get(kVncPwdKey).toString();

    m_pSecurityFrame->switchButton()->setChecked(need);
    m_pViewFrame->switchButton()->setChecked(!isShared);
    if (pwd == "vnc") {
        if (m_secpwd == "keyring") {
            m_pSecurityPwdFrame->switchButton()->setChecked(false);
            m_pPwdEditBtn->hide();
            m_pVinoGsetting->set(kAuthenticationKey, "none");
        } else {
            m_pSecurityPwdFrame->switchButton()->setChecked(true);
        }
    } else {
        m_pSecurityPwdFrame->switchButton()->setChecked(false);
        m_pPwdEditBtn->setVisible(false);
    }

    QProcess *process = new QProcess;

    process->start("systemctl", QStringList() << "--user" << "is-active" << "vino-server.service");
    process->waitForFinished();

    setFrameVisible((process->readAllStandardOutput().replace("\n", "") == "active"));
    delete process;
}

void ShareMain::setFrameVisible(bool visible)
{
    if (!UkccCommon::isWayland()) {
        m_pVncEnableFrame->switchButton()->setChecked(visible);
        m_pViewFrame->setVisible(visible);
        m_pSecurityFrame->setVisible(visible);
        m_pSecurityPwdFrame->setVisible(visible);
    } else {
        m_pVncFrameGroup->setVisible(visible);
        m_pDisplayWithClientsFrameGroup->setVisible(visible);
    }
}

void ShareMain::setVinoService(bool status)
{
    QDBusInterface vinoIfc("org.ukui.SettingsDaemon",
                           "/org/ukui/SettingsDaemon/Sharing",
                           "org.ukui.SettingsDaemon.Sharing",
                           QDBusConnection::sessionBus());
    if (vinoIfc.isValid()) {
        if (status) {
            vinoIfc.call("EnableService", "vino-server");
        } else {
            vinoIfc.call("DisableService", "vino-server");
        }
    }
}

void ShareMain::enableSlot(bool status)
{
    if (status) {
        m_pUkccGsettings->set(kUkccEnableKey, status);
        initServiceStatus();
        if (m_isExsitXrdp) {
            if (m_pServiceDbus->isValid()) {
                m_pRdpEnableFrame->switchButton()->setChecked(status);
                setXrdpService(status);
            }
            m_pRdpEnableFrame->setVisible(true);
        }
        m_pVncEnableFrame->switchButton()->setChecked(status);
        vinoEnableSlot(status);
        m_pVncFrameGroup->setVisible(true);
    } else {
        m_pUkccGsettings->set(kUkccEnableKey, status);
        if (m_isExsitXrdp) {
            m_pUkccGsettings->set(kUkccRdpEnableKey, status);
            m_pRdpEnableFrame->hide();
            if (m_pServiceDbus->isValid()) {
                setXrdpService(false);
            }
            m_pRdpEnableFrame->switchButton()->blockSignals(true);
            m_pRdpEnableFrame->switchButton()->setChecked(false);
            m_pRdpEnableFrame->switchButton()->blockSignals(false);
        }
        m_pUkccGsettings->set(kUkccVncEnableKey, status);
        vinoEnableSlot(false);
        m_pVncFrameGroup->setVisible(false);
    }
    UkccCommon::buriedSettings(QStringLiteral("Vino"), "whether allow others to connect to your desktop remotely", QString("settings"), status ? "true" : "false");
}

void ShareMain::xrdpEnabledSlot(bool status)
{
    if (UkccCommon::isWayland()) {
        m_pDisplayWithClientsFrameGroup->setVisible(status);
        if (status) {
            QString output = m_pKrd->GetCurrentDevice();
            qDebug() << "GetCurrentDevice" << output;
            if (!output.isEmpty()) {
                m_pKrd->StartRDP(m_pKrd->GetCurrentDevice());
            } else {
                m_pKrd->StartRDP(m_pOutputDisplayFrame->comboBox()->itemText(0));
            }
        } else {
            m_pKrd->Exit();
        }
    } else {
        m_pUkccGsettings->set(kUkccRdpEnableKey, status);
        if (m_isExsitXrdp) {
            if (m_pServiceDbus->isValid()) {
                setXrdpService(status);
            }
        } else {
            m_pRdpEnableFrame->setVisible(false);
        }
    }

    UkccCommon::buriedSettings(QStringLiteral("Vino"), "Allow others to connect to your desktop remotely using RDP", QString("settings"), status ? "true" : "false");
}

void ShareMain::vinoEnableSlot(bool status)
{
    setFrameVisible(status);

    setVinoService(status);

    UkccCommon::buriedSettings(QStringLiteral("Vino"), "Allow others to connect to your desktop remotely using VNC", QString("settings"), status ? "true" : "false");
}

void ShareMain::viewBoxSlot(bool status)
{
    if (UkccCommon::isWayland()) {
        m_pKrd->SetControlPermissionStatus(status);
    } else {
        m_pVinoGsetting->set(kVinoViewOnlyKey, !status);
    }

    UkccCommon::buriedSettings(QStringLiteral("Vino"), "whether allow others to control your desktop remotely", QString("settings"), status ? "true" : "false");
}

void ShareMain::accessSlot(bool status)
{
    if (UkccCommon::isWayland()) {
        m_pKrd->SetVisitConfirmStatus(status);
    } else {
        if (status) {
            m_pVinoGsetting->set(kVinoPromptKey, true);
        } else {
            m_pVinoGsetting->set(kVinoPromptKey, false);
        }
    }

    UkccCommon::buriedSettings(QStringLiteral("Vino"), "whether to confirm each visit for the local mach", QString("settings"), status ? "true" : "false");
}

void ShareMain::pwdEnableSlot(bool status)
{
    if (UkccCommon::isWayland()) {
        if (status) {
            QString pwd = m_pKrd->GetPassword();
            m_pPwdEditBtn->setVisible(!pwd.isEmpty());
            pwdInputSlot();
            if (!m_pKrd->GetAuthMethod()) {
                m_pSecurityPwdFrame->switchButton()->setChecked(false);
            }
        }
        m_pPwdEditBtn->setVisible(m_pSecurityPwdFrame->switchButton()->isChecked());
        m_pKrd->SetAuthMethod(m_pSecurityPwdFrame->switchButton()->isChecked());
    } else {
        if (status) {
            m_pPwdEditBtn->setVisible(m_secpwd == "keyring" ? false:true);
            pwdInputSlot();
            m_pPwdEditBtn->setVisible(status);
            if (m_pVinoGsetting->get(kAuthenticationKey).toString() == "none") {
                m_pSecurityPwdFrame->switchButton()->setChecked(false);
            }

        } else {
            m_pPwdEditBtn->setVisible(status);
            m_pVinoGsetting->set(kAuthenticationKey, "none");
        }
    }
    UkccCommon::buriedSettings(QStringLiteral("Vino"), "whether password verification is required", QString("settings"), status ? "true" : "false");
}

void ShareMain::pwdInputSlot()
{
    if (UkccCommon::isWayland()) {
        InputPwdDialog *mwindow = new InputPwdDialog(m_pKrd,this);
        mwindow->exec();
    } else {
        InputPwdDialog *mwindow = new InputPwdDialog(m_pVinoGsetting,this);
        mwindow->exec();
        m_secpwd = m_pVinoGsetting->get(kVncPwdKey).toString();
    }
    UkccCommon::buriedSettings(QStringLiteral("Vino"), "set password", QString("clicked"));
}

void ShareMain::initServiceStatus()
{
    bool isEnabled = m_pUkccGsettings->get(kUkccEnableKey).toBool();
    if (isEnabled) {
        m_pEnableFrame->switchButton()->setChecked(true);
        XrdpServiceStatus xrdpstatus = getXrdpServiceStatus();
        switch (xrdpstatus) {
        case NONE:
            m_pRdpEnableFrame->hide();
            m_isExsitXrdp = false;
            break;
        case RUNNING:
            m_pRdpEnableFrame->switchButton()->setChecked(true);
            break;
        case INACTIVE:
            m_pRdpEnableFrame->switchButton()->setChecked(false);
            break;
        }
    } else {
        m_pEnableFrame->switchButton()->setChecked(false);
        m_pRdpEnableFrame->hide();
        m_pVncFrameGroup->hide();
    }
}

XrdpServiceStatus ShareMain::getXrdpServiceStatus()
{
    QProcess process;
    QString cmd = "systemctl status xrdp.service | grep Active:";
    process.start("bash", QStringList() <<"-c" << cmd);
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput()+process.readAllStandardError();
    if (strResult.replace("\n", "") == "Unit xrdp.service could not be found.") {
        return NONE;
    }
    cmd = "systemctl is-failed xrdp.service";
    process.start("bash", QStringList() <<"-c" << cmd);
    process.waitForFinished();
    strResult = process.readAllStandardOutput()+process.readAllStandardError();
    if ((strResult.replace("\n", "") == "active"))
        return RUNNING;
    else
        return INACTIVE;

}

void ShareMain::setXrdpService(bool status)
{
    QtConcurrent::run([=]() {
        m_pServiceDbus->call("setXrdpService", status);
    });
}
