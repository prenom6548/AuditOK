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
#include "verifykyidwidget.h"
#include <QHBoxLayout>
#include <QDebug>
#include "klabel.h"
#include "kyiconbutton.h"
#include "iconedit.h"

VerifyKyidWidget::VerifyKyidWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    initUi();
}

void VerifyKyidWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 40, 0, 0);
    mainLayout->setSpacing(16);

    QHBoxLayout *kylinIdLayout = new QHBoxLayout(this);
    kylinIdLayout->setAlignment(Qt::AlignCenter);
    kylinIdLayout->setContentsMargins(0, 0, 0, 0);
    m_pKylinIdEdit = new IconEdit(m_modelLockDialog, this, KYLINIDEDIT);
    m_pKylinIdEdit->setFixedSize(240, 40);
    QPalette palett(palette());
    palett.setColor(QPalette::Background, Qt::white);
    m_pKylinIdEdit->setPalette(palett);
    m_pKylinIdEdit->setPrompt(tr("Enter your Kylin ID account"));
    kylinIdLayout->addWidget(m_pKylinIdEdit);
    connect(m_pKylinIdEdit, &IconEdit::textChanged, this, [=](const QString &text) {
        Q_EMIT textChanged(text);
        if (text.isEmpty() || (m_pCountdown && m_pCountdown->isActive())) {
            m_pGetCaptchaBtn->setEnabled(false);
        } else {
            m_pGetCaptchaBtn->setEnabled(true);
        }
    });
    setFocusProxy(m_pKylinIdEdit);

    QHBoxLayout *passwordLayout = new QHBoxLayout(this);
    passwordLayout->setAlignment(Qt::AlignCenter);
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    m_pPasswordEdit = new IconEdit(m_modelLockDialog, this, RESETPWDEDIT);
    m_pPasswordEdit->setFixedSize(240, 40);
    m_pPasswordEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pPasswordEdit->setPalette(palett);
    m_pPasswordEdit->setPrompt(tr("Input password"));
    passwordLayout->addWidget(m_pPasswordEdit);
    connect(m_pPasswordEdit, &IconEdit::textChanged, this, [=](const QString &text) { Q_EMIT textChanged(text); });

    QHBoxLayout *captchaLayout = new QHBoxLayout(this);
    captchaLayout->setAlignment(Qt::AlignCenter);
    captchaLayout->setContentsMargins(0, 0, 0, 0);
    captchaLayout->setSpacing(16);
    captchaLayout->addStretch();
    m_pCaptchaEdit = new IconEdit(m_modelLockDialog, this, KYLINIDEDIT);
    m_pCaptchaEdit->setFixedSize(112, 40);
    m_pCaptchaEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    m_pCaptchaEdit->setPalette(palett);
    m_pCaptchaEdit->setPrompt(tr("Enter the verification code"));
    captchaLayout->addWidget(m_pCaptchaEdit);
    connect(m_pCaptchaEdit, &IconEdit::textChanged, this, [=](const QString &text) { Q_EMIT textChanged(text); });

    m_pGetCaptchaBtn = new KyIconButton(CAPCHABUTTON, this);
    m_pGetCaptchaBtn->setFocusPolicy(Qt::ClickFocus);
    m_pGetCaptchaBtn->installEventFilter(this);
    m_pGetCaptchaBtn->setText(tr("Get verification code"));
    m_pGetCaptchaBtn->setFixedSize(112, 40);
    m_pGetCaptchaBtn->setEnabled(false);
    connect(m_pGetCaptchaBtn, &QPushButton::clicked, this, &VerifyKyidWidget::captchaBtnclicked);
    captchaLayout->addWidget(m_pGetCaptchaBtn);
    captchaLayout->addStretch();

    mainLayout->addLayout(kylinIdLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(captchaLayout);
    mainLayout->addSpacing(2);
}

void VerifyKyidWidget::onCaptchaBtnclicked(bool success)
{
    if (success) {
        m_lastDateTime = QDateTime::currentDateTime();
        if (!m_pCountdown) {
            m_pCountdown = new QTimer(this);
            m_pCountdown->setInterval(500);
            connect(m_pCountdown, &QTimer::timeout, this, [&] {
                int interval = (60 - m_lastDateTime.secsTo(QDateTime::currentDateTime()));
                if (interval <= 0) {
                    m_pCountdown->stop();
                    m_pGetCaptchaBtn->setEnabled(true);
                    m_pGetCaptchaBtn->setText(tr("Get verification code"));
                } else {
                    m_pGetCaptchaBtn->setEnabled(false);
                    m_pGetCaptchaBtn->setText(tr("Reacquire(%1)").arg(interval));
                }
            });
        }
        m_pCaptchaEdit->setFocus();
        m_pCountdown->start(500);
    } else {
        m_pKylinIdEdit->setFocus();
    }
}

QString VerifyKyidWidget::getKylinIdEditText()
{
    return m_pKylinIdEdit->text();
}

QString VerifyKyidWidget::getPwdEditText()
{
    return m_pPasswordEdit->text();
}

QString VerifyKyidWidget::getCaptchaEditText()
{
    return m_pCaptchaEdit->text();
}

void VerifyKyidWidget::clearText()
{
    m_pKylinIdEdit->clearText();
    m_pPasswordEdit->clearText();
    m_pCaptchaEdit->clearText();
}
