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
#include "klabel.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "statusbutton.h"
#include "commonfunc.h"
#include "biobuttonlistwidget.h"
#include "definetypes.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QPixmapCache>
#include "definetypes.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "../lock-dialog/pam-tally.h"
#include "global_utils.h"
#include "securityquestionanswer.h"

#define WIDGET_HEIGHT_HEADTIP (40)
#define WIDGET_HEIGHT_HEADIMAGE ((int)(154 * scale))
#define WIDGET_HEIGHT_USERNAME (40)
#define WIDGET_HEIGHT_PASSWDEDIT (80)
#define WIDGET_HEIGHT_SPACING ((int)(16 * scale))
#define WIDGET_HEIGHT_CUSTOM                                                                                           \
    (WIDGET_HEIGHT_HEADTIP + WIDGET_HEIGHT_HEADIMAGE + WIDGET_HEIGHT_USERNAME + WIDGET_HEIGHT_PASSWDEDIT               \
     + WIDGET_HEIGHT_SPACING * 3)
#define WIDGET_HEIGHT_LOGINOPTS (104)

AuthDialog::AuthDialog(LockDialogModel *model, UserInfoPtr userInfo, QWidget *parent)
    : QWidget(parent)
    , m_modelLockDialog(model)
    , m_isLockingFlg(false)
    , m_isInputPasswd(false)
    , m_curUserInfo(userInfo)
    , m_uniauthService(new UniAuthService(this))
    , m_pSecurityQuestionAnswer(new SecurityQuestionAnswer(this))
{
    initUI();
}

void AuthDialog::initUI()
{
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

    mainLayout->addStretch();
    mainLayout->addWidget(m_widgetPlaceholder);
    mainLayout->addWidget(m_tipWidget);
    mainLayout->addWidget(m_headerWidget);
    mainLayout->addWidget(m_nameWidget);
    mainLayout->addWidget(m_editWidget);
    mainLayout->addWidget(m_ukeyPasswdWidget);
    mainLayout->addWidget(m_customWidget);
    mainLayout->addWidget(m_widgetLoginOpts);
    mainLayout->addStretch();

    initTipWidget();
    initHeaderWidget();
    initNameWidget();
    initEditWidget();
    initUkeyPasswordWidget();
    initCustomAuthWidget();
    initLoginoptionWidget();

    initConnections();
    updateUI();
}

void AuthDialog::initConnections()
{
    connect(m_modelLockDialog, &LockDialogModel::pamShowMessage, this, &AuthDialog::onPamShowMessage);
    connect(m_modelLockDialog, &LockDialogModel::pamShowPrompt, this, &AuthDialog::onPamShowPrompt);
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
        }
    }

    return QWidget::eventFilter(obj, event);
}

void AuthDialog::resizeEvent(QResizeEvent *event)
{
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
    if (m_editWidget)
        m_editWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);
    if (m_widgetPlaceholder) {
        m_widgetPlaceholder->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);
    }
    if (m_ukeyPasswdWidget)
        m_ukeyPasswdWidget->setFixedSize(this->width(), WIDGET_HEIGHT_PASSWDEDIT);

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
    if (m_labelQRCodeTip)
        m_labelQRCodeTip->setFixedSize(22 * scale, 22 * scale);
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
    m_labelLoginTypeTip = new KLabel(m_tipWidget);
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
    m_labelHeadImg = new QLabel(m_headerWidget);
    headerLayout->addWidget(m_labelHeadImg);

    // 人脸识别头像框
    m_labelFace = new QLabel(m_headerWidget);
    headerLayout->addWidget(m_labelFace);
    m_labelFace->hide();

    // 生物特征等待验证动画
    m_labelBioWaiting = new QLabel(m_headerWidget);
    headerLayout->addWidget(m_labelBioWaiting);
    m_labelBioWaiting->hide();

    // 人脸识别重试
    m_fRetryButton = new QPushButton(m_headerWidget);
    m_fRetryButton->setFocusPolicy(Qt::NoFocus);
    m_fRetryButton->setStyleSheet(
        QString("QPushButton{border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);}"
                "QPushButton::hover{background-color: rgba(255,255,255,30%);}"
                "QPushButton::pressed {background-color: rgba(255,255,255,40%);}")
            .arg((int)(77 * scale)));

    QPixmap retryIcon = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    m_fRetryButton->setIcon(drawSymbolicColoredPixmap(retryIcon, "white"));
    m_fRetryButton->setIconSize(QSize(48, 48));
    connect(m_fRetryButton, &QPushButton::clicked, this, &AuthDialog::onFRetryButtonClicked);
    m_fRetryButton->hide();
    headerLayout->addWidget(m_fRetryButton);

    // 微信认证窗口
    m_labelQRCode = new QLabel(m_headerWidget);
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->setStyleSheet(
        QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);")
            .arg((int)(6)));
    m_labelQRCode->hide();
    QVBoxLayout *layoutQRCode = new QVBoxLayout();
    layoutQRCode->setAlignment(Qt::AlignCenter);
    layoutQRCode->setSpacing(10);
    m_labelQRCode->setLayout(layoutQRCode);
    m_labelQRCodeTip = new QLabel();
    layoutQRCode->addWidget(m_labelQRCodeTip, 0, Qt::AlignHCenter);
    m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22, 22)));
    m_labelQRCodeTip->show();
    m_labelQRCodeMsg = new KLabel();
    m_labelQRCodeMsg->setStyleSheet("QLabel{background-color:rgba(255,255,255,0);color:rgba(255,0,0)}");
    m_labelQRCodeMsg->hide();
    layoutQRCode->addWidget(m_labelQRCodeMsg, 0, Qt::AlignHCenter);
    headerLayout->addWidget(m_labelQRCode);
}

void AuthDialog::initNameWidget()
{
    QVBoxLayout *nameLayout = new QVBoxLayout(m_nameWidget);
    nameLayout->setAlignment(Qt::AlignHCenter);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    m_nameLabel = new KLabel(m_nameWidget);
    m_nameLabel->setAlignment(Qt::AlignCenter);
}

void AuthDialog::initEditWidget()
{
    QVBoxLayout *editWidgetLayout = new QVBoxLayout(m_editWidget);
    editWidgetLayout->setContentsMargins(0, 0, 0, 0);
    editWidgetLayout->setAlignment(Qt::AlignHCenter);
    m_pEditLayout = new QHBoxLayout();
    m_pEditLayout->setContentsMargins(0, 0, 0, 0);

    m_pEditLayout->setAlignment(Qt::AlignHCenter);
    m_passwordEdit = new IconEdit(m_modelLockDialog, m_editWidget);
    // m_passwordEdit->setPrompt("password");
    m_pEditLayout->addWidget(m_passwordEdit);
    connect(m_passwordEdit, &IconEdit::clickedPassword, this, [=] { Q_EMIT lineEditClicked(); });

    //  密码提示按钮
    m_pPwdTipMessage = new StatusButton(m_editWidget, PWDTIPBTN);
    m_pEditLayout->addWidget(m_pPwdTipMessage);
    m_pPwdTipMessage->setFixedSize(40, 40);
    QPixmap tipIcon = QIcon::fromTheme("help-about-symbolic").pixmap(16, 16);
    m_pPwdTipMessage->setIcon(drawSymbolicColoredPixmap(tipIcon, "white"));

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

    m_messageLabel = new KLabel(m_editWidget);
    m_messageLabel->setAlignment(Qt::AlignHCenter);
    editWidgetLayout->addWidget(m_messageLabel);
}

void AuthDialog::initUkeyPasswordWidget()
{
    QVBoxLayout *ukeyWidgetLayout = new QVBoxLayout(m_ukeyPasswdWidget);
    ukeyWidgetLayout->setContentsMargins(0, 0, 0, 0);
    ukeyWidgetLayout->setAlignment(Qt::AlignHCenter);
    QVBoxLayout *editLayout = new QVBoxLayout();
    editLayout->setContentsMargins(0, 0, 0, 0);
    editLayout->setAlignment(Qt::AlignHCenter);
    m_ukeyPasswordEdit = new IconEdit(m_modelLockDialog);
    m_ukeyPasswordEdit->setPrompt(tr(""));
    editLayout->addWidget(m_ukeyPasswordEdit);
    ukeyWidgetLayout->addLayout(editLayout);
    connect(m_ukeyPasswordEdit, &IconEdit::clicked, this, [=] { Q_EMIT lineEditClicked(); });
    m_ukeyMessageLabel = new KLabel(m_ukeyPasswdWidget);
    m_ukeyMessageLabel->setText("");
    m_ukeyMessageLabel->setAlignment(Qt::AlignHCenter);
    ukeyWidgetLayout->addWidget(m_ukeyMessageLabel);
    m_ukeyPasswdWidget->hide();
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
    //    m_optionTip = new KLabel();
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
    QPixmap p(m_curUserInfo->headImage());
    QPixmap userIcon = scaledPixmap(p);
    userIcon = userIcon.scaled(154 * scale, 154 * scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    userIcon = PixmapToRound(userIcon, 77 * scale);
    m_labelHeadImg->setPixmap(userIcon);

    if (m_curUserInfo->name() == "*login") {
        m_nameLabel->setText(tr("Login"));
    } else if (m_curUserInfo->name() == "*guest") {
        m_nameLabel->setText(tr("Guest"));
    } else {
        m_nameLabel->setText(m_curUserInfo->fullName());
    }

    m_fRetryButton->setStyleSheet(
        QString("QPushButton{border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);}"
                "QPushButton::hover{background-color: rgba(255,255,255,30%);}"
                "QPushButton::pressed {background-color: rgba(255,255,255,40%);}")
            .arg((int)(77 * scale)));
    m_fRetryButton->setFixedSize(154 * scale, 154 * scale);
    m_fRetryButton->setIconSize(QSize(48 * scale, 48 * scale));
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
    // isLoadingUkey = true;
    if (m_ukeyLoadingWidget)
        m_ukeyLoadingWidget->show();
    m_ukeyPasswordEdit->hide();
    m_ukeyMessageLabel->hide();
    if (!m_loadingTimer) {
        m_loadingTimer = new QTimer(this);
        m_loadingTimer->setInterval(150);
        connect(m_loadingTimer, &QTimer::timeout, this, &AuthDialog::updateLoadingPixmap);
    }

    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(27, 27);
    m_ukeyLoadingPixmap = drawSymbolicColoredPixmap(icon, "white");
    m_loadingButton->setIcon(m_ukeyLoadingPixmap);
    m_loadingButton->setIconSize(QSize(27, 27));
    m_loadingTimer->start();
}

void AuthDialog::stopLoadingUkey()
{
    isLoadingUkey = false;
    if (m_ukeyLoadingWidget)
        m_ukeyLoadingWidget->hide();
    m_ukeyPasswordEdit->show();
    m_ukeyMessageLabel->show();
    if (m_loadingTimer) {
        m_loadingTimer->stop();
    }
}

void AuthDialog::updateLoadingPixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_ukeyLoadingPixmap = m_ukeyLoadingPixmap.transformed(matrix, Qt::FastTransformation);
    m_loadingButton->setIcon(QIcon(m_ukeyLoadingPixmap));
}

void AuthDialog::switchLoginOptType(unsigned uLoginOptType, bool faceBtnClicked)
{
    switch (uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD: {
            m_tipWidget->show();
            m_editWidget->show();
            m_widgetPlaceholder->hide();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->show();
            m_labelQRCode->hide();
            m_labelFace->hide();
            m_labelBioWaiting->hide();
            stopBioWaiting();
            m_fRetryButton->hide();
            m_ukeyPasswdWidget->hide();
            m_ukeyPasswordEdit->clearFocus();
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
            m_labelQRCode->hide();
            m_labelFace->show();
            m_labelBioWaiting->hide();
            stopBioWaiting();
            m_fRetryButton->hide();
            m_ukeyPasswdWidget->hide();
            m_ukeyPasswordEdit->clearFocus();
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
            m_labelQRCode->hide();
            m_labelFace->hide();
            m_labelBioWaiting->show();
            startBioWaiting(uLoginOptType);
            m_fRetryButton->hide();
            m_ukeyPasswdWidget->hide();
            m_ukeyPasswordEdit->clearFocus();
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
            setQRCodeMsg("");
            m_labelQRCode->show();
            m_labelFace->hide();
            m_labelBioWaiting->hide();
            stopBioWaiting();
            m_fRetryButton->hide();
            m_ukeyPasswdWidget->hide();
            m_ukeyPasswordEdit->clearFocus();
            m_customWidget->hide();
            setFocusProxy(m_passwordEdit);
        } break;
        case LOGINOPT_TYPE_GENERAL_UKEY: {
            m_tipWidget->show();
            m_editWidget->show();
            m_widgetPlaceholder->hide();
            m_headerWidget->show();
            m_nameWidget->show();
            m_labelHeadImg->show();
            m_labelQRCode->hide();
            m_labelFace->hide();
            m_labelBioWaiting->hide();
            stopBioWaiting();
            m_fRetryButton->hide();
            m_customWidget->hide();
            m_ukeyPasswdWidget->show();
            setFocusProxy(m_ukeyPasswordEdit);
            m_ukeyPasswordEdit->setFocusPolicy(Qt::StrongFocus);
            if (m_ukeyPasswordEdit->isVisible())
                m_ukeyPasswordEdit->setFocus();
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
            m_ukeyPasswordEdit->clearFocus();
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
                setLoginTypeTip(tr("Use the bound wechat scanning code to unlock"));
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
        m_labelQRCodeTip->hide();
    }
    m_imgQRCode = m_imgQRCode.scaled(QSize(148 * scale, 148 * scale));
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->setPixmap(QPixmap::fromImage(m_imgQRCode));
}

void AuthDialog::setQRCodeMsg(QString strMsg)
{
    if (strMsg.isEmpty()) {
        m_labelQRCodeMsg->hide();
        // m_labelQRCodeTip->hide();
    } else {
        // 一开始认证时就没有网，直接停止加载状态
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        m_labelQRCodeMsg->setTipText(strMsg);
        m_labelQRCodeMsg->show();
        m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22, 22)));
        m_labelQRCodeTip->show();
    }
}

void AuthDialog::setFaceImg(QImage &imgFace, int nStatus)
{
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

void AuthDialog::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE)
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
    else if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFace->setPixmap(m_waitingPixmap);
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
            strText = m_preStrMessage + "," + strText;
        }
        m_messageLabel->setTipText(strText);
    }
    m_bHasUnacknowledgedMsg = true;
}

void AuthDialog::showPwdTipBtn()
{
    m_pPwdTipMessage->setVisible(m_userFailedTimesMap[m_curUserInfo->uid()] >= 1);
    if (m_userFailedTimesMap[m_curUserInfo->uid()] >= 1 &&
            m_pSecurityQuestionAnswer->GetUserPasswdHint(m_curUserInfo->uid()) != "") {
        m_pEditLayout->setContentsMargins(48, 0, 0, 0);
        m_pPwdTipMessage->show();
        m_pPwdTipMessage->setToolTip(m_pSecurityQuestionAnswer->GetUserPasswdHint(m_curUserInfo->uid()));
    } else {
        m_pEditLayout->setContentsMargins(0, 0, 0, 0);
    }
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
    qDebug() << result << authUid << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    if (result == 0 && authUid > 0) {
        struct passwd *pwd;
        pwd = getpwuid(authUid);
        if (pwd && pwd->pw_name) {
            if (m_uniauthService && m_uniauthService->isActivatable()) {
                bool isEnable = m_uniauthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_BIO);
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

void AuthDialog::onPamShowPrompt(QString strPrompt, int nType)
{
    qDebug() << "prompt: " << strPrompt << authMode;
    //    if(isPowerup && m_configuration->getIs990()){
    //        waitBiometricServiceStatus();
    //        startFirstLogin();
    //    }
    if (strPrompt == BIOMETRIC_PAM || strPrompt == BIOMETRIC_PAM_DOUBLE || strPrompt == BIOMETRIC_PAM_QRCODE) {
        if (m_bDirectLogin) {
            onRespond(BIOMETRIC_SUCCESS);
            return;
        }
        if (authMode == PASSWORD) {
            skipBiometricAuth();
        } else {
            skipBiometricAuth();
            //            if(isPowerup  && m_configuration->getIs990()) {
            //                QTimer::singleShot(1500, [&]{
            //                    performBiometricAuth();
            //                });
            //                isPowerup = false;
            //            } else {
            performBiometricAuth();
            //            }
        }
    } else {
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
        if (!m_biometricProxy || !m_biometricProxy->isValid() || !m_deviceInfo) {
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            if (m_loginOpts)
                m_loginOpts->setSelectedPassword();
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
    stopBioWaiting();
    bool isAuthenticated = (Q_EMIT m_modelLockDialog->pamIsAuthenticated());
    qDebug() << "onPamAuthCompleted:" << isAuthenticated << m_curUserInfo->name();
    if (isAuthenticated) {
        if ((m_bRecvPrompt && !m_bHasUnacknowledgedMsg) || m_bDirectLogin) {
            m_bDirectLogin = false;

            // 认证完成后清空密码框
            m_passwordEdit->readOnly(false);
            m_passwordEdit->clear();
            clearMessage();
            m_failMap.remove(m_curUserInfo->uid());
            setLoginTypeTip("");
            setUkeyTypeTip("");
            authMode = UNKNOWN;
            m_uCurLoginOptType = LOGINOPT_TYPE_PASSWORD;

            Q_EMIT authSucceed(m_curUserInfo->name());
            m_userFailedTimesMap[m_curUserInfo->uid()] = 0;
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
                    if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg)
                        onPamShowMessage(tr("User name input error!"), PamAuth::MessageTypeError);
                } else {
                    if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg)
                        onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeError);
                }
            } else {
                if (!m_bHasUnacknowledgedMsg && !m_isLockingFlg) {
                    onPamShowMessage(tr("Authentication failure, Please try again"), PamAuth::MessageTypeError);
                }
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
    }
}

void AuthDialog::startAuth()
{
    if (!m_curUserInfo->name().isEmpty()) {
        qint64 lnDiffTime = qAbs(m_lastPamAuthTime.msecsTo(QDateTime::currentDateTime()));
        if (lnDiffTime >= 1000 || m_strLastPamAuthName != m_curUserInfo->name()) {
            m_lastPamAuthTime = QDateTime::currentDateTime();
            m_bRecvPrompt = false;
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
    }
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
    if (m_curUserInfo->uid() > -1 && userInfo->uid() == m_curUserInfo->uid()) {
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
        QString strMsgTip = m_messageLabel->text();
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        if (!strMsgTip.isEmpty()) {
            m_messageLabel->setText(strMsgTip);
        }
    } else {
        m_bNeedRetry = true;
        m_messageButton->setText(tr("Retry"));
    }
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

    // qDebug() << "user->pw_uid = " << user->pw_uid << "failed_count:" << failed_count << "time_left:" <<time_left
    // <<"deny:"<<deny<<"fail_time:"<< fail_time<<"unlock_time:" << unlock_time;
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
        return true;
    } else if (time_left > 0 && time_left < 60) { // 请多少秒后重试
        m_messageLabel->setTipText(tr("Please try again in %1 seconds.").arg(time_left % 60));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        m_isLockingFlg = true;
        return true;
    } else if (failed_count == 0xFFFF) { // 账号被永久锁定
        m_messageLabel->setTipText(tr("Account locked permanently."));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        m_isLockingFlg = true;
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
        }
        if (m_timerUserUnlock)
            m_timerUserUnlock->stop();
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

    if (!m_bioTimer) {
        m_bioTimer = new QTimer(this);
        connect(m_bioTimer, SIGNAL(timeout()), this, SLOT(pamBioSuccess()));
    }
    m_bioTimer->start(uTimeout);
}

void AuthDialog::waitBiometricServiceStatus()
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~` get biometric status";
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

    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   get activeState";
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
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ times = " << times;
}

void AuthDialog::performBiometricAuth()
{
    if (!m_biometricProxy) {
        waitBiometricServiceStatus();
        m_biometricProxy = new BiometricHelper(this);
        maxFailedTimes = m_uniauthService->getMaxFailedTimes();
    }

    // 服务没启动，或者打开DBus连接出错
    if (!m_biometricProxy->isValid()) {
        qWarning() << "An error occurs when connect to the biometric DBus";
        // skipBiometricAuth();
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
    initBiometricWidget();

    // 没有可用设备，不启用生物识别认证
    if (m_loginOpts->getLoginOptCount() < 1) {
        qWarning() << "No available devices";
        // skipBiometricAuth();
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
        // skipBiometricAuth();
        return;
    }
    // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
    if (!m_deviceInfo) {
        m_deviceInfo = m_loginOpts->findDeviceByName(m_deviceName);
        if (!m_deviceInfo) {
            qDebug() << "Device Not found: " << m_deviceName;
            // skipBiometricAuth();
            return;
        }
    }

    // 已经错误超过3次
    if (m_failMap.contains(m_curUserInfo->uid())
        && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
        if (m_deviceInfo->deviceType == UniT_Remote) {
            setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
        } else {
            setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
            if (m_deviceInfo->deviceType == BioT_Face) {
                QImage imgFailed;
                setFaceImg(imgFailed, 1);
            }
            stopBioWaiting(true);
        }
        m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);
        // skipBiometricAuth();
        return;
    }

    if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_loginOpts->setSelectedPassword();
        // skipBiometricAuth();
        return;
    }

    switchLoginOptType(m_loginOpts->convertDeviceType(m_deviceInfo->deviceType));
    // 自行校验生物设备，pam greeter校验密码
    startBioAuth();
    // skipBiometricAuth();
}

void AuthDialog::skipBiometricAuth()
{
    Q_EMIT m_modelLockDialog->pamRespond(BIOMETRIC_IGNORE);
}

void AuthDialog::initBiometricWidget()
{
    if (m_loginOpts) {
        m_loginOpts->setUser(m_curUserInfo->uid());
    } else {
        QHBoxLayout *optsLayout = new QHBoxLayout(m_widgetLoginOpts);
        optsLayout->setContentsMargins(0, 0, 0, 0);
        m_loginOpts
            = new LoginOptionsWidget(m_biometricProxy, m_curUserInfo->uid(), m_uniauthService, m_widgetLoginOpts);
        optsLayout->addWidget(m_loginOpts);
        connect(m_loginOpts, &LoginOptionsWidget::authComplete, this, &AuthDialog::onBiometricAuthComplete);
        connect(m_loginOpts, &LoginOptionsWidget::optionSelected, this, &AuthDialog::onDeviceChanged);
        connect(m_loginOpts, &LoginOptionsWidget::updateImage, this, &AuthDialog::onLoginOptImage);
        connect(m_loginOpts, &LoginOptionsWidget::setLoadingImage, this, &AuthDialog::onLoadingImage);
        connect(m_loginOpts, &LoginOptionsWidget::notifyOptionsChange, this, &AuthDialog::onLoginOptsCount);
        connect(m_loginOpts, &LoginOptionsWidget::updateAuthMsg, this, &AuthDialog::setLoginMsg);
        m_loginOpts->setUser(m_curUserInfo->uid());
        m_loginOpts->installEventFilter(this);
    }

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

    setBiometricWidgetGeometry();
}

void AuthDialog::setBiometricWidgetGeometry()
{
    // 生物识别
    if (m_loginOpts) {
        m_widgetLoginOpts->setFixedSize(width(), 98);
        qDebug() << "LoginOptGeometry:" << m_widgetLoginOpts->geometry() << "," << geometry()
                 << ",isHidden:" << m_widgetLoginOpts->isHidden();
    }
}

void AuthDialog::onLoginOptsCount(unsigned uCount)
{
    qDebug() << "----------------------onLoginOptsCount Count:" << uCount;
    if (!m_loginOpts)
        return;
    if (uCount > 0) {
        setBiometricWidgetGeometry();
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
        initBiometricWidget();

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
            return;
        }
        // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
        if (!m_deviceInfo) {
            m_deviceInfo = m_loginOpts->findDeviceByName(m_deviceName);
            if (!m_deviceInfo) {
                qDebug() << "Device Not found: " << m_deviceName;
                return;
            }
        }

        // 已经错误超过3次
        if (m_failMap.contains(m_curUserInfo->uid())
            && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
            if (m_deviceInfo->deviceType == UniT_Remote) {
                setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                    .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
            } else {
                setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
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
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    m_labelFace->setAlignment(Qt::AlignCenter);
    if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFace->setPixmap(m_waitingPixmap);
        m_labelQRCodeTip->show();
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
        m_labelQRCodeTip->show();
    }
    w_timer->start();
}

void AuthDialog::onDeviceChanged(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo)
{
    isLoadingUkey = false;
    if (uCurLoginOptType == LOGINOPT_TYPE_PASSWORD) {
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
            m_ukeyPasswordEdit->stopWaiting();
            m_ukeyPasswordEdit->clearText();
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
                        setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        QImage nullImage;
                        setQRCode(nullImage);
                    } else if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
                        setUkeyTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                           .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                    } else {
                        setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                            .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                        stopBioWaiting(true);
                    }
                    if (m_loginOpts)
                        m_loginOpts->setDeviceDisable(m_deviceInfo->id, true);

                    if (m_ukeyPasswdWidget && m_ukeyPasswdWidget->isVisible()) {
                        m_ukeyPasswordEdit->readOnly(true);
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
                setQRCodeMsg(tr("Abnormal network"));
                startBioAuth(10000);
            } else if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE && nStatus == 1) {
                m_fTimeoutTimes += 1;
                if (m_fTimeoutTimes == m_uniauthService->getFTimeoutTimes()) {
                    m_fRetryButton->show();
                    m_labelFace->hide();
                    setLoginTypeTip(
                        tr("Face recognition waiting time out, please click refresh or enter the password to unlock."));
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
    QString textTip = text;
    if (!textTip.isEmpty()) {
        //        QFontMetrics font(m_ukeyMessageLabel->font());
        //        QString textTip = font.elidedText(textTip, Qt::ElideRight, m_messageLabel->width()-8);
        m_ukeyMessageLabel->setTipText(textTip);
        m_ukeyMessageLabel->show();
    } else {
        m_ukeyMessageLabel->hide();
    }
}

void AuthDialog::setLoginTypeTip(QString strLoginTypeTip)
{
    m_strLoginTypeTip = strLoginTypeTip;
    if (!m_strLoginTypeTip.isEmpty()) {
        //        QFontMetrics font(m_labelLoginTypeTip->font());
        //        QString strDisplay = font.elidedText(m_strLoginTypeTip, Qt::ElideRight, m_messageLabel->width()-8);
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
                maxFailedTimes = m_uniauthService->getMaxFailedTimes();
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
            initBiometricWidget();

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
                if (!m_deviceInfo) {
                    qDebug() << "Device Not found: " << m_deviceName;
                    return;
                }
            }

            // 已经错误超过3次
            if (m_failMap.contains(m_curUserInfo->uid())
                && m_failMap[m_curUserInfo->uid()][m_deviceInfo->id] >= maxFailedTimes) {
                if (m_deviceInfo->deviceType == UniT_Remote) {
                    setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                        .arg(getDeviceType_lowerTr(m_deviceInfo->deviceType)));
                } else {
                    setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
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
    m_fRetryButton->hide();
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
                    m_labelBioWaiting->setPixmap(pixmap);
                } else {
                    pixmap = scaledSmoothPixmap(QPixmap(strPicName), WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
                    if (!pixmap.isNull()) {
                        QPixmapCache::insert(strPicName, pixmap);
                        m_labelBioWaiting->setPixmap(pixmap);
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
                m_labelBioWaiting->setPixmap(pixmap);
            } else {
                pixmap = scaledSmoothPixmap(QPixmap(strPicName), WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    m_labelBioWaiting->setPixmap(pixmap);
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
                m_labelBioWaiting->setPixmap(pixmap);
            } else {
                pixmap = scaledSmoothPixmap(QPixmap(strPicName), WIDGET_HEIGHT_HEADIMAGE, WIDGET_HEIGHT_HEADIMAGE);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    m_labelBioWaiting->setPixmap(pixmap);
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
