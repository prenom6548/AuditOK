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
#ifndef SHAREMAIN_H
#define SHAREMAIN_H

#include <QWidget>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QButtonGroup>
#include <QByteArray>
#include <QDebug>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QDBusInterface>
#include <QComboBox>
#include "krd.h"
#include "inputpwddialog.h"
#include "titlelabel.h"
#include "fixlabel.h"
#include "switchwidget.h"
#include "comboxwidget.h"
#include "radiobuttonwidget.h"
#include "settinggroup.h"

#include "passwordlabel.h"
#include "kswitchbutton.h"
using namespace kdk;

const QByteArray kVinoSchemas = "org.gnome.Vino";
const QString kVinoViewOnlyKey = "view-only";
const QString kVinoPromptKey = "prompt-enabled";
const QString kAuthenticationKey = "authentication-methods";
const QString kVncPwdKey = "vnc-password";

const QByteArray kUkccSchmas = "org.ukui.control-center";
const QString kUkccEnableKey = "enabled";
const QString kUkccRdpEnableKey = "xrdp-enabled";
const QString kUkccVncEnableKey = "vino-enabled";

enum RequestPwd {
    NOPWD,
    NEEDPWD
};

enum XrdpServiceStatus {
    RUNNING,
    INACTIVE,
    NONE
};

class ShareMain : public QWidget
{
    Q_OBJECT
public:
    ShareMain(QWidget *parent = nullptr);
    ~ShareMain();

    void initServiceStatus();

private:
    SwitchWidget *m_pEnableFrame;        // 允许其他人查看桌面
    SwitchWidget *m_pRdpEnableFrame;     // 允许其他用户使用rdp远程连接这台电脑
    SettingGroup *m_pVncFrameGroup;
    SwitchWidget *m_pVncEnableFrame;     // 允许其他用户使用vnc远程连接这台电脑
    SwitchWidget *m_pViewFrame;          // 允许连接控制屏幕
    SwitchWidget *m_pSecurityFrame;      // 为本机确认每次访问
    SwitchWidget *m_pSecurityPwdFrame;    // 要求用户输入密码
    RadioButtonWidget *m_pProtocolFrame;  // 远程连接协议
    QPushButton *m_pPwdEditBtn;

    QGSettings *m_pVinoGsetting;
    QGSettings *m_pUkccGsettings = nullptr;

    bool m_isExsitXrdp = true;
    QDBusInterface *m_pServiceDbus;

    QString m_secpwd;

    ComKylinRemoteDesktopInterface *m_pKrd;
    ComboxWidget *m_pOutputDisplayFrame;   // 允许远程访问的显示器
    ComboxWidget *m_pClientsFrame;         // 允许连接的远程客户端数量
    SettingGroup *m_pDisplayWithClientsFrameGroup;

private:
    void initWaylandData();
    void initUI();
    void initConnection();
    void initShareStatus(bool isConnnect, bool isPwd);
    void initEnableStatus();
    void setFrameVisible(bool visible);
    void setVinoService(bool status);

    XrdpServiceStatus getXrdpServiceStatus();
    void setXrdpService(bool status);

    void initOutputs();
    void initMaxClientValue();

private slots:
    void enableSlot(bool status);
    void xrdpEnabledSlot(bool status);
    void vinoEnableSlot(bool status);
    void viewBoxSlot(bool status);
    void accessSlot(bool status);
    void pwdEnableSlot(bool status);
    void pwdInputSlot();

    void waylandEnableSlot(bool status);
    void protocolChangedSlot(int id);
    void onChanged(int type);
    void currentDisplayChanged(QString output);
    void maxClientValueChangedSlot(QString cNum);

    void setSecurityControl();
};

#endif // SHAREMAIN_H
