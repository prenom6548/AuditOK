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
#include "changeusernickname.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

#include <QDBusInterface>
#include <QDBusReply>
#include <QRegularExpression>

#include <glib.h>

#define NICKNAME_LENGTH 32

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserNickname::ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent) :
    KDialog(parent),
    namesIsExists(ns),
    realname(nn)
{
    this->setFixedSize(QSize(480, 254));
    this->setWindowTitle(tr("Set Nickname"));
    this->setWindowIcon("ukui-control-center");

    m_pAccountServiceIface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus(), this);


    initUI();
    setConnect();
}

ChangeUserNickname::~ChangeUserNickname()
{
}

void ChangeUserNickname::initUI()
{
    //用户名
    m_pUserNameLabel = new KLabel();
    m_pUserNameLabel->setFixedWidth(100);
    m_pUserNameLabel->setText(tr("UserName"));

    QLineEdit *userNameLineEdit = new QLineEdit();
    userNameLineEdit->setFixedSize(QSize(300, 36));
    userNameLineEdit->setPlaceholderText(QString(g_get_user_name()));
    userNameLineEdit->setReadOnly(true);
    userNameLineEdit->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *userNameHorLayout = new QHBoxLayout;
    userNameHorLayout->setSpacing(25);
    userNameHorLayout->setContentsMargins(0, 0, 0, 0);
    userNameHorLayout->addWidget(m_pUserNameLabel);
    userNameHorLayout->addWidget(userNameLineEdit);

    //用户昵称
    m_pNickNameLabel = new KLabel();
    m_pNickNameLabel->setFixedWidth(100);
    m_pNickNameLabel->setText(tr("NickName"));

    m_pTipLabel = new KLabel();
    QFont ft;
    ft.setPixelSize(12);
    m_pTipLabel->setFont(ft);
    m_pTipLabel->setFixedSize(QSize(300, 36));
    m_pTipLabel->setFontColor(Qt::red);

    m_pNickNameLineEdit = new QLineEdit(this);
    m_pNickNameLineEdit->setFocus();
    m_pNickNameLineEdit->setFixedSize(QSize(300, 36));
    m_pNickNameLineEdit->setText(realname);
    m_pNickNameLineEdit->setAccessibleName(tr("NickName"));

    QHBoxLayout *nickNameHorLayout = new QHBoxLayout;
    nickNameHorLayout->setSpacing(25);
    nickNameHorLayout->setContentsMargins(0, 0, 0, 0);
    nickNameHorLayout->addWidget(m_pNickNameLabel);
    nickNameHorLayout->addWidget(m_pNickNameLineEdit);

    QHBoxLayout *tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(m_pTipLabel);

    QVBoxLayout *nickNameWithTipVerLayout = new QVBoxLayout;
    nickNameWithTipVerLayout->setSpacing(4);
    nickNameWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    nickNameWithTipVerLayout->addLayout(nickNameHorLayout);
    nickNameWithTipVerLayout->addLayout(tipHorLayout);

    //中部输入区域
    QVBoxLayout *contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(24);
    contentVerLayout->setContentsMargins(0, 0, 0, 0);
    contentVerLayout->addLayout(userNameHorLayout);
    contentVerLayout->addLayout(nickNameWithTipVerLayout);
    contentVerLayout->addStretch();

    //底部“取消”、“确定”按钮
    m_pCancelBtn = new KPushButton();
    m_pCancelBtn->setMinimumWidth(96);
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton();
    m_pConfirmBtn->setMinimumWidth(96);
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);

    QHBoxLayout *bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    //主布局
    QVBoxLayout *mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(24, 24, 24, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    mainWidget()->setLayout(mainVerLayout);
}

void ChangeUserNickname::setConnect()
{
    connect(m_pNickNameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (txt.startsWith(' ')) {
            txt.remove(QRegularExpression("^\\s+"));  // 移除前导空白
            m_pNickNameLineEdit->blockSignals(true);
            m_pNickNameLineEdit->setText(txt);
            m_pNickNameLineEdit->blockSignals(false);
        }
        nameLegalityCheck(txt);
        refreshConfirmBtnStatus();
    });

    connect(m_pConfirmBtn, &QPushButton::clicked, this, [=]{
        m_pAccountServiceIface->call("SetRealName", m_pNickNameLineEdit->text());
        close();
    });

    connect(m_pCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
}

void ChangeUserNickname::nameLegalityCheck(QString nickname)
{
    if (nickname.isEmpty()){
        tipString = tr("NickName's length must between 1~%1 characters").arg(NICKNAME_LENGTH);
    } else if (namesIsExists.contains(nickname)){
        tipString = tr("nickName already in use.");
    } else if (m_pNickNameLineEdit->text().contains(":")) {
        tipString = tr("Can't contains ':'.");
    } else if(nickname.length() > NICKNAME_LENGTH) {
        m_pNickNameLineEdit->setText(oldNickName);
    } else {
        oldNickName = nickname;
        tipString = tr("");
    }

    m_pTipLabel->setText(tipString);

    refreshConfirmBtnStatus();

}

void ChangeUserNickname::refreshConfirmBtnStatus()
{
    if (!m_pTipLabel->text().isEmpty() || m_pNickNameLineEdit->text().isEmpty()) {
        m_pConfirmBtn->setEnabled(false);
    } else {
        m_pConfirmBtn->setEnabled(true);
    }
}

void ChangeUserNickname::keyPressEvent(QKeyEvent * event)
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

bool ChangeUserNickname::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == m_pNickNameLineEdit){
                if (QString::compare(m_pNickNameLineEdit->text(), g_get_real_name()) == 0){
                    m_pNickNameLineEdit->setText("");
                    refreshConfirmBtnStatus();
                }
            }
        }
    }

    return KDialog::eventFilter(watched, event);
}

