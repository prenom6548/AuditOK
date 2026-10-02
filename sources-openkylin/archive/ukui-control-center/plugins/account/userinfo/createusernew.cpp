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
#include "createusernew.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QFrame>
#include <QPushButton>
#include <QDir>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QRegularExpression>

#include <kylin-chkname.h>

#include <kysdk/applications/accessinfohelper.h>

#include "ukcccommon.h"
using namespace ukcc;

#define USER_LENGTH 32
#define NICKNAME_LENGTH 32
#define DEFAULTFACECOMMUNITY "/usr/share/ukui/faces/01-default-community.png"
#define DEFAULTFACECOMMERCIAL "/usr/share/ukui/faces/01-default-commercial.png"
#define DEFAULTFACE (UkccCommon::isCommunity() || UkccCommon::isOpenkylin())?DEFAULTFACECOMMUNITY:DEFAULTFACECOMMERCIAL

CreateUserNew::CreateUserNew(QStringList allUsers, QWidget *parent) :
    KDialog(parent),
    _allNames(allUsers)
{
    //确认密码检测是否开启
    makeSurePwdNeedCheck();

    initUI();
    setConnect();

    refreshConfirmBtnStatus();
}

CreateUserNew::~CreateUserNew()
{
    if (m_pBubbleTips != nullptr) {
        m_pBubbleTips = nullptr;
        delete m_pBubbleTips;
    }
}

void CreateUserNew::initUI()
{
    setFixedSize(QSize(540, 562));
    setWindowTitle(tr("CreateUserNew"));
    setWindowIcon("ukui-control-center");

    KLabel *userTypeLabel = new KLabel;
    userTypeLabel->setFixedWidth(100);
    userTypeLabel->setText(tr("User Type"));

    KLabel *usernameLabel = new KLabel;
    usernameLabel->setFixedSize(100, 24);
    usernameLabel->setText(tr("UserName"));
    m_pUsernameTipLabel = new KLabel();
    QFont ft;
    ft.setPixelSize(12);
    m_pUsernameTipLabel->setFont(ft);
    m_pUsernameTipLabel->setFixedSize(QSize(340, 24));
    m_pUsernameTipLabel->setFontColor(Qt::red);

    KLabel *nicknameLabel = new KLabel;
    nicknameLabel->setFixedSize(100, 24);
    nicknameLabel->setText(tr("NickName"));
    m_pNicknameTipLabel = new KLabel();
    m_pNicknameTipLabel->setFont(ft);
    m_pNicknameTipLabel->setFixedSize(QSize(340, 24));
    m_pNicknameTipLabel->setFontColor(Qt::red);

    KLabel *hostnameLabel = new KLabel();
    hostnameLabel->setText(tr("HostName"));
    hostnameLabel->setFixedSize(100, 24);
    m_pHostnameTipLabel = new KLabel();
    m_pHostnameTipLabel->setFont(ft);
    m_pHostnameTipLabel->setFixedSize(QSize(340, 24));
    m_pHostnameTipLabel->setFontColor(Qt::red);

    KLabel *newPwdLabel = new KLabel;
    newPwdLabel->setFixedSize(100, 24);
    newPwdLabel->setText(tr("Pwd"));
    m_pNewpwdTipLabel = new KLabel();
    m_pNewpwdTipLabel->setFont(ft);
    m_pNewpwdTipLabel->setFixedSize(QSize(340, 24));
    m_pNewpwdTipLabel->setFontColor(Qt::red);

    KLabel *surePwdLabel = new KLabel;
    surePwdLabel->setFixedSize(100, 24);
    surePwdLabel->setText(tr("SurePwd"));
    m_pSurepwdTipLabel = new KLabel;
    m_pSurepwdTipLabel->setFont(ft);
    m_pSurepwdTipLabel->setFixedSize(340, 24);
    m_pSurepwdTipLabel->setFontColor(Qt::red);

    m_pUserTypeComboBox = new QComboBox;
    m_pUserTypeComboBox->setFixedWidth(296);
    m_pUserTypeComboBox->addItem(tr("Standard User"));
    m_pUserTypeComboBox->addItem(tr("Administrator"));
    m_pUserTypeComboBox->setAccessibleName(tr("User Type"));
    m_pUsernameLineEdit = new QLineEdit;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pUsernameLineEdit, "UserInfo", "", "");
    m_pUsernameLineEdit->setAccessibleName(tr("UserName"));
    m_pUsernameLineEdit->setFixedWidth(340);
    m_pUsernameLineEdit->setPlaceholderText(tr("Required"));
    m_pNicknameLineEdit = new QLineEdit;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNicknameLineEdit, "UserInfo", "", "");
    m_pNicknameLineEdit->setAccessibleName(tr("NickName"));
    m_pNicknameLineEdit->setFixedWidth(340);
    m_pNicknameLineEdit->setPlaceholderText(tr("Required"));
    m_pHostnameLineEdit = new QLineEdit;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pHostnameLineEdit, "UserInfo", "", "");
    m_pHostnameLineEdit->setAccessibleName(tr("HostName"));
    m_pHostnameLineEdit->setFixedWidth(340);
    m_pHostnameLineEdit->setPlaceholderText(tr("Required"));
    m_pNewPwdLineEdit = new QLineEdit;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNewPwdLineEdit, "UserInfo", "", "");
    m_pNewPwdLineEdit->setAccessibleName(tr("Pwd"));
    m_pNewPwdLineEdit->setFixedWidth(340);
    m_pNewPwdLineEdit->setPlaceholderText(tr("Required"));
    m_pNewPwdLineEdit->setEchoMode(QLineEdit::Password);
    m_pSurePwdLineEdit = new QLineEdit;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSurePwdLineEdit, "UserInfo", "", "");
    m_pSurePwdLineEdit->setAccessibleName(tr("SurePwd"));
    m_pSurePwdLineEdit->setFixedWidth(340);
    m_pSurePwdLineEdit->setPlaceholderText(tr("Verification"));
    m_pSurePwdLineEdit->setEchoMode(QLineEdit::Password);

    m_pUserTypeDetailBtn = new QPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pUserTypeDetailBtn, "UserInfo", "", "");
    m_pUserTypeDetailBtn->setProperty("useButtonPalette", true);
    m_pUserTypeDetailBtn->setFlat(true);
    m_pUserTypeDetailBtn->setFixedWidth(36);
    m_pUserTypeDetailBtn->setIcon(QIcon::fromTheme("help-about-symbolic"));
    m_pUserTypeDetailBtn->installEventFilter(this);
    if (m_pBubbleTips == nullptr) {
        m_pBubbleTips = new KBubbleWidget(this);
        m_pBubbleTips->setTailPosition(TailDirection::TopDirection, TailLocation::RightLocation);
        m_pBubbleTips->setTailSize(QSize(24,14));
        m_pBubbleTips->setBorderRadius(6);
        m_pBubbleTips->setWindowFlag(Qt::ToolTip);
        m_pBubbleTips->setFixedWidth(380);
        QVBoxLayout* layout = new QVBoxLayout(m_pBubbleTips);
        layout->setContentsMargins(16, 16, 16, 16);
        m_pBubbleTips->setLayout(layout);
        KLabel* adminLabel = new KLabel(m_pBubbleTips);
        adminLabel->setWordWrap(true);
        adminLabel->setText(tr("Administrator:Administrators can change any system configuration, including installing and upgrading software."));
        adminLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        adminLabel->setFixedWidth(348);

        KLabel* standardLabel = new KLabel(m_pBubbleTips);
        standardLabel->setWordWrap(true);
        standardLabel->setText(tr("Standard User:Standard users can use the vast majority of software, and can change system configurations that do not affect other users."));
        standardLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        standardLabel->setFixedWidth(348);

        layout->addWidget(adminLabel);
        layout->addSpacing(8);
        layout->addWidget(standardLabel);
        m_pBubbleTips->hide();
        layout->invalidate();
        layout->activate();
        adminLabel->adjustSize();
        standardLabel->adjustSize();
        m_pBubbleTips->adjustSize();
    }
    // 账户类型
    QHBoxLayout *m_pUserTypeHLayout = new QHBoxLayout;
    m_pUserTypeHLayout->setSpacing(8);
    m_pUserTypeHLayout->setContentsMargins(0, 0, 0, 0);
    m_pUserTypeHLayout->addWidget(userTypeLabel);
    m_pUserTypeHLayout->addStretch();
    m_pUserTypeHLayout->addWidget(m_pUserTypeComboBox);
    m_pUserTypeHLayout->addWidget(m_pUserTypeDetailBtn);

    //用户名
    QHBoxLayout *usernameHorLayout = new QHBoxLayout;
    usernameHorLayout->setSpacing(8);
    usernameHorLayout->setContentsMargins(0, 0, 0, 0);
    usernameHorLayout->addWidget(usernameLabel);
    usernameHorLayout->addStretch();
    usernameHorLayout->addWidget(m_pUsernameLineEdit);
    QHBoxLayout *usernameTipHorLayout = new QHBoxLayout;
    usernameTipHorLayout->setSpacing(0);
    usernameTipHorLayout->setMargin(0);
    usernameTipHorLayout->addStretch();
    usernameTipHorLayout->addWidget(m_pUsernameTipLabel);

    //用户昵称
    QHBoxLayout *nicknameHorLayout = new QHBoxLayout;
    nicknameHorLayout->setSpacing(8);
    nicknameHorLayout->setMargin(0);
    nicknameHorLayout->addWidget(nicknameLabel);
    nicknameHorLayout->addStretch();
    nicknameHorLayout->addWidget(m_pNicknameLineEdit);
    QHBoxLayout *nicknameTipHorLayout = new QHBoxLayout;
    nicknameTipHorLayout->setSpacing(0);
    nicknameTipHorLayout->setMargin(0);
    nicknameTipHorLayout->addStretch();
    nicknameTipHorLayout->addWidget(m_pNicknameTipLabel);

    // 计算机名
    QHBoxLayout *hostnameHorLayout = new QHBoxLayout;
    hostnameHorLayout->setSpacing(8);
    hostnameHorLayout->setMargin(0);
    hostnameHorLayout->addWidget(hostnameLabel);
    hostnameHorLayout->addStretch();
    hostnameHorLayout->addWidget(m_pHostnameLineEdit);
    QHBoxLayout *hostnameTipHorLayout = new QHBoxLayout;
    hostnameTipHorLayout->setSpacing(0);
    hostnameTipHorLayout->setMargin(0);
    hostnameTipHorLayout->addStretch();
    hostnameTipHorLayout->addWidget(m_pHostnameTipLabel);

    //密码
    QHBoxLayout *newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setMargin(0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addStretch();
    newPwdHorLayout->addWidget(m_pNewPwdLineEdit);
    QHBoxLayout *newPwdTipHorLayout = new QHBoxLayout;
    newPwdTipHorLayout->setSpacing(0);
    newPwdTipHorLayout->setMargin(0);
    newPwdTipHorLayout->addStretch();
    newPwdTipHorLayout->addWidget(m_pNewpwdTipLabel);

    //确认密码
    QHBoxLayout *surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setMargin(0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addStretch();
    surePwdHorLayout->addWidget(m_pSurePwdLineEdit);

    //提示信息
    QHBoxLayout *tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setMargin(0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(m_pSurepwdTipLabel);

    // 密码提示
    KLabel *passwordPromptLabel = new KLabel();
    passwordPromptLabel->setFixedSize(100, 24);
    passwordPromptLabel->setText(tr("Password Prompt"));

    m_pPasswordPromptLineedit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPasswordPromptLineedit, "UserInfo", "", "");
    m_pPasswordPromptLineedit->setFixedWidth(340);
    m_pPasswordPromptLineedit->setPlaceholderText(tr("Optional(Recommended)"));
    m_pPasswordPromptLineedit->setTextMargins(0,0,30,0);
    m_pPasswordPromptLineedit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pPasswordPromptLineedit->installEventFilter(this);
    m_pPasswordPromptLineedit->setAccessibleName(tr("Password Prompt"));

    QHBoxLayout *pwdPromptHLayout = new QHBoxLayout;
    pwdPromptHLayout->setSpacing(8);
    pwdPromptHLayout->setContentsMargins(0, 0, 0, 0);
    pwdPromptHLayout->addWidget(passwordPromptLabel);
    pwdPromptHLayout->addStretch();
    pwdPromptHLayout->addWidget(m_pPasswordPromptLineedit);
    m_pPasswordPromptTips = new KLabel();
    m_pPasswordPromptTips->setFont(ft);
    m_pPasswordPromptTips->setFixedSize(340, 24);
    m_pPasswordPromptTips->setFontColor(Qt::red);
    QHBoxLayout *passwordPromptTipsHLayout = new QHBoxLayout;
    passwordPromptTipsHLayout->setSpacing(0);
    passwordPromptTipsHLayout->addStretch();
    passwordPromptTipsHLayout->addWidget(m_pPasswordPromptTips);

    QVBoxLayout *pwdPromptWithTipVerLayout = new QVBoxLayout;
    pwdPromptWithTipVerLayout->setSpacing(0);
    pwdPromptWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    pwdPromptWithTipVerLayout->addLayout(pwdPromptHLayout);
    pwdPromptWithTipVerLayout->addLayout(passwordPromptTipsHLayout);

    // 安全问题
    m_pSecurityQusetionChecbox = new QCheckBox();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSecurityQusetionChecbox, "UserInfo", "", "");
    KLabel *securityQusetionLabel = new KLabel();
    securityQusetionLabel->setText(tr("Set up password security questions"));
    QHBoxLayout *securityQusetionHLayout = new QHBoxLayout();
    securityQusetionHLayout->setContentsMargins(0, 0, 0, 0);
    securityQusetionHLayout->addWidget(m_pSecurityQusetionChecbox);
    securityQusetionHLayout->addWidget(securityQusetionLabel);
    securityQusetionHLayout->addStretch();
    m_pSecurityQusetionChecbox->setAccessibleName(tr("Set up password security questions"));

    m_pSecurityQusetionTitleLabel = new KLabel();
    m_pSecurityQusetionTitleLabel->setText(tr("Setting security questions to reset password when forget it, "
                                  "please remember answer."));
    m_pSecurityQusetionTitleLabel->setFixedWidth(470);

    m_pSecurityQusetionVLayout = new QVBoxLayout;
    m_pSecurityQusetionVLayout->setContentsMargins(0, 16, 0, 0);
    m_pSecurityQusetionVLayout->setSpacing(24);
    addSecurityQuestionItem(SECURITY_QUESTION_COUNT, true);
    m_pSecurityQuestionWidget = new QWidget();
    m_pSecurityQuestionWidget->setLayout(m_pSecurityQusetionVLayout);

    m_pCancelBtn = new KPushButton;
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton;
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);

    QHBoxLayout *bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setContentsMargins(0, 0, 24, 24);
    bottomHorLayout->setSpacing(16);
    bottomHorLayout->addStretch();
    bottomHorLayout->addWidget(m_pCancelBtn);
    bottomHorLayout->addWidget(m_pConfirmBtn);

    m_pScrollVLayout = new QVBoxLayout;
    m_pScrollVLayout->setContentsMargins(24, 0, 24, 16);
    m_pScrollVLayout->setSpacing(0);
    m_pScrollVLayout->addLayout(m_pUserTypeHLayout);
    m_pScrollVLayout->addSpacing(24);
    m_pScrollVLayout->addLayout(usernameHorLayout);
    m_pScrollVLayout->addLayout(usernameTipHorLayout);

    m_pScrollVLayout->addLayout(nicknameHorLayout);
    m_pScrollVLayout->addLayout(nicknameTipHorLayout);

//    mainVerLayout->addLayout(hostnameHorLayout);
//    mainVerLayout->addLayout(hostnameTipHorLayout);

    m_pScrollVLayout->addLayout(newPwdHorLayout);
    m_pScrollVLayout->addLayout(newPwdTipHorLayout);

    m_pScrollVLayout->addLayout(surePwdHorLayout);
    m_pScrollVLayout->addLayout(tipHorLayout);

    m_pScrollVLayout->addLayout(pwdPromptWithTipVerLayout);

    m_pScrollVLayout->addLayout(securityQusetionHLayout);
    m_pScrollVLayout->addWidget(m_pSecurityQusetionTitleLabel);
    m_pScrollVLayout->addWidget(m_pSecurityQuestionWidget);

    m_pUsernameLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pNicknameLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pNewPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pSurePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pUsernameLineEdit->installEventFilter(this);
    m_pNicknameLineEdit->installEventFilter(this);
    m_pNewPwdLineEdit->installEventFilter(this);
    m_pSurePwdLineEdit->installEventFilter(this);

    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollWidget =  new QWidget(m_pScrollArea);
    m_pScrollWidget->setLayout(m_pScrollVLayout);
    m_pScrollArea->setWidget(m_pScrollWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->addWidget(m_pScrollArea);
    mainVLayout->addSpacing(16);
    mainVLayout->addLayout(bottomHorLayout);
    mainWidget()->setLayout(mainVLayout);
    setSecurityQuestionVisiable(false);
}

void CreateUserNew::setSecurityQuestionVisiable(bool visible)
{
    m_pSecurityQusetionTitleLabel->setVisible(visible);
    m_pSecurityQuestionWidget->setVisible(visible);
    m_pScrollArea->verticalScrollBar()->setMaximum(m_pScrollWidget->height());
    if (visible) {
        m_pScrollVLayout->setContentsMargins(24, 28, 8, 8);
        //1：获取滚动条的总长度
        int nTotalValue = m_pScrollArea->verticalScrollBar()->height();
        //2：将滚动条的位置设置成三分之一处。
        int nNewLoaction = nTotalValue/3;
        m_pScrollArea->verticalScrollBar()->setValue(nNewLoaction);
    } else {
        m_pScrollVLayout->setContentsMargins(24, 28, 24, 8);
    }
}

void CreateUserNew::addSecurityQuestionItem(const int count, bool mutex)
{
    for (int i = 1; i <= count; i++) {
        KLabel *questionLabel = new KLabel();
        questionLabel->setText(tr("Security question%1").arg(i));
        questionLabel->setFixedWidth(LABELWIDTH);
        QComboBox *questionCombox = new QComboBox();
        KDK_EXTEND_ALL_INFO_FORMAT(questionCombox, "UserInfo", QString::number(i), "");
        QHBoxLayout *questionHlayout = new QHBoxLayout();
        questionHlayout->setContentsMargins(0, 0, 0, 0);
        questionHlayout->addWidget(questionLabel);
        questionHlayout->addSpacing(16);
        questionHlayout->addWidget(questionCombox);
        questionCombox->setAccessibleName(tr("Security question%1").arg(i));

        KLabel *answerLabel =  new KLabel();
        answerLabel->setText(tr("Answer"));
        answerLabel->setFixedWidth(LABELWIDTH);
        QLineEdit *answerLineEdit = new QLineEdit();
        KDK_EXTEND_ALL_INFO_FORMAT(answerLineEdit, "UserInfo", QString::number(i), "");
        answerLineEdit->setPlaceholderText(tr("Required"));
        QHBoxLayout *answerHlayout = new QHBoxLayout();
        answerHlayout->setContentsMargins(0, 0, 0, 0);
        answerHlayout->addWidget(answerLabel);
        answerHlayout->addSpacing(16);
        answerHlayout->addWidget(answerLineEdit);
        answerLineEdit->setAccessibleName(tr("Answer"));

        QVBoxLayout *itemVLayout = new QVBoxLayout();
        itemVLayout->setSpacing(8);
        itemVLayout->addLayout(questionHlayout);
        itemVLayout->addLayout(answerHlayout);

        m_pSecurityQusetionVLayout->addLayout(itemVLayout);

        m_questionComboxList.append(questionCombox);
        m_answerLineeditList.append(answerLineEdit);

        connect(answerLineEdit, &QLineEdit::textChanged, this, &CreateUserNew::refreshConfirmBtnStatus);

        if (mutex) {
            connect(questionCombox, &QComboBox::currentTextChanged, this, &CreateUserNew::setQustionCombox);
        }
    }
}

// 安全问题下拉框问题不可重复，已选择问题下拉框中置灰
void CreateUserNew::setQustionCombox()
{
    QStringList allComboxCurrentText;
    for (int i = 0; i < m_questionComboxList.count(); i++) {
        QComboBox *tempCombox = m_questionComboxList.at(i);
        // 获取所有combox当前选择的text
        allComboxCurrentText.append(tempCombox->currentText());
    }

    for (int j = 0; j < m_questionComboxList.count(); j++) {
        QComboBox *tempCombox = m_questionComboxList.at(j);
        QString currentText =  tempCombox->currentText();
        if (m_questionlist.contains(currentText)) {
            tempCombox->blockSignals(true);
            tempCombox->clear();
            tempCombox->addItems(m_questionlist);
            tempCombox->setCurrentText(currentText);
            tempCombox->blockSignals(false);
            // 将在当前combox的其他combox的当前项置灰
            foreach (QString text, allComboxCurrentText) {
                if (text != currentText) {
                    QVariant v(0);
                    tempCombox->setItemData(tempCombox->findText(text), v, Qt::UserRole - 1);
                }
            }
        }
    }
}

void CreateUserNew::initQustionCombox(const QStringList &questionlist)
{
    m_questionlist = questionlist;
    for (int i = 0; i < m_questionComboxList.count(); i++) {
        QComboBox *question = m_questionComboxList.at(i);
        question->blockSignals(true);
        question->addItems(questionlist);
        question->blockSignals(false);
        question->setCurrentIndex(i);
    }
}

void CreateUserNew::setConnect()
{
    UniAuthService *m_pUniauthBackendDbus = new UniAuthService();
    // 将以后所有DBus调用的超时设置为 milliseconds
    m_pUniauthBackendDbus->setTimeout(2147483647); // -1 为默认的25s超时
    // 获取安全问题列表；
    QList<QuestionInfo> listQuestionInfo = m_pUniauthBackendDbus->GetPresetSecurityQuestions(QLocale::system().name());
    QStringList questionList;
    foreach (auto question, listQuestionInfo) {
        questionList << question.strQuestionText;
    }
    initQustionCombox(questionList);

    connect(m_pSecurityQusetionChecbox, &QCheckBox::clicked, this, [=](bool checked){
        setSecurityQuestionVisiable(checked);
        refreshConfirmBtnStatus();
    });

    connect(m_pUsernameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if(!hasNickName) {
            m_pNicknameLineEdit->setText(txt);
            nameLegalityCheck2(txt);
        }

        if (m_pUsernameLineEdit->text().length() > USER_LENGTH) {
            m_pUsernameLineEdit->setText(oldName);
        } else {
            oldName = txt;
            nameLegalityCheck(txt);
        }
    });

    connect(m_pNicknameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (txt.startsWith(' ')) {
            txt.remove(QRegularExpression("^\\s+"));  // 移除前导空白
            m_pNicknameLineEdit->blockSignals(true);
            m_pNicknameLineEdit->setText(txt);
            m_pNicknameLineEdit->blockSignals(false);
        }
        if (m_pNicknameLineEdit->text().isEmpty()) {
            hasNickName = false;
        } else {
            hasNickName = true;
        }

        nameLegalityCheck2(txt);

    });

    connect(m_pNewPwdLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        pwdLegalityCheck(txt);
        passwordPromptChangedSlot();
    });

    connect(m_pSurePwdLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (!txt.isEmpty() && txt != m_pNewPwdLineEdit->text()){
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }
        m_pSurepwdTipLabel->setText(surePwdTip);

        refreshConfirmBtnStatus();
    });

    connect(m_pPasswordPromptLineedit, &QLineEdit::textChanged, this, &CreateUserNew::passwordPromptChangedSlot);

    connect(m_pCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(m_pConfirmBtn, &QPushButton::clicked, this, [=]{
        // 绑定安全问题需要用户id，先创建后绑定
        if (m_pSecurityQusetionChecbox->isChecked()) {
            QList<AnswerInfo> listAnswerInfo;
            for (int n = 0; n < SECURITY_QUESTION_COUNT; n++) {
                AnswerInfo answerInfo;
                QComboBox *questionCombox = m_questionComboxList.at(n);
                if (questionCombox != nullptr) {
                    answerInfo.nQuestionId = questionCombox->currentIndex();
                }
                QLineEdit *answerLineedit = m_answerLineeditList.at(n);
                if (answerLineedit != nullptr) {
                    answerInfo.strAnswerText = m_pUniauthBackendDbus->EncodeSecurityInfo(answerLineedit->text());
                }
                listAnswerInfo.append(answerInfo);
            }
            emit bindSecurityQuestion(m_pUsernameLineEdit->text(), listAnswerInfo);
        }

        // 绑定密码提示需要用户id，先创建后绑定
        if (!m_pPasswordPromptLineedit->text().isEmpty() && m_pPasswordPromptTips->text().isEmpty()) {
            emit bindPasswordTips(m_pPasswordPromptLineedit->text());
        }

        //底层创建用户存在延时，先隐藏掉创建用户界面

        emit confirmCreated(m_pUsernameLineEdit->text(), m_pNicknameLineEdit->text(), m_pUserTypeComboBox->currentIndex(), m_pNewPwdLineEdit->text());

        m_pUniauthBackendDbus->deleteLater();
        close();
    });
}

void CreateUserNew::passwordPromptChangedSlot()
{
   if (!m_pPasswordPromptLineedit->text().isEmpty() && m_pPasswordPromptLineedit->text().contains(m_pNewPwdLineEdit->text())) {
        m_pPasswordPromptTips->setText(tr("Your password cannot be included in the password prompt."));
   }  else {
        m_pPasswordPromptTips->setText("");
   }

   refreshConfirmBtnStatus();
}

void CreateUserNew::makeSurePwdNeedCheck()
{

#ifdef ENABLEPQ
    int ret;
    void *auxerror;
    char buf[255];

    settings = pwquality_default_settings();
    if (settings == NULL) {
        enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        enablePwdQuality = true;
    }

    ret = pwquality_read_config(settings, PWCONF, &auxerror);
    if (ret != 0){
        enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: " << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState()){
        enablePwdQuality = true;
    } else {
        enablePwdQuality = false;
    }

#else
    enablePwdQuality = false;
#endif

}

void CreateUserNew::refreshConfirmBtnStatus()
{
    if (m_pUsernameLineEdit->text().isEmpty() ||
            m_pNicknameLineEdit->text().isEmpty() ||
            m_pNewPwdLineEdit->text().isEmpty() ||
            m_pSurePwdLineEdit->text().isEmpty() ||
            !userNameTip.isEmpty() || !nickNameTip.isEmpty() ||
            !newPwdTip.isEmpty() || !surePwdTip.isEmpty() ||
            !m_pPasswordPromptTips->text().isEmpty()) {
        m_pConfirmBtn->setEnabled(false);
    } else {
        m_pConfirmBtn->setEnabled(true);
    }

    if (((m_pUsernameLineEdit->text().length() >= USER_LENGTH) || (m_pNicknameLineEdit->text().length() >= NICKNAME_LENGTH))
            && newPwdTip.isEmpty() && surePwdTip.isEmpty() && userNameTip.isEmpty() && nickNameTip.isEmpty()
            && m_pPasswordPromptTips->text().isEmpty()
            && !m_pUsernameLineEdit->text().isEmpty()
            && !m_pNicknameLineEdit->text().isEmpty()
            && !m_pNewPwdLineEdit->text().isEmpty()
            && !m_pSurePwdLineEdit->text().isEmpty()) {
        m_pConfirmBtn->setEnabled(true);
    }

    if (m_pSecurityQusetionChecbox->isChecked()) {
        foreach (QLineEdit *temp, m_answerLineeditList) {
            if (temp->text().isEmpty()) {
                m_pConfirmBtn->setEnabled(false);
            }
        }
    }
}

bool CreateUserNew::isHomeUserExists(QString username)
{
    if (username.isEmpty())
        return false;

    QStringList homeList;
    QDir dir("/home");
    if (dir.exists())
    {
        homeList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        if (homeList.contains(username))
            return true;
    }

    return false;
}

bool CreateUserNew::isGroupNameEixsts(QString username)
{
    QString cmd = QString("getent group %1").arg(username);
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
    if (!output.isEmpty()) {
        return true;
    }

    return false;
}


bool CreateUserNew::nameTraverse(QString username)
{
    QString::const_iterator cit = NULL;
    for (cit = username.cbegin(); cit < username.cend(); cit++){
        QString str = *cit;
        if (str.contains(QRegExp("[a-z]"))){
        } else if (str.contains(QRegExp("[0-9]"))){
        } else if (str.contains("_")){
        } else{
            return false;
        }
    }
    return true;
}

void CreateUserNew::nameLegalityCheck2(QString nickname)
{
    if (nickname.isEmpty()){
        nickNameTip = tr("NickName's length must between 1~%1 characters").arg(NICKNAME_LENGTH);
    } else if (_allNames.contains(nickname)){
        nickNameTip = tr("nickName already in use.");
    } else if(nickname.length() > NICKNAME_LENGTH) {
        m_pNicknameLineEdit->setText(oldNickName);
    } else {
        oldNickName = nickname;
        nickNameTip = tr("");
    }

    m_pNicknameTipLabel->setText(nickNameTip);

    refreshConfirmBtnStatus();

}

void CreateUserNew::nameLegalityCheck(QString username)
{
    int isValued = kylin_username_check(username.toLatin1().data(), 1);

    if (isValued != 0) {
        qDebug() <<"err_num:" <<  isValued << ";" << kylin_username_strerror(isValued);
        userNameTip = kylin_username_strerror(isValued);
    } else {
        userNameTip = "";
    }

    if (isHomeUserExists(username) && userNameTip.isEmpty()) {
        userNameTip = tr("Username's folder exists, change another one");
    }

    if (isGroupNameEixsts(username) && userNameTip.isEmpty()) {
        userNameTip = tr("Name corresponds to group already exists.");
    }

    if (!m_pNewPwdLineEdit->text().isEmpty()) {
        pwdLegalityCheck(m_pNewPwdLineEdit->text());
    }

    m_pUsernameTipLabel->setText(userNameTip);

    refreshConfirmBtnStatus();
}

bool CreateUserNew::checkCharLegitimacy(QString password)
{
    //密码不能包含非标准字符
    foreach (QChar ch, password){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)){
            return false;
        }
    }
    return true;
}

void CreateUserNew::pwdLegalityCheck(QString pwd)
{
    if (!checkCharLegitimacy(pwd)){
        newPwdTip = tr("Contains illegal characters!");
    } else {
        if (enablePwdQuality){
    #ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
            char buf[256];

            QByteArray ba = pwd.toLatin1();
            QByteArray ba1 = m_pUsernameLineEdit->text().toLatin1();

            ret = pwquality_check(settings, ba.data(), NULL, ba1.data(), &auxerror);
            if (ret < 0 && pwd.length() > 0){
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                newPwdTip = QString(msg);
            } else {
                newPwdTip = "";
            }
    #endif
        } else {
            newPwdTip = "";
        }
    }

    //防止先输入确认密码，再输入密码后surePwdTip无法刷新
    if (!m_pSurePwdLineEdit->text().isEmpty()){
        if (m_pNewPwdLineEdit->text() == m_pSurePwdLineEdit->text()) {
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    m_pNewpwdTipLabel->setText(newPwdTip);

    m_pSurepwdTipLabel->setText(surePwdTip);

    refreshConfirmBtnStatus();
}

void CreateUserNew::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}

bool CreateUserNew::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pUserTypeDetailBtn) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease
                || event->type() == QEvent::MouseButtonDblClick) {
            return true;
        }

        if (event->type() == QEvent::HoverEnter) {
            if (m_pBubbleTips != nullptr) {
                m_pBubbleTips->setParent(this);
                auto pos = m_pUserTypeDetailBtn->mapTo(this, QPoint(0,0));
                qDebug() << "move point" << m_pUserTypeDetailBtn->x() << m_pUserTypeDetailBtn->width() << m_pUserTypeDetailBtn->y() << m_pUserTypeDetailBtn->height() << m_pBubbleTips->tailSize();
                m_pBubbleTips->move(this->x() + pos.x() + m_pUserTypeDetailBtn->width()/2*3 - m_pBubbleTips->width() + m_pBubbleTips->tailSize().width() / 3,
                                    this->y() + pos.y() + m_pUserTypeDetailBtn->height() + 2);
                m_pBubbleTips->show();
                m_pBubbleTips->adjustSize();
            }
        } else if (event->type() == QEvent::HoverLeave) {
            if (m_pBubbleTips != nullptr) {
                m_pBubbleTips->hide();
            }
        }
    }

    if (event->type() == QEvent::FocusOut) {
        if (watched == m_pUsernameLineEdit) {
            if (m_pUsernameLineEdit->text().isEmpty()) {
                userNameTip = tr("Username's length must be between 1 and %1 characters").arg(USER_LENGTH);
                m_pUsernameTipLabel->setText(userNameTip);
            }
        } else if (watched == m_pNicknameLineEdit) {
            if (m_pNicknameLineEdit->text().isEmpty()) {
                nickNameTip = tr("NickName's length must between 1~%1 characters").arg(NICKNAME_LENGTH);
                m_pNicknameTipLabel->setText(nickNameTip);
            }
        } else if (watched == m_pNewPwdLineEdit) {
            if (m_pNewPwdLineEdit->text().isEmpty()) {
                newPwdTip = tr("New pwd cannot be empty!");
                m_pNewpwdTipLabel->setText(newPwdTip);
            }
        } else if (watched == m_pSurePwdLineEdit) {
            if (m_pSurePwdLineEdit->text().isEmpty()) {
                surePwdTip = tr("Sure pwd cannot be empty!");
                m_pSurepwdTipLabel->setText(surePwdTip);
            }
        }
    }

    //事件过滤如下
    //当接收到这些事件时，需要被过滤掉，所以返回true
    if (watched == m_pUsernameLineEdit  || watched == m_pNicknameLineEdit || watched == m_pNewPwdLineEdit || watched == m_pSurePwdLineEdit) {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut))
            {
                qDebug() <<"Copy || Cut";
                return true;
            }
        }
    }

    return KDialog::eventFilter(watched, event);
}
