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
#include "authdialog.h"
#include "iconedit.h"
#include "customlabel.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "statusbutton.h"
#include "commonfunc.h"
#include "common/configuration.h"
#include "biobuttonlistwidget.h"
#include "definetypes.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QDialogButtonBox>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QPixmapCache>
#include "definetypes.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
#include "../lock-dialog/pam-tally.h"
#include "../../src/dbusifs/freedesktophelper.h"
#include "global_utils.h"
#include "binddialog.h"
#include "securityquestionanswer.h"
#include "circlelabel.h"

#define WIDGET_HEIGHT_HEADTIP (40)
#define WIDGET_HEIGHT_HEADIMAGE ((int)(154 * scale))
#define WIDGET_HEIGHT_USERNAME (40)
#define WIDGET_HEIGHT_AUTHMETHOD (40)
#define WIDGET_HEIGHT_PASSWDEDIT (80)
#define WIDGET_HEIGHT_PAMUKEYLOADINGWDEDIT (100)
#define WIDGET_HEIGHT_SPACING ((int)(16 * scale))
#define WIDGET_HEIGHT_CUSTOM                                                                                           \
    (WIDGET_HEIGHT_HEADTIP + WIDGET_HEIGHT_HEADIMAGE + WIDGET_HEIGHT_USERNAME + WIDGET_HEIGHT_PASSWDEDIT               \
     + WIDGET_HEIGHT_SPACING * 3)
#define WIDGET_HEIGHT_LOGINOPTS (110)

AuthDialog::AuthDialog(LockDialogModel *model, UserInfoPtr userInfo, QWidget *parent)
    : QWidget(parent), m_modelLockDialog(model), m_isLockingFlg(false), m_isInputPasswd(false), m_curUserInfo(userInfo)
{
    pam_tally_init();
    initUI();
}

void AuthDialog::initUI()
{
    installEventFilter(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(WIDGET_HEIGHT_SPACING);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    // 生物认证空占位
    m_widgetPlaceholder = new QWidget(this);
    m_widgetPlaceholder->hide();
    // 生物认证提示
    m_tipWidget = new QWidget(this);

    // 头像
    m_headerWidget = new QWidget(this);

    // 用户名
    m_nameWidget = new QWidget(this);

    // 密码输入框
    m_editWidget = new QWidget(this);

    // ukey
    m_ukeyPasswdWidget = new QWidget(this);

    // custom widget
    m_customWidget = new QWidget(this);

    // login option
    m_widgetLoginOpts = new QWidget(this);
    // pam ukey loading widget
    m_pamUkeyLoadingWidget = new QWidget(this);
    m_pamUkeyLoadingWidget->hide();

    mainLayout->addStretch();
    mainLayout->addWidget(m_widgetPlaceholder);
    mainLayout->addWidget(m_tipWidget);
    mainLayout->addWidget(m_headerWidget);
    mainLayout->addWidget(m_nameWidget);
    mainLayout->addWidget(m_editWidget);
    mainLayout->addWidget(m_pamUkeyLoadingWidget);
    mainLayout->addWidget(m_ukeyPasswdWidget);
    mainLayout->addWidget(m_customWidget);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(m_widgetLoginOpts);
    mainLayout->addStretch();
    initTipWidget();
    initHeaderWidget();
    initNameWidget();
    initEditWidget();
    initCustomAuthWidget();
    initLoginoptionWidget();

    initConnections();
}

void AuthDialog::initConnections()
{
    connect(m_modelLockDialog, &LockDialogModel::pamShowMessage, this, &AuthDialog::onPamShowMessage);
    connect(m_modelLockDialog, &LockDialogModel::pamShowPrompt, this, &AuthDialog::onPamShowPrompt);
    connect(m_modelLockDialog, &LockDialogModel::pamShowBinaryPrompt, this, &AuthDialog::onPamShowBinaryPrompt);
    connect(m_modelLockDialog, &LockDialogModel::pamAuthCompleted, this, &AuthDialog::onPamAuthCompleted);

    connect(m_passwordEdit, SIGNAL(clicked(const QString &)), this, SLOT(onRespond(const QString &)));

    connect(m_messageButton, &QPushButton::clicked, this, &AuthDialog::onMessageButtonClicked);
}

bool AuthDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        MyListWidget *listWidget = qobject_cast<MyListWidget *>(obj);
        if (listWidget) {
            if (listWidget && listWidget->hasFocus()) { // Tab键切入时，设置焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::SelectCurrent);
                listWidget->currentItemChanged(listWidget->currentItem(), nullptr);
            } else { // Tab键切出时，清空焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::Clear);
            }
        }
    } else if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        if (m_loginOpts == obj) {
            if (m_widgetLoginOpts && event->type() == QEvent::Show) {
                Q_EMIT bottomPositionChanged(
                    mapToParent(m_widgetLoginOpts->geometry().topLeft()).y() + m_widgetLoginOpts->height());
            } else {
                Q_EMIT bottomPositionChanged(mapToParent(m_widgetLoginOpts->geometry().topLeft()).y());
            }
        } else if (this == obj && event->type() == QEvent::Show) {
            pam_tally_init();
        }
    }

    return QWidget::eventFilter(obj, event);
}

void AuthDialog::resizeEvent(QResizeEvent *event)
{
    if (m_widgetSize == event->size()) {
        return;
    }
    m_widgetSize = event->size();
    updateAuthSize();
}

void AuthDialog::updateAuthSize()
{
    this->layout()->setSpacing(WIDGET_HEIGHT_SPACING);
    if (m_tipWidget)
        m_tipWidget->setFixedSize(this->width(), WIDGET_HEIGHT_HEADTIP);
    if (m_headerWidget)
        m_headerWidget->setFixedSize(this->width(), WIDGET_HEIGHT_HEADIMAGE);
    if (m_nameWidget)
        m_nameWidget->setFixedSize(this->width(), WIDGET_HEIGHT_USERNAME);
    if (m_editWidget && m_authMethodLabel && m_authMethodLabel->isVisible())
        m_editWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT + WIDGET_HEIGHT_AUTHMETHOD);
    else
        m_editWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);
    if (m_widgetPlaceholder) {
        m_widgetPlaceholder->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);
    }
    if (m_ukeyPasswdWidget)
        m_ukeyPasswdWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);
    if (m_pamUkeyLoadingWidget)
        m_pamUkeyLoadingWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PAMUKEYLOADINGWDEDIT);

    if (m_widgetLoginOpts) {
        m_widgetLoginOpts->setFixedSize(this->width(), WIDGET_HEIGHT_LOGINOPTS);
        if (m_loginOpts && m_loginOpts->isVisible()) {
            Q_EMIT bottomPositionChanged(
                mapToParent(m_widgetLoginOpts->geometry().topLeft()).y() + m_widgetLoginOpts->height());
        } else {
            Q_EMIT bottomPositionChanged(mapToParent(m_widgetLoginOpts->geometry().topLeft()).y());
        }
    }

    if (m_labelLoginTypeTip) {
        m_labelLoginTypeTip->setFixedSize(m_tipWidget->size());
        m_labelLoginTypeTip->setFontSize((26 + m_curFontSize) * m_ptToPx);
        m_labelLoginTypeTip->setFontWeight(QFont::Medium);
    }
    if (m_labelHeadImg)
        m_labelHeadImg->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
    if (m_labelFace) {
        m_labelFace->setStyleSheet(
            QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,20%);")
                .arg((int)(WIDGET_HEIGHT_HEADIMAGE / 2)));
        m_labelFace->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
        QImage faceImg;
        setFaceImg(faceImg);
    }
    if (m_labelQRCode) {
        m_labelQRCode->setStyleSheet(
            QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);")
                .arg((int)(6 * scale)));
        m_labelQRCode->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
        setQRCode(m_imgQRCode);
    }
    if (m_labelQRCodeTip) {
        m_labelQRCodeTip->setFixedSize(22 * scale, 22 * scale);
    }
    if (m_labelQRCodeMsg) {
        m_labelQRCodeMsg->setFixedHeight(30 * scale);
        m_labelQRCodeMsg->setFontSize((14 + m_curFontSize) * m_ptToPx);
    }
    if (m_labelBioWaiting) {
        m_labelBioWaiting->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
    }
    if (m_nameLabel) {
        m_nameLabel->setFixedSize(this->width(), WIDGET_HEIGHT_USERNAME);
        m_nameLabel->setFontSize((24 + m_curFontSize) * m_ptToPx);
    }
    if (m_passwordEdit) {
        m_passwordEdit->setFixedSize(240, 40);
        m_passwordEdit->updateUI();
    }
    if (m_messageLabel) {
        m_messageLabel->setFixedSize(this->width(), 40);
        m_messageLabel->setFontSize((14 + m_curFontSize) * m_ptToPx);
    }
    if (m_messageButton) {
        m_messageButton->setFixedSize(200, 40);
        QFont font = m_messageButton->font();
        font.setPointSize((14 + m_curFontSize) * m_ptToPx);
        m_messageButton->setFont(font);
    }
    if (m_ukeyPasswordEdit) {
        m_ukeyPasswordEdit->setFixedSize(240, 40);
        m_ukeyPasswordEdit->updateUI();
    }
    if (m_ukeyMessageLabel) {
        m_ukeyMessageLabel->setFixedSize(this->width(), 40);
        m_ukeyMessageLabel->setFontSize((14 + m_curFontSize) * m_ptToPx);
    }
    if (bottomListWidget) {
        bottomListWidget->setFixedSize(
            QSize(48 * bottomListWidget->count() + bottomListWidget->spacing() * 2 * bottomListWidget->count(), 64));
        bottomListWidget->updateUI();
    }
    if (m_tipWidget && bottomListWidget)
        m_optionTip->setFixedSize(bottomListWidget->width(), 32);
    if (m_customWidget)
        m_customWidget->setFixedSize(width(), WIDGET_HEIGHT_CUSTOM);
    updateUI();
}

void AuthDialog::updateAuthFont(QString fontFamily)
{
    if (m_labelLoginTypeTip) {
        m_labelLoginTypeTip->setFontFamily(fontFamily);
    }
    if (m_ukeyMessageLabel) {
        m_ukeyMessageLabel->setFontFamily(fontFamily);
    }
    if (m_nameLabel) {
        m_nameLabel->setFontFamily(fontFamily);
    }
    if (m_messageLabel) {
        m_messageLabel->setFontFamily(fontFamily);
    }
    if (m_loginOpts) {
        m_loginOpts->updateLoginOptionFont(fontFamily);
    }
    if (m_labelQRCodeMsg) {
        m_labelQRCodeMsg->setFontFamily(fontFamily);
    }
}

void AuthDialog::updateAuthFontSize(double fontSize)
{
    if (m_labelLoginTypeTip) {
        m_labelLoginTypeTip->setFontSize((26 + fontSize) * m_ptToPx);
    }
    if (m_ukeyMessageLabel) {
        m_ukeyMessageLabel->setFontSize((14 + fontSize) * m_ptToPx);
    }
    if (m_nameLabel) {
        m_nameLabel->setFontSize((24 + fontSize) * m_ptToPx);
    }
    if (m_messageLabel) {
        m_messageLabel->setFontSize((14 + fontSize) * m_ptToPx);
    }
    if (m_loginOpts) {
        m_loginOpts->updateLoginOptionFontSize((16 + fontSize) * m_ptToPx);
    }
    if (m_labelQRCodeMsg) {
        m_labelQRCodeMsg->setFontSize((14 + fontSize) * m_ptToPx);
    }
    if (m_passwordEdit) {
        m_passwordEdit->setFontSize((16 + fontSize) * m_ptToPx);
    }
}

qint32 AuthDialog::getCurAuthUserUid()
{
    return (double)(m_curUserInfo->uid());
}

QString AuthDialog::getCurAuthRealName()
{
    return m_curUserInfo->fullName();
}

QPixmap AuthDialog::getCurAuthUserFace()
{
    return m_curUserInfo->headImage();
}

int AuthDialog::getUserAuthFailedTimes()
{
    return m_userFailedTimesMap[m_curUserInfo->uid()];
}

void AuthDialog::initTipWidget()
{
    QVBoxLayout *tipLayout = new QVBoxLayout(m_tipWidget);
    tipLayout->setAlignment(Qt::AlignCenter);
    m_labelLoginTypeTip = new CustomLabel(m_tipWidget);
    m_labelLoginTypeTip->setText("");
    m_labelLoginTypeTip->setAlignment(Qt::AlignCenter);
    m_labelLoginTypeTip->setFixedSize(m_tipWidget->size());
}

void AuthDialog::initHeaderWidget()
{
    // 用户头像
    QVBoxLayout *headerLayout = new QVBoxLayout(m_headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    m_labelHeadImg = new CircleLabel(m_headerWidget);
    headerLayout->addWidget(m_labelHeadImg);
}

void AuthDialog::initNameWidget()
{
    QVBoxLayout *nameLayout = new QVBoxLayout(m_nameWidget);
    nameLayout->setAlignment(Qt::AlignHCenter);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    m_nameLabel = new CustomLabel(m_nameWidget);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_lastUserName = m_curUserInfo ? m_curUserInfo->name() : "";
}

void AuthDialog::initEditWidget()
{
    QVBoxLayout *editWidgetLayout = new QVBoxLayout(m_editWidget);
    editWidgetLayout->setContentsMargins(0, 0, 0, 0);
    editWidgetLayout->setAlignment(Qt::AlignHCenter);
    m_pEditLayout = new QHBoxLayout();
    m_pEditLayout->setContentsMargins(0, 0, 0, 0);

    m_authMethodLabel = new CustomLabel(m_editWidget);
    m_authMethodLabel->setAlignment(Qt::AlignHCenter);
    editWidgetLayout->addWidget(m_authMethodLabel);
    m_authMethodLabel->hide();

    syslog(LOG_INFO, "[ukui-screensaver-dialog] init edit contol");
    m_pEditLayout->setAlignment(Qt::AlignHCenter);
    m_passwordEdit = new IconEdit(m_modelLockDialog, m_editWidget);
    m_passwordEdit->setType(QLineEdit::Password);
    syslog(LOG_INFO, "[ukui-screensaver-dialog] init edit contol end");
    // m_passwordEdit->setPrompt("password");
    m_pEditLayout->addWidget(m_passwordEdit);
    connect(m_passwordEdit, &IconEdit::clickedPassword, this, [=] { Q_EMIT lineEditClicked(); });
    connect(m_passwordEdit, &IconEdit::mousePressed, this, [=] { Q_EMIT lineEditMousePressed(); });

    editWidgetLayout->addLayout(m_pEditLayout);

    m_messageButton = new QPushButton(m_editWidget);
    m_messageButton->setObjectName(QStringLiteral("messageButton"));
    QHBoxLayout *msgBtnLayout = new QHBoxLayout();
    msgBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_messageButton->setStyleSheet(
        "QPushButton{text-align:center;color:white;background-color: rgba(255,255,255,15%);border: none;border-radius: 4px;outline: none;} \
                                   QPushButton::hover{background-color: rgba(255,255,255,40%);} \
                                   QPushButton::pressed {background-color: rgba(255,255,255,30%);}");
    m_messageButton->hide();
    msgBtnLayout->addWidget(m_messageButton);
    editWidgetLayout->addLayout(msgBtnLayout);

    m_messageLabel = new CustomLabel(m_editWidget);
    m_messageLabel->setAlignment(Qt::AlignHCenter);
    editWidgetLayout->addWidget(m_messageLabel);
}

void AuthDialog::initUkeyPasswordWidget()
{
    if (!m_ukeyPasswordEdit) {
        QVBoxLayout *ukeyWidgetLayout = new QVBoxLayout(m_ukeyPasswdWidget);
        ukeyWidgetLayout->setContentsMargins(0, 0, 0, 0);
        ukeyWidgetLayout->setAlignment(Qt::AlignHCenter);
        QVBoxLayout *editLayout = new QVBoxLayout();
        editLayout->setContentsMargins(0, 0, 0, 0);
        editLayout->setAlignment(Qt::AlignHCenter);
        m_ukeyPasswordEdit = new IconEdit(m_modelLockDialog, m_ukeyPasswdWidget);
        m_ukeyPasswordEdit->setType(QLineEdit::Password);
        m_ukeyPasswordEdit->setPrompt(tr("Enter the ukey password"));
        editLayout->addWidget(m_ukeyPasswordEdit);
        ukeyWidgetLayout->addLayout(editLayout);
        connect(m_ukeyPasswordEdit, &IconEdit::clicked, this, [=] { Q_EMIT lineEditClicked(); });
        connect(m_ukeyPasswordEdit, &IconEdit::mousePressed, this, [=] { Q_EMIT lineEditMousePressed(); });
        connect(m_ukeyPasswordEdit, SIGNAL(clicked(const QString &)), this, SLOT(onRespondUkey(const QString &)));
        m_ukeyMessageLabel = new CustomLabel(m_ukeyPasswdWidget);
        m_ukeyMessageLabel->setText("");
        m_ukeyMessageLabel->setAlignment(Qt::AlignHCenter);
        ukeyWidgetLayout->addWidget(m_ukeyMessageLabel);
        m_ukeyPasswdWidget->hide();

        m_ukeyLoadingWidget = new QWidget(m_ukeyPasswdWidget);
        m_ukeyLoadingText = new CustomLabel(m_ukeyLoadingWidget);
        m_ukeyLoadingText->setFontSize(18);
        m_ukeyLoadingText->setText(tr("Insert the ukey into the USB port"));
        m_ukeyLoadingButton = new QPushButton(m_ukeyLoadingWidget);
        m_ukeyLoadingButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_ukeyLoadingButton->setFlat(true);
        m_ukeyLoadingButton->installEventFilter(this);
        m_ukeyLoadingButton->setStyleSheet("QPushButton{text-align:center;color: rgba(255, 255, 255, 255);border: "
                                           "none;border-radius: 4px;outline: none;}");
        QVBoxLayout *loadingLayout = new QVBoxLayout(m_ukeyLoadingWidget);
        loadingLayout->setContentsMargins(0, 0, 0, 0);
        loadingLayout->setAlignment(Qt::AlignHCenter);
        loadingLayout->addWidget(m_ukeyLoadingButton);
        loadingLayout->addWidget(m_ukeyLoadingText);
        m_ukeyLoadingWidget->adjustSize();
        ukeyWidgetLayout->addWidget(m_ukeyLoadingWidget);
        m_ukeyLoadingWidget->hide();

        updateUI();
    }
}

void AuthDialog::initLoginoptionWidget()
{
    //    QVBoxLayout *loginoptionLayout = new QVBoxLayout(m_widgetLoginOpts);
    //    loginoptionLayout->setContentsMargins(0, 0, 0, 0);
    //    loginoptionLayout->setSpacing(0);
    //    loginoptionLayout->setContentsMargins(0, 0, 0, 0);
    //    loginoptionLayout->setAlignment(Qt::AlignHCenter);

    //    QHBoxLayout *tipsLayout = new QHBoxLayout();
    //    tipsLayout->setContentsMargins(0,0,0,0);
    //    m_optionTip = new CustomLabel();
    //    m_optionTip->setText("Login Option");
    //    m_optionTip->setAlignment(Qt::AlignHCenter);
    //    tipsLayout->addWidget(m_optionTip);
    //    loginoptionLayout->addLayout(tipsLayout);

    //    QHBoxLayout *btnsLayout = new QHBoxLayout();
    //    bottomListWidget = new BioButtonListWidget(m_widgetLoginOpts);
    //    bottomListWidget->installEventFilter(this);
    //    btnsLayout->addWidget(bottomListWidget);
    //    loginoptionLayout->addLayout(btnsLayout);
}

void AuthDialog::initCustomAuthWidget()
{
    if (m_customWidget) {
        m_layoutCustom = new QHBoxLayout();
        m_layoutCustom->setSpacing(0);
        m_layoutCustom->setContentsMargins(0, 0, 0, 0);
        m_layoutCustom->setAlignment(Qt::AlignCenter);
        m_customWidget->setLayout(m_layoutCustom);
        m_customWidget->hide();
    }
}

void AuthDialog::updateUI()
{
    m_labelHeadImg->setPixmap(m_curUserInfo->headImage());
    if (m_curUserInfo->name() == "*login") {
        m_nameLabel->setText(tr("Enter username"));
    } else if (m_curUserInfo->name() == "*guest") {
        m_nameLabel->setText(tr("Guest"));
    } else {
        m_nameLabel->setText(m_curUserInfo->fullName());
    }
    if (m_fRetryButton) {
        m_fRetryButton->setStyleSheet(
            QString("QPushButton{border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);}"
                    "QPushButton::hover{background-color: rgba(255,255,255,30%);}"
                    "QPushButton::pressed {background-color: rgba(255,255,255,40%);}")
                .arg((int)(77 * scale)));
        m_fRetryButton->setFixedSize(154 * scale, 154 * scale);
        m_fRetryButton->setIconSize(QSize(48 * scale, 48 * scale));
    }
    qDebug() << "AuthDialog updateUI:" << isGreeterMode() << "," << m_curUserInfo->isLoggedIn();
    if (!isGreeterMode() && m_curUserInfo->isLoggedIn()) {
        QPixmap iconLogin = QIcon::fromTheme("system-lock-screen-symbolic").pixmap(12, 12);
        iconLogin = drawSymbolicColoredPixmap(iconLogin, "white");
        if (m_passwordEdit) {
            m_passwordEdit->setIcon(iconLogin, QSize(12, 12));
        }
        if (m_ukeyPasswordEdit) {
            m_ukeyPasswordEdit->setIcon(iconLogin, QSize(12, 12));
        }
    } else {
        QPixmap iconLogin = QIcon::fromTheme("go-next-symbolic").pixmap(20, 20);
        iconLogin = drawSymbolicColoredPixmap(iconLogin, "white");
        if (m_passwordEdit) {
            m_passwordEdit->setIcon(iconLogin, QSize(20, 20));
        }
        if (m_ukeyPasswordEdit) {
            m_ukeyPasswordEdit->setIcon(iconLogin, QSize(20, 20));
        }
    }
    showPwdTipBtn();
}

void AuthDialog::startLoadingUkey()
{
    if (!m_ukeyLoadingWidget) {
        initUkeyPasswordWidget();
    }
    // isLoadingUkey = true;
    if (m_ukeyLoadingWidget)
        m_ukeyLoadingWidget->show();
    if (m_ukeyPasswordEdit) {
        m_ukeyPasswordEdit->hide();
    }
    if (m_ukeyMessageLabel) {
        m_ukeyMessageLabel->hide();
    }
    if (!m_loadingTimer) {
        m_loadingTimer = new QTimer(this);
        m_loadingTimer->setInterval(150);
        connect(m_loadingTimer, &QTimer::timeout, this, &AuthDialog::updateLoadingPixmap);
    }

    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(27, 27);
    m_ukeyLoadingPixmap = drawSymbolicColoredPixmap(icon, "white");
    m_ukeyLoadingButton->setIcon(m_ukeyLoadingPixmap);
    m_ukeyLoadingButton->setIconSize(QSize(27, 27));
    m_loadingTimer->start();

}

void AuthDialog::stopLoadingUkey()
{
    isLoadingUkey = false;
    if (m_ukeyLoadingWidget)
        m_ukeyLoadingWidget->hide();
    if (m_ukeyPasswordEdit) {
        m_ukeyPasswordEdit->show();
    }
    if (m_ukeyMessageLabel) {
        m_ukeyMessageLabel->show();
    }
    if (m_loadingTimer) {
        m_loadingTimer->stop();
    }
}

void AuthDialog::updateLoadingPixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_ukeyLoadingPixmap = m_ukeyLoadingPixmap.transformed(matrix, Qt::FastTransformation);
    m_ukeyLoadingButton->setIcon(QIcon(m_ukeyLoadingPixmap));
}

void AuthDialog::switchLoginOptType(unsigned uLoginOptType, bool faceBtnClicked)
{
    switch (uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD: {
            setLoginTypeTip("");
            setUkeyTypeTip("");
            m_tipWidget->show();
            m_editWidget->show();
            m_widgetPlaceholder->hide();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->show();
            if (m_labelQRCode) {
                m_labelQRCode->hide();
            }
            if (m_labelFace) {
                m_labelFace->hide();
            }
            if (m_labelBioWaiting) {
                m_labelBioWaiting->hide();
            }
            stopBioWaiting();
            if (m_fRetryButton) {
                m_fRetryButton->hide();
            }
            m_ukeyPasswdWidget->hide();
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->clearFocus();
            }
            m_customWidget->hide();
            setFocusProxy(m_passwordEdit);
        } break;
        case LOGINOPT_TYPE_FACE: {
            m_tipWidget->show();
            m_editWidget->hide();
            m_widgetPlaceholder->show();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->hide();
            if (m_labelQRCode) {
                m_labelQRCode->hide();
            }
            if (m_labelFace) {
                m_labelFace->show();
            }
            if (m_labelBioWaiting) {
                m_labelBioWaiting->hide();
            }
            stopBioWaiting();
            if (m_fRetryButton) {
                m_fRetryButton->hide();
            }
            m_ukeyPasswdWidget->hide();
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->clearFocus();
            }
            m_customWidget->hide();
            setFocusProxy(m_passwordEdit);
            if (faceBtnClicked) {
                m_fTimeoutTimes = 0;
            }
        } break;
        case LOGINOPT_TYPE_FINGERPRINT:
        case LOGINOPT_TYPE_VOICEPRINT:
        case LOGINOPT_TYPE_FINGERVEIN:
        case LOGINOPT_TYPE_IRIS: {
            m_tipWidget->show();
            m_editWidget->hide();
            m_widgetPlaceholder->show();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->hide();
            if (m_labelQRCode) {
                m_labelQRCode->hide();
            }
            if (m_labelFace) {
                m_labelFace->hide();
            }
            if (m_labelBioWaiting) {
                m_labelBioWaiting->show();
            }
            startBioWaiting(uLoginOptType);
            if (m_fRetryButton) {
                m_fRetryButton->hide();
            }
            m_ukeyPasswdWidget->hide();
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->clearFocus();
            }
            m_customWidget->hide();
            setFocusProxy(m_passwordEdit);
        } break;
        case LOGINOPT_TYPE_QRCODE: {
            m_tipWidget->show();
            m_editWidget->hide();
            m_widgetPlaceholder->show();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->hide();
            // 延迟检查错误状态
            m_isNetworkErr = false;
            QTimer::singleShot(500, this, [&, this]() {
                if (!this->m_isNetworkErr) {
                    setQRCodeMsg("");
                }
            });
            if (m_labelQRCode) {
                m_labelQRCode->show();
            }
            if (m_labelFace) {
                m_labelFace->hide();
            }
            if (m_labelBioWaiting) {
                m_labelBioWaiting->hide();
            }
            stopBioWaiting();
            if (m_fRetryButton) {
                m_fRetryButton->hide();
            }
            m_ukeyPasswdWidget->hide();
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->clearFocus();
            }
            m_customWidget->hide();
            setFocusProxy(m_passwordEdit);
        } break;
        case LOGINOPT_TYPE_GENERAL_UKEY: {
            initUkeyPasswordWidget();
            m_tipWidget->show();
            m_editWidget->hide();
            m_widgetPlaceholder->hide();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->show();
            if (m_labelQRCode) {
                m_labelQRCode->hide();
            }
            if (m_labelFace) {
                m_labelFace->hide();
            }
            if (m_labelBioWaiting) {
                m_labelBioWaiting->hide();
            }
            stopBioWaiting();
            if (m_fRetryButton) {
                m_fRetryButton->hide();
            }
            m_customWidget->hide();
            m_ukeyPasswdWidget->show();
            if (m_ukeyPasswordEdit) {
                setFocusProxy(m_ukeyPasswordEdit);
                m_ukeyPasswordEdit->setFocusPolicy(Qt::StrongFocus);
                if (m_ukeyPasswordEdit->isVisible()) {
                    m_ukeyPasswordEdit->setFocus();
                }
            }
        } break;
        case LOGINOPT_TYPE_CUSTOM: {
            m_tipWidget->hide();
            m_editWidget->hide();
            m_widgetPlaceholder->hide();
            m_headerWidget->hide();
            m_nameWidget->hide();
            stopBioWaiting();
            setQRCodeMsg("");
            m_ukeyPasswdWidget->hide();
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->clearFocus();
            }
            LoginAuthInterface *plugin = m_loginOpts->getCustomLoginAuth();
            if (plugin) {
                QWidget *authWidget = plugin->getPluginMainWnd();
                if (authWidget) {
                    if (m_customWidget != authWidget->parent()) {
                        m_layoutCustom->addWidget(authWidget);
                        qDebug() << "CustomWidget Size:" << authWidget->size();
                        QObject *obj = dynamic_cast<QObject *>(plugin);
                        connect(obj, SIGNAL(requestAuthAccount(QString)), this, SLOT(onCustomRequestAccount(QString)));
                        connect(
                            obj,
                            SIGNAL(authenticateResult(int, QString)),
                            this,
                            SLOT(onCustomAuthResult(int, QString)));
                        connect(obj, SIGNAL(onRequest(QString)), this, SLOT(onCustomRequest(QString)));
                        connect(obj, SIGNAL(getLoginPluginEnv()), this, SLOT(onCustomPlugEnv()));
                    }
                    setFocusProxy(authWidget);
                    authWidget->setFixedSize(m_customWidget->size());
                    authWidget->setFocus();
                    authWidget->show();
                }
            }
            m_customWidget->show();
        } break;
        default:
            return;
    }

    if (uLoginOptType != m_uCurLoginOptType || (m_deviceInfo && m_deviceInfo->id != m_nLastDeviceId)
        || m_curUserInfo->name() != m_lastUserName) {
        clearMessage();
        switch (uLoginOptType) {
            case LOGINOPT_TYPE_PASSWORD: {
                setLoginTypeTip("");
            } break;
            case LOGINOPT_TYPE_FACE: {
                setLoginTypeTip(tr("Facial recognition to unlock"));
            } break;
            case LOGINOPT_TYPE_FINGERPRINT: {
                setLoginTypeTip(tr("Fingerprint recognition to unlock"));
            } break;
            case LOGINOPT_TYPE_VOICEPRINT: {
                setLoginTypeTip(tr("Voiceprint recognition to unlock"));
            } break;
            case LOGINOPT_TYPE_FINGERVEIN: {
                setLoginTypeTip(tr("Finger vein recognition to unlock"));
            } break;
            case LOGINOPT_TYPE_IRIS: {
                setLoginTypeTip(tr("Iris recognition to unlock"));
            } break;
            case LOGINOPT_TYPE_QRCODE: {
                QString strDrivName = tr("wechat");
                if (m_deviceInfo && m_deviceInfo->shortName != "wechat_driver") {
                    strDrivName = m_deviceInfo->shortName;
                }
                setLoginTypeTip(tr("Use the bound %1 scanning code to unlock").arg(strDrivName));
            } break;
            case LOGINOPT_TYPE_GENERAL_UKEY: {
                setLoginTypeTip("");
            } break;
            case LOGINOPT_TYPE_CUSTOM: {
                setLoginTypeTip("");
            } break;
            default:
                return;
        }
    } else if (uLoginOptType == m_uCurLoginOptType && uLoginOptType == LOGINOPT_TYPE_FACE && !faceBtnClicked) {
        clearMessage();
        setLoginTypeTip(tr("Facial recognition to unlock"));
    }
    if (m_deviceInfo) {
        m_nLastDeviceId = m_deviceInfo->id;
    } else {
        m_nLastDeviceId = -1;
    }

    m_lastUserName = m_curUserInfo->name();
    m_uCurLoginOptType = uLoginOptType;
}

void AuthDialog::setQRCode(QImage &imgQRCode)
{
    if (imgQRCode.isNull()) {
        m_imgQRCode.load(":/image/assets/ukui-qrcode-null.svg");
    } else {
        m_imgQRCode = imgQRCode;
        if (m_labelQRCodeTip) {
            m_labelQRCodeTip->hide();
        }
    }
    m_imgQRCode = m_imgQRCode.scaled(QSize(148 * scale, 148 * scale));
    if (m_labelQRCode) {
        m_labelQRCode->setAlignment(Qt::AlignCenter);
        m_labelQRCode->setPixmap(QPixmap::fromImage(m_imgQRCode));
    }
}

void AuthDialog::setQRCodeMsg(QString strMsg)
{
    if (strMsg.isEmpty()) {
        if (m_labelQRCodeMsg) {
            m_labelQRCodeMsg->hide();
        }
        // m_labelQRCodeTip->hide();
    } else {
        // 一开始认证时就没有网，直接停止加载状态
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        if (m_labelQRCodeMsg) {
            m_labelQRCodeMsg->setTipText(strMsg);
            m_labelQRCodeMsg->show();
        }
        if (m_labelQRCodeTip) {
            m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22 * scale, 22 * scale)));
            m_labelQRCodeTip->show();
        }
    }
}

void AuthDialog::setFaceImg(QImage &imgFace, int nStatus)
{
    if (m_labelFace) {
        QPixmap faceImage;
        m_labelFace->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
        // 如果头像文件不存在，则使用默认头像
        if (!imgFace.isNull()) {
            faceImage = PixmapToRound(QPixmap::fromImage(imgFace), WIDGET_HEIGHT_HEADIMAGE / 2);
        } else {
            switch (nStatus) {
                case 1:
                    faceImage = QPixmap(":/image/assets/ukui-loginopt-lose.svg");
                    break;
                case 2:
                    faceImage = m_loginOpts->loadSvg(":/image/assets/ukui-loginopt-smile.svg", "gray", 48);
                    break;
                default:
                    faceImage = QPixmap(":/image/assets/ukui-loginopt-smile.svg");
                    break;
            }
        }

        m_labelFace->setAlignment(Qt::AlignCenter);
        m_labelFace->setPixmap(faceImage);
        m_labelFace->setStyleSheet(
            QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,20%);")
                .arg((int)(WIDGET_HEIGHT_HEADIMAGE / 2)));
    }
}

void AuthDialog::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        if (m_labelQRCodeTip) {
            m_labelQRCodeTip->setPixmap(m_waitingPixmap);
        }
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        if (m_labelFace) {
            m_labelFace->setPixmap(m_waitingPixmap);
        }
    }
}

void AuthDialog::onRespondUkey(const QString &text)
{
    if (m_loginOpts && m_deviceInfo) {
        m_loginOpts->SetExtraInfo(text, "pincode");
        m_loginOpts->startAuth(m_deviceInfo, m_curUserInfo->uid());
    }
}

void AuthDialog::onPamShowMessage(QString strMsg, int nType)
{
    qDebug() << "onPamShowMessage:" << strMsg;
    // 设置label最小高度
    m_messageLabel->setMinimumHeight(36);
    if (strMsg.indexOf("account locked") != -1 || strMsg.indexOf("账户已锁定") != -1
        || strMsg.indexOf("Account locked") != -1 || strMsg.indexOf("永久锁定") != -1) {
        if (unlockUserTimer()) {
            if (!m_timerUserUnlock) {
                m_timerUserUnlock = new QTimer(this);
                m_timerUserUnlock->setInterval(500);
                connect(m_timerUserUnlock, &QTimer::timeout, this, &AuthDialog::unlockUserTimer);
            } else {
                if (m_timerUserUnlock->isActive()) {
                    m_timerUserUnlock->stop();
                }
            }
            m_timerUserUnlock->start();
        }
        return;
    } else if (strMsg.indexOf("No password received, please input password") != -1) {
        m_messageLabel->setTipText(tr("Password cannot be empty"));
    } else {
        QString strText = strMsg;
        if (!m_preStrMessage.isEmpty() && m_preStrMessageType != nType && m_preStrMessageType != -1) {
            if (m_preStrMessage.back().isPunct()) {
                strText = m_preStrMessage + strText;
            } else {
                strText = m_preStrMessage + "," + strText;
            }
        }
        m_messageLabel->setTipText(strText);
        m_preStrMessage = strMsg;
        m_preStrMessageType = nType;
    }
    m_bHasUnacknowledgedMsg = true;
    if (!m_isInhibitShow && isHidden()) {
        if (!m_biometricProxy || !m_biometricProxy->isValid() || !m_deviceInfo) {
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            if (m_loginOpts) {
                m_loginOpts->setSelectedPassword();
            }
        }
        show();
    }
}

void AuthDialog::showPwdTipBtn()
{
    bool isNeedShowPwdTip = false;
    if (m_userFailedTimesMap[m_curUserInfo->uid()] >= 1) {
        if (!m_pSecurityQuestionAnswer) {
            m_pSecurityQuestionAnswer = new SecurityQuestionAnswer(this);
        }
        QString pwdHint = m_pSecurityQuestionAnswer->GetUserPasswdHint(m_curUserInfo->uid());
        qDebug() << "User tip:" << m_curUserInfo->name() << pwdHint;
        if (!pwdHint.isEmpty()) {
            if (!m_pPwdTipMessage) {
                m_pPwdTipMessage = new StatusButton(m_editWidget, PWDTIPBTN);
                m_pEditLayout->addWidget(m_pPwdTipMessage);
                m_pPwdTipMessage->setFixedSize(40, 40);
                m_pPwdTipMessage->setFocusPolicy(Qt::NoFocus);
                QPixmap tipIcon = QIcon::fromTheme("help-about-symbolic").pixmap(16, 16);
                m_pPwdTipMessage->setIcon(drawSymbolicColoredPixmap(tipIcon, "white"));
            }
            m_pPwdTipMessage->show();
            m_pEditLayout->setContentsMargins(48, 0, 0, 0);
            m_pPwdTipMessage->setToolTip(tr("Password Prompt:%1").arg(pwdHint));
            isNeedShowPwdTip = true;
        }
    }
    if (!isNeedShowPwdTip) {
        if (m_pPwdTipMessage) {
            m_pPwdTipMessage->hide();
        }
        m_pEditLayout->setContentsMargins(0, 0, 0, 0);
    }
}

void AuthDialog::hidePwdTipBtn()
{
    if (m_pPwdTipMessage) {
        m_pPwdTipMessage->hide();
    }
    m_pEditLayout->setContentsMargins(0, 0, 0, 0);
}

void AuthDialog::startFirstLogin()
{
    bioService = new QDBusInterface(
        "org.ukui.Biometric", "/org/ukui/Biometric", "org.ukui.Biometric", QDBusConnection::systemBus(), this);

    if (!bioService->isValid())
        return;

    QDBusMessage result = bioService->call(QStringLiteral("GetDevList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return;
    }
    auto dbusArg = result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    dbusArg >> variantList;

    if (variantList.count() == 0)
        return;

    DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
    auto arg = variantList.at(0).value<QDBusArgument>();
    arg >> *pDeviceInfo;

    qDebug() << "pDeviceInfo->id = " << pDeviceInfo->id;

    if (pDeviceInfo->id <= 0)
        return;

    QList<QVariant> argList;
    argList << pDeviceInfo->id << -1 << 0 << -1;

    drvid = pDeviceInfo->id;
    QDBusPendingCall call = bioService->asyncCallWithArgumentList(QStringLiteral("Identify"), argList);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &AuthDialog::onFirstLoginIdentifyComplete);

    isFirstAuth = true;

    QTimer::singleShot(3000, [&] {
        if (isFirstAuth) {
            bioService->call(QStringLiteral("StopOps"), drvid, 3000);
        }
    });
}

void AuthDialog::onFirstLoginIdentifyComplete(QDBusPendingCallWatcher *watcher)
{
    isFirstAuth = false;
    QDBusPendingReply<int, int> reply = *watcher;

    if (reply.isError()) {
        qWarning() << "Identify error: " << reply.error().message();
        return;
    }
    int result = reply.argumentAt(0).toInt();
    int authUid = reply.argumentAt(1).toInt();
    qDebug() << result << authUid;

    if (result == 0 && authUid > 0) {
        struct passwd *pwd;
        pwd = getpwuid(authUid);
        if (pwd && pwd->pw_name) {
            if (UniAuthService::instance()->isActivatable()) {
                bool isEnable = UniAuthService::instance()->getBioAuthStatus(pwd->pw_name, ENABLETYPE_BIO);
                if (isEnable == false)
                    return;
            }
            setDirLogin();
            isBioSuccess = true;
            authMode = BIOMETRIC;
            startAuth();
        }
    }
}

void AuthDialog::onPamhideOtpDialog()
{
    if (m_msgDialog) {
        m_msgDialog->hide();
        m_msgDialog->close();
        m_msgDialog->deleteLater();
        m_msgDialog = nullptr;
    }

    if (m_bindOtpDialog) {
        m_bindOtpDialog->hide();
        m_bindOtpDialog->setOtpMessageText("");
        m_bindOtpDialog->close();
        m_bindOtpDialog->deleteLater();
        m_bindOtpDialog = nullptr;
    }

    sureBind = false;
}

void AuthDialog::onPamShowOtpDialog(int type, QVariant value)
{
    if (!m_msgDialog) {
        m_msgDialog = new KMessageDialog(ENSUREDIALOG, this);
        connect(m_msgDialog, &KMessageDialog::accepted, [=](QString text) {
            m_msgDialog->hide();
            sureBind = true;
            if (m_bindOtpDialog) {
                m_bindOtpDialog->show();
            }
        });
        connect(m_msgDialog, &KMessageDialog::rejected, [=]() {
            onPamShowMessage(tr("The authentication has been cancelled!"), PamAuth::MessageTypeError);
            startAuth();
            qDebug() << "MsgBox rejected";
        });
    }
    if (!m_bindOtpDialog) {
        m_bindOtpDialog = new KMessageDialog(BINDOTPDIALOG, this);
        connect(m_bindOtpDialog, &KMessageDialog::accepted, [=](QString text) {
            m_bRecvPrompt = true;
            m_bHasUnacknowledgedMsg = false;
            m_preStrMessage = "";
            onRespond(text);
            n_prompts--;
            if (n_prompts > 0) {
                while (n_prompts > 0) {
                    n_prompts--;
                    onRespond("");
                }
            }
        });
        connect(m_bindOtpDialog, &KMessageDialog::rejected, [=]() {
            sureBind = false;
            onPamShowMessage(tr("The authentication has been cancelled!"), PamAuth::MessageTypeError);
            startAuth();
            qDebug() << "MsgBox rejected";
        });
    }
    switch (type) {
        case BinaryPromptAuthType:
            break;
        case BinaryPromptMessage:
            if (m_bindOtpDialog) {
                m_bindOtpDialog->setOtpPromptText(value.toString());
            }
            break;
        case BinaryPromptPromptMessage:
            if (m_bindOtpDialog) {
                m_bindOtpDialog->setOtpPrpmptPromptText(value.toString());
            }
            break;
        case BinaryPromptQrcodeStr:
            if (m_bindOtpDialog) {
                m_bindOtpDialog->setOtpQrcodeText(value.toString());
            }
            break;
        case BinaryPromptErrorMessage:
            if (m_bindOtpDialog) {
                m_bindOtpDialog->setOtpMessageText(value.toString());
            }
            break;
        case BinaryPromptNumber:
            // 暂不不处理
            break;
        case BinaryPromptImage:
            // 暂不处理
            break;
        case BinaryPromptEmergMsg:
            if (m_bindOtpDialog) {
                m_bindOtpDialog->setOtpEmergMsg(value.toString());
            }
            break;
        default:
            break;
    }
    if (sureBind) {
        m_msgDialog->hide();
        m_bindOtpDialog->show();
    } else {
        m_msgDialog->show();
    }
}

void AuthDialog::onPamShowBinaryPrompt(QVariant val, int nType)
{
    QString binaryPromptString;
    int binaryInt;
    QByteArray binaryByteArray;
    switch (nType) {
        case BinaryPromptAuthType:
            factorMode = val.toString();
        case BinaryPromptMessage:
        case BinaryPromptErrorMessage:
        case BinaryPromptPromptMessage:
        case BinaryPromptEmergMsg:
        case BinaryPromptQrcodeStr:
            binaryPromptString = val.toString();
            break;
        case BinaryPromptNumber:
            binaryInt = val.toInt();
            break;
        case BinaryPromptImage:
            binaryByteArray = QByteArray::fromBase64(val.toString().toUtf8());
        default:
            break;
    }
    // 消息类型为BinaryPromptEmergMsg时，不打印日志，避免将紧急口令打印到日志里
    if (nType != BinaryPromptEmergMsg) {
        qDebug() << "onPamShowBinaryPrompt " << val << nType;
    }
    if (factorMode == "2fa_otp") {
        // 目前otp场景下，只有字符串类型数据，
        onPamShowOtpDialog(nType, val);
        n_prompts++;
    } else if (factorMode == "2fa_ukey") {
        onPamShowUkeyLoading();
        onRespond("");
    }
}

void AuthDialog::updatePamLoadingPixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_pamUkeyLoadingPixmap = m_pamUkeyLoadingPixmap.transformed(matrix, Qt::FastTransformation);
    if (m_pamUkeyLoadingButton) {
        m_pamUkeyLoadingButton->setIcon(QIcon(m_pamUkeyLoadingPixmap));
    }
}

void AuthDialog::onPamShowUkeyLoading()
{
    initPamUkeyLoadingWidget();

    if (!m_pamUkeyLoadingWidget)
        return;

    if (m_pamUkeyLoadingTimer && m_pamUkeyLoadingTimer->isActive()) {
        return;
    }

    if (m_editWidget)
        m_editWidget->hide();

    m_pamUkeyLoadingWidget->show();

    if (!m_pamUkeyLoadingTimer) {
        m_pamUkeyLoadingTimer = new QTimer(this);
        m_pamUkeyLoadingTimer->setInterval(150);
        connect(m_pamUkeyLoadingTimer, &QTimer::timeout, this, &AuthDialog::updatePamLoadingPixmap);
    }

    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(27, 27);
    m_pamUkeyLoadingPixmap = drawSymbolicColoredPixmap(icon, "white");
    if (m_pamUkeyLoadingButton) {
        m_pamUkeyLoadingButton->setIcon(m_pamUkeyLoadingPixmap);
        m_pamUkeyLoadingButton->setIconSize(QSize(27, 27));
    }
    m_pamUkeyLoadingTimer->start();
}

void AuthDialog::onPamStopUkeyLoading()
{
    if (m_pamUkeyLoadingWidget)
        m_pamUkeyLoadingWidget->hide();

    if (m_pamUkeyLoadingTimer)
        m_pamUkeyLoadingTimer->stop();
}

void AuthDialog::onPamShowPrompt(QString strPrompt, int nType)
{
    qDebug() << "prompt: " << strPrompt << authMode << nType;
    //    if(isPowerup && m_configuration->getIs990()){
    //        waitBiometricServiceStatus();
    //        startFirstLogin();
    //    }
    factorMode = "";
    n_prompts = 0;
    if (m_pamUkeyLoadingWidget) {
        onPamStopUkeyLoading();
    }

    if (m_messageButton) {
        m_messageButton->hide();
    }

    onPamhideOtpDialog();
    sureBind = false;
    if (m_authMethodLabel && !m_authMethodLabel->isHidden()) {
        m_authMethodLabel->hide();
        updateAuthSize();
    }

    if (strPrompt == BIOMETRIC_PAM || strPrompt == BIOMETRIC_PAM_DOUBLE || strPrompt == BIOMETRIC_PAM_QRCODE) {
        if (m_bDirectLogin) {
            onRespond(BIOMETRIC_SUCCESS);
            return;
        }
        if (authMode == PASSWORD) {
            skipBiometricAuth();
            if (m_loginOpts) {
                if (m_loginOpts->getLoginOptCount() < 1) {
                    m_loginOpts->hide();
                } else {
                    m_loginOpts->show();
                }

                if (m_loginOpts->getHasUkeyOptions() && m_loginOpts->getLoginOptCount() < 1) {
                    m_loginOpts->show();
                }
            }
        } else {
            skipBiometricAuth();
            //            if(isPowerup  && m_configuration->getIs990()) {
            //                QTimer::singleShot(1500, [&]{
            //                    performBiometricAuth();
            //                });
            //                isPowerup = false;
            //            } else {
            syslog(LOG_INFO, "[ukui-screensaver-dialog] check bio auth begin!!");
            performBiometricAuth();
            syslog(LOG_INFO, "[ukui-screensaver-dialog] check bio auth done!!");
            //            }
        }
    } else {
        syslog(LOG_INFO, "[ukui-screensaver-dialog] auth alraedy!!");
        m_isNameLogin = false;
        qDebug() << "m_isNameLogin = false";
        m_passwordEdit->stopWaiting();
        if (!strPrompt.isEmpty()) {
            m_passwordEdit->readOnly(false);
            m_passwordEdit->show();
        }

        if (m_passwordEdit->isVisible()) {
            m_passwordEdit->setFocus();
            this->setFocusProxy(m_passwordEdit);
        }
        if (nType != PamAuth::PromptTypeSecret) {
            m_isAutoSwitch = true;
            m_passwordEdit->setType(QLineEdit::Normal);
        } else if (m_isAutoSwitch) {
            m_passwordEdit->setType(QLineEdit::Password);
            m_isAutoSwitch = false;
        }

        m_bRecvPrompt = true;
        m_bHasUnacknowledgedMsg = false;
        m_preStrMessage = "";
        const bool staleBioDevice = m_deviceInfo && m_loginOpts
            && (!m_loginOpts->findDeviceById(m_deviceInfo->id) || m_loginOpts->isDeviceDisable(m_deviceInfo->id));
        if (!m_biometricProxy || !m_biometricProxy->isValid() || !m_deviceInfo || staleBioDevice) {
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            if (m_loginOpts)
                m_loginOpts->setSelectedPassword();
            if (staleBioDevice)
                m_deviceInfo = DeviceInfoPtr();
        }
        if (strPrompt == "Password: " || strPrompt == "密码：" || strPrompt == "密码： ") {
            strPrompt = tr("Password:");
            m_isPassWdInput = true;
        } else if (strPrompt == "Input password" || strPrompt == "Input Password" || strPrompt == "输入密码") {
            strPrompt = tr("Input Password");
            m_isPassWdInput = true;
        }

        if (strPrompt == "login:") {
            strPrompt = tr("Username");
            /// TODO
            // m_name = "*login";
            m_isNameLogin = true;
            m_isPassWdInput = false;
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            qDebug() << "m_isNameLogin = true";
        }

        m_passwordEdit->clear();
        m_passwordEdit->setPrompt(strPrompt);
        m_passwordEdit->show();

        if (unlockUserTimer()) {
            if (!m_timerUserUnlock) {
                m_timerUserUnlock = new QTimer(this);
                m_timerUserUnlock->setInterval(500);
                connect(m_timerUserUnlock, &QTimer::timeout, this, &AuthDialog::unlockUserTimer);
            } else {
                if (m_timerUserUnlock->isActive()) {
                    m_timerUserUnlock->stop();
                }
            }
            m_timerUserUnlock->start();
        }

        if (hasSendPassword) {
            showAuthMethodTip();
        }
    }
    if (!m_isInhibitShow) {
        show();
    }
}

void AuthDialog::onPamAuthCompleted()
{
    if (m_curUserInfo->name().isEmpty())
        return;
    m_passwordEdit->stopWaiting();

    if (m_ukeyPasswordEdit) {
        m_ukeyPasswordEdit->clearText();
        m_ukeyPasswordEdit->stopWaiting();
    }
    stopBioWaiting();

    hasSendPassword = false;
    onPamhideOtpDialog();
    n_prompts = 0;

    bool isAuthenticated = (Q_EMIT m_modelLockDialog->pamIsAuthenticated());
    qDebug() << "onPamAuthCompleted:" << isAuthenticated << m_curUserInfo->name();
    if (isAuthenticated) {
        if (m_loginOpts) {
            m_loginOpts->stopAuth();
        }
        connectBioSlots(false);

        if ((m_bRecvPrompt && !m_bHasUnacknowledgedMsg) || m_bDirectLogin) {
            // 记录认证成功事件
            if (m_bDirectLogin && m_deviceInfo) {
                recordLoginAuthEvent(m_curUserInfo->name(), 0, m_deviceInfo->deviceType); // 0 表示成功
            } else {
                recordLoginAuthEvent(m_curUserInfo->name(), 0, -1); // 0 表示成功 -1表示pam
            }
            m_bDirectLogin = false;

            m_failMap.remove(m_curUserInfo->uid());
            setLoginTypeTip("");
            setUkeyTypeTip("");
            authMode = UNKNOWN;
            m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD;
            if (m_loginOpts) {
                m_loginOpts->clearDeviceDisable(m_curUserInfo->uid());
            }

            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->readOnly(false);
                m_ukeyPasswordEdit->clearText();
            }
            Q_EMIT authSucceed(m_curUserInfo->name());
            m_userFailedTimesMap[m_curUserInfo->uid()] = 0;
            // 认证完成后清空密码框
            m_passwordEdit->clear();
            clearMessage();
            showPwdTipBtn();
        } else {
            qDebug() << "m_bRecvPrompt  = " << m_bRecvPrompt
                     << "  m_bHasUnacknowledgedMsg = " << m_bHasUnacknowledgedMsg;
            m_bRecvPrompt = true;
            showAuthenticated();
        }
    } else {
        // 因为有个概率性会认证失败的问题，导致重试按钮显示出来，因此这里暂时屏蔽
        if (m_bRecvPrompt) {
            m_passwordEdit->clear();
            // 如果用户输入了不存在的用户名导致的认证失败，让用户重新输入用户名
            if (m_isManual) {
                if (m_isNameLogin) {
                    m_isNameLogin = false;
                    /// TODO
                    // m_name = "*login";
                }
                if (!m_isPassWdInput) {
                    if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg) {
                        onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeError);
                    }
                } else {
                    if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg)
                        onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeError);
                }
            } else {
                if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg) {
                    onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeError);
                }
                recordLoginAuthEvent(m_curUserInfo->name(), 1, -1); // 1 表示失败 -1表示pam
            }
            m_isInputPasswd = false;
            m_isManual = false;
            if (unlockUserTimer()) {
                if (!m_timerUserUnlock) {
                    m_timerUserUnlock = new QTimer(this);
                    m_timerUserUnlock->setInterval(500);
                    connect(m_timerUserUnlock, &QTimer::timeout, this, &AuthDialog::unlockUserTimer);
                } else {
                    if (m_timerUserUnlock->isActive()) {
                        m_timerUserUnlock->stop();
                    }
                }
                m_timerUserUnlock->start();
            }
            startAuth();
            if (++m_userFailedTimesMap[m_curUserInfo->uid()] >= 1) {
                qDebug() << "m_authFailedTimes = " << m_userFailedTimesMap[m_curUserInfo->uid()];
                showPwdTipBtn();
                Q_EMIT showResetPasswdBtn();
            }
        } else {
            showAuthenticated(false);
        }
    }
    if (!m_isInhibitShow) {
        show();
    }
}

void AuthDialog::onRespond(const QString &strRes)
{
    if (!m_bRecvPrompt && strRes != BIOMETRIC_SUCCESS) {
        qInfo() << "Wait for input passwd!";
        return;
    }
    if (m_curUserInfo->name().isEmpty()) {
        qInfo() << "No user be selected!!";
        return;
    }
    m_bHasUnacknowledgedMsg = false;
    clearMessage();

    qDebug() << "onRespond:" << m_curUserInfo->name();
    if (m_curUserInfo->name() == "*guest") {
        Q_EMIT m_modelLockDialog->pamAuthenticate(m_curUserInfo->name());
    } else if (m_curUserInfo->name() == "*login") { // 用户输入用户名
        m_isManual = true;
        m_isNameLogin = true;

        Q_EMIT m_modelLockDialog->pamRespond(strRes);

        m_strManualLoginName = strRes;
        if (!m_isInputPasswd) {
            Q_EMIT userChangedByManual(strRes);
            m_isInputPasswd = true;
        }
    } else { // 当密码长度超过1000多时，lightm会出异常，这里暂时先截断
        if (strRes.length() > 1000)
            Q_EMIT m_modelLockDialog->pamRespond(strRes.mid(0, 1000));
        else
            Q_EMIT m_modelLockDialog->pamRespond(strRes);
        m_passwordEdit->readOnly(true);

        if (strRes != BIOMETRIC_SUCCESS || strRes != BIOMETRIC_IGNORE) {
            hasSendPassword = true;
        }
    }
}

void AuthDialog::startAuth()
{
    syslog(LOG_INFO, "[ukui-screensaver-dialog] auth start!!");
    if (!m_curUserInfo->name().isEmpty()) {
        m_bRecvPrompt = false;
        n_prompts = 0;
        m_bHasUnacknowledgedMsg = false;
        // 用户认证
        if (m_curUserInfo->name() == "*guest") { // 游客登录
            qDebug() << "guest login";
            m_passwordEdit->show();
            m_passwordEdit->setPrompt(tr("login"));
            m_strLastPamAuthName = "*guest";
        } else if (m_curUserInfo->name() == "*login") { // 手动输入用户名
            Q_EMIT m_modelLockDialog->pamAuthenticate("");
            m_strLastPamAuthName = "*login";
        } else {
            qDebug() << "login: " << m_curUserInfo->name();
            Q_EMIT m_modelLockDialog->pamAuthenticate(m_curUserInfo->name());
            m_passwordEdit->clear();
            m_passwordEdit->readOnly(true);
            m_strLastPamAuthName = m_curUserInfo->name();
        }
    }

    hasSendPassword = false;
    authMethod = Configuration::instance()->getAuthMethod();
}

void AuthDialog::stopAuth()
{
    if (m_loginOpts) {
        if (m_bioTimer && m_bioTimer->isActive())
            m_bioTimer->stop();
        m_loginOpts->stopAuth();

        m_loginOpts->hide();
    }
    clearMessage();
}

void AuthDialog::onCurUserInfoChanged(UserInfoPtr userInfo)
{
    if (!(m_curUserInfo->uid() < 0) && userInfo->uid() == m_curUserInfo->uid()) {
        m_curUserInfo = userInfo;
        updateUI();
    }
}

void AuthDialog::onCurUserChanged(UserInfoPtr userInfo)
{
    if (m_loginOpts) {
        m_loginOpts->stopAuth();
        if (m_bioTimer && m_bioTimer->isActive())
            m_bioTimer->stop();
    }
    m_curUserInfo = userInfo;
    m_isInputPasswd = false;
    updateUI();
    startAuth();
    if (m_passwordEdit) {
        m_passwordEdit->clearText();
        m_passwordEdit->setType(QLineEdit::Password);
    }
    if (m_messageButton)
        m_messageButton->hide();
    clearMessage();
    m_bDirectLogin = false;
    m_isLockingFlg = false;
    authMode = UNKNOWN;
    m_deviceInfo = DeviceInfoPtr();
    m_deviceName = "";
    isBioSuccess = false;
    if (m_loginOpts) {
        m_loginOpts->hide();
    }
}

void AuthDialog::showAuthenticated(bool successful /* = true*/)
{
    m_editWidget->show();
    m_widgetPlaceholder->hide();
    m_passwordEdit->hide();
    m_messageButton->show();
    m_messageButton->setDefault(true);

    if (successful) {
        m_bNeedRetry = false;
        m_messageButton->setText(tr("Login"));
        m_messageButton->setFocus();
        QString strMsgTip = m_messageLabel->text();
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        if (!strMsgTip.isEmpty()) {
            m_messageLabel->setText(strMsgTip);
        }
    } else {
        m_bNeedRetry = true;
        m_messageButton->setText(tr("Retry"));
    }
    this->setFocusProxy(m_messageButton);
}

void AuthDialog::onMessageButtonClicked()
{
    m_messageButton->setDefault(false);
    if (!m_bNeedRetry) {
        // 认证完成后清空密码框
        m_passwordEdit->readOnly(false);
        m_passwordEdit->clear();
        clearMessage();

        m_failMap.remove(m_curUserInfo->uid());
        setLoginTypeTip("");
        setUkeyTypeTip("");
        authMode = UNKNOWN;
        m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD;
        // 记录认证成功事件
        if (m_bDirectLogin && m_deviceInfo) {
            recordLoginAuthEvent(m_curUserInfo->name(), 0, m_deviceInfo->deviceType); // 0 表示成功
        } else {
            recordLoginAuthEvent(m_curUserInfo->name(), 0, -1); // 0 表示成功 -1表示pam
        }

        Q_EMIT authSucceed(m_curUserInfo->name());
    } else {
        m_messageButton->hide();
        m_messageLabel->setText("");
        authMode = PASSWORD;
        startAuth();
    }
}

void AuthDialog::clearMessage()
{
    m_messageLabel->clear();
    m_preStrMessage = "";
    m_preStrMessageType = -1;
}

bool AuthDialog::unlockUserTimer()
{
    if (m_curUserInfo->name().isEmpty())
        return false;
    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time = 0;
    int unlock_time = 0;
    QString curUserName = m_curUserInfo->name();

    if (NULL == curUserName.toLatin1().data()) {
        qDebug() << "get user info failed.";
        return false;
    }

    struct passwd *user = NULL;
    user = getpwnam(curUserName.toLatin1().data());
    if (user)
        pam_tally_unlock_time_left(user->pw_uid, &failed_count, &time_left, &deny, &fail_time, &unlock_time);
    else
        return false;

    if (time_left >= 60) { // 请多少分钟后重试
        int nMinuteleft = time_left / 60;
        if (m_isLockingFlg == false) {
            m_nCurLockMin = unlock_time / 60; // 获取当前需要锁定的分钟数
        }

        // 如果当前设置的不是1min钟锁定，那么1min显示成2min，由2min直接跳到59s ||
        // 剩余分钟数小于当前设置的锁定时间，并且大于1min，自增+1
        if ((nMinuteleft == 1 && m_nCurLockMin != 1) || (nMinuteleft > 1 && nMinuteleft < m_nCurLockMin)) {
            nMinuteleft = nMinuteleft + 1;
        }
        m_messageLabel->setTipText(tr("Please try again in %1 minutes.").arg(nMinuteleft));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        m_isLockingFlg = true;
        setBiometricAuthDisabledStatus(m_isLockingFlg);
        return true;
    } else if (time_left > 0 && time_left < 60) { // 请多少秒后重试
        m_messageLabel->setTipText(tr("Please try again in %1 seconds.").arg(time_left % 60));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        m_isLockingFlg = true;
        setBiometricAuthDisabledStatus(m_isLockingFlg);
        return true;
    } else if (failed_count == 0xFFFF) { // 账号被永久锁定
        m_messageLabel->setTipText(tr("Account locked permanently."));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        m_isLockingFlg = true;
        setBiometricAuthDisabledStatus(m_isLockingFlg);
        return true;
    } else {
        if (m_passwordEdit) {
            m_passwordEdit->readOnly(false);
            m_passwordEdit->setLocked(false);
            if (m_passwordEdit->isVisible()) {
                m_passwordEdit->setFocus();
                this->setFocusProxy(m_passwordEdit);
            }
        }
        //    qDebug() << "清理 1 clearMessage";
        if (m_isLockingFlg) {
            onPamShowMessage("", PamAuth::MessageTypeError);
            // onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeInfo);
            m_isLockingFlg = false;
            setBiometricAuthDisabledStatus(m_isLockingFlg);
        }
        if (m_timerUserUnlock) {
            m_timerUserUnlock->stop();
        }
    }
    return false;
}

void AuthDialog::restartBioAuth()
{
    startBioAuth();
}

void AuthDialog::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    if (sleep) {
        if (m_loginOpts) {
            manualStopBio = true;
            m_loginOpts->stopAuth();
            if (m_bioTimer && m_bioTimer->isActive())
                m_bioTimer->stop();
        }
    } else {
        manualStopBio = false;
        restartBioAuth();
    }
}

void AuthDialog::setDirLogin()
{
    m_bDirectLogin = true;
}

void AuthDialog::pamBioSuccess()
{
    m_bioTimer->stop();
    if (!m_loginOpts) {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        return;
    }

    if (m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
        // ukey时不调用ukey认证
    } else {
        m_loginOpts->startAuth(m_deviceInfo, m_curUserInfo->uid());
    }

    if (m_deviceInfo) {
        switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
    } else {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
    }
}

void AuthDialog::startBioAuth(unsigned uTimeout)
{
    if (m_loginOpts)
        m_loginOpts->stopAuth();

    if (m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
        // ukey时不调用ukey认证
        switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
        return;
    }

    if (m_bioAuthLoop && m_bioAuthLoop->isRunning()) {
        qDebug() << "Already to start bio auth!";
        return;
    }
    if (!m_modelLockDialog->sessionActive()) {
        if (!m_bioAuthLoop) {
            m_bioAuthLoop = new QEventLoop(qApp);
            connect(m_modelLockDialog, &LockDialogModel::sessionActiveChanged, [this](bool isActive) {
                qDebug() << "sessionActiveChanged:" << isActive;
                if (isActive && this->m_bioAuthLoop->isRunning()) {
                    this->m_bioAuthLoop->quit();
                }
            });
        }
        qDebug() << "Session inactive blocked!";
        m_bioAuthLoop->exec();
        qDebug() << "Session inactive blocked end!";
    }
    if (!m_bioTimer) {
        m_bioTimer = new QTimer(this);
        connect(m_bioTimer, SIGNAL(timeout()), this, SLOT(pamBioSuccess()));
    }
    m_bioTimer->start(uTimeout);
}

void AuthDialog::waitBiometricServiceStatus()
{
    qDebug() << "get biometric status";
    QDBusInterface iface(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> bioPath = iface.call("GetUnit", "biometric-authentication.service");
    if (!bioPath.isValid()) {
        return;
    }

    QDBusInterface bioface(
        "org.freedesktop.systemd1",
        bioPath.value().path(),
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "UnitFileState");
    if (!sessionReply.isValid())
        qWarning() << sessionReply.error();
    else {
        QString res = sessionReply.value().variant().toString();
        if (res == "disable")
            return;
    }

    qDebug() << "get activeState";
    int times = 0;
    while (times < 20) {
        QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "ActiveState");
        if (!sessionReply.isValid()) {
            qWarning() << sessionReply.error();
            return;
        } else {
            QString res = sessionReply.value().variant().toString();
            if (res == "activating") {
                times++;
                usleep(100000);
            } else {
                break;
            }
        }
    }
    qDebug() << "times = " << times;
}

void AuthDialog::performBiometricAuth()
{
    if (!m_biometricProxy) {
        if (!FreedesktopHelper::getInstance(false)->NameHasOwner(BIOMETRIC_DBUS_SERVICE)) {
            disconnect(FreedesktopHelper::getInstance(false), nullptr, this, nullptr);
            connect(
                FreedesktopHelper::getInstance(false),
                &FreedesktopHelper::serviceStatusChanged,
                this,
                [=](const QString &strService, bool bActive) {
                    if (strService == BIOMETRIC_DBUS_SERVICE) {
                        this->onBiometricDbusChanged(bActive);
                    }
                });
            return;
        }
        m_biometricProxy = new BiometricHelper(this);
        maxFailedTimes = UniAuthService::instance()->getMaxFailedTimes();
    }

    // 服务没启动，或者打开DBus连接出错
    if (!m_biometricProxy->isValid()) {
        qWarning() << "An error occurs when connect to the biometric DBus";
        if (m_deviceInfo) {
            if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        return;
    }

    // 初始化生物识别认证UI
    initLoginOptsWidget();

    // 没有可用设备，不启用生物识别认证
    if (m_loginOpts->getLoginOptCount() < 1) {
        qWarning() << "No available devices";
        if (m_deviceInfo) {
            if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        return;
    }

    // 获取默认设备
    if (m_loginOpts) {
        if (m_curUserInfo->name() == "*login")
            m_deviceName = m_loginOpts->getDefaultDevice(m_strManualLoginName);
        else
            m_deviceName = m_loginOpts->getDefaultDevice(m_curUserInfo->name());

        if (m_isCustomDefault) { // 认证插件配置默认插件认证
            if (m_loginOpts) {
                m_deviceName = m_loginOpts->getCustomDevName();
            }
        }
    }
    qDebug() << m_deviceName;
    if (m_deviceInfo) {
        if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
            || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
            m_deviceInfo = DeviceInfoPtr();
        }
    }
    // 如果默认设备为空的话，第一次不启动生物识别认证
    if (m_deviceName.isEmpty() && !m_deviceInfo) {
        qDebug() << "No default device";
        if (Configuration::instance()->getProductNameN90F3() && m_loginOpts->getLoginOptCount() > 0) {
            initBiometricUI();
        }
        return;
    }
    // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
    if (!m_deviceInfo) {
        m_deviceInfo = m_loginOpts->findDeviceByName(m_deviceName);
        if (!m_deviceInfo || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
            qDebug() << "Device Not found or disable: " << m_deviceName;
            // 因错误超限而禁用时仅提示不切换认证类型
            if (m_deviceInfo && m_loginOpts->isDeviceDisable(m_deviceInfo->id)
                && m_loginOpts->convertDeviceType(m_deviceInfo->deviceType) == m_uCurLoginOptType) {
                // 已经错误超过限制
                if (m_failMap.contains(m_curUserInfo->uid())
                    && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                    if (m_deviceInfo->deviceType == UniT_Remote) {
                        setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                    } else {
                        setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        if (m_deviceInfo->deviceType == BioT_Face) {
                            QImage imgFailed;
                            setFaceImg(imgFailed, 1);
                        }
                        stopBioWaiting(true);
                    }
                    m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
                    return;
                }
            }
            m_deviceInfo = DeviceInfoPtr();
            return;
        }
    }
    initBiometricUI();
    // 已经错误超过3次
    if (m_failMap.contains(m_curUserInfo->uid())
        && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
        if (m_deviceInfo->deviceType == UniT_Remote) {
            setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
        } else {
            setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
            if (m_deviceInfo->deviceType == BioT_Face) {
                QImage imgFailed;
                setFaceImg(imgFailed, 1);
            }
            stopBioWaiting(true);
        }
        m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
        return;
    }

    if (!Configuration::instance()->getProductNameN90F3()) {
        if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            m_loginOpts->setSelectedPassword();
            return;
        }
    }

    switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
    // 自行校验生物设备，pam greeter校验密码
    startBioAuth();
}

void AuthDialog::skipBiometricAuth()
{
    Q_EMIT m_modelLockDialog->pamRespond(BIOMETRIC_IGNORE);
}

void AuthDialog::connectBioSlots(bool isConnect)
{
    if (m_isBioSlotsConnected != isConnect) {
        m_isBioSlotsConnected = isConnect;
        if (m_isBioSlotsConnected) {
            disconnect(FreedesktopHelper::getInstance(false), nullptr, this, nullptr);
            connect(
                FreedesktopHelper::getInstance(false),
                &FreedesktopHelper::serviceStatusChanged,
                this,
                [=](const QString &strService, bool bActive) {
                    if (strService == BIOMETRIC_DBUS_SERVICE) {
                        this->onBiometricDbusChanged(bActive);
                    }
                });
            if (m_loginOpts) {
                connect(m_loginOpts, &LoginOptionsWidget::authComplete, this, &AuthDialog::onBiometricAuthComplete);
                connect(m_loginOpts, &LoginOptionsWidget::optionSelected, this, &AuthDialog::onDeviceChanged);
                connect(m_loginOpts, &LoginOptionsWidget::authHardwareDetached, this, &AuthDialog::onAuthHardwareDetached);
                connect(m_loginOpts, &LoginOptionsWidget::updateImage, this, &AuthDialog::onLoginOptImage);
                connect(m_loginOpts, &LoginOptionsWidget::setLoadingImage, this, &AuthDialog::onLoadingImage);
                connect(m_loginOpts, &LoginOptionsWidget::notifyOptionsChange, this, &AuthDialog::onLoginOptsCount);
                connect(m_loginOpts, &LoginOptionsWidget::updateAuthMsg, this, &AuthDialog::setLoginMsg);
            }
        } else {
            disconnect(FreedesktopHelper::getInstance(false), nullptr, this, nullptr);
            if (m_loginOpts) {
                disconnect(m_loginOpts, nullptr, this, nullptr);
            }
        }
    }
}

void AuthDialog::initBiometricUI()
{
    QVBoxLayout *headerLayout = qobject_cast<QVBoxLayout *>(m_headerWidget->layout());
    if (headerLayout) {
        // 人脸识别头像框
        if (!m_labelFace) {
            m_labelFace = new QLabel(m_headerWidget);
            headerLayout->addWidget(m_labelFace);
            m_labelFace->setStyleSheet(
                QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,20%);")
                    .arg((int)(WIDGET_HEIGHT_HEADIMAGE / 2)));
            m_labelFace->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
            QImage faceImg;
            setFaceImg(faceImg);
            m_labelFace->hide();
        }

        // 生物特征等待验证动画
        if (!m_labelBioWaiting) {
            m_labelBioWaiting = new QLabel(m_headerWidget);
            m_labelBioWaiting->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
            headerLayout->addWidget(m_labelBioWaiting);
            m_labelBioWaiting->hide();
        }

        // 人脸识别重试
        if (!m_fRetryButton) {
            m_fRetryButton = new QPushButton(m_headerWidget);
            m_fRetryButton->setFocusPolicy(Qt::NoFocus);
            m_fRetryButton->setStyleSheet(QString("QPushButton{border-radius: %1px;  border:0px solid "
                                                  "white;background-color: rgba(255,255,255,10%);}"
                                                  "QPushButton::hover{background-color: rgba(255,255,255,30%);}"
                                                  "QPushButton::pressed {background-color: rgba(255,255,255,40%);}")
                                              .arg((int)(77 * scale)));

            QPixmap retryIcon = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
            m_fRetryButton->setFixedSize(154 * scale, 154 * scale);
            m_fRetryButton->setIcon(drawSymbolicColoredPixmap(retryIcon, "white"));
            m_fRetryButton->setIconSize(QSize(48, 48));
            connect(m_fRetryButton, &QPushButton::clicked, this, &AuthDialog::onFRetryButtonClicked);
            m_fRetryButton->hide();
            headerLayout->addWidget(m_fRetryButton);
        }

        // 微信认证窗口
        if (!m_labelQRCode) {
            m_labelQRCode = new QLabel(m_headerWidget);
            m_labelQRCode->setAlignment(Qt::AlignCenter);
            m_labelQRCode->setStyleSheet(
                QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);")
                    .arg((int)(6 * scale)));
            m_labelQRCode->setFixedSize(WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
            setQRCode(m_imgQRCode);
            m_labelQRCode->hide();
            QVBoxLayout *layoutQRCode = new QVBoxLayout();
            layoutQRCode->setAlignment(Qt::AlignCenter);
            layoutQRCode->setSpacing(10);
            m_labelQRCode->setLayout(layoutQRCode);
            m_labelQRCodeTip = new QLabel();
            m_labelQRCodeTip->setFixedSize(22 * scale, 22 * scale);
            layoutQRCode->addWidget(m_labelQRCodeTip, 0, Qt::AlignHCenter);
            m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22 * scale, 22 * scale)));
            m_labelQRCodeTip->show();
            m_labelQRCodeMsg = new CustomLabel();
            m_labelQRCodeMsg->setFixedHeight(30 * scale);
            m_labelQRCodeMsg->setFontSize((14 + m_curFontSize) * m_ptToPx);
            m_labelQRCodeMsg->setStyleSheet("QLabel{background-color:rgba(255,255,255,0);color:rgba(255,0,0)}");
            m_labelQRCodeMsg->hide();
            layoutQRCode->addWidget(m_labelQRCodeMsg, 0, Qt::AlignHCenter);
            headerLayout->addWidget(m_labelQRCode);
        }
    }
}

void AuthDialog::initLoginOptsWidget()
{
    if (m_loginOpts) {
        m_loginOpts->setUser(m_curUserInfo->uid());
    } else {
        QHBoxLayout *optsLayout = new QHBoxLayout(m_widgetLoginOpts);
        optsLayout->setContentsMargins(0, 0, 0, 0);
        m_loginOpts = new LoginOptionsWidget(m_biometricProxy, m_curUserInfo->uid(), m_widgetLoginOpts);
        optsLayout->addWidget(m_loginOpts);
        m_loginOpts->setUser(m_curUserInfo->uid());
        m_loginOpts->installEventFilter(this);

        // 自定义Tab焦点导航顺序：m_passwordEdit -> m_iconButton -> m_loginOpts->m_buttonListWidget
        setTabOrder(m_passwordEdit, m_passwordEdit->getIconButton());
        setTabOrder(m_passwordEdit->getIconButton(), m_loginOpts->getButtonListWidget());

        // 通知LockWidget设置跨组件Tab顺序
        Q_EMIT loginOptsCreated();
    }
    connectBioSlots(true);

    qDebug() << "OptsCount:" << m_loginOpts->getLoginOptCount();
    if (m_loginOpts->getLoginOptCount() < 1) {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_loginOpts->hide();
    } else {
        m_loginOpts->show();
    }

    if (m_loginOpts->getHasUkeyOptions() && m_loginOpts->getLoginOptCount() < 1) {
        m_loginOpts->show();
        m_loginOpts->setSelectedPassword();
    }

    m_loginOpts->setEnabled(true);

    setLoginOptsWidgetGeometry();
}

void AuthDialog::setLoginOptsWidgetGeometry()
{
    // 生物识别
    if (m_loginOpts) {
        m_widgetLoginOpts->setFixedSize(width(), WIDGET_HEIGHT_LOGINOPTS);
        qDebug() << "LoginOptGeometry:" << m_widgetLoginOpts->geometry() << "," << geometry()
                 << ",isHidden:" << m_widgetLoginOpts->isHidden();
    }
}

void AuthDialog::onLoginOptsCount(unsigned uCount)
{
    qDebug() << "onLoginOptsCount Count:" << uCount;
    if (!m_loginOpts)
        return;
    if (uCount > 0) {
        setLoginOptsWidgetGeometry();
        m_loginOpts->show();
        qDebug() << "LoginOptsShow!!";
    } else {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_loginOpts->hide();
    }
    if (!m_deviceInfo || !m_loginOpts->findDeviceById(m_deviceInfo->id)
        || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
        m_loginOpts->stopAuth();
        authMode = BIOMETRIC;
        // 初始化生物识别认证UI
        initLoginOptsWidget();

        // 没有可用设备，不启用生物识别认证
        if (m_loginOpts->getLoginOptCount() < 1) {
            qWarning() << "No available devices";
            if (m_deviceInfo) {
                m_deviceInfo = DeviceInfoPtr();
            }
            return;
        }
        // 获取默认设备
        if (m_curUserInfo->name() == "*login") {
            if (isLoadingUkey) {
                m_deviceName = m_loginOpts->getDefaultDevice(m_strManualLoginName, UniT_General_Ukey);
            } else {
                m_deviceName = m_loginOpts->getDefaultDevice(m_strManualLoginName);
            }
        } else {
            if (isLoadingUkey) {
                m_deviceName = m_loginOpts->getDefaultDevice(m_curUserInfo->name(), UniT_General_Ukey);
            } else {
                m_deviceName = m_loginOpts->getDefaultDevice(m_curUserInfo->name());
            }
        }

        qDebug() << m_deviceName;
        if (m_deviceInfo) {
            if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        // 如果默认设备为空的话，第一次不启动生物识别认证
        if (m_deviceName.isEmpty() && !m_deviceInfo) {
            qDebug() << "No default device";
            // 拔出当前认证设备时已由 LoginOptionsWidget::onUSBDeviceHotPlug 发出 PASSWORD 同步界面，此处不再二次 startAuth
            return;
        }
        // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
        if (!m_deviceInfo) {
            m_deviceInfo = m_loginOpts->findDeviceByName(m_deviceName);
            if (!m_deviceInfo || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                qDebug() << "Device Not found or Disable: " << m_deviceName;
                // 因错误超限而禁用时仅提示不切换认证类型
                if (m_deviceInfo && m_loginOpts->isDeviceDisable(m_deviceInfo->id)
                    && m_loginOpts->convertDeviceType(m_deviceInfo->deviceType) == m_uCurLoginOptType) {
                    // 已经错误超过限制
                    if (m_failMap.contains(m_curUserInfo->uid())
                        && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                        if (m_deviceInfo->deviceType == UniT_Remote) {
                            setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        } else {
                            setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                            if (m_deviceInfo->deviceType == BioT_Face) {
                                QImage imgFailed;
                                setFaceImg(imgFailed, 1);
                            }
                            stopBioWaiting(true);
                        }
                        m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
                        return;
                    }
                }
                m_deviceInfo = DeviceInfoPtr();
                m_loginOpts->setSelectedPassword();
                return;
            }
        }
        initBiometricUI();

        // 已经错误超过3次
        if (m_failMap.contains(m_curUserInfo->uid())
            && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
            if (m_deviceInfo->deviceType == UniT_Remote) {
                setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                    .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
            } else {
                setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                    .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                if (m_deviceInfo->deviceType == BioT_Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                }
                stopBioWaiting(true);
            }
            m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
            return;
        }

        if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
            stopLoadingUkey();
        }

        if (m_deviceInfo) {
            m_loginOpts->setCurrentDevice(m_deviceInfo);
            m_loginOpts->updateUIStatus();
        }

        switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
        // 自行校验生物设备，pam greeter校验密码
        startBioAuth();
    }
}

void AuthDialog::onLoginOptImage(QImage img)
{
    if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        setFaceImg(img);
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        setQRCode(img);
    }
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
}

void AuthDialog::onLoadingImage()
{
    if (!w_timer) {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &AuthDialog::updatePixmap);
    }
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(22 * scale, 22 * scale);
    if (m_labelFace) {
        m_labelFace->setAlignment(Qt::AlignCenter);
    }
    if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        if (m_labelFace) {
            m_labelFace->setPixmap(m_waitingPixmap);
        }
        if (m_labelQRCodeTip) {
            m_labelQRCodeTip->show();
        }
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        if (m_labelQRCodeTip) {
            m_labelQRCodeTip->setPixmap(m_waitingPixmap);
            m_labelQRCodeTip->show();
        }
    }
    w_timer->start();
}

void AuthDialog::onAuthHardwareDetached(int drvid, int detachedBioType)
{
    if (drvid < 0)
        return;

    const bool idsMatch = m_deviceInfo && m_deviceInfo->id == drvid;
    // UKey 等：pamBioSuccess 尚未调用 LoginOptionsWidget::startAuth 时 m_curDevInfo 为空，但 AuthDialog 已是 UKey 界面
    bool uiMatchesDetached = false;
    if (!idsMatch && !m_deviceInfo && detachedBioType >= 0 && m_loginOpts) {
        uiMatchesDetached = static_cast<unsigned>(m_loginOpts->convertDeviceType(detachedBioType)) == m_uCurLoginOptType;
    }

    if (!idsMatch && !uiMatchesDetached)
        return;

    // 避免 notifyOptionsChange 与拔出信号交错时已处于密码态时重复 startAuth（本轮拔出链路引入）
    const bool alreadyPasswordFlow = (authMode == PASSWORD && m_uCurLoginOptType == LOGINOPT_TYPE_PASSWORD);

    if (m_bioTimer && m_bioTimer->isActive())
        m_bioTimer->stop();
    if (isLoadingUkey)
        stopLoadingUkey();

    switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
    if (m_loginOpts) {
        m_loginOpts->stopAuth();
        m_loginOpts->setSelectedPassword();
    }
    m_deviceInfo = nullptr;
    authMode = PASSWORD;
    if (!alreadyPasswordFlow)
        startAuth();
}

void AuthDialog::onDeviceChanged(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo)
{
    isLoadingUkey = false;
    if (uCurLoginOptType == LOGINOPT_TYPE_PASSWORD) {
        if (m_bioTimer && m_bioTimer->isActive())
            m_bioTimer->stop();
        switchLoginOptType(uCurLoginOptType);
        if (m_loginOpts) {
            m_loginOpts->stopAuth();
        }
        m_deviceInfo = nullptr;
        authMode = PASSWORD;
        startAuth();
        return;
    }
    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo)
        return;

    if (deviceInfo)
        qDebug() << "device changed: " << *deviceInfo;
    if (deviceInfo && m_failMap[m_curUserInfo->uid()][deviceInfo->id] >= maxFailedTimes) {
        qDebug() << "Failed MAX!!";
        return;
    }

    initBiometricUI();

    if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo) {
        isLoadingUkey = true;
        startLoadingUkey();
    } else if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo) {
        stopLoadingUkey();
    }

    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo == m_deviceInfo) {
        return;
    }

    if (m_bioTimer && m_bioTimer->isActive())
        m_bioTimer->stop();

    authMode = BIOMETRIC;
    m_deviceInfo = deviceInfo;
    if (uCurLoginOptType == LOGINOPT_TYPE_FACE)
        switchLoginOptType(uCurLoginOptType, true);
    else
        switchLoginOptType(uCurLoginOptType);
    if (!isBioSuccess && deviceInfo)
        startBioAuth();
}

void AuthDialog::onBiometricAuthComplete(bool result, int nStatus)
{
    if (!result) {
        if (m_deviceInfo && m_deviceInfo->deviceType == UniT_General_Ukey && m_ukeyPasswdWidget->isVisible()) {
            if (m_ukeyPasswordEdit) {
                m_ukeyPasswordEdit->stopWaiting();
                m_ukeyPasswordEdit->clearText();
            }
        }

        if (manualStopBio) {
            manualStopBio = false;
            return;
        }

        if (nStatus == 5 && m_deviceInfo) {
            if (w_timer && w_timer->isActive())
                w_timer->stop();
            QImage imgFailed;
            setFaceImg(imgFailed, 2);
            return;
        } else if (nStatus >= 2 && nStatus != 5) {
            if (m_deviceInfo) {
                // 记录认证失败事件（用户触发的失败）
                recordLoginAuthEvent(m_curUserInfo->name(), 1, m_deviceInfo->deviceType); // 1 表示认证失败
                if (m_failMap.contains(m_curUserInfo->uid())
                    && m_failMap[m_curUserInfo->uid()].contains(m_deviceInfo->id)) {
                    m_failMap[m_curUserInfo->uid()][m_deviceInfo->id]
                        = m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] + 1;
                } else {
                    m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] = 1;
                }
                qDebug() << "Failed count:" << m_failMap[m_curUserInfo->uid()][m_deviceInfo->id]
                         << ",Max:" << maxFailedTimes;
                if (m_deviceInfo->deviceType == BioT_Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                    m_fTimeoutTimes = 0;
                }
                if (m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                    if (m_deviceInfo->deviceType == UniT_Remote) {
                        setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        QImage nullImage;
                        setQRCode(nullImage);
                    } else if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
                        setUkeyTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                           .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                    } else {
                        setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        stopBioWaiting(true);
                    }
                    if (m_loginOpts)
                        m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);

                    if (m_ukeyPasswdWidget && m_ukeyPasswdWidget->isVisible()) {
                        if (m_ukeyPasswordEdit) {
                            m_ukeyPasswordEdit->readOnly(true);
                        }
                    }

                    return;
                }

                if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
                    setUkeyTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                                       .arg(getDeviceType_tr(m_deviceInfo->deviceType))
                                       .arg(maxFailedTimes - m_failMap[m_curUserInfo->uid()][m_deviceInfo->id]));
                } else {
                    setLoginTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                                        .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType))
                                        .arg(maxFailedTimes - m_failMap[m_curUserInfo->uid()][m_deviceInfo->id]));
                }
            }
        }
        if (!isBioSuccess) {
            if (nStatus <= 0) {
                qDebug() << "Biometric dbus error:" << nStatus;
            }
            if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE && nStatus == 1) {
                m_isNetworkErr = true;
                setQRCodeMsg(tr("Abnormal network"));
                startBioAuth(10000);
            } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE && nStatus == -4) {
                m_isNetworkErr = true;
                setQRCodeMsg(tr("Acquisition failure"));
                startBioAuth(10000);
            } else if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE && nStatus == 1) {
                m_fTimeoutTimes += 1;
                if (m_fTimeoutTimes == UniAuthService::instance()->getFTimeoutTimes()) {
                    if (m_fRetryButton) {
                        m_fRetryButton->show();
                    }
                    if (m_labelFace) {
                        m_labelFace->hide();
                    }
                    setLoginTypeTip(tr("Face recognition waiting time out, please click refresh or enter the "
                                       "password to unlock."));
                    m_fTimeoutTimes = 0;
                } else {
                    startBioAuth();
                }
            } else {
                startBioAuth();
            }
            if (nStatus >= 2 && m_deviceInfo) {
                if (m_deviceInfo->deviceType == BioT_Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                }
            }
        }
    } else {
        // onShowMessage("验证成功!", QLightDM::Greeter::MessageTypeInfo);
        setDirLogin();
        isBioSuccess = true;
        onBiometricButtonClicked();
    }
}

void AuthDialog::onBiometricButtonClicked()
{
    // 当前没有设备
    if (!m_deviceInfo) {
        qWarning() << "未选择生物设备--";
        return;
    }
    authMode = BIOMETRIC;
    startAuth();
}

void AuthDialog::setUkeyTypeTip(QString text)
{
    if (m_ukeyMessageLabel) {
        QString textTip = text;
        if (!textTip.isEmpty()) {
            //        QFontMetrics font(m_ukeyMessageLabel->font());
            //        QString textTip = font.elidedText(textTip, Qt::ElideRight, m_messageLabel->width()-8);
            m_ukeyMessageLabel->setTipText(textTip);
            m_ukeyMessageLabel->show();
        } else {
            m_ukeyMessageLabel->hide();
            m_ukeyMessageLabel->setText("");
        }
    }
}

void AuthDialog::setLoginTypeTip(QString strLoginTypeTip)
{
    m_strLoginTypeTip = strLoginTypeTip;
    if (!m_strLoginTypeTip.isEmpty()) {
        m_labelLoginTypeTip->setTipText(m_strLoginTypeTip);
        m_labelLoginTypeTip->show();
    } else {
        m_labelLoginTypeTip->hide();
    }
}

void AuthDialog::setLoginMsg(QString strLoginMsg)
{
    setLoginTypeTip(strLoginMsg);
}

void AuthDialog::onBiometricDbusChanged(bool bActive)
{
    qDebug() << "BiometricDbus:" << bActive;
    if (bActive) {
        QTimer::singleShot(1000, this, [=]() {
            qDebug() << "OnDelay init biometric!!";
            if (!m_biometricProxy) {
                m_biometricProxy = new BiometricHelper(this);
                maxFailedTimes = UniAuthService::instance()->getMaxFailedTimes();
            }

            // 服务没启动，或者打开DBus连接出错
            if (!m_biometricProxy->isValid()) {
                qWarning() << "An error occurs when connect to the biometric DBus";
                if (m_deviceInfo) {
                    if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                        || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                        m_deviceInfo = DeviceInfoPtr();
                    }
                }
                switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
                return;
            }

            // 初始化生物识别认证UI
            initLoginOptsWidget();

            // 没有可用设备，不启用生物识别认证
            if (m_loginOpts->getLoginOptCount() < 1) {
                qWarning() << "No available devices";
                if (m_deviceInfo) {
                    if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                        || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                        m_deviceInfo = DeviceInfoPtr();
                    }
                }
                return;
            }

            // 获取默认设备
            if (m_loginOpts) {
                if (m_curUserInfo->name() == "*login")
                    m_deviceName = m_loginOpts->getDefaultDevice(m_strManualLoginName);
                else
                    m_deviceName = m_loginOpts->getDefaultDevice(m_curUserInfo->name());
            }
            qDebug() << m_deviceName;
            if (m_deviceInfo) {
                if (!m_loginOpts || !m_loginOpts->findDeviceById(m_deviceInfo->id)
                    || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                    m_deviceInfo = DeviceInfoPtr();
                }
            }
            // 如果默认设备为空的话，第一次不启动生物识别认证
            if (m_deviceName.isEmpty() && !m_deviceInfo) {
                qDebug() << "No default device";
                return;
            }
            // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
            if (!m_deviceInfo) {
                m_deviceInfo = m_loginOpts->findDeviceByName(m_deviceName);
                if (!m_deviceInfo || m_loginOpts->isDeviceDisable(m_deviceInfo->id)) {
                    qDebug() << "Device Not found or disable: " << m_deviceName;
                    // 因错误超限而禁用时仅提示不切换认证类型
                    if (m_deviceInfo && m_loginOpts->isDeviceDisable(m_deviceInfo->id)
                        && m_loginOpts->convertDeviceType(m_deviceInfo->deviceType) == m_uCurLoginOptType) {
                        // 已经错误超过限制
                        if (m_failMap.contains(m_curUserInfo->uid())
                            && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                            if (m_deviceInfo->deviceType == UniT_Remote) {
                                setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                                    .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                            } else {
                                setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                                    .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                                if (m_deviceInfo->deviceType == BioT_Face) {
                                    QImage imgFailed;
                                    setFaceImg(imgFailed, 1);
                                }
                                stopBioWaiting(true);
                            }
                            m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
                            return;
                        }
                    }
                    m_deviceInfo = DeviceInfoPtr();
                    return;
                }
            }
            initBiometricUI();

            // 已经错误超过3次
            if (m_failMap.contains(m_curUserInfo->uid())
                && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                if (m_deviceInfo->deviceType == UniT_Remote) {
                    setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                        .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                } else {
                    setLoginTypeTip(tr("Failed to verify %1, please try another way to unlock")
                                        .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                    if (m_deviceInfo->deviceType == BioT_Face) {
                        QImage imgFailed;
                        setFaceImg(imgFailed, 1);
                    }
                }
                m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
                return;
            }

            if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
                switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
                m_loginOpts->setSelectedPassword();
                return;
            }

            switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
            // 自行校验生物设备，pam greeter校验密码
            startBioAuth();
        });
    }
}

void AuthDialog::onFRetryButtonClicked()
{
    if (m_fRetryButton) {
        m_fRetryButton->hide();
    }
    switchLoginOptType(LOGINOPT_TYPE_FACE);
    startBioAuth();
}

void AuthDialog::setWillLoginUser(QString strUserName, bool isOneKeyLogin)
{
    m_strServiceUser = strUserName;
    m_isOneKeyLogin = isOneKeyLogin;
}

void AuthDialog::onCustomRequestAccount(QString strName)
{
    qDebug() << "onCustomRequestAccount:" << strName;
    Q_EMIT customRequestAccount(strName);
}

void AuthDialog::onCustomAuthResult(int nResult, QString strMsg)
{
    if (m_uCurLoginOptType != LOGINOPT_TYPE_CUSTOM) {
        qDebug() << "onCustomAuthResult error Current Auth type is not costom!!";
        return;
    }
    if (nResult) {
        if (!isBioSuccess) {
            qDebug() << "onCustomAuthResult:" << nResult << strMsg;
            if (m_deviceInfo) {
                recordLoginAuthEvent(m_curUserInfo->name(), 1, m_deviceInfo->deviceType);
            }
            startBioAuth();
        }
    } else {
        qDebug() << "onCustomAuthResult success!!";
        setDirLogin();
        isBioSuccess = true;
        onBiometricButtonClicked();
    }
}

QString AuthDialog::onCustomRequest(QString strReqJson)
{
    qDebug() << "onCustomRequest:" << strReqJson;
    QJsonParseError jsonParseError;
    QJsonObject retObj;
    retObj["Ret"] = 0;
    retObj["Message"] = "Success";
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strReqJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        retObj["Ret"] = -1;
        retObj["Message"] = "Parse request json failed!!";
        qWarning() << "Parse request json failed!!";
        return QString(QJsonDocument(retObj).toJson());
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
            qWarning() << "JSON串为空";
            retObj["Ret"] = -1;
            retObj["Message"] = "Request Json is null!!";
            qWarning() << "Request Json is null!!";
            return QString(QJsonDocument(retObj).toJson());
        } else {
            QJsonObject contentObj;
            QString reqType = rootObj.value("ReqType").toString();
            qInfo() << "Req type: " << reqType;
            if (reqType == "GetInformations") { // 请求信息
                QString strType = rootObj.value("Type").toString();
                if (strType.contains("CurrentUser")) { // 当前用户信息
                    if (m_curUserInfo->uid() >= 0 && !m_curUserInfo->name().isEmpty()
                        && !m_curUserInfo->name().startsWith("*")) {
                        QJsonObject user;
                        user["Name"] = m_curUserInfo->name();
                        user["Id"] = (double)(m_curUserInfo->uid());
                        user["HeadImg"] = m_curUserInfo->headImage();
                        user["RealName"] = m_curUserInfo->fullName();
                        contentObj["CurrentUser"] = user;
                    } else {
                        retObj["Ret"] = -1;
                        retObj["Message"] = "Current user is invalid!";
                    }
                }
            }
            retObj["Content"] = contentObj;
            return QString(QJsonDocument(retObj).toJson());
        }
    }
}

int AuthDialog::onCustomPlugEnv()
{
    return LoginPluginInterface::LOGINPLUGINENV_LOGIN;
}

void AuthDialog::setCustomAuthDefault(bool isDefault)
{
    m_isCustomDefault = isDefault;
}

QString AuthDialog::getDeviceType_tr(int deviceType)
{
    switch (deviceType) {
        case BioT_FingerPrint:
            return tr("FingerPrint");
        case BioT_FingerVein:
            return tr("FingerVein");
        case BioT_Iris:
            return tr("Iris");
        case BioT_Face:
            return tr("Face");
        case BioT_VoicePrint:
            return tr("VoicePrint");
        case UniT_General_Ukey:
            return tr("Ukey");
        case UniT_Advanced_Ukey:
            return tr("Ukey");
        case UniT_Remote:
            return tr("QRCode");
        default:
            return "";
    }
}

QString AuthDialog::getDeviceType_lowerTr(int deviceType)
{
    switch (deviceType) {
        case BioT_FingerPrint:
            return tr("fingerprint");
        case BioT_FingerVein:
            return tr("fingervein");
        case BioT_Iris:
            return tr("iris");
        case BioT_Face:
            return tr("face");
        case BioT_VoicePrint:
            return tr("voiceprint");
        case UniT_General_Ukey:
            return tr("Ukey");
        case UniT_Remote:
            return tr("QRCode");
        default:
            return "";
    }
}

void AuthDialog::startBioWaiting(unsigned uCurLoginOptType)
{
    if (uCurLoginOptType == m_nCurBioWaitingType
        || (uCurLoginOptType != LOGINOPT_TYPE_FINGERPRINT && uCurLoginOptType != LOGINOPT_TYPE_FINGERVEIN
            && uCurLoginOptType != LOGINOPT_TYPE_IRIS && uCurLoginOptType != LOGINOPT_TYPE_VOICEPRINT)) {
        return;
    }
    m_nCurBioWaitingType = uCurLoginOptType;
    if (!m_timerBioWaiting) {
        m_timerBioWaiting = new QTimer(this);
        m_timerBioWaiting->setInterval(100);
    }
    if (m_timerBioWaiting) {
        if (m_timerBioWaiting->isActive()) {
            m_timerBioWaiting->stop();
        }
        // 重新连接信号槽
        disconnect(m_timerBioWaiting, nullptr, this, nullptr);
        connect(m_timerBioWaiting, &QTimer::timeout, this, [&, this, uCurLoginOptType]() {
            m_nBioWaitingFrame += 1;
            QString strPicName = "";
            switch (uCurLoginOptType) {
                case LOGINOPT_TYPE_FINGERPRINT: {
                    if (m_nBioWaitingFrame >= 15) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString(":/image/assets/biowaiting/fingerprint%1.svg").arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_FINGERVEIN: {
                    if (m_nBioWaitingFrame >= 12) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString(":/image/assets/biowaiting/fingervein%1.svg").arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_IRIS: {
                    if (m_nBioWaitingFrame >= 12) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString(":/image/assets/biowaiting/iris%1.svg").arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_VOICEPRINT: {
                    if (m_nBioWaitingFrame >= 8) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString(":/image/assets/biowaiting/voiceprint%1.svg").arg(m_nBioWaitingFrame);
                } break;
                default:
                    break;
            }
            if (!strPicName.isEmpty()) {
                QPixmap pixmap;
                if (QPixmapCache::find(strPicName, &pixmap)) {
                    if (m_labelBioWaiting) {
                        m_labelBioWaiting->setPixmap(
                            scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                    }
                } else {
                    pixmap = QPixmap(strPicName);
                    if (!pixmap.isNull()) {
                        QPixmapCache::insert(strPicName, pixmap);
                        if (m_labelBioWaiting) {
                            m_labelBioWaiting->setPixmap(
                                scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                        }
                    }
                }
            }
        });
        m_timerBioWaiting->start();
        m_nBioWaitingFrame = 0;
        QString strPicName = "";
        switch (uCurLoginOptType) {
            case LOGINOPT_TYPE_FINGERPRINT: {
                strPicName = QString(":/image/assets/biowaiting/fingerprint%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_FINGERVEIN: {
                strPicName = QString(":/image/assets/biowaiting/fingervein%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_IRIS: {
                strPicName = QString(":/image/assets/biowaiting/iris%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_VOICEPRINT: {
                strPicName = QString(":/image/assets/biowaiting/voiceprint%1.svg").arg(m_nBioWaitingFrame);
            } break;
            default:
                break;
        }
        if (!strPicName.isEmpty()) {
            QPixmap pixmap;
            if (QPixmapCache::find(strPicName, &pixmap)) {
                if (m_labelBioWaiting) {
                    m_labelBioWaiting->setPixmap(
                        scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                }
            } else {
                pixmap = QPixmap(strPicName);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    if (m_labelBioWaiting) {
                        m_labelBioWaiting->setPixmap(
                            scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                    }
                }
            }
        }
    }
}

void AuthDialog::stopBioWaiting(bool isDisable /* = false*/)
{
    if (m_timerBioWaiting) {
        if (m_timerBioWaiting->isActive()) {
            m_timerBioWaiting->stop();
        }
    }
    m_nBioWaitingFrame = 0;
    if (isDisable) {
        QString strPicName = "";
        switch (m_nCurBioWaitingType) {
            case LOGINOPT_TYPE_FINGERPRINT: {
                strPicName = QString(":/image/assets/biowaiting/fingerprint%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_FINGERVEIN: {
                strPicName = QString(":/image/assets/biowaiting/fingervein%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_IRIS: {
                strPicName = QString(":/image/assets/biowaiting/iris%1.svg").arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_VOICEPRINT: {
                strPicName = QString(":/image/assets/biowaiting/voiceprint%1.svg").arg(m_nBioWaitingFrame);
            } break;
            default:
                break;
        }
        if (!strPicName.isEmpty()) {
            QPixmap pixmap;
            if (QPixmapCache::find(strPicName, &pixmap)) {
                if (m_labelBioWaiting) {
                    m_labelBioWaiting->setPixmap(
                        scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                }
            } else {
                pixmap = QPixmap(strPicName);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    if (m_labelBioWaiting) {
                        m_labelBioWaiting->setPixmap(
                            scaledSmoothPixmap(pixmap, WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE));
                    }
                }
            }
        }
    }
    m_nCurBioWaitingType = -1;
}

void AuthDialog::inhibitShow(bool isInhibit)
{
    if (m_isInhibitShow != isInhibit) {
        if (!isInhibit) {
            if (isHidden() || !isVisible()) {
                show();
            }
        } else {
            if (!isHidden() || isVisible()) {
                hide();
            }
        }
        m_isInhibitShow = isInhibit;
    }
}

void AuthDialog::initPamUkeyLoadingWidget()
{
    if (!m_pamUkeyLoadingWidget)
        return;

    if (!m_pamUkeyLoadingButton) {
        m_pamUkeyLoadingButton = new QPushButton(m_pamUkeyLoadingWidget);
        m_pamUkeyLoadingButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_pamUkeyLoadingButton->setFlat(true);
        m_pamUkeyLoadingButton->installEventFilter(this);
        m_pamUkeyLoadingButton->setStyleSheet("QPushButton{text-align:center;color: rgba(255, 255, 255, 255);border: "
                                              "none;border-radius: 4px;outline: none;}");

        m_pamUkeyLoadingText = new CustomLabel(m_pamUkeyLoadingWidget);
        m_pamUkeyLoadingText->setText(tr("Insert the ukey into the USB port"));

        QVBoxLayout *m_pamukeyLoadingLayout = new QVBoxLayout(m_pamUkeyLoadingWidget);
        m_pamukeyLoadingLayout->addWidget(m_pamUkeyLoadingButton);
        m_pamukeyLoadingLayout->addWidget(m_pamUkeyLoadingText);
        m_pamukeyLoadingLayout->setAlignment(Qt::AlignCenter);
    }
}

void AuthDialog::showAuthMethodTip()
{
    if (m_authMethodLabel) {
        if (authMethod == "otp") {
            m_authMethodLabel->setText(tr("Two-Factor OTP Dynamic Password Authentication"));
            m_authMethodLabel->show();
        } else if (authMethod == "ukey") {
            m_authMethodLabel->setText(tr("Two-Factor Security Key Authentication"));
            m_authMethodLabel->show();
        }
    }

    updateAuthSize();
}

void AuthDialog::setBiometricAuthDisabledStatus(bool locked)
{
    if (locked) {
        if (m_loginOpts) {
            m_loginOpts->stopAuth();
            m_loginOpts->setAllDeviceDisable(true);
            if (m_bioTimer && m_bioTimer->isActive()) {
                m_bioTimer->stop();
            }
        }
        if (m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_FACE) {
            QImage imgFailed;
            setFaceImg(imgFailed, 1);
        } else if (m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_QRCODE) {
            QImage nullImage;
            setQRCode(nullImage);
        }
    } else {
        if (m_loginOpts) {
            m_loginOpts->setAllDeviceDisable(false);
        }
        if (m_deviceInfo) {
            m_loginOpts->startAuth(m_deviceInfo, m_curUserInfo->uid());
        }
    }
}

void AuthDialog::recordLoginAuthEvent(QString userName, int loginResult, int deviceType)
{
    // 准备参数
    if (userName.isEmpty() || userName.startsWith("*")) {
        // 如果没有有效用户名，跳过记录
        return;
    }

    // 将 deviceType 转换为对应的数字字符串
    QString authMethod = "";
    switch (deviceType) {
        case -1: {
            authMethod = "";
        } break;
        case BioT_FingerPrint:
        case BioT_FingerVein:
        case BioT_Iris:
        case BioT_Face:
        case BioT_VoicePrint:
        case UniT_KCM:
        case UniT_General_Ukey:
        case UniT_Advanced_Ukey:
        case UniT_Remote: {
            authMethod = getDeviceType_tr(deviceType);
        } break;
        case UniT_Custom: {
            authMethod = tr("Other");
        } break;
        default: {
            authMethod = QString::number(deviceType);
        } break;
    }
    if (m_modelLockDialog) {
        m_modelLockDialog->recordLoginAuthEvent(userName, loginResult, authMethod);
    }
}

void AuthDialog::beginLockEnterRowAnimation(int yOffsetPx)
{
    if (yOffsetPx <= 0) {
        return;
    }
    if (m_lockLoginOptsDetached) {
        endLockLoginOptsEnterAnimation();
    }
    if (m_lockEnterRowsDetached) {
        endLockEnterRowAnimation();
    }
    m_lockEnterIncludesUkey = false;
    auto *vl = qobject_cast<QVBoxLayout *>(layout());
    if (!vl || !m_headerWidget || !m_nameWidget || !m_editWidget) {
        return;
    }
    m_lockEnterHeaderInsertIndex = vl->indexOf(m_headerWidget);
    if (m_lockEnterHeaderInsertIndex < 0) {
        return;
    }

    m_lockEnterIncludesUkey = m_ukeyPasswdWidget && m_ukeyPasswdWidget->isVisible() && m_ukeyPasswdWidget->height() > 0;

    const QRect gHead = m_headerWidget->geometry();
    const QRect gName = m_nameWidget->geometry();
    const QRect gEdit = m_editWidget->geometry();
    const QRect gUkey = m_lockEnterIncludesUkey ? m_ukeyPasswdWidget->geometry() : QRect();
    const int blockBottom = m_lockEnterIncludesUkey ? qMax(gEdit.bottom(), gUkey.bottom()) : gEdit.bottom();
    const int blockH = qMax(1, blockBottom - gHead.top());

    vl->removeWidget(m_headerWidget);
    vl->removeWidget(m_nameWidget);
    vl->removeWidget(m_editWidget);
    if (m_lockEnterIncludesUkey) {
        vl->removeWidget(m_ukeyPasswdWidget);
    }

    if (!m_lockEnterPlaceholder) {
        m_lockEnterPlaceholder = new QWidget(this);
    }
    m_lockEnterPlaceholder->setFixedSize(qMax(width(), 1), blockH);
    m_lockEnterPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    vl->insertWidget(m_lockEnterHeaderInsertIndex, m_lockEnterPlaceholder);

    m_headerWidget->setParent(this);
    m_nameWidget->setParent(this);
    m_editWidget->setParent(this);

    m_headerWidget->setGeometry(gHead.translated(0, -yOffsetPx));
    m_nameWidget->setGeometry(gName.translated(0, -yOffsetPx));
    m_editWidget->setGeometry(gEdit.translated(0, -yOffsetPx));

    m_headerWidget->raise();
    m_nameWidget->raise();
    m_editWidget->raise();

    if (m_lockEnterIncludesUkey) {
        m_ukeyPasswdWidget->setParent(this);
        m_ukeyPasswdWidget->setGeometry(gUkey.translated(0, -yOffsetPx));
        m_ukeyPasswdWidget->raise();
    }

    m_lockEnterRowsDetached = true;
}

void AuthDialog::endLockEnterRowAnimation()
{
    if (!m_lockEnterRowsDetached) {
        return;
    }
    auto *vl = qobject_cast<QVBoxLayout *>(layout());
    if (!vl) {
        m_lockEnterRowsDetached = false;
        return;
    }
    int idx = (m_lockEnterPlaceholder && m_lockEnterPlaceholder->parent() == this) ? vl->indexOf(m_lockEnterPlaceholder) : -1;
    if (idx < 0) {
        idx = m_lockEnterHeaderInsertIndex;
    }
    if (m_lockEnterPlaceholder && m_lockEnterPlaceholder->parent() == this) {
        vl->removeWidget(m_lockEnterPlaceholder);
    }
    if (m_lockEnterPlaceholder) {
        m_lockEnterPlaceholder->hide();
    }
    vl->insertWidget(idx, m_headerWidget);
    vl->insertWidget(idx + 1, m_nameWidget);
    vl->insertWidget(idx + 2, m_editWidget);
    if (m_lockEnterIncludesUkey && m_ukeyPasswdWidget) {
        vl->insertWidget(idx + 3, m_ukeyPasswdWidget);
    }
    m_lockEnterIncludesUkey = false;
    m_lockEnterRowsDetached = false;
    updateAuthSize();
}

void AuthDialog::beginLockLoginOptsEnterAnimation(int yOffsetPx)
{
    if (yOffsetPx <= 0 || !m_widgetLoginOpts) {
        return;
    }
    if (!m_widgetLoginOpts->isVisible() || m_widgetLoginOpts->height() <= 0) {
        return;
    }
    if (m_lockLoginOptsDetached) {
        endLockLoginOptsEnterAnimation();
    }
    auto *vl = qobject_cast<QVBoxLayout *>(layout());
    if (!vl) {
        return;
    }
    m_lockLoginOptsInsertIndex = vl->indexOf(m_widgetLoginOpts);
    if (m_lockLoginOptsInsertIndex < 0) {
        return;
    }

    const QRect gOpts = m_widgetLoginOpts->geometry();
    vl->removeWidget(m_widgetLoginOpts);

    if (!m_lockLoginOptsPlaceholder) {
        m_lockLoginOptsPlaceholder = new QWidget(this);
    }
    m_lockLoginOptsPlaceholder->setFixedSize(qMax(gOpts.width(), 1), qMax(gOpts.height(), 1));
    m_lockLoginOptsPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    vl->insertWidget(m_lockLoginOptsInsertIndex, m_lockLoginOptsPlaceholder);

    m_widgetLoginOpts->setParent(this);
    m_widgetLoginOpts->setGeometry(gOpts.translated(0, -yOffsetPx));
    m_widgetLoginOpts->raise();

    m_lockLoginOptsDetached = true;
}

void AuthDialog::endLockLoginOptsEnterAnimation()
{
    if (!m_lockLoginOptsDetached || !m_widgetLoginOpts) {
        return;
    }
    auto *vl = qobject_cast<QVBoxLayout *>(layout());
    if (!vl) {
        m_lockLoginOptsDetached = false;
        return;
    }
    int idx = (m_lockLoginOptsPlaceholder && m_lockLoginOptsPlaceholder->parent() == this)
                  ? vl->indexOf(m_lockLoginOptsPlaceholder)
                  : -1;
    if (idx < 0) {
        idx = m_lockLoginOptsInsertIndex;
    }
    if (m_lockLoginOptsPlaceholder && m_lockLoginOptsPlaceholder->parent() == this) {
        vl->removeWidget(m_lockLoginOptsPlaceholder);
    }
    if (m_lockLoginOptsPlaceholder) {
        m_lockLoginOptsPlaceholder->hide();
    }
    vl->insertWidget(idx, m_widgetLoginOpts);
    m_lockLoginOptsDetached = false;
    updateAuthSize();
}
