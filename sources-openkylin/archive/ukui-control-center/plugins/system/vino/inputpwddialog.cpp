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
#include "inputpwddialog.h"

#include <QDebug>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include "sharemain.h"
#include "fixlabel.h"

#include <kysdk/applications/accessinfohelper.h>

#include "ukcccommon.h"
using namespace ukcc;

InputPwdDialog::InputPwdDialog(QGSettings *Keygsettiings,QWidget *parent) :
    QDialog(parent),
    m_pGsettings(Keygsettiings)
{
    setupInit();
    initConnect();
}

InputPwdDialog::InputPwdDialog(ComKylinRemoteDesktopInterface *waylandDbus, QWidget *parent) :
    QDialog(parent),
    m_pWaylandDbus(waylandDbus)
{
    setupInit();
    initConnect();
}

InputPwdDialog::~InputPwdDialog()
{
}

void InputPwdDialog::setupInit()
{
    setWindowTitle(tr("VNC password"));
    this->setFixedSize(480, 160);

    QVBoxLayout *mInputPwdLyt = new QVBoxLayout(this);
    mInputPwdLyt->setContentsMargins(24, 24, 24, 24);
    mInputPwdLyt->setSpacing(0);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFixedSize(432, 36);
    mInputPwdFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->setSpacing(8);

    FixLabel *mSetPwdLabel = new FixLabel(mInputPwdFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mSetPwdLabel, "Vino", "InputPwdDialog", "");
    mSetPwdLabel->setFixedSize(72, 36);
    mSetPwdLabel->setText(tr("Password"));

    m_pPasswordEdit = new KPasswordEdit(mInputPwdFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPasswordEdit, "Vino", "InputPwdDialog", "");
    m_pPasswordEdit->setAccessibleName(tr("Password"));
    m_pPasswordEdit->setFixedSize(352, 36);
    QRegExp inputRegExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/?:;\"'|\{\}\ ]+$");
    QRegExpValidator *inputLimits = new QRegExpValidator(inputRegExp, this);
    m_pPasswordEdit->setValidator(inputLimits);

    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(m_pPasswordEdit);

    m_isEdited = false;

    m_pHintLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pHintLabel, "Vino", "InputPwdDialog", "");
    m_pHintLabel->setFixedSize(432,24);
    m_pHintLabel->setContentsMargins(84, 0, 0, 0);
    m_pHintLabel->setText(tr("Must be 1-8 characters long"));
    m_pHintLabel->setStyleSheet("QLabel{color:red; font-size : 14px}");

    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFixedSize(432, 48);
    mInputPwdFrame_1->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->setSpacing(16);

    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(m_pPasswordEdit);

    m_pCancelBtn = new QPushButton(mInputPwdFrame_1);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCancelBtn, "Vino", "InputPwdDialog", "");
    m_pCancelBtn->setMinimumWidth(96);
    m_pCancelBtn->setText(tr("Cancel"));
    m_pCancelBtn->setProperty("useButtonPalette", true);
    m_pCancelBtn->setAccessibleName(tr("Cancel"));

    m_pConfirmBtn = new QPushButton(mInputPwdFrame_1);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pConfirmBtn, "Vino", "InputPwdDialog", "");
    m_pConfirmBtn->setMinimumWidth(96);
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setProperty("isImportant", true);
    m_pConfirmBtn->setAccessibleName(tr("Confirm"));

    mLyt_2->addStretch();
    mLyt_2->addWidget(m_pCancelBtn);
    mLyt_2->addWidget(m_pConfirmBtn);

    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addWidget(m_pHintLabel);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);


    if (UkccCommon::isWayland()) {
        QString pwd = m_pWaylandDbus->GetPassword();
        m_pPasswordEdit->setText(pwd);
        m_pConfirmBtn->setEnabled(!pwd.isEmpty());
        m_pHintLabel->setVisible(pwd.isEmpty());
    } else {
        if(QByteArray::fromBase64(m_pGsettings->get(kVncPwdKey).toString().toLatin1()).length() <= 8) {
            if (m_pGsettings->get(kVncPwdKey).toString() == "keyring") {
                m_pPasswordEdit->setText("");
                m_pConfirmBtn->setEnabled(false);
                m_pHintLabel->setVisible(true);
            } else {
                m_pPasswordEdit->setText(QByteArray::fromBase64(m_pGsettings->get(kVncPwdKey).toString().toLatin1()));
                m_pHintLabel->setVisible(false);
            }
        }
    }
}

void InputPwdDialog::mpwdInputSlot(const QString &pwd)
{
    Q_UNUSED(pwd);
    m_isEdited = true;
    m_pConfirmBtn->setEnabled(true);
    if (pwd.length() <= 8 && !pwd.isEmpty()) {
        QByteArray text = pwd.toLocal8Bit();
        m_password = text.toBase64();
        m_pHintLabel->setVisible(false);
    } else if (pwd.isEmpty()) {
        m_pConfirmBtn->setEnabled(false);
        m_pHintLabel->setVisible(true);
        m_password = NULL;
    } else {
        m_pPasswordEdit->setText(pwd.mid(0, 8));
        QByteArray text = pwd.mid(0, 8).toLocal8Bit();
        m_pHintLabel->setVisible(true);
        m_password = text.toBase64();
    }
}

void InputPwdDialog::initConnect()
{
    connect(m_pCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(m_pConfirmBtn, &QPushButton::clicked, this, &InputPwdDialog::setPasswordSlot);

    //使用textEdited信号是为了防止密码框setText时触发信号
    connect(m_pPasswordEdit, &QLineEdit::textChanged, this, &InputPwdDialog::mpwdInputSlot);
}

void InputPwdDialog::setPasswordSlot()
{
    if (m_isEdited && m_password.length() == 0) { // 输入框被编辑，密码为空
        return;
    } else if (!m_isEdited){ // 输入框未编辑
        if (UkccCommon::isWayland()) {
            m_pWaylandDbus->SetAuthMethod(true);
        } else {
            m_pGsettings->set(kAuthenticationKey, "vnc");
        }
        this->close();
    } else { // 输入框被编辑，密码不为空
        if (UkccCommon::isWayland()) {
            m_pWaylandDbus->SetPassword(m_pPasswordEdit->text());
            m_pWaylandDbus->SetAuthMethod(true);
        } else {
            m_pGsettings->set(kVncPwdKey, m_password);
            m_pGsettings->set(kAuthenticationKey, "vnc");
        }
        this->close();
    }
}

void InputPwdDialog::keyPressEvent(QKeyEvent * event)
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

