#include "resetpwdwidget.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QLineEdit>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPalette>
#include "commonfunc.h"
#include "talkswidget.h"
#include "klabel.h"
#include "iconbutton.h"

#define     RESETPWD_WIDGET_HEIGHT_SPACING      (12*scale)
#define     FACELABEL_HEIGHT                    (154*scale)
#define     NAMELABEL_HEIGHT                    (40)


ResetPwdWidget::ResetPwdWidget(LockDialogModel *model, QString userName, QPixmap facePix, int uid, SecurityQuestionAnswer *securityQuestionAnswer, QWidget *parent) :
    QWidget(parent),
    m_modelLockDialog(model),
    m_pSecurityQuestionAnswer(securityQuestionAnswer),
    m_nameText(userName),
    m_facePixmap(facePix),
    m_uid(uid)
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

    mainLayout->addStretch(126);
    /* 头像 */
    QHBoxLayout *faceLayout = new QHBoxLayout();
    faceLayout->setAlignment(Qt::AlignHCenter);
    faceLayout->setContentsMargins(0, 0, 0, 0);
    const QString SheetStyle = QString("border-radius: %1px;  border:0px   solid white;").arg((int)(77*scale));
    m_pFaceLabel = new QLabel(this);
    m_pFaceLabel->setStyleSheet(SheetStyle);

    QPixmap facePixmap(m_facePixmap);
    facePixmap = scaledPixmap(facePixmap);
    facePixmap = facePixmap.scaled(154*scale, 154*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    facePixmap = PixmapToRound(facePixmap, 77*scale);
    m_pFaceLabel->setAlignment(Qt::AlignCenter);
    m_pFaceLabel->setPixmap(facePixmap);
    m_pFaceLabel->setFixedSize(154*scale,154*scale);
    faceLayout->addWidget(m_pFaceLabel);
    mainLayout->addLayout(faceLayout);

    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setAlignment(Qt::AlignHCenter);
    nameLayout->setContentsMargins(0, 24, 0, 16);
    m_pNameLabel = new KLabel(this);
    m_pNameLabel->setText(m_nameText);
    m_pNameLabel->setFontSize(24);
    m_pNameLabel->setFixedSize(width(), 40);
    m_pNameLabel->setAlignment(Qt::AlignCenter);
    nameLayout->addWidget(m_pNameLabel);
    mainLayout->addLayout(nameLayout);

    //  密保界面
    m_pConftScrollArea = new QScrollArea(this);
    m_pConftScrollArea->setAutoFillBackground(true);
    m_pConftScrollArea->setFocusPolicy(Qt::NoFocus);
    m_pConftScrollArea->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明
    m_pConftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //屏蔽水平滚动条
    m_pConftScrollArea->setObjectName("scrollArea");
    m_pConftScrollArea->setStyleSheet("#scrollArea{border:none}");
    m_pConftScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, Qt::transparent);
    m_pConftScrollArea->setPalette(pal);
    m_pConftScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_pQustionMainWidget = new QWidget(m_pConftScrollArea);
    m_pQustionMainWidget->setPalette(pal);
    QVBoxLayout *questionLayout = new QVBoxLayout(m_pQustionMainWidget);
    questionLayout->setAlignment(Qt::AlignHCenter);
    questionLayout->setContentsMargins(0, 0, 0, 0);
    questionLayout->setSpacing(0);
    for (int i = 0; i < m_listQuestionInfo.count(); ++i) {
        TalksWidget *questionWidget = new TalksWidget(m_modelLockDialog, m_pConftScrollArea, CONFIDENTIALEDIT);
        QuestionInfo question = m_listQuestionInfo.at(i);
        questionWidget->setQuestion(question.strQuestionText);
        connect(questionWidget, &TalksWidget::textChanged, [=](QString str){
            refreshConfirmBtnStatus();
        });
        connect(questionWidget, &TalksWidget::talkEditClicked, [=]{
            Q_EMIT resetEditClicked();
        });
        questionLayout->addWidget(questionWidget);
        m_listTalkWidget.append(questionWidget);
    }
    m_pConftScrollArea->setWidget(m_pQustionMainWidget);
    mainLayout->addWidget(m_pConftScrollArea);

    //  错误答案提示
    QVBoxLayout *messageLayout = new QVBoxLayout();
    messageLayout->setAlignment(Qt::AlignHCenter);
    messageLayout->setContentsMargins(0, 2, 0, 4);
    m_pMessageLabel = new KLabel(this);
    m_pMessageLabel->setFontSize(14);
    m_pMessageLabel->setMinimumHeight(30);
    m_pMessageLabel->setAlignment(Qt::AlignTop);
    messageLayout->addWidget(m_pMessageLabel);
    mainLayout->addLayout(messageLayout);

    //  确认密码
    m_pResetPwdScrollArea = new QScrollArea(this);
    m_pResetPwdScrollArea->setAutoFillBackground(true);
    m_pResetPwdScrollArea->setFocusPolicy(Qt::NoFocus);
    m_pResetPwdScrollArea->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明
    m_pResetPwdScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //屏蔽水平滚动条
    m_pResetPwdScrollArea->setObjectName("ResetPwdScrollArea");
    m_pResetPwdScrollArea->setStyleSheet("#ResetPwdScrollArea{border:none}");
    m_pResetPwdScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_pResetPwdScrollArea->setPalette(pal);
    m_pResetPwdScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_pSureWidget = new QWidget(m_pResetPwdScrollArea);
    m_pSureWidget->setObjectName("SureWidget");
    m_pSureWidget->setStyleSheet("#SureWidget{background-color:transparent; border:none}");
    QVBoxLayout *sureLayout = new QVBoxLayout(m_pSureWidget);
    sureLayout->setContentsMargins(0, 4, 0, 8);
    sureLayout->setAlignment(Qt::AlignHCenter);
    sureLayout->setSpacing(0);

    m_pNewPwdWidget = new TalksWidget(m_modelLockDialog, m_pSureWidget, RESETPWDEDIT);
    m_pNewPwdWidget->setQuestion(tr("Reset your password"));
    m_pNewPwdWidget->setPrompt(tr("Enter new password"));
    connect(m_pNewPwdWidget, &TalksWidget::textChanged, [=](QString str){
        checkPwdLegality();
        refreshConfirmBtnStatus();
    });
    connect(m_pNewPwdWidget, &TalksWidget::talkEditClicked, [=]{
        Q_EMIT resetEditClicked();
    });
    sureLayout->addWidget(m_pNewPwdWidget);
    sureLayout->addSpacing(3);

    m_pNewPwdTipLabel = new KLabel(m_pSureWidget);
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
    connect(m_pSureEdit, &IconEdit::textChanged, [=](QString str){
        if (!str.isEmpty() && str != m_pNewPwdWidget->getEditText()){
            m_surePwdTip = tr("Inconsistency with pwd");
        } else {
            m_surePwdTip = "";
        }
        m_pSureTipLabel->setText(m_surePwdTip);
        checkPwdLegality();
        refreshConfirmBtnStatus();
    });
    connect(m_pSureEdit, &IconEdit::clickedPassword, this, [=]{
        Q_EMIT resetEditClicked();
    });
    sureEditLayout->addWidget(m_pSureEdit);
    sureLayout->addLayout(sureEditLayout);
    sureLayout->addSpacing(3);

    m_pSureTipLabel = new KLabel(m_pSureWidget);
    m_pSureTipLabel->setFontSize(12);
    m_pSureTipLabel->setMinimumHeight(25);
    m_pSureTipLabel->setAlignment(Qt::AlignHCenter);
    sureLayout->addWidget(m_pSureTipLabel);

    m_pResetPwdScrollArea->setWidget(m_pSureWidget);
    mainLayout->addWidget(m_pResetPwdScrollArea);
    m_pResetPwdScrollArea->hide();

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignHCenter);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);
    m_pConfirmBtn = new IconButton(NORMALBUTTON, this);
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
        m_pConftScrollArea->show();
        m_pMessageLabel->show();
        m_pResetPwdScrollArea->hide();
        clearLineEditText();
        Q_EMIT backToAuthDialog(false);
    });
    buttonLayout->addWidget(m_pConfirmBtn);
    buttonLayout->addWidget(m_pCancelBtn);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch(317);
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
        qDebug() << "Reading pwquality configuration file failed: " << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
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
    foreach (QChar ch, word){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)) {
            return false;
        }
    }
    return true;
}

void ResetPwdWidget::checkPwdLegality()
{
    //判断用户是否输入了非法字符
    if (!isContainLegitimacyChar(m_pNewPwdWidget->getEditText())) {
        m_newPwdTip = tr("Contains illegal characters!");
    } else {
        if (m_enablePwdQuality) {
#ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
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
            //系统未开启pwdquality模块
            m_newPwdTip = "";
        }
    }

    //防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!m_pSureEdit->text().isEmpty()) {
        if (QString::compare(m_pNewPwdWidget->getEditText(), m_pSureEdit->text()) == 0) {
            m_surePwdTip = "";
        } else {
            m_surePwdTip = tr("Inconsistency with pwd");
        }
    }

    //设置新密码的提示
    if (!m_pNewPwdWidget->getEditText().isEmpty() || m_pSureEdit->hasFocus()) {
        m_pNewPwdTipLabel->setText(m_newPwdTip);
    } else if (m_pNewPwdWidget->getEditText().isEmpty()) {
        m_pNewPwdTipLabel->setText(m_newPwdTip);
    }

    m_pSureTipLabel->setText(m_surePwdTip);
}

void ResetPwdWidget::refreshConfirmBtnStatus()
{
    if (m_pConftScrollArea->isVisible()) {
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
    } else if (m_pResetPwdScrollArea->isVisible()) {
        if (m_pNewPwdWidget->getEditText().isEmpty() || m_pNewPwdWidget->getEditText() == tr("Enter new password") ||
                m_pSureEdit->text().isEmpty() || m_pSureEdit->text() == tr("Confirm your password") ||
                !m_newPwdTip.isEmpty() || !m_surePwdTip.isEmpty()) {
            m_pConfirmBtn->setEnabled(false);
        } else {
            m_pConfirmBtn->setEnabled(true);
        }
    }
}

void ResetPwdWidget::onConfirmBtnclicked()
{
    if (m_pConftScrollArea->isVisible()) {
        QList<AnswerInfo> listAnswerInfo;
        for (int i = m_listQuestionInfo.count() - 1; i >= 0; i--) {
            AnswerInfo answerInfo;
            answerInfo.nQuestionId = m_listQuestionInfo.at(i).nQuestionId;
            answerInfo.strAnswerText = m_pSecurityQuestionAnswer->EncodeSecurityInfo(m_listTalkWidget.at(i)->getEditText());
            listAnswerInfo.append(answerInfo);
        }

        m_verifyToken = m_pSecurityQuestionAnswer->VerifyUserSecurityAnswers(m_uid, listAnswerInfo);
        qDebug() << "Verify ret:" << m_verifyToken;
        if (!m_verifyToken.isEmpty()) {
            makeSurePwqualityEnabled();
            switchToResetPwdWdg();
        } else {
            verifyFailed();
        }
    } else {
        if (!m_verifyToken.isEmpty()) {
            int nRet = m_pSecurityQuestionAnswer->ChangePassword(m_uid, m_pSureEdit->text(), m_verifyToken);
            qDebug() << "ChangePwd ret:" << nRet;
            if (!nRet) {
                m_pConftScrollArea->show();
                m_pMessageLabel->show();
                m_pResetPwdScrollArea->hide();
                clearLineEditText();
                Q_EMIT backToAuthDialog(true);
            } else {
                qDebug() << "ChangePwd failed!!!" ;
            }
        }
    }
}

void ResetPwdWidget::updateResetPwdSize()
{
    QPixmap facePixmap(m_facePixmap);
    facePixmap = scaledPixmap(facePixmap);
    facePixmap = facePixmap.scaled(154*scale, 154*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    facePixmap = PixmapToRound(facePixmap, 77*scale);
    m_pFaceLabel->setAlignment(Qt::AlignCenter);
    m_pFaceLabel->setPixmap(facePixmap);
    m_pFaceLabel->setFixedSize(154*scale,154*scale);
    m_pNameLabel->setFixedSize(width(), 40);
    m_pSureWidget->setFixedWidth(width());
    int talkWidgetHeight = 0;
    for (int i = 0; i < m_listTalkWidget.count(); ++i) {
       m_listTalkWidget.at(i)->setFixedWidth(width());
       talkWidgetHeight += m_listTalkWidget.at(i)->getTalksWdgHeight();
    }
    m_pQustionMainWidget->setFixedSize(width(), talkWidgetHeight + 8 * 2);
    m_pSureWidget->setFixedSize(width(), m_pNewPwdWidget->getTalksWdgHeight() + m_pNewPwdTipLabel->height() + m_pSureEdit->height() + m_pSureTipLabel->height() + 30);
}

void ResetPwdWidget::switchToResetPwdWdg()
{
    m_pConftScrollArea->hide();
    m_pMessageLabel->hide();
    m_pResetPwdScrollArea->show();
    m_pNewPwdWidget->setFocus();
    clearLineEditText();
    refreshConfirmBtnStatus();
}

void ResetPwdWidget::switchToConfidentialDig()
{
    m_pConftScrollArea->show();
    m_pMessageLabel->show();
    m_pResetPwdScrollArea->hide();
    clearLineEditText();
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
    m_pNewPwdWidget->setEchoMode(QLineEdit::Password);
    m_pSureEdit->clearText();
    m_pSureEdit->setType(QLineEdit::Password);
    m_pMessageLabel->clear();
    m_pSureTipLabel->clear();
    m_pNewPwdTipLabel->clear();
}

void ResetPwdWidget::keyPressEvent(QKeyEvent * event)
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
    if (event->type() ==QEvent::MouseButtonPress && (obj == m_pConfirmBtn || obj == m_pCancelBtn)) {
        Q_EMIT resetEditClicked();
    }
    return QWidget::eventFilter(obj, event);
}

