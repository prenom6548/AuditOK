/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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

#include "securitykeysetdlg.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <kysdk/applications/accessinfohelper.h>

SecurityKeySetDlg::SecurityKeySetDlg(QString username, PwdlessService *service, QWidget* parent)
    : KDialog(parent)
    , m_username(username)
    , m_serviceInterface(service)
    , m_timerLoading(nullptr)
    , m_curUIState(-1)
{
    setFixedSize(420,396);
    setWindowTitle(tr("Security Key"));
    setWindowIcon("ukui-control-center");
    initUI();
    initConnetions();
    onSwitchUIState(UISTATE_UNBOUND);
}

SecurityKeySetDlg::~SecurityKeySetDlg()
{

}

void SecurityKeySetDlg::initUI()
{
    m_layoutTop = new QVBoxLayout();
    m_layoutTop->setContentsMargins(0,0,0,0);
    m_layoutTop->setSpacing(0);

    // 分页面初始化UI
    m_widgetUnbound = new QWidget();
    QVBoxLayout *layoutUnBound = new QVBoxLayout(m_widgetUnbound);
    layoutUnBound->setContentsMargins(0,0,0,0);
    layoutUnBound->setAlignment(Qt::AlignCenter);
    m_labelUnboundLogo = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelUnboundLogo, "userinfo", "SecurityKeySet", "");
    m_labelUnboundLogo->setPixmap(QIcon(":/img/plugins/userinfo/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelUnboundLogo->setAlignment(Qt::AlignCenter);
    m_labelUnboundLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelUnboundTitle = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelUnboundTitle, "userinfo", "SecurityKeySet", "");
    m_labelUnboundTitle->setText(tr("Bind Security Key"));
    m_labelUnboundTitle->setAlignment(Qt::AlignCenter);
    QFont fontLabel = m_labelUnboundTitle->font();
    fontLabel = m_labelUnboundTitle->font();
    fontLabel.setBold(true);
    m_labelUnboundTitle->setFont(fontLabel);
    m_labelUnboundTip = new KLabel();
    m_labelUnboundTip->setText(tr("You will bind your security key so that you can log in to the system as '%1'. If you need to bind, please click 'OK'.\n").arg(m_username));
    m_labelUnboundTip->setWordWrap(true);
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelUnboundTip, "userinfo", "SecurityKeySet", "");
    m_labelUnboundTip->setFixedWidth(372);
    layoutUnBound->addWidget(m_labelUnboundLogo);
    layoutUnBound->addSpacing(40);
    layoutUnBound->addWidget(m_labelUnboundTitle);
    layoutUnBound->addSpacing(12);
    layoutUnBound->addWidget(m_labelUnboundTip);
    layoutUnBound->addSpacing(48);
    m_layoutTop->addWidget(m_widgetUnbound);

    // wait for ukey ui
    m_widgetNoDev = new QWidget();
    QVBoxLayout *layoutNoDev = new QVBoxLayout(m_widgetNoDev);
    layoutNoDev->setContentsMargins(0,0,0,0);
    layoutNoDev->setAlignment(Qt::AlignCenter);
    m_labelLoading = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelLoading, "userinfo", "SecurityKeySet", "");
    m_loadingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(28, 28);
    m_labelLoading->setProperty("useIconHighlightEffect", 0x2);
    m_labelLoading->setPixmap(m_loadingPixmap);
    m_labelLoading->setAlignment(Qt::AlignCenter);
    m_labelDetachedTip = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelDetachedTip, "userinfo", "SecurityKeySet", "");
    m_labelDetachedTip->setText(tr("Please insert the security key into the USB port"));
    m_labelDetachedTip->setAlignment(Qt::AlignCenter);
    m_labelDetachedTip->setWordWrap(true);
    layoutNoDev->addWidget(m_labelLoading);
    layoutNoDev->addWidget(m_labelDetachedTip);
    m_layoutTop->addWidget(m_widgetNoDev);

    m_timerLoading = new QTimer();
    m_timerLoading->setInterval(200);

    // set password ui
    m_widgetChkPin = new QWidget();
    QVBoxLayout *layoutChkPin = new QVBoxLayout(m_widgetChkPin);
    layoutChkPin->setContentsMargins(0, 0, 0, 0);
    layoutChkPin->setAlignment(Qt::AlignCenter);
    m_labelChkLogo = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelChkLogo, "userinfo", "SecurityKeySet", "");
    m_labelChkLogo->setPixmap(QIcon(":/img/plugins/userinfo/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelChkLogo->setAlignment(Qt::AlignCenter);
    m_labelChkLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelSecurityKey = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelChkLogo, "userinfo", "SecurityKeySet", "");
    m_labelSecurityKey->setText(tr("Enter security key password"));
    m_labelSecurityKey->setAlignment(Qt::AlignCenter);

    fontLabel.setBold(true);
    m_labelSecurityKey->setFont(fontLabel);
    m_editSecurityKey = new QLineEdit();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelChkLogo, "userinfo", "SecurityKeySet", "");
    m_editSecurityKey->setFixedSize(QSize(370, 36));
    m_editSecurityKey->setPlaceholderText(tr("Password"));
    m_editSecurityKey->setEchoMode(QLineEdit::Password);
    m_editSecurityKey->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_editSecurityKey->setTextMargins(0,0,30,0);
    m_editSecurityKey->setContextMenuPolicy(Qt::NoContextMenu);
    m_editSecurityKey->installEventFilter(this);
    m_labelChkMsg = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelChkMsg, "userinfo", "SecurityKeySet", "");
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    m_labelChkMsg->setPalette(pe);
    QHBoxLayout *layoutPwd = new QHBoxLayout;
    layoutPwd->addStretch();
    layoutPwd->setContentsMargins(0,0,8,0);
    m_editSecurityKey->setLayout(layoutPwd);
    layoutChkPin->addWidget(m_labelChkLogo);
    layoutChkPin->addSpacing(54);
    layoutChkPin->addWidget(m_labelSecurityKey);
    layoutChkPin->addSpacing(12);
    layoutChkPin->addWidget(m_editSecurityKey);
    layoutChkPin->addWidget(m_labelChkMsg);
    layoutChkPin->addStretch();
    m_layoutTop->addWidget(m_widgetChkPin);

    // bound
    m_widgetBound = new QWidget();
    QVBoxLayout *layoutBound = new QVBoxLayout(m_widgetBound);
    layoutBound->setContentsMargins(0,0,0,0);
    layoutBound->setAlignment(Qt::AlignCenter);
    m_labelBoundLogo = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelChkLogo, "Userinfo", "SecurityKeySet", "");
    m_labelBoundLogo->setPixmap(QIcon(":/img/plugins/userinfo/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelBoundLogo->setAlignment(Qt::AlignCenter);
    m_labelBoundLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelBoundTip = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_labelBoundTip, "Userinfo", "SecurityKeySet", "");
    m_labelBoundTip->setText(tr("The security key password has been set, please keep it properly. To unbind the security key, click Unbind.\n"));
    m_labelBoundTip->setFixedWidth(372);
    m_labelBoundTip->setWordWrap(true);
    m_btnUnbind = new QPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_btnUnbind, "Userinfo", "SecurityKeySet", "");
    m_btnUnbind->setFixedSize(QSize(96, 36));
    QFontMetrics fontMetrics(m_btnUnbind->font());
    int fontSize = fontMetrics.width(tr("Unbind"));
    QString str = tr("Unbind");
    int pSize = m_btnUnbind->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(tr("Unbind"), Qt::ElideRight, pSize);
        m_btnUnbind->setToolTip(tr("Unbind"));
    } else {
        m_btnUnbind->setToolTip("");
    }
    m_btnUnbind->setText(str);
    layoutBound->addWidget(m_labelBoundLogo);
    layoutBound->addSpacing(40);
    layoutBound->addWidget(m_labelBoundTip);
    layoutBound->addSpacing(12);
    layoutBound->addWidget(m_btnUnbind, 0, Qt::AlignLeft);
    layoutBound->addSpacing(48);
    m_layoutTop->addWidget(m_widgetBound);

    m_btnCancel = new KPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_btnCancel, "userinfo", "SecurityKeySet", "");
    m_btnCancel->setText(tr("Cancel"));
    m_btnCancel->setMinimumWidth(96);
    m_btnCancel->setProperty("useButtonPalette", true);
    m_btnCancel->setAccessibleName(tr("Cancel"));
    m_btnSure = new KPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_btnSure, "userinfo", "SecurityKeySet", "");
    m_btnSure->setAccessibleName(tr("OK"));
    m_btnSure->setText(tr("OK"));
    m_btnSure->setMinimumWidth(96);
    m_btnSure->setBackgroundColorHighlight(true);
    m_layoutBottom = new QHBoxLayout();
    m_layoutBottom->setContentsMargins(0,0,0,0);
    m_layoutBottom->setSpacing(16);
    m_layoutBottom->setAlignment(Qt::AlignRight);
    m_layoutBottom->addWidget(m_btnCancel);
    m_layoutBottom->addWidget(m_btnSure);

    m_layoutMain = new QVBoxLayout();
    m_layoutMain->setContentsMargins(24,24,24,24);
    m_layoutMain->setSpacing(0);
    m_layoutMain->addLayout(m_layoutTop);
    m_layoutMain->addLayout(m_layoutBottom);
    mainWidget()->setLayout(m_layoutMain);
}

void SecurityKeySetDlg::initConnetions()
{
    connect(m_timerLoading, &QTimer::timeout, this, &SecurityKeySetDlg::updateLoading);
    connect(m_btnCancel, &QPushButton::clicked, this, [=](){
        close();
    });
    connect(m_btnSure, &QPushButton::clicked, this, &SecurityKeySetDlg::onBtnSure);

    connect(m_editSecurityKey, &QLineEdit::textEdited, [=](QString txt){
        refreshConfirmBtnStatus();
    });

    connect(m_btnUnbind, &QPushButton::clicked, this, &SecurityKeySetDlg::onUnBind);

    // QDBusInterface *iface = new QDBusInterface("org.freedesktop.login1",
    //                                            "/org/freedesktop/login1",
    //                                            "org.freedesktop.login1.Manager",
    //                                            QDBusConnection::systemBus(),
    //                                            this);
    // connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
}

void SecurityKeySetDlg::onBtnSure()
{
    if (m_curUIState == UISTATE_UNBOUND) {
        onBind();
    } else if (m_curUIState == UISTATE_CHKPIN) {
        QString errorMsg;
        int reply = m_serviceInterface->BindUkeyVerify(m_username, m_editSecurityKey->text(), errorMsg);
        if (reply == 0) {
            m_isUKeyDevBinding = false;
            onSwitchUIState(UISTATE_BOUND);
        } else if (reply != -1){
            showErrorMessageTip(errorMsg);
        }
    } else {
        accept();
    }
}

void SecurityKeySetDlg::onBind()
{
    m_isUKeyDevBinding = true;
    disableDialog(true);
    QTimer::singleShot(0, this, [=](){
        // 判断ukey是否插入
        int ret = m_serviceInterface->IsInsertUkey();
        if (ret == 1) {
            disableDialog(false);
            onSwitchUIState(UISTATE_CHKPIN);
        } else {
            // 监听ukey是否插入
            QDBusConnection::systemBus().connect("com.kylin.pwdless.daemon",
                                                 "/",
                                                 "com.kylin.pwdless.daemon",
                                                 "ukeyInserted",
                                                 this,
                                                 SLOT(onUKeyInserted(QString)));
            onSwitchUIState(UISTATE_NODEV);
            QTimer::singleShot(60000, this, [=](){
                if (!m_isInsertUkey) {
                    onSwitchUIState(UISTATE_TIMEOUT);
                    QDBusConnection::systemBus().disconnect("com.kylin.pwdless.daemon",
                                                         "/",
                                                         "com.kylin.pwdless.daemon",
                                                         "ukeyInserted",
                                                         this,
                                                         SLOT(onUKeyInserted(QString)));

                }
            });
        }

    });
}

void SecurityKeySetDlg::onUnBind()
{
    disableDialog(true);
    QTimer::singleShot(0, this, [=](){
        QString errorMsg;
        int reply = m_serviceInterface->UnBindUkey(m_username, errorMsg);
        if (reply == 0) {
            accept();
        } else {
            showErrorMessageTip(errorMsg);
        }
        disableDialog(false);
    });
}

void SecurityKeySetDlg::onUKeyInserted(QString bindUser)
{
    qDebug() << "ukeyInserted" << bindUser;
    m_isInsertUkey = true;
    disableDialog(false);
    onSwitchUIState(UISTATE_CHKPIN);
}

void SecurityKeySetDlg::onUKeyRemoved(QString bindUser)
{
    QDBusConnection::systemBus().disconnect("com.kylin.pwdless.daemon",
                                         "/",
                                         "com.kylin.pwdless.daemon",
                                         "ukeyRemoved",
                                         this,
                                         SLOT(onUKeyRemoved(QString)));
    qDebug() << "ukeyRemoved" << bindUser;
    showErrorMessageTip(tr("Security key binding failed!"));
}

void SecurityKeySetDlg::updateLoading()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_loadingPixmap = m_loadingPixmap.transformed(matrix, Qt::FastTransformation);
    m_labelLoading->setPixmap(m_loadingPixmap);
}

void SecurityKeySetDlg::onSwitchUIState(int nState)
{
    if (m_curUIState == nState)
        return ;

    switch(nState) {
    case UISTATE_NODEV:
    {
        if (m_widgetNoDev) {
            m_widgetNoDev->show();
            if (m_timerLoading)
                m_timerLoading->start();
        }
        if (m_widgetChkPin) {
            m_editSecurityKey->setFocusPolicy(Qt::NoFocus);
            m_editSecurityKey->clear();
            m_labelChkMsg->clear();
            m_widgetChkPin->hide();
        }
        if (m_widgetBound)
            m_widgetBound->hide();
        if (m_widgetUnbound)
            m_widgetUnbound->hide();
        m_btnSure->hide();
        m_btnSure->setFocusPolicy(Qt::NoFocus);
        m_btnCancel->setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_btnCancel);
        m_btnCancel->setFocus();
        m_btnCancel->setDefault(true);
        m_btnCancel->setEnabled(true);
    }
    break;
    case UISTATE_TIMEOUT:
    {
        if (m_widgetNoDev) {
            if (m_timerLoading) {
                m_timerLoading->stop();
            }
        }

        m_labelLoading->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(64, 64)));
        m_labelDetachedTip->setText(tr("The security key was not found, Please check if the security key has been inserted and the corresponding security key driver has been installed."));
        m_btnCancel->hide();
        m_btnCancel->setFocusPolicy(Qt::NoFocus);
        m_btnSure->show();
        m_btnSure->setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_btnSure);
        m_btnSure->setFocus();
        m_btnSure->setDefault(true);
        m_btnSure->setEnabled(true);
    }
    break;
    case UISTATE_CHKPIN:
    {
        if (m_widgetNoDev) {
            m_widgetNoDev->hide();
            if (m_timerLoading)
                m_timerLoading->stop();
        }
        if (m_widgetChkPin) {
            m_editSecurityKey->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_editSecurityKey);
            m_editSecurityKey->setFocus();
            m_widgetChkPin->show();
        }
        if (m_widgetBound)
            m_widgetBound->hide();
        if (m_widgetUnbound)
            m_widgetUnbound->hide();
        m_btnSure->show();
        m_btnCancel->setFocusPolicy(Qt::NoFocus);
        m_btnSure->setFocusPolicy(Qt::NoFocus);
        refreshConfirmBtnStatus();
        // 监听ukey是否移除
        QDBusConnection::systemBus().connect("com.kylin.pwdless.daemon",
                                             "/",
                                             "com.kylin.pwdless.daemon",
                                             "ukeyRemoved",
                                             this,
                                             SLOT(onUKeyRemoved(QString)));
    }
    break;
    case UISTATE_BOUND:
    {
        if (m_widgetNoDev) {
            m_widgetNoDev->hide();
            if (m_timerLoading)
                m_timerLoading->stop();
        }
        if (m_widgetChkPin) {
            m_editSecurityKey->setFocusPolicy(Qt::NoFocus);
            m_editSecurityKey->clear();
            m_labelChkMsg->clear();
            m_widgetChkPin->hide();
        }
        if (m_widgetBound)
            m_widgetBound->show();
        if (m_widgetUnbound)
            m_widgetUnbound->hide();
        m_btnSure->hide();
        m_btnSure->setFocusPolicy(Qt::NoFocus);
        m_btnCancel->setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_btnCancel);
        m_btnCancel->setFocus();
        m_btnCancel->setDefault(true);
        m_btnCancel->setEnabled(true);
    }
    break;
    case UISTATE_UNBOUND:
    {
        if (m_widgetNoDev) {
            m_widgetNoDev->hide();
            if (m_timerLoading)
                m_timerLoading->stop();
        }
        if (m_widgetChkPin) {
            m_editSecurityKey->setFocusPolicy(Qt::NoFocus);
            m_editSecurityKey->clear();
            m_labelChkMsg->clear();
            m_widgetChkPin->hide();
        }
        if (m_widgetBound)
            m_widgetBound->hide();
        if (m_widgetUnbound)
            m_widgetUnbound->show();
        m_btnSure->show();
        m_btnCancel->setFocusPolicy(Qt::NoFocus);
        m_btnSure->setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_btnSure);
        m_btnSure->setFocus();
        m_btnSure->setDefault(true);
        m_btnSure->setEnabled(true);
        m_btnCancel->setEnabled(true);
    }
    break;
    default:
        return;
    }
    m_curUIState = nState;
}

void SecurityKeySetDlg::showErrorMessageTip(QString message)
{
    hide();
    QMessageBox criitical(QMessageBox::Information, "", message
                          , QMessageBox::NoButton, this);
    criitical.addButton(tr("OK"), QMessageBox::AcceptRole);
    criitical.exec();
    accept();
}

void SecurityKeySetDlg::showChkMessageTip(QString message)
{
    if (message.isEmpty()) {
        m_labelChkMsg->clear();
    } else {
        m_labelChkMsg->setText(message);
    }
    m_editSecurityKey->clear();
    refreshConfirmBtnStatus();
}

void SecurityKeySetDlg::disableDialog(bool bDisable)
{
    if (bDisable) {
        m_btnSure->setEnabled(false);
        m_btnCancel->setEnabled(false);
        m_editSecurityKey->setEnabled(false);
    } else {
        m_btnSure->setEnabled(true);
        m_btnCancel->setEnabled(true);
        m_editSecurityKey->setEnabled(true);
    }
}

void SecurityKeySetDlg::refreshConfirmBtnStatus(){
    if (m_editSecurityKey->text().isEmpty() || m_editSecurityKey->text() == tr("Password")) {
        m_btnSure->setEnabled(false);
        m_btnCancel->setDefault(true);
        m_btnSure->setDefault(false);
    } else {
        m_btnSure->setEnabled(true);
        m_btnCancel->setDefault(false);
        m_btnSure->setDefault(true);
    }
}
