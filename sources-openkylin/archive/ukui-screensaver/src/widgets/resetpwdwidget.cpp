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
#include "resetpwdwidget.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QLineEdit>
#include <QKeyEvent>
#include <QScrollBar>
#include <QRegularExpression>
#include <QPalette>
#include "commonfunc.h"
#include "talkswidget.h"
#include "customlabel.h"
#include "kyiconbutton.h"
#include "verifykyidwidget.h"
#include "kyrsac.h"

#define RESETPWD_WIDGET_HEIGHT_SPACING (12 * scale)
#define FACELABEL_HEIGHT (154 * scale)
#define NAMELABEL_HEIGHT (40)

//  用正则表达式在一段字符串中提取出数字
static QString extractContinuousNumbers(const QString &input)
{
    QRegularExpression re("\\d+");
    QRegularExpressionMatchIterator i = re.globalMatch(input);
    QString result;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        result += match.captured() + " ";
    }
    return result.trimmed();
}

ResetPwdWidget::ResetPwdWidget(
    LockDialogModel *model, QString userName, int uid, SecurityQuestionAnswer *securityQuestionAnswer, QWidget *parent)
    : QWidget(parent)
    , m_modelLockDialog(model)
    , m_pSecurityQuestionAnswer(securityQuestionAnswer)
    , m_nameText(userName)
    , m_uid(uid)
{
    m_pSecurityQuestionAnswer = new SecurityQuestionAnswer();
    m_listQuestionInfo = m_pSecurityQuestionAnswer->GetUserSecurityQuestions(m_uid, getenv("LANGUAGE"));
    initUI();
}

void ResetPwdWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setAlignment(Qt::AlignHCenter);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    m_pTabBar = new KTabBar(KTabBarStyle::SegmentLight, this);
    QPalette tabBarpal(palette());
    tabBarpal.setColor(QPalette::ButtonText, Qt::white);
    m_pTabBar->setPalette(tabBarpal);
    m_pTabBar->addTab(tr("Security Question"));
    m_pTabBar->addTab(tr("Kylin ID"));
    m_pTabBar->setFocusPolicy(Qt::NoFocus);
    m_pTabBar->setFixedSize(200, 36);

    m_pTitleLabel = new CustomLabel(this);
    m_pTitleLabel->setFontSize(18);
    m_pTitleLabel->setMinimumHeight(30);
    m_pTitleLabel->setAlignment(Qt::AlignTop);

    titleLayout->addWidget(m_pTabBar);
    titleLayout->addWidget(m_pTitleLabel);
    mainLayout->addLayout(titleLayout);

    QPalette pal(palette());
    pal.setColor(QPalette::Window, Qt::transparent);

    m_pQustionMainWidget = new QWidget(this);
    m_pQustionMainWidget->setPalette(pal);
    QVBoxLayout *questionLayout = new QVBoxLayout(m_pQustionMainWidget);
    questionLayout->setAlignment(Qt::AlignHCenter);
    questionLayout->setContentsMargins(0, 0, 0, 0);
    questionLayout->setSpacing(0);
    for (int i = 0; i < m_listQuestionInfo.count(); ++i) {
        TalksWidget *questionWidget = new TalksWidget(m_modelLockDialog, m_pQustionMainWidget, CONFIDENTIALEDIT);
        QuestionInfo question = m_listQuestionInfo.at(i);
        questionWidget->setQuestion(question.strQuestionText);
        connect(questionWidget, &TalksWidget::textChanged, [=](QString str) { refreshConfirmBtnStatus(); });
        connect(questionWidget, &TalksWidget::talkEditClicked, [=] { Q_EMIT resetEditClicked(); });
        questionLayout->addWidget(questionWidget);
        m_listTalkWidget.append(questionWidget);
    }
    mainLayout->addWidget(m_pQustionMainWidget);

    //  麒麟ID
    QHBoxLayout *verifyKyidLayout = new QHBoxLayout();
    verifyKyidLayout->setAlignment(Qt::AlignHCenter);
    verifyKyidLayout->setContentsMargins(0, 0, 0, 0);
    m_pVerifyKyidWidget = new VerifyKyidWidget(m_modelLockDialog, this);
    verifyKyidLayout->addWidget(m_pVerifyKyidWidget);
    mainLayout->addLayout(verifyKyidLayout);
    m_pVerifyKyidWidget->hide();
    connect(m_pVerifyKyidWidget, &VerifyKyidWidget::textChanged, [=](QString str) { refreshConfirmBtnStatus(); });
    connect(m_pVerifyKyidWidget, &VerifyKyidWidget::captchaBtnclicked, this, &ResetPwdWidget::onCaptchaBtnclicked);

    connect(m_pTabBar, &KTabBar::tabBarClicked, this, [=](int index) {
        if (index == 0) {
            m_pQustionMainWidget->show();
            m_pVerifyKyidWidget->hide();
            m_pMessageLabel->clear();
        } else if (index == 1) {
            m_pQustionMainWidget->hide();
            m_pVerifyKyidWidget->show();
            m_pVerifyKyidWidget->setFocus();
            m_pMessageLabel->clear();
        }
    });

    //  错误答案提示
    QVBoxLayout *messageLayout = new QVBoxLayout();
    messageLayout->setAlignment(Qt::AlignHCenter);
    messageLayout->setContentsMargins(0, 2, 0, 4);
    m_pMessageLabel = new CustomLabel(this);
    m_pMessageLabel->setFontSize(14);
    m_pMessageLabel->setMinimumHeight(30);
    m_pMessageLabel->setAlignment(Qt::AlignTop);
    messageLayout->addWidget(m_pMessageLabel);
    mainLayout->addLayout(messageLayout);

    //  确认密码
    m_pSureWidget = new QWidget(this);
    m_pSureWidget->setObjectName("SureWidget");
    m_pSureWidget->setStyleSheet("#SureWidget{background-color:transparent; border:none}");
    QVBoxLayout *sureLayout = new QVBoxLayout(m_pSureWidget);
    sureLayout->setContentsMargins(0, 4, 0, 8);
    sureLayout->setAlignment(Qt::AlignHCenter);
    sureLayout->setSpacing(0);

    m_pNewPwdWidget = new TalksWidget(m_modelLockDialog, m_pSureWidget, RESETPWDEDIT);
    m_pNewPwdWidget->setQuestion("");
    m_pNewPwdWidget->setPrompt(tr("Enter new password"));
    connect(m_pNewPwdWidget, &TalksWidget::textChanged, [=](QString str) {
        checkPwdLegality();
        refreshConfirmBtnStatus();
    });
    connect(m_pNewPwdWidget, &TalksWidget::talkEditClicked, [=] { Q_EMIT resetEditClicked(); });
    sureLayout->addWidget(m_pNewPwdWidget);
    sureLayout->addSpacing(3);

    m_pNewPwdTipLabel = new CustomLabel(m_pSureWidget);
    m_pNewPwdTipLabel->setFontSize(12);
    m_pNewPwdTipLabel->setMinimumHeight(25);
    m_pNewPwdTipLabel->setAlignment(Qt::AlignHCenter);
    sureLayout->addWidget(m_pNewPwdTipLabel);
    sureLayout->addSpacing(7);

    QVBoxLayout *sureEditLayout = new QVBoxLayout();
    sureEditLayout->setAlignment(Qt::AlignHCenter);
    sureEditLayout->setContentsMargins(0, 0, 0, 0);
    m_pSureEdit = new IconEdit(m_modelLockDialog, m_pSureWidget, RESETPWDEDIT);
    m_pSureEdit->setFixedSize(240, 40);
    QPalette palett(palette());
    palett.setColor(QPalette::Background, Qt::white);
    m_pSureEdit->setPalette(palett);
    m_pSureEdit->setPrompt(tr("Confirm your password"));
    connect(m_pSureEdit, &IconEdit::textChanged, [=](QString str) {
        if (!str.isEmpty() && str != m_pNewPwdWidget->getEditText()) {
            m_surePwdTip = tr("Inconsistency with pwd");
        } else {
            m_surePwdTip = "";
        }
        m_pSureTipLabel->setText(m_surePwdTip);
        checkPwdLegality();
        refreshConfirmBtnStatus();
    });
    connect(m_pSureEdit, &IconEdit::clickedPassword, this, [=] { Q_EMIT resetEditClicked(); });
    sureEditLayout->addWidget(m_pSureEdit);
    sureLayout->addLayout(sureEditLayout);
    sureLayout->addSpacing(3);

    m_pSureTipLabel = new CustomLabel(m_pSureWidget);
    m_pSureTipLabel->setFontSize(12);
    m_pSureTipLabel->setMinimumHeight(25);
    m_pSureTipLabel->setAlignment(Qt::AlignHCenter);
    sureLayout->addWidget(m_pSureTipLabel);
    mainLayout->addWidget(m_pSureWidget);
    m_pSureWidget->hide();

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignHCenter);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);
    m_pConfirmBtn = new KyIconButton(NORMALBUTTON, this);
    m_pConfirmBtn->setFocusPolicy(Qt::ClickFocus);
    m_pConfirmBtn->installEventFilter(this);
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setFixedSize(240, 40);
    connect(m_pConfirmBtn, &QPushButton::clicked, this, &ResetPwdWidget::onConfirmBtnclicked);

    m_pCancelBtn = new QPushButton(this);
    m_pCancelBtn->setFocusPolicy(Qt::NoFocus);
    m_pCancelBtn->installEventFilter(this);
    m_pCancelBtn->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.15);border-radius: 6px;color: white;}"
                                "QPushButton:hover{background: rgba(255, 255, 255, 0.20);border-radius: 6px;}"
                                "QPushButton:pressed {background: rgba(255, 255, 255, 0.4);border-radius: 6px;}");
    m_pCancelBtn->setText(tr("Cancel"));
    m_pCancelBtn->setFixedSize(240, 40);
    connect(m_pCancelBtn, &QPushButton::clicked, this, [&]() {
        setBindIdentity(m_nBindIdentity);
        clearLineEditText();
        m_pSureWidget->hide();
        Q_EMIT backToAuthDialog(false);
        if (m_nBindIdentity == ALLDEFINED) {
            m_pTabBar->setCurrentIndex(0);
        }
    });
    buttonLayout->addWidget(m_pConfirmBtn);
    buttonLayout->addWidget(m_pCancelBtn);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch();
}

void ResetPwdWidget::makeSurePwqualityEnabled()
{
#ifdef ENABLEPQ
    int ret;
    void *auxerror;
    char buf[255];

    m_pSettings = pwquality_default_settings();
    if (m_pSettings == NULL) {
        m_enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        m_enablePwdQuality = true;
    }

    ret = pwquality_read_config(m_pSettings, PWCONF, &auxerror);
    if (ret != 0) {
        m_enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: "
                 << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        m_enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState())
        m_enablePwdQuality = true;
    else
        m_enablePwdQuality = false;

#else
    m_enablePwdQuality = false;
#endif
}

bool ResetPwdWidget::isContainLegitimacyChar(QString word)
{
    foreach (QChar ch, word) {
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)) {
            return false;
        }
    }
    return true;
}

void ResetPwdWidget::checkPwdLegality()
{
    // 判断用户是否输入了非法字符
    if (!isContainLegitimacyChar(m_pNewPwdWidget->getEditText())) {
        m_newPwdTip = tr("Contains illegal characters!");
    } else {
        if (m_enablePwdQuality) {
#ifdef ENABLEPQ
            void *auxerror;
            int ret;
            const char *msg;
            char buf[256];
            QByteArray ba = m_pNewPwdWidget->getEditText().toLatin1();
            ret = pwquality_check(m_pSettings, ba.data(), NULL, m_nameText.toLatin1().data(), &auxerror);
            if (ret == PWQ_ERROR_SAME_PASSWORD) {
                ret = pwquality_check(m_pSettings, ba.data(), NULL, m_nameText.toLatin1().data(), &auxerror);
            }
            if (ret < 0 && m_pNewPwdWidget->getEditText().length() > 0) {
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                m_newPwdTip = QString(msg);
            } else {
                m_newPwdTip = "";
            }
#endif
        } else {
            // 系统未开启pwdquality模块
            m_newPwdTip = "";
        }
    }

    // 防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!m_pSureEdit->text().isEmpty()) {
        if (QString::compare(m_pNewPwdWidget->getEditText(), m_pSureEdit->text()) == 0) {
            m_surePwdTip = "";
        } else {
            m_surePwdTip = tr("Inconsistency with pwd");
        }
    }

    // 设置新密码的提示
    if (!m_pNewPwdWidget->getEditText().isEmpty() || m_pSureEdit->hasFocus()) {
        m_pNewPwdTipLabel->setText(m_newPwdTip);
    } else if (m_pNewPwdWidget->getEditText().isEmpty()) {
        m_pNewPwdTipLabel->setText(m_newPwdTip);
    }

    m_pSureTipLabel->setText(m_surePwdTip);
}

void ResetPwdWidget::refreshConfirmBtnStatus()
{
    if (m_pTabBar->currentIndex() == 0 && m_pQustionMainWidget->isVisible()) {
        bool editTextIsEmpty = false;
        for (int i = 0; i < m_listQuestionInfo.count(); ++i) {
            if (m_listTalkWidget.at(i)->getEditText().isEmpty()) {
                editTextIsEmpty = true;
                break;
            }
        }
        if (!editTextIsEmpty) {
            m_pConfirmBtn->setEnabled(true);
        } else {
            m_pConfirmBtn->setEnabled(false);
        }
    } else if ((m_pTabBar->currentIndex() == 1 || m_pTabBar->isHidden()) && m_pVerifyKyidWidget->isVisible()) {
        if (m_pVerifyKyidWidget->getKylinIdEditText().isEmpty() || m_pVerifyKyidWidget->getPwdEditText().isEmpty()
            || m_pVerifyKyidWidget->getCaptchaEditText().isEmpty()) {
            m_pConfirmBtn->setEnabled(false);
        } else {
            m_pConfirmBtn->setEnabled(true);
        }
    } else if (m_pSureWidget->isVisible()) {
        if (m_pNewPwdWidget->getEditText().isEmpty() || m_pNewPwdWidget->getEditText() == tr("Enter new password")
            || m_pSureEdit->text().isEmpty() || m_pSureEdit->text() == tr("Confirm your password")
            || !m_newPwdTip.isEmpty() || !m_surePwdTip.isEmpty()) {
            m_pConfirmBtn->setEnabled(false);
        } else {
            m_pConfirmBtn->setEnabled(true);
        }
    }
}

void ResetPwdWidget::onCaptchaBtnclicked()
{
    QDBusInterface interface(
        "org.kylinID.service", "/org/kylinID/path", "org.kylinID.interface", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("checkKyidAccounts", m_uid, m_pVerifyKyidWidget->getKylinIdEditText());
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0 && varResult.takeFirst().toBool()) { // 验证麒麟ID账户是否输入正确
        //  异步调用，防止阻塞主进程
        QDBusPendingCall call = interface.asyncCall(
            QStringLiteral("getCodeByKyidAccounts"), m_pVerifyKyidWidget->getKylinIdEditText()); //  获取验证码
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QStringList> reply = *watcher;
            if (!reply.isValid()) {
                qWarning() << "getCodeByKyidAccounts error: " << reply.error().message();
                m_pMessageLabel->setText(tr("The service is abnormal"));
            } else {
                QStringList resultList = reply.value();
                qDebug() << " getCodeByKyidAccounts reply: " << resultList;
                if (resultList.count() > 1) {
                    QString code = resultList.at(0);
                    if (code.toInt() == 200) { // 只有返回值为200时为认证成功
                        m_pVerifyKyidWidget->onCaptchaBtnclicked(true);
                        QString message = tr("The SMS verification code has been sent to the mobile phone with the "
                                             "tail number %1 (Valid for 5 minutes)")
                                              .arg(extractContinuousNumbers(resultList.at(1)));
                        Q_EMIT getCodeByKyidAccountsed(message);
                        m_pMessageLabel->clear();
                    } else {
                        qWarning() << "getCodeByKyidAccounts error: " << resultList;
                        m_pVerifyKyidWidget->onCaptchaBtnclicked(false);
                        m_pMessageLabel->setText(tr("Failed to get the verification code"));
                    }
                } else {
                    qWarning() << "getCodeByKyidAccounts error: " << resultList;
                    m_pVerifyKyidWidget->onCaptchaBtnclicked(false);
                    m_pMessageLabel->setText(tr("Failed to get the verification code"));
                }
            }
            watcher->deleteLater();
        });
    } else {
        qWarning() << " checkKyidAccounts error!!! ";
        m_pVerifyKyidWidget->onCaptchaBtnclicked(false);
        m_pMessageLabel->setText(tr("The account does not exist, please re-enter it"));
    }
}

void ResetPwdWidget::onConfirmBtnclicked()
{
    if (m_pTabBar->currentIndex() == 0 && m_pQustionMainWidget->isVisible()) {
        m_verifyToken = "";
        QList<AnswerInfo> listAnswerInfo;
        for (int i = m_listQuestionInfo.count() - 1; i >= 0; i--) {
            AnswerInfo answerInfo;
            answerInfo.nQuestionId = m_listQuestionInfo.at(i).nQuestionId;
            answerInfo.strAnswerText
                = m_pSecurityQuestionAnswer->EncodeSecurityInfo(m_listTalkWidget.at(i)->getEditText());
            listAnswerInfo.append(answerInfo);
        }
        m_verifyToken = m_pSecurityQuestionAnswer->VerifyUserSecurityAnswers(m_uid, listAnswerInfo);
        qDebug() << "Verify ret:" << m_verifyToken;
        if (!m_verifyToken.isEmpty()) {
            makeSurePwqualityEnabled();
            switchToResetPwdWdg();
            m_nCurrentState = SECURITYQUESTION;
        } else {
            verifyFailed();
        }
    } else if ((m_pTabBar->currentIndex() == 1 || m_pTabBar->isHidden()) && m_pVerifyKyidWidget->isVisible()) {
        m_verifyToken = "";
        QByteArray pwdByteArray = m_pVerifyKyidWidget->getPwdEditText().toUtf8();
        QByteArray pwd;
        QByteArray publickeyByteQrray = ("-----BEGIN PUBLIC KEY-----\n"
                                         "MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJlLIik423nKkNY/TN4BCsDqfuyR1+Wx\n"
                                         "yzpHZJrP4R2XChrzHluHkJXvQaEA8jgCp74Q1NY/5TIJQlyiXktEHm0CAwEAAQ==\n"
                                         "-----END PUBLIC KEY-----\n");
        KyRSAC rsac;
        rsac.encryptData(pwdByteArray, pwd, publickeyByteQrray);
        QStringList result = m_pSecurityQuestionAnswer->VerifyAccountsInfo(
            m_uid,
            m_pVerifyKyidWidget->getKylinIdEditText(),
            pwd.toBase64(),
            m_pVerifyKyidWidget->getCaptchaEditText());
        qDebug() << " VerifyAccountsInfo result = " << result;
        if (result.isEmpty()) {
            m_pMessageLabel->setText(tr("The service is abnormal"));
        } else if (result.count() == 2) {
            qDebug() << " error code = " << result.at(0) << " error = " << result.at(1);
            // 麒麟ID服务端修改，直接返回已翻译错误
            m_pMessageLabel->setText(result.at(1));
        } else if (result.count() == 3) {
            m_verifyToken = result.at(2);
        }
        if (!m_verifyToken.isEmpty()) {
            makeSurePwqualityEnabled();
            switchToResetPwdWdg();
            m_nCurrentState = KYLINID;
        }
    } else {
        if (!m_verifyToken.isEmpty()) {
            if (m_nCurrentState == SECURITYQUESTION) {
                int nRet = m_pSecurityQuestionAnswer->ChangePassword(m_uid, m_pSureEdit->text(), m_verifyToken);
                qDebug() << "SecurityQuestionAnswer ChangePwd ret:" << nRet;
                if (!nRet) {
                    m_pQustionMainWidget->show();
                    m_pMessageLabel->show();
                    m_pSureWidget->hide();
                    clearLineEditText();
                    if (m_nBindIdentity == ALLDEFINED) {
                        m_pTabBar->setCurrentIndex(0);
                    }
                    Q_EMIT backToAuthDialog(true);
                    m_pSecurityQuestionAnswer->SetUserPasswdHint(m_uid, "");
                    Q_EMIT hideKylinIDCodesTips();
                } else {
                    qInfo() << "SecurityQuestionAnswer ChangePwd failed!!!";
                }
            } else if (m_nCurrentState == KYLINID) {
                int nRet = m_pSecurityQuestionAnswer->ChangePassword2(m_uid, m_pSureEdit->text(), m_verifyToken);
                qDebug() << "KylinID ChangePwd ret:" << nRet;
                if (!nRet) {
                    m_pQustionMainWidget->show();
                    m_pMessageLabel->show();
                    m_pSureWidget->hide();
                    clearLineEditText();
                    if (m_nBindIdentity == ALLDEFINED) {
                        m_pTabBar->setCurrentIndex(0);
                    }
                    Q_EMIT backToAuthDialog(true);
                    m_pSecurityQuestionAnswer->SetUserPasswdHint(m_uid, "");
                    Q_EMIT hideKylinIDCodesTips();
                } else {
                    qInfo() << "KylinID ChangePwd failed!!!";
                }
            }
        }
    }
}

void ResetPwdWidget::updateResetPwdSize()
{
    m_pSureWidget->setFixedWidth(width());
    int talkWidgetHeight = 0;
    for (int i = 0; i < m_listTalkWidget.count(); ++i) {
        m_listTalkWidget.at(i)->setFixedWidth(width());
        talkWidgetHeight += m_listTalkWidget.at(i)->getTalksWdgHeight();
    }
    m_pQustionMainWidget->setFixedSize(width(), talkWidgetHeight + 8 * 2);
}

void ResetPwdWidget::switchToResetPwdWdg()
{
    m_pQustionMainWidget->hide();
    m_pSureWidget->show();
    m_pNewPwdWidget->setFocus();
    m_pVerifyKyidWidget->hide();
    m_pTabBar->hide();
    if (m_nBindIdentity == ALLDEFINED && m_pTabBar->currentIndex() == 1) {
        m_pTitleLabel->setText(tr("Verify Kylin ID Reset Password"));
        Q_EMIT hideKylinIDCodesTips();
    } else if (m_nBindIdentity == ALLDEFINED && m_pTabBar->currentIndex() == 0) {
        m_pTitleLabel->setText(tr("Answer the security questions and reset your password"));
    }
    m_pTitleLabel->show();
    clearLineEditText();
    refreshConfirmBtnStatus();
}

void ResetPwdWidget::switchToConfidentialDig()
{
    m_pQustionMainWidget->show();
    m_pMessageLabel->show();
    m_pSureWidget->hide();
    clearLineEditText();
}

void ResetPwdWidget::setBindIdentity(int type)
{
    m_nBindIdentity = type;
    if (m_nBindIdentity == SECURITYQUESTION) {
        m_pTitleLabel->setText(tr("Answer the security questions and reset your password"));
        m_pTitleLabel->show();
        m_pQustionMainWidget->show();
        m_pVerifyKyidWidget->hide();
        m_pTabBar->hide();
    } else if (m_nBindIdentity == KYLINID) {
        m_pTitleLabel->setText(tr("Verify Kylin ID Reset Password"));
        m_pTitleLabel->show();
        m_pQustionMainWidget->hide();
        m_pVerifyKyidWidget->show();
        m_pVerifyKyidWidget->setFocus();
        m_pTabBar->hide();
    } else if (m_nBindIdentity == ALLDEFINED) {
        m_pTitleLabel->setText(tr(""));
        m_pTitleLabel->hide();
        m_pQustionMainWidget->show();
        m_pVerifyKyidWidget->hide();
        m_pTabBar->show();
        m_pTabBar->setCurrentIndex(0);
    }
}

void ResetPwdWidget::updateFontSize(double lfFontSize)
{
    if (m_pNewPwdWidget) {
        m_pNewPwdWidget->updateFontSize(lfFontSize);
    }

    for (int i = 0; i < m_listTalkWidget.count(); ++i) {
        m_listTalkWidget.at(i)->updateFontSize(lfFontSize);
    }
}

void ResetPwdWidget::verifyFailed()
{
    m_pMessageLabel->setText(tr("One or more wrong answers"));
}

void ResetPwdWidget::setFocusToEdit()
{
    if (m_listTalkWidget.count() > 0) {
        m_listTalkWidget.at(0)->setFocus();
    }
    refreshConfirmBtnStatus();
}

void ResetPwdWidget::clearLineEditText()
{
    for (int i = 0; i < m_listTalkWidget.count(); ++i) {
        m_listTalkWidget.at(i)->clearText();
    }
    m_pNewPwdWidget->clearText();
    m_pSureEdit->clearText();
    m_pMessageLabel->clear();
    m_pSureTipLabel->clear();
    m_pNewPwdTipLabel->clear();
    m_pVerifyKyidWidget->clearText();
}

void ResetPwdWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Enter:
            if (m_pConfirmBtn->isEnabled()) {
                m_pConfirmBtn->clicked();
                m_pConfirmBtn->setFocus();
            }
            break;
        case Qt::Key_Return:
            if (m_pConfirmBtn->isEnabled()) {
                m_pConfirmBtn->clicked();
                m_pConfirmBtn->setFocus();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

bool ResetPwdWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && (obj == m_pConfirmBtn || obj == m_pCancelBtn)) {
        Q_EMIT resetEditClicked();
    }
    return QWidget::eventFilter(obj, event);
}
