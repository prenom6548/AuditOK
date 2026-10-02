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
#include "mulverifydialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QPainter>

class SettingFrame : public QFrame
{
public:
    SettingFrame(QWidget *parent){}
    ~SettingFrame() {}

protected:
    void paintEvent(QPaintEvent *event)
    {
        int radius = qApp->property("kradius-normal").toInt();
        QPainter painter(this);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        if (qApp->property("kcontain-secondary-normal").canConvert<QBrush>()) {
            QBrush containColor = qApp->property("kcontain-secondary-normal").value<QBrush>();
            painter.setBrush(containColor);
        }
        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        painter.drawRoundedRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2, radius, radius);
        painter.restore();
        QFrame::paintEvent(event);
    }
};

MulVerifyDialog::MulVerifyDialog(bool isPushBtnUI, QWidget *parent) :
    KDialog(parent),
    m_isPushBtnUI(isPushBtnUI)
{
    this->setWindowTitle(tr("Two-factor Authentication"));
    this->setWindowIcon(QIcon::fromTheme("ukui-control-center"));
    this->setFixedSize(424, 306);
    initUI();
    initConnect();
}

MulVerifyDialog::~MulVerifyDialog()
{
}

void MulVerifyDialog::initUI()
{
    KLabel *titleLabel =  new KLabel();
    titleLabel->setText(tr("Choose the two factor authentication method"));

    QHBoxLayout *securityHLayout = new QHBoxLayout;
    QHBoxLayout *otpHLayout  = new QHBoxLayout;
    securityHLayout->setContentsMargins(16, 0, 8, 0);
    otpHLayout->setContentsMargins(16, 0, 8, 0);
    m_pSecurityBindLabel = new KLabel();
    m_pSecurityBindLabel->setContentsMargins(0,0,8,0);
    m_pSecurityBindLabel->setAlignment(Qt::AlignLeft);
    m_pOtpBindLabel = new KLabel();
    m_pOtpBindLabel->setContentsMargins(0,0,8,0);
    m_pOtpBindLabel->setAlignment(Qt::AlignLeft);

    if (m_isPushBtnUI) {
        titleLabel->setText(tr("Set more login methods for this user to enable two factor authentication"));
        KLabel *m_pSecurityLabel = new KLabel;
        m_pSecurityLabel->setText(tr("Security Key"));
        KLabel *m_pOtpLabel = new KLabel;
        m_pOtpLabel->setText(tr("OTP Dynamic Password"));
        m_pSecurityBindBtn = new KPushButton;
        m_pSecurityBindBtn->setText(tr("Bind"));
        m_pSecurityBindBtn->setAccessibleName(tr("Security Key"));
        m_pOtpBindBtn = new KPushButton;
        m_pOtpBindBtn->setText(tr("Bind"));
        m_pOtpBindBtn->setAccessibleName(tr("OTP Dynamic Password"));
        securityHLayout->addWidget(m_pSecurityLabel);
        securityHLayout->addStretch();
        securityHLayout->addWidget(m_pSecurityBindBtn);
        otpHLayout->addWidget(m_pOtpLabel);
        otpHLayout->addStretch();
        otpHLayout->addWidget(m_pOtpBindBtn);
        m_pSecurityBindLabel->setVisible(false);
        m_pOtpBindLabel->setVisible(false);
    } else {
        m_pSecurityBtn = new QRadioButton(tr("Security Key"), this);
        m_pSecurityBtn->setAccessibleName(tr("Security Key"));
        m_pOtpBtn = new QRadioButton(tr("OTP Dynamic Password"), this);
        m_pOtpBtn->setAccessibleName(tr("OTP Dynamic Password"));
        m_pRadioBtnGroup = new QButtonGroup(this);
        m_pRadioBtnGroup->addButton(m_pSecurityBtn, 1);
        m_pRadioBtnGroup->addButton(m_pOtpBtn, 0);
        securityHLayout->addWidget(m_pSecurityBtn);
        securityHLayout->addStretch();
        otpHLayout->addWidget(m_pOtpBtn);
        otpHLayout->addStretch();
    }

    securityHLayout->addWidget(m_pSecurityBindLabel);
    otpHLayout->addWidget(m_pOtpBindLabel);
    SettingFrame *settingFrame = new SettingFrame(this);
    settingFrame->setFixedHeight(60);
    settingFrame->setLayout(securityHLayout);
    SettingFrame *settingFrame1 = new SettingFrame(this);
    settingFrame1->setFixedHeight(60);
    settingFrame1->setLayout(otpHLayout);

    m_pCancelBtn = new KPushButton(this);
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton(this);
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    m_pConfirmBtn->setEnabled(false);
    QHBoxLayout *bottomBtnsHorLayout= new QHBoxLayout(this);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    QVBoxLayout *mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(24, 10, 24, 24);
    mainVerLayout->addWidget(titleLabel);
    mainVerLayout->addWidget(settingFrame);
    mainVerLayout->addWidget(settingFrame1);
    mainVerLayout->addSpacing(24);
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    mainWidget()->setLayout(mainVerLayout);
}

void MulVerifyDialog::initConnect()
{
    connect(m_pCancelBtn, &KPushButton::clicked, this, [=](){
        this->close();
    });

    connect(m_pSecurityBindBtn, &KPushButton::clicked, this, [=](){
        emit onSelectFactorySignal(1);
    });

    connect(m_pOtpBindBtn, &KPushButton::clicked, this, [=](){
        emit onSelectFactorySignal(0);
    });

    connect(m_pConfirmBtn, &KPushButton::clicked, this, [=](){
        onConfirmBtnPressed();
    });

    connect(m_pRadioBtnGroup,  QOverload<int>::of(&QButtonGroup::buttonClicked),this, [=](){
        m_pConfirmBtn->setEnabled(true);
    });
}

void MulVerifyDialog::onConfirmBtnPressed()
{
    if (m_isPushBtnUI) {
        this->accept();
    } else {
        if ((m_pRadioBtnGroup->checkedId() == 0 && m_isBoundOtp) || (m_pRadioBtnGroup->checkedId() == 1 && m_isBoundUkey)) {
            this->accept();
        }
        emit onSelectFactorySignal(m_pRadioBtnGroup->checkedId());
    }
}

void MulVerifyDialog::refreshBindStatus(const QString &method, bool status)
{
    if (method == "otp") {
        m_isBoundOtp = status;
        if (status) {
            if (m_isPushBtnUI) {
                m_pOtpBindBtn->setVisible(false);
                m_pOtpBindLabel->setVisible(true);
                m_pConfirmBtn->setEnabled(true);
            }
            m_pOtpBindLabel->setFontColor(Qt::green);
            m_pOtpBindLabel->setText(tr("Bound"));
        } else {
            m_pOtpBindLabel->setFontColor(Qt::red);
            m_pOtpBindLabel->setText(tr("Unbound"));
        }
    }

    if (method == "ukey") {
        m_isBoundUkey = status;
        if (status) {
            if (m_isPushBtnUI) {
                m_pSecurityBindBtn->setVisible(false);
                m_pSecurityBindLabel->setVisible(true);
                m_pConfirmBtn->setEnabled(true);
            }
            m_pSecurityBindLabel->setFontColor(Qt::green);
            m_pSecurityBindLabel->setText(tr("Bound"));
        } else {
            m_pSecurityBindLabel->setFontColor(Qt::red);
            m_pSecurityBindLabel->setText(tr("Unbound"));
        }
    }
}

void MulVerifyDialog::keyPressEvent(QKeyEvent * event)
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
