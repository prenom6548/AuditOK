/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2023 KYLINOS Information Technology Co., Ltd.
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
#include <QMatrix>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QPalette>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <ukuiwindowhelper.h>
#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

bool SecurityKeySetDlg::isPwdlessKey()
{
    if (ukeyType == -1) {
        ukeyType = 1;
        QDBusInterface interface(
            PWDLESS_DBUS_SERVICE, PWDLESS_DBUS_PATH, PWDLESS_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
        if (!interface.isValid()) {
            return false;
        }
        QDBusMessage result = interface.call(QStringLiteral("ukeyStatus"));
        if (result.type() == QDBusMessage::ErrorMessage) {
            return false;
        }

        int ret = result.arguments().at(0).toInt();
        if (ret == 1) {
            ukeyType = 2;
            return true;
        }
        return false;
    }
    return ukeyType == 2;
}
SecurityKeySetDlg::SecurityKeySetDlg(
    QDBusInterface *service, DeviceInfoPtr devInfo, int uid, QWidget *parent /* = nullptr*/)
    : KDialog(parent)
    , m_isUKeyDevAttached(devInfo ? true : false)
    , m_serviceInterface(service)
    , m_devInfo(devInfo)
    , m_uid(uid)
    , ops(IDLE)
    , m_hisOps(IDLE)
    , m_timerLoading(nullptr)
    , m_curUIState(-1)
{
    m_isUKeyDevBound = false;
    m_isUKeyDevBinding = false;
    m_isUKeyDevUnBinding = false;
    installEventFilter(this);
    initUI();

    struct passwd *pwInfo = getpwuid(m_uid);
    if (pwInfo) {
        if (pwInfo->pw_name && pwInfo->pw_name[0] != '\0') {
            m_name = QString(pwInfo->pw_name);
        }
    }

    //这里根据传进来的service名称，判断当前ukey是走多因子的还是生物特征框架的。
    if (service) {
        if (service->service() == PWDLESS_DBUS_SERVICE) {
            ukeyType = 2;
        } else {
            ukeyType = 1;
        }
    }

    if (!isPwdlessKey()) {
        initConnetions();
        if (chkBound()) {
            onSwitchUIState(UISTATE_BOUND);
        } else {
            onSwitchUIState(UISTATE_UNBOUND);
        }
    }
}

SecurityKeySetDlg::~SecurityKeySetDlg() {}

void SecurityKeySetDlg::onUKeyDevStateChanged(bool isAttached, DeviceList devInfoList)
{
    Q_UNUSED(isAttached);
    bool isUKeyDevAttached = m_isUKeyDevAttached;
    if (devInfoList.empty()) {
        m_isUKeyDevAttached = false;
        m_devInfo = nullptr;
    } else if (!m_devInfo) { // 设备接入
        for (auto devInfo : devInfoList) {
            if (devInfo->deviceType == DeviceType::GeneralUkey) {
                m_isUKeyDevAttached = true;
                m_devInfo = devInfo;
                break;
            }
        }
    } else if (m_devInfo) {
        bool isFound = false;
        for (auto devInfo : devInfoList) {
            if (m_devInfo->id == devInfo->id) {
                m_isUKeyDevAttached = true;
                m_devInfo = devInfo;
                isFound = true;
                break;
            }
        }
        if (!isFound) {
            m_isUKeyDevAttached = false;
            m_devInfo = nullptr;
        }
    } else {
        return;
    }
    if (isUKeyDevAttached != m_isUKeyDevAttached) {
        if (!m_isUKeyDevAttached || !m_devInfo) {
            if (m_isUKeyDevBinding) {
                // onSwitchUIState(UISTATE_NODEV);
                showErrorMessageTip(tr("Security key binding failed!"));
            }
        } else {
            if (m_isUKeyDevBinding) {
                onBind();
            }
        }
    }
}

void SecurityKeySetDlg::setServiceProxy(PwdlessProxy *proxy)
{
    if (!proxy)
        return;

    m_pwdlessProxy = proxy;
    initConnetions();

    if (chkServiceBound()) {
        onSwitchUIState(UISTATE_BOUND);
    } else {
        onSwitchUIState(UISTATE_UNBOUND);
    }

    connect(m_pwdlessProxy, &PwdlessProxy::ukeyInserted, this, &SecurityKeySetDlg::onServiceUkeyInserted);
    connect(m_pwdlessProxy, &PwdlessProxy::ukeyRemoved, this, &SecurityKeySetDlg::onServiceUkeyRemoved);
}

void SecurityKeySetDlg::initUI()
{
    setFixedSize(420, 396);
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
    this->setWindowTitle(tr("Security Key"));
    this->setWindowIcon("ukui-control-center");

    m_layoutMain = new QVBoxLayout();
    m_layoutMain->setContentsMargins(24, 24, 24, 24);
    m_layoutMain->setSpacing(0);
    m_layoutTop = new QVBoxLayout();
    m_layoutTop->setContentsMargins(0, 0, 0, 0);
    m_layoutTop->setSpacing(0);
    m_layoutBottom = new QHBoxLayout();
    m_layoutBottom->setContentsMargins(0, 0, 0, 0);
    m_layoutBottom->setSpacing(16);

    // 分页面初始化UI
    m_widgetNoDev = new QWidget();
    QVBoxLayout *layoutNoDev = new QVBoxLayout(m_widgetNoDev);
    layoutNoDev->setContentsMargins(0, 0, 0, 0);
    layoutNoDev->setAlignment(Qt::AlignCenter);
    m_labelLoading = new QLabel();
    m_loadingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(28, 28);
    m_labelLoading->setProperty("useIconHighlightEffect", 0x2);
    m_labelLoading->setPixmap(m_loadingPixmap);
    m_labelLoading->setAlignment(Qt::AlignCenter);
    m_labelDetachedTip = new QLabel();
    m_labelDetachedTip->setText(tr("Please insert the security key into the USB port"));
    m_labelDetachedTip->setAlignment(Qt::AlignCenter);
    m_labelDetachedTip->setWordWrap(true);
    layoutNoDev->addWidget(m_labelLoading);
    layoutNoDev->addWidget(m_labelDetachedTip);
    m_layoutTop->addWidget(m_widgetNoDev);

    m_widgetNoDevTip = new QWidget();
    QVBoxLayout *layoutNoDevTip = new QVBoxLayout(m_widgetNoDevTip);
    layoutNoDevTip->setContentsMargins(0, 0, 0, 0);
    layoutNoDevTip->setAlignment(Qt::AlignCenter);
    m_labelNoDevTipLabel = new QLabel();
    m_labelNoDevTipLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32, 32));
    m_labelNoDevTipLabel->setAlignment(Qt::AlignCenter);
    m_labelNoDevTipText = new QLabel();
    m_labelNoDevTipText->setText(tr("The security key was not found, Please check if the security key has been "
                                    "inserted and the corresponding security key driver has been installed."));
    m_labelNoDevTipText->setAlignment(Qt::AlignCenter);
    m_labelNoDevTipText->setWordWrap(true);
    layoutNoDevTip->addWidget(m_labelNoDevTipLabel);
    layoutNoDevTip->addWidget(m_labelNoDevTipText);
    m_layoutTop->addWidget(m_widgetNoDevTip);
    m_widgetNoDevTip->hide();

    m_timerLoading = new QTimer();
    m_timerLoading->setInterval(200);

    m_timerLoadingTip = new QTimer();
    m_timerLoadingTip->setInterval(60000);

    m_widgetChkPin = new QWidget();
    QVBoxLayout *layoutChkPin = new QVBoxLayout(m_widgetChkPin);
    layoutChkPin->setContentsMargins(0, 0, 0, 0);
    layoutChkPin->setAlignment(Qt::AlignCenter);
    m_labelChkLogo = new QLabel();
    m_labelChkLogo->setPixmap(QIcon(":/imgs/resources/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelChkLogo->setAlignment(Qt::AlignCenter);
    m_labelChkLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelSecurityKey = new QLabel();
    m_labelSecurityKey->setText(tr("Enter security key password"));
    m_labelSecurityKey->setAlignment(Qt::AlignCenter);
    QFont fontLabel = m_labelSecurityKey->font();
    fontLabel.setBold(true);
    m_labelSecurityKey->setFont(fontLabel);
    m_editSecurityKey = new QLineEdit();
    m_editSecurityKey->setFixedSize(QSize(370, 36));
    m_editSecurityKey->setPlaceholderText(tr("Password"));
    m_editSecurityKey->setEchoMode(QLineEdit::Password);
    m_editSecurityKey->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_editSecurityKey->setTextMargins(0, 0, 30, 0);
    m_editSecurityKey->setContextMenuPolicy(Qt::NoContextMenu);
    m_editSecurityKey->installEventFilter(this);
    m_btnPwdEye = new QPushButton;
    m_btnPwdEye->setFixedSize(QSize(24, 24));
    m_btnPwdEye->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    m_btnPwdEye->setCursor(Qt::PointingHandCursor);
    m_btnPwdEye->setFlat(true);
    m_btnPwdEye->setStyleSheet("QPushButton::pressed{border:none;background-color:transparent}"
                               "QPushButton::hover::!pressed{border:none;background-color:transparent}");
    m_btnPwdEye->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_btnPwdEye->hide();
    m_labelChkMsg = new QLabel();
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    m_labelChkMsg->setPalette(pe);
    QHBoxLayout *layoutPwd = new QHBoxLayout;
    layoutPwd->addStretch();
    layoutPwd->addWidget(m_btnPwdEye);
    layoutPwd->setContentsMargins(0, 0, 8, 0);
    m_editSecurityKey->setLayout(layoutPwd);
    layoutChkPin->addWidget(m_labelChkLogo);
    layoutChkPin->addSpacing(54);
    layoutChkPin->addWidget(m_labelSecurityKey);
    layoutChkPin->addSpacing(12);
    layoutChkPin->addWidget(m_editSecurityKey);
    layoutChkPin->addWidget(m_labelChkMsg);
    layoutChkPin->addStretch();
    m_layoutTop->addWidget(m_widgetChkPin);

    m_widgetBound = new QWidget();
    QVBoxLayout *layoutBound = new QVBoxLayout(m_widgetBound);
    layoutBound->setContentsMargins(0, 0, 0, 0);
    layoutBound->setAlignment(Qt::AlignCenter);
    m_labelBoundLogo = new QLabel();
    m_labelBoundLogo->setPixmap(QIcon(":/imgs/resources/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelBoundLogo->setAlignment(Qt::AlignCenter);
    m_labelBoundLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelBoundTip = new QTextEdit();
    m_labelBoundTip->setText(tr("The security key password has been set, please keep it properly. To unbind the "
                                "security key, click Unbind.\n"));
    m_labelBoundTip->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_labelBoundTip->setFixedWidth(372);
    m_labelBoundTip->setStyleSheet("QTextEdit{background:palette(base)}");
    m_labelBoundTip->setTextInteractionFlags(Qt::NoTextInteraction);
    m_labelBoundTip->viewport()->setCursor(Qt::ArrowCursor);
    //    m_labelBoundTip->setWordWrap(true);
    m_btnUnbind = new QPushButton();
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

    m_widgetUnbound = new QWidget();
    QVBoxLayout *layoutUnBound = new QVBoxLayout(m_widgetUnbound);
    layoutUnBound->setContentsMargins(0, 0, 0, 0);
    layoutUnBound->setAlignment(Qt::AlignCenter);
    m_labelUnboundLogo = new QLabel();
    m_labelUnboundLogo->setPixmap(QIcon(":/imgs/resources/ukui-ukey-symbolic.svg").pixmap(64, 64));
    m_labelUnboundLogo->setAlignment(Qt::AlignCenter);
    m_labelUnboundLogo->setProperty("useIconHighlightEffect", 0x2);
    m_labelUnboundTitle = new QLabel();
    m_labelUnboundTitle->setText(tr("Bind Security Key"));
    m_labelUnboundTitle->setAlignment(Qt::AlignCenter);
    fontLabel = m_labelUnboundTitle->font();
    fontLabel.setBold(true);
    m_labelUnboundTitle->setFont(fontLabel);
    m_labelUnboundTip = new QTextEdit();
    m_labelUnboundTip->setReadOnly(true);
    m_labelUnboundTip->setTextInteractionFlags(Qt::NoTextInteraction);
    m_labelUnboundTip->setFrameShape(QFrame::Shape::NoFrame);
    m_labelUnboundTip->viewport()->setCursor(Qt::ArrowCursor);
    QString strUserName = QString("%1").arg(m_uid);
    struct passwd *pwInfo = getpwuid(m_uid);
    if (pwInfo) {
        if (pwInfo->pw_name && pwInfo->pw_name[0] != '\0') {
            strUserName = QString(pwInfo->pw_name);
        }
        if (pwInfo->pw_gecos && pwInfo->pw_gecos[0] != '\0') {
            QString strGecos = QString(pwInfo->pw_gecos);
            QStringList strGecosList = strGecos.split(QLatin1Char(','));
            strUserName = strGecosList[0];
        }
    }
    m_labelUnboundTip->setText(tr("You will bind your security key so that you can log in to the system as '%1'. If "
                                  "you need to bind, please click 'OK'.\n")
                                   .arg(strUserName));
    m_labelUnboundTip->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_labelUnboundTip->setFixedWidth(372);
    m_labelUnboundTip->setStyleSheet("QTextEdit{background:palette(base)}");
    m_labelUnboundTip->setTextInteractionFlags(Qt::NoTextInteraction);
    m_labelUnboundTip->viewport()->setCursor(Qt::ArrowCursor);
    //    m_labelUnboundTip->setWordWrap(true);
    layoutUnBound->addWidget(m_labelUnboundLogo);
    layoutUnBound->addSpacing(40);
    layoutUnBound->addWidget(m_labelUnboundTitle);
    layoutUnBound->addSpacing(12);
    layoutUnBound->addWidget(m_labelUnboundTip);
    layoutUnBound->addSpacing(48);
    m_layoutTop->addWidget(m_widgetUnbound);

    m_layoutBottom->setAlignment(Qt::AlignRight);
    m_btnCancel = new QPushButton();
    m_btnCancel->setText(tr("Cancel"));
    m_btnCancel->setMinimumSize(96, 36);
    m_layoutBottom->addWidget(m_btnCancel);
    m_btnCancel->setProperty("useButtonPalette", true);
    m_btnSure = new QPushButton();
    m_btnSure->setText(tr("OK"));
    m_btnSure->setMinimumSize(96, 36);
    m_btnSure->setProperty("isImportant", true);
    m_layoutBottom->addWidget(m_btnSure);

    m_layoutMain->addLayout(m_layoutTop);
    m_layoutMain->addLayout(m_layoutBottom);
    this->mainWidget()->setLayout(m_layoutMain);
}

void SecurityKeySetDlg::initConnetions()
{
    connect(m_timerLoading, &QTimer::timeout, this, &SecurityKeySetDlg::updateLoading);
    connect(m_timerLoadingTip, &QTimer::timeout, this, &SecurityKeySetDlg::updateLoadingTip);
    connect(m_btnCancel, &QPushButton::clicked, this, &SecurityKeySetDlg::onBtnCancel);
    connect(m_btnSure, &QPushButton::clicked, this, &SecurityKeySetDlg::onBtnSure);

    connect(m_btnPwdEye, &QPushButton::clicked, this, [=]() {
        if (m_editSecurityKey->echoMode() == QLineEdit::Password) {
            m_editSecurityKey->setEchoMode(QLineEdit::Normal);
            m_btnPwdEye->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
        } else {
            m_editSecurityKey->setEchoMode(QLineEdit::Password);
            m_btnPwdEye->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
        }
    });

    connect(m_editSecurityKey, &QLineEdit::textEdited, [=](QString txt) { refreshConfirmBtnStatus(); });

    connect(m_btnUnbind, &QPushButton::clicked, this, &SecurityKeySetDlg::onUnBind);

    QDBusInterface *iface = new QDBusInterface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus(),
        this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
    QDBusInterface *interfaceScreensaver = nullptr;
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(sessionDbus)) {
        interfaceScreensaver
            = new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);
    } else {
        interfaceScreensaver
            = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);
    }

    connect(interfaceScreensaver, SIGNAL(lock()), this, SLOT(onSysLock()));
    connect(interfaceScreensaver, SIGNAL(unlock()), this, SLOT(onSysUnLock()));
}

void SecurityKeySetDlg::updateLoading()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_loadingPixmap = m_loadingPixmap.transformed(matrix, Qt::FastTransformation);
    m_labelLoading->setPixmap(m_loadingPixmap);
}

void SecurityKeySetDlg::updateLoadingTip()
{
    onSwitchUIState(UISTATE_NODEVTIP);
}

void SecurityKeySetDlg::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent:" << ops;
    if (m_devInfo && ops != IDLE) {
        qDebug() << "StopOps:" << m_devInfo->id;
        m_serviceInterface->call("StopOps", m_devInfo->id, 3000);
    }
}

void SecurityKeySetDlg::accept()
{
    close();
}

void SecurityKeySetDlg::reject()
{
    close();
}

void SecurityKeySetDlg::onSwitchUIState(int nState)
{
    if (m_curUIState == nState)
        return;

    switch (nState) {
        case UISTATE_NODEV: {
            if (m_widgetNoDev) {
                m_widgetNoDev->show();
                if (m_timerLoading)
                    m_timerLoading->start();
                if (m_timerLoadingTip)
                    m_timerLoadingTip->start();
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
            if (m_widgetNoDevTip) {
                m_widgetNoDevTip->hide();
            }
            m_btnSure->hide();
            m_btnSure->setFocusPolicy(Qt::NoFocus);
            m_btnCancel->show();
            m_btnCancel->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_btnCancel);
            m_btnCancel->setFocus();
            m_btnCancel->setDefault(true);
            m_btnCancel->setEnabled(true);
        } break;
        case UISTATE_CHKPIN: {
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
            if (m_widgetNoDevTip) {
                m_widgetNoDevTip->hide();
            }
            if (m_timerLoadingTip)
                m_timerLoadingTip->stop();
            m_btnSure->show();
            m_btnCancel->show();
            m_btnCancel->setFocusPolicy(Qt::NoFocus);
            m_btnSure->setFocusPolicy(Qt::NoFocus);
            refreshConfirmBtnStatus();
        } break;
        case UISTATE_BOUND: {
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
            if (m_widgetNoDevTip) {
                m_widgetNoDevTip->hide();
            }
            if (m_timerLoadingTip)
                m_timerLoadingTip->stop();
            m_btnSure->hide();
            m_btnSure->setFocusPolicy(Qt::NoFocus);
            m_btnCancel->show();
            m_btnCancel->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_btnCancel);
            m_btnCancel->setFocus();
            m_btnCancel->setDefault(true);
            m_btnCancel->setEnabled(true);
        } break;
        case UISTATE_UNBOUND: {
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
            if (m_widgetNoDevTip) {
                m_widgetNoDevTip->hide();
            }
            if (m_timerLoadingTip)
                m_timerLoadingTip->stop();
            m_btnSure->show();
            m_btnCancel->show();
            m_btnCancel->setFocusPolicy(Qt::NoFocus);
            m_btnSure->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_btnSure);
            m_btnSure->setFocus();
            m_btnSure->setDefault(true);
            m_btnSure->setEnabled(true);
            m_btnCancel->setEnabled(true);
        } break;
        case UISTATE_NODEVTIP: {
            if (m_widgetNoDev) {
                m_widgetNoDev->hide();
                if (m_timerLoading)
                    m_timerLoading->stop();
                if (m_timerLoadingTip)
                    m_timerLoadingTip->stop();
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
            if (m_widgetNoDevTip) {
                m_widgetNoDevTip->show();
            }
            m_btnCancel->hide();
            m_btnSure->show();
            m_btnSure->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_btnSure);
            m_btnSure->setFocus();
            m_btnSure->setDefault(true);
            m_btnSure->setEnabled(true);
        } break;
        default:
            return;
    }
    m_curUIState = nState;
}

void SecurityKeySetDlg::onBtnCancel()
{
    reject();
}

void SecurityKeySetDlg::onBtnSure()
{
    if (isPwdlessKey()) {
        onServiceBtnSure();
        return;
    }

    if (m_curUIState == UISTATE_UNBOUND) {
        onBind();
    } else if (m_curUIState == UISTATE_CHKPIN) {
        if (m_isUKeyDevBinding) {
            SetExtraInfo("pin code", m_editSecurityKey->text());
        }
    } else {
        accept();
    }
}

int SecurityKeySetDlg::enroll(int drvId, int u_id, int i_dx, const QString &idx_Name)
{
    if (ops == ENROLL) {
        return 1;
    }
    QList<QVariant> args;
    args << drvId << u_id << i_dx << idx_Name;
    qDebug() << "args:" << args;
    m_serviceInterface->callWithCallback(
        "Enroll", args, this, SLOT(enrollCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = ENROLL;
    m_hisOps = ENROLL;
    m_listEnrollArgs = args;
    return 0;
}

void SecurityKeySetDlg::enrollCallBack(const QDBusMessage &reply)
{
    int result = reply.arguments()[0].value<int>();
    qDebug() << "Enroll result: " << result;

    switch (result) {
        case DBUS_RESULT_SUCCESS: { /* 录入成功 */
            m_hisOps = IDLE;
            m_isUKeyDevBinding = false;
            onSwitchUIState(UISTATE_BOUND);
            break;
        }
        default:
            handleErrorResult(result);
            break;
    }
    ops = IDLE;
}

void SecurityKeySetDlg::errorCallBack(const QDBusError &error)
{
    m_hisOps = IDLE;
    qDebug() << "DBus Error: " << error.message();
    accept();
}

void SecurityKeySetDlg::handleErrorResult(int error)
{
    if (!m_devInfo) {
        showErrorMessageTip(tr("Security key binding failed!"));
        return;
    }
    qDebug() << "error = " << error;
    switch (error) {
        case DBUS_RESULT_ERROR: {
            // 操作失败，需要进一步获取失败原因
            QDBusMessage msg = m_serviceInterface->call("GetOpsMesg", m_devInfo->id);
            if (msg.type() == QDBusMessage::ErrorMessage) {
                qDebug() << "UpdateStatus error: " << msg.errorMessage();
                m_hisOps = IDLE;
                showErrorMessageTip(tr("Security key binding failed!"));
                return;
            }
            QString strMsg = msg.arguments().first().value<QString>();
            qDebug() << "OpsMesg:" << strMsg;
            StatusReslut ret = UpdateStatus(m_devInfo->id);
            qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                     << "," << ret.opsStatus << "," << ret.notifyMessageId;

            if (ret.opsStatus == OPS_ENROLL_FAIL) {
                showChkMessageTip(tr("Security key binding failed!"));
            }
            if (ret.opsStatus == OPS_ENROLL_FAIL || ret.opsStatus == OPS_ENROLL_TIMEOUT) {
                if (!m_uSysState) {
                    QTimer::singleShot(1, this, [&, this]() { enroll(m_devInfo->id, m_uid, m_idx, m_idxName); });
                }
            } else {
                m_hisOps = IDLE;
                showErrorMessageTip(tr("Security key binding failed!"));
            }
            break;
        }
        case DBUS_RESULT_DEVICEBUSY:
            // 设备忙
            m_hisOps = IDLE;
            showErrorMessageTip(tr("Security key binding failed!"));
            break;
        case DBUS_RESULT_NOSUCHDEVICE:
            // 设备不存在
            m_hisOps = IDLE;
            showErrorMessageTip(tr("Security key binding failed!"));
            break;
        case DBUS_RESULT_PERMISSIONDENIED:
            // 没有权限
            onBtnCancel();
            m_hisOps = IDLE;
            break;
    }
}

StatusReslut SecurityKeySetDlg::UpdateStatus(int drvid)
{
    StatusReslut status;
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("UpdateStatus"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "UpdateStatus error:" << result.errorMessage();
        status.result = -1;
        return status;
    }

    status.result = result.arguments().at(0).toInt();
    status.enable = result.arguments().at(1).toInt();
    status.devNum = result.arguments().at(2).toInt();
    status.devStatus = result.arguments().at(3).toInt();
    status.opsStatus = result.arguments().at(4).toInt();
    status.notifyMessageId = result.arguments().at(5).toInt();

    return status;
}

QStringList SecurityKeySetDlg::getFeaturelist(int uid, int indexStart, int indexEnd)
{
    QStringList list;
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("GetAllFeatureList"), uid, indexStart, indexEnd);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return list;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        if (featureInfo->biotype == DeviceType::GeneralUkey)
            list.append(featureInfo->index_name);
        delete featureInfo;
    }
    return list;
}

QStringList SecurityKeySetDlg::getFeatureDevNamelist(int uid)
{
    QStringList list;
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("GetAllFeatureList"), uid, 0, -1);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return list;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        if (featureInfo->biotype == DeviceType::GeneralUkey)
            list.append(featureInfo->device_shortname);
        delete featureInfo;
    }
    return list;
}

DeviceList SecurityKeySetDlg::GetUKeyDrvList()
{
    if (!m_serviceInterface)
        return DeviceList();
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("GetDrvList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return DeviceList();
    }
    auto dbusArg = result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    DeviceList deviceList;
    dbusArg >> variantList;
    for (int i = 0; i < variantList.size(); i++) {
        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();

        auto arg = variantList.at(i).value<QDBusArgument>();
        arg >> *pDeviceInfo;
        if (pDeviceInfo->deviceType == DeviceType::GeneralUkey)
            deviceList.push_back(pDeviceInfo);
    }

    return deviceList;
}

bool SecurityKeySetDlg::chkServiceBound()
{
    if (!m_pwdlessProxy)
        return false;

    QStringList authMethods = m_pwdlessProxy->userAuthMethods(m_name);
    if (authMethods.contains("ukey"))
        return true;
    else
        return false;
}

bool SecurityKeySetDlg::chkBound()
{
    QStringList featureList = getFeaturelist(m_uid, 0, -1);
    if (featureList.size() > 0) {
        return true;
    } else {
        return false;
    }
}

int SecurityKeySetDlg::deleteFeature(int drvid, int uid, int indexStart, int indexEnd)
{
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("Clean"), drvid, uid, indexStart, indexEnd);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "deleteFeature error:" << result.errorMessage();
        return false;
    }
    return result.arguments().first().value<int>();
}

int SecurityKeySetDlg::SetExtraInfo(QString strInfoType, QString strExtraInfo)
{
    QDBusMessage result = m_serviceInterface->call(QStringLiteral("SetExtraInfo"), strInfoType, strExtraInfo);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "SetExtraInfo error:" << result.errorMessage();
        return false;
    }
    return result.arguments().first().value<int>();
}

void SecurityKeySetDlg::onBind()
{
    m_isUKeyDevBinding = true;
    disableDialog(true);
    QTimer::singleShot(0, this, [=]() {
        if (authoriyUser(true)) {
            if (m_devInfo) {
                QStringList list = getFeaturelist(m_uid, 0, -1);
                QString featurename;
                int num = 1;
                while (1) {
                    featurename = "UKey" + QString::number(num);
                    if (!list.contains(featurename))
                        break;
                    num++;
                }
                int nResult = enroll(m_devInfo->id, m_uid, -1, featurename);
                if (nResult == 0 || nResult == 1) {
                    disableDialog(false);
                    onSwitchUIState(UISTATE_CHKPIN);
                } else {
                    disableDialog(false);
                }
            } else {
                onSwitchUIState(UISTATE_NODEV);
            }
        } else {
            disableDialog(false);
        }
    });
}

void SecurityKeySetDlg::onUnBind()
{
    if (isPwdlessKey()) {
        onServiceUnBind();
        return;
    }
    m_isUKeyDevUnBinding = true;
    disableDialog(true);
    QTimer::singleShot(0, this, [=]() {
        if (authoriyUser(false)) {
            QStringList featureDevNameList = getFeatureDevNamelist(m_uid);
            DeviceList deviceList = GetUKeyDrvList();
            int delResult = 0;
            for (auto device : deviceList) {
                for (auto devName : featureDevNameList) {
                    if (devName == device->shortName) {
                        qDebug() << "deleteFeature:" << device->id << "," << m_uid;
                        delResult = deleteFeature(device->id, m_uid, 0, -1);
                        break;
                    }
                }
            }
            if (delResult == 0) {
                accept();
            } else {
                showErrorMessageTip(tr("Security key unbinding failed!"));
            }
            disableDialog(false);
        } else {
            disableDialog(false);
        }
    });
}

void SecurityKeySetDlg::showErrorMessageTip(QString message)
{
    hide();
    QMessageBox criitical(QMessageBox::Information, "", message, QMessageBox::NoButton, this);
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

bool SecurityKeySetDlg::authoriyUser(bool isEnroll)
{
    PolkitQt1::Authority::Result result;

    if (isEnroll) {
        result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
            "org.ukui.biometric.enroll-ukey-data",
            PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
            PolkitQt1::Authority::AllowUserInteraction);
    } else {
        result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
            "org.ukui.biometric.clean-ukey-data",
            PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
            PolkitQt1::Authority::AllowUserInteraction);
    }

    if (result == PolkitQt1::Authority::Yes) { // 认证通过
        qDebug() << QString("operation authorized") << result;
        return true;
    } else {
        qDebug() << QString("not authorized") << result;
        return false;
    }
}

void SecurityKeySetDlg::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    bool isChanged = false;
    if (sleep) {
        if (!(m_uSysState & SYSSTATE_SLEEP)) {
            m_uSysState |= SYSSTATE_SLEEP;
            isChanged = true;
        }
    } else {
        if (m_uSysState & SYSSTATE_SLEEP) {
            m_uSysState &= ~SYSSTATE_SLEEP;
            isChanged = true;
        }
    }
    if (isChanged) {
        chkSysState();
    }
}

void SecurityKeySetDlg::onSysLock()
{
    if (!(m_uSysState & SYSSTATE_LOCK)) {
        m_uSysState |= SYSSTATE_LOCK;
        chkSysState();
    }
}

void SecurityKeySetDlg::onSysUnLock()
{
    if (m_uSysState & SYSSTATE_LOCK) {
        m_uSysState &= ~SYSSTATE_LOCK;
        chkSysState();
    }
}

void SecurityKeySetDlg::chkSysState()
{
    qInfo() << "chkSysState:" << m_uSysState << "," << m_hisOps;
    if (!m_devInfo) {
        return;
    }
    if (m_uSysState) { // 停止
        m_serviceInterface->call("StopOps", m_devInfo->id, 1);
    } else { // 恢复
        if (m_hisOps == IDLE)
            return;
        switch (m_hisOps) {
            case ENROLL: {
                m_serviceInterface->callWithCallback(
                    "Enroll",
                    this->m_listEnrollArgs,
                    this,
                    SLOT(enrollCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            default:
                break;
        }
    }
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

void SecurityKeySetDlg::refreshConfirmBtnStatus()
{
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

void SecurityKeySetDlg::onServiceBtnSure()
{
    if (m_curUIState == UISTATE_UNBOUND) {
        if (m_pwdlessProxy) {
            if (m_pwdlessProxy->ukeyStatus() == 1) {
                onSwitchUIState(UISTATE_CHKPIN);
            } else {
                onSwitchUIState(UISTATE_NODEV);
            }
        }
    } else if (m_curUIState == UISTATE_CHKPIN) {
        onServiceBind();
    } else {
        accept();
    }
}

void SecurityKeySetDlg::onServiceBind()
{
    if (m_pwdlessProxy) {
        int ret;
        QString errMsg;
        ret = m_pwdlessProxy->ukeyBind(m_name, m_editSecurityKey->text(), errMsg);
        if (ret == 0) {
            // success
            onSwitchUIState(UISTATE_BOUND);
        } else {
            showErrorMessageTip(errMsg);
        }
    }
}

void SecurityKeySetDlg::onServiceUnBind()
{
    if (m_pwdlessProxy) {
        QString errorMsg;
        disableDialog(true);
        int ret = m_pwdlessProxy->ukeyUnbind(m_name, errorMsg);
        if (ret == 0) {
            qDebug() << "success";
            accept();
        } else {
            qDebug() << "failed";
            showErrorMessageTip(errorMsg);
        }
        disableDialog(false);
    }
}

void SecurityKeySetDlg::onServiceUkeyInserted(QString user)
{
    if (m_curUIState == UISTATE_NODEVTIP) {
        return;
    }

    if (user == m_name || user == "nobody")
        onSwitchUIState(UISTATE_CHKPIN);
}

void SecurityKeySetDlg::onServiceUkeyRemoved(QString user)
{
    if (user == m_name || user == "nobody")
        showErrorMessageTip(tr("Security key binding failed!"));
}
