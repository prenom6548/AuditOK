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
#include "grubverifydialog.h"

#include <QRegExpValidator>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>

GrubVerify::GrubVerify(QWidget *parent) : KDialog(parent)
{
    this->setWindowIcon("ukui-control-center");
    setFixedSize(QSize(480, 294));
    setWindowTitle(tr("Grub verify"));
    initUI();
    setupConnect();
    refreshConfirmBtnStatus();
}


GrubVerify::~GrubVerify()
{

}

void GrubVerify::initUI()
{
    // 用户
    m_pUserLabel = new KLabel();
    m_pUserLabel->setText(tr("Username"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pUserLabel, "Boot", "GrubVerify", "");
    m_pUserLabel->setFixedSize(100,36);
    m_pUsernameLabel = new KLabel();
    m_pUsernameLabel->setText("root");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pUsernameLabel, "Boot", "GrubVerify", "");
    m_pUsernameLabel->setFixedSize(QSize(322, 36));

    userHorLayout = new QHBoxLayout;
    userHorLayout->setSpacing(8);
    userHorLayout->setContentsMargins(0, 0, 0, 0);
    userHorLayout->addWidget(m_pUserLabel);
    userHorLayout->addWidget(m_pUsernameLabel);

    //新密码
    m_pNewPwdLabel = new KLabel();
    m_pNewPwdLabel->setText(tr("Pwd"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNewPwdLabel, "Boot", "GrubVerify", "");
    m_pNewPwdLabel->setFixedWidth(100);
    m_pNewTipLabel = new KLabel();
    QFont ft;
    ft.setPixelSize(12);
    m_pNewTipLabel->setFont(ft);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNewTipLabel, "Boot", "GrubVerify", "");
    m_pNewTipLabel->setFixedSize(QSize(322, 24));
    m_pNewTipLabel->setFontColor(Qt::red);
    newTipHorLayout = new QHBoxLayout;
    newTipHorLayout->setContentsMargins(110, 0, 0, 0);
    newTipHorLayout->addStretch();
    newTipHorLayout->addWidget(m_pNewTipLabel);
    m_pNewPwdLineEdit = new KPasswordEdit(this);
    auto placehoderText = tr("Required");
    m_pNewPwdLineEdit->setPlaceholderText(placehoderText);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNewPwdLineEdit, "Boot", "GrubVerify", "");
    m_pNewPwdLineEdit->setAccessibleName(tr("Pwd"));
    QRegExp inputRegExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/?:;\"'|\{\}]+$");
    QRegExpValidator *inputLimits = new QRegExpValidator(inputRegExp, this);
    m_pNewPwdLineEdit->setValidator(inputLimits);
    m_pNewPwdLineEdit->setFixedSize(QSize(322, 36));
    m_pNewPwdLineEdit->setEchoMode(QLineEdit::Password);
    m_pNewPwdLineEdit->setTextMargins(0,0,30,0);
    m_pNewPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pNewPwdLineEdit->installEventFilter(this);

    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    newPwdHorLayout->addWidget(m_pNewPwdLabel);
    newPwdHorLayout->addWidget(m_pNewPwdLineEdit);

    //确认密码
    m_pSurePwdLabel = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSurePwdLabel, "Boot", "GrubVerify", "");
    m_pSurePwdLabel->setFixedWidth(100);
    m_pSurePwdLabel->setText(tr("Sure Pwd"));

    m_pSurePwdLineEdit = new KPasswordEdit(this);
    m_pSurePwdLineEdit->setPlaceholderText(placehoderText);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSurePwdLineEdit, "Boot", "GrubVerify", "");
    m_pSurePwdLineEdit->setAccessibleName(tr("Sure Pwd"));
    m_pSurePwdLineEdit->setValidator(inputLimits);
    m_pSurePwdLineEdit->setFixedSize(QSize(322, 36));
    m_pSurePwdLineEdit->setEchoMode(QLineEdit::Password);
    m_pSurePwdLineEdit->setTextMargins(0,0,30,0);
    m_pSurePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pSurePwdLineEdit->installEventFilter(this);

    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setContentsMargins(0, 0, 0, 0);
    surePwdHorLayout->addWidget(m_pSurePwdLabel);
    surePwdHorLayout->addWidget(m_pSurePwdLineEdit);

    m_pSureTipLabel = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSureTipLabel, "Boot", "GrubVerify", "");
    m_pSureTipLabel->setFont(ft);
    m_pSureTipLabel->setFixedSize(QSize(322, 30));
    m_pSureTipLabel->setFontColor(Qt::red);

    sureTipHorLayout = new QHBoxLayout;
    sureTipHorLayout->setSpacing(0);
    sureTipHorLayout->setContentsMargins(110, 0, 0, 0);
    sureTipHorLayout->addStretch();
    sureTipHorLayout->addWidget(m_pSureTipLabel);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 35, 0);
    contentVerLayout->addLayout(userHorLayout);
    contentVerLayout->addSpacing(24);
    contentVerLayout->addLayout(newPwdHorLayout);
    contentVerLayout->addLayout(newTipHorLayout);
    contentVerLayout->addLayout(surePwdHorLayout);
    contentVerLayout->addLayout(sureTipHorLayout);

    //底部“取消”、“确定”按钮
    m_pCancelBtn = new KPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCancelBtn, "Boot", "GrubVerify", "");
    m_pCancelBtn->setMinimumWidth(96);
    m_pCancelBtn->setText(tr("Cancel"));
    m_pCancelBtn->setProperty("useButtonPalette", true);
    m_pConfirmBtn = new KPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pConfirmBtn, "Boot", "GrubVerify", "");
    m_pConfirmBtn->setMinimumWidth(96);
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(0, 10, 0, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    mainWidget()->setLayout(mainVerLayout);
}

void GrubVerify::setupConnect()
{
    //通用的connect
    connect(m_pCancelBtn, &KPushButton::clicked, this, [=]{
        close();
    });

    connect(m_pNewPwdLineEdit, &KPasswordEdit::textChanged, this, [=](){
        checkPwdLegality();
        refreshConfirmBtnStatus();
    });

    connect(m_pSurePwdLineEdit, &KPasswordEdit::textChanged, this, [=](QString txt){
        if (!txt.isEmpty() && txt != m_pNewPwdLineEdit->text()){
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }

        m_pSureTipLabel->setText(surePwdTip);

        refreshConfirmBtnStatus();
    });

    connect(m_pConfirmBtn, &KPushButton::clicked, this, [=](){
        accept();
    });
}

QPushButton *GrubVerify::getConfirmBtn()
{
    return m_pConfirmBtn;
}

QString GrubVerify::getPwd()
{
    return m_pNewPwdLineEdit->text();
}

void GrubVerify::checkPwdLegality()
{
    if (m_pNewPwdLineEdit->text().isEmpty()) {
        newPwdTip = tr("pwd cannot be empty!");
    } else {
        newPwdTip = tr("");
    }

    //防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!m_pSurePwdLineEdit->text().isEmpty()){
        if (QString::compare(m_pNewPwdLineEdit->text(), m_pSurePwdLineEdit->text()) == 0){
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    //设置新密码的提示
    m_pNewTipLabel->setText(newPwdTip);
    m_pSureTipLabel->setText(surePwdTip);
}

void GrubVerify::refreshConfirmBtnStatus()
{
    if (m_pNewPwdLineEdit->text().isEmpty() || m_pSurePwdLineEdit->text().isEmpty() || \
        !newPwdTip.isEmpty() || !surePwdTip.isEmpty()) {
        m_pConfirmBtn->setEnabled(false);
    } else {
        m_pConfirmBtn->setEnabled(true);
    }
}

bool GrubVerify::eventFilter(QObject *target, QEvent *event)
{
//    if (target == m_pNewPwdLineEdit || target == m_pSurePwdLineEdit) {
//        if (event->type() == QEvent::KeyPress)
//        {
//            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//            if(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut))
//            {
//                qDebug() <<"Copy || Cut";
//                return true;
//            }
//        }
//    }

    if (event->type() == QEvent::FocusOut) {
       if (target == m_pNewPwdLineEdit) {
            if (m_pNewPwdLineEdit->text().isEmpty()) {
                newPwdTip = tr("pwd cannot be empty!");
                m_pNewTipLabel->setText(newPwdTip);
            }
        }
    }

    return KDialog::eventFilter(target, event);
    //继续传递该事件到被观察者，由其本身调用相应的事件。
}

void GrubVerify::keyPressEvent(QKeyEvent * event){
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

