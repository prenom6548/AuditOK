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
#include "otpbinddialog.h"

#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>

#define DIALOG_WIDTH 416

OtpBindDialog::OtpBindDialog(QString username, PwdlessService *service, QWidget *parent) :
    m_username(username)
    , m_serviceInterface(service)
    , KDialog(parent)
{
    this->setFixedWidth(DIALOG_WIDTH);
    UkuiWindowHelper *m_pUkuiWindowHelper = new UkuiWindowHelper(this);
    // 禁用最大化
    UkuiWindowHelper::States defaultState = UkuiWindowHelper::States(0xff);
    m_pUkuiWindowHelper->setWindowState(defaultState & ~UkuiWindowHelper::State::Maximizable);

    setWindowTitle(tr("OTP Dynamic Password"));
    setWindowIcon("ukui-control-center");
    initUI();
    initConnect();
}

OtpBindDialog::~OtpBindDialog()
{

}

void OtpBindDialog::initUI()
{
    QVBoxLayout *mainVerLayout = new QVBoxLayout(this);

    m_pTitleLabel = new KLabel();
    m_pTitleLabel->setWordWrap(true);
    m_pTitleLabel->setText(tr("Scan the QR code below using an OTP scanning program (a mobile app or WeChat mini program"
                              " that supports TOTP standard 30 second refresh interval) to obtain a dynamic password, and"
                              " fill the obtained dynamic password into the input box below"));

    m_pQrcodePixmap = new KLabel;
    m_pQrcodePixmap->setFixedSize(150, 150);
    QHBoxLayout *codeHLayout = new QHBoxLayout;
    codeHLayout->setAlignment(Qt::AlignCenter);
    codeHLayout->addWidget(m_pQrcodePixmap);

    m_pOtpLineEdit = new QLineEdit();
    m_pOtpLineEdit->setFocus();
    m_pOtpLineEdit->setPlaceholderText(tr("Input Dynamic Password"));
    m_pTitleLabel->setAccessibleName(tr("Scan the QR code below using an OTP scanning program (a mobile app or WeChat mini program"
                                     " that supports TOTP standard 30 second refresh interval) to obtain a dynamic password, and"
                                     " fill the obtained dynamic password into the input box below"));
    m_pTitleLabel->setBuddy(m_pOtpLineEdit);

    m_tipMsgLabel = new KLabel;
    m_tipMsgLabel->setFontColor(Qt::red);

    m_pCancelBtn = new KPushButton(this);
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton(this);
    m_pConfirmBtn->setText(tr("Bind"));
    m_pConfirmBtn->setEnabled(false);
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    QHBoxLayout *bottomBtnsHorLayout= new QHBoxLayout(this);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    m_pFinishWidget = new QWidget(this);
    m_pFinishWidget->setFixedWidth(DIALOG_WIDTH-48);
    QVBoxLayout *finishLayout = new QVBoxLayout(m_pFinishWidget);
    finishLayout->setContentsMargins(0, 0, 0, 0);
    finishLayout->addStretch();
    m_pFinishLabel = new QLabel(this);
    m_pFinishLabel->setFixedSize(64, 64);
    QHBoxLayout *finishHLayout = new QHBoxLayout(m_pFinishWidget);
    finishHLayout->setContentsMargins(0, 0, 0, 0);
    finishHLayout->setAlignment(Qt::AlignHCenter);
    finishHLayout->addWidget(m_pFinishLabel);
    m_pFinishMsg = new KLabel(this);
    m_pFinishMsg->setWordWrap(true);
    QFont finishMsgFont = m_pFinishMsg->font();
    finishMsgFont.setPointSize(16);
    m_pFinishMsg->setFont(finishMsgFont);
    m_pFinishTip = new QLabel(this);
    m_pFinishTip->setWordWrap(true);
    finishLayout->addLayout(finishHLayout);
    finishLayout->addWidget(m_pFinishMsg);
    finishLayout->addWidget(m_pFinishTip);
    finishLayout->addStretch();
    m_pFinishMsg->setAlignment(Qt::AlignCenter);
    m_pFinishWidget->hide();

    mainVerLayout->setContentsMargins(24, 0, 24, 24);
    mainVerLayout->addWidget(m_pTitleLabel);
    mainVerLayout->addSpacing(24);
    mainVerLayout->addLayout(codeHLayout);
    mainVerLayout->addSpacing(24);
    mainVerLayout->addWidget(m_pOtpLineEdit);
    mainVerLayout->addWidget(m_tipMsgLabel);
    mainVerLayout->addWidget(m_pFinishWidget);
    mainVerLayout->addLayout(bottomBtnsHorLayout);
    mainVerLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainVerLayout->invalidate();
    mainVerLayout->activate();

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    mainWidget()->setLayout(mainVerLayout);

    this->adjustSize();
}

QSize OtpBindDialog::sizeHint() const
{
    QSize size = QDialog::sizeHint();
    size.setWidth(DIALOG_WIDTH);  // 强制宽度
    return size;
}

void OtpBindDialog::setQrCodePixmap(QString code)
{
    QPixmap base64Image;
    base64Image.loadFromData(QByteArray::fromBase64(code.toLocal8Bit()));
    m_pQrcodePixmap->setPixmap(base64Image, m_pQrcodePixmap->width(), m_pQrcodePixmap->height());
}

void OtpBindDialog::initConnect()
{
    connect(m_pCancelBtn, &KPushButton::clicked, this, [=](){
        this->close();
    });

    connect(m_pOtpLineEdit, &QLineEdit::textEdited, this, [=](){
        if (m_pOtpLineEdit->text().isEmpty()) {
            m_pConfirmBtn->setEnabled(false);
        } else {
            m_pConfirmBtn->setEnabled(true);
        }
    });

    connect(m_pConfirmBtn, &KPushButton::clicked, this, &OtpBindDialog::onBntnOkClicked);
}

void OtpBindDialog::setErrorMsg(QString string)
{
    m_tipMsgLabel->setText(string);
}

QPushButton *OtpBindDialog::GetConfirmBtn()
{
    return m_pConfirmBtn;
}

QString OtpBindDialog::GetToken()
{
    return m_pOtpLineEdit->text();
}

void OtpBindDialog::onBntnOkClicked()
{
    if (!isFinished) {
        QString errorMsg;
        int reply = m_serviceInterface->BindOtpVerify(m_username, GetToken(), errorMsg);
        showFinishedPrompt(reply, errorMsg);
    } else {
        close();
        Q_EMIT accept();
    }
}

void OtpBindDialog::showFinishedPrompt(bool ret, QString errorMsg)
{
    isFinished = true;

    if (m_pTitleLabel)
        m_pTitleLabel->hide();
    if (m_pOtpLineEdit)
        m_pOtpLineEdit->hide();
    if (m_pQrcodePixmap)
        m_pQrcodePixmap->hide();
    if (m_tipMsgLabel)
        m_tipMsgLabel->hide();
    if (m_pCancelBtn)
        m_pCancelBtn->hide();

    if (m_pFinishWidget)
        m_pFinishWidget->show();

    m_pConfirmBtn->setText(tr("Ok"));
    m_pConfirmBtn->setEnabled(true);

    if (ret == 0 && m_pFinishLabel) {
        m_pFinishLabel->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(64, 64)));
        m_pFinishMsg->setText(tr("OTP added successfully"));
        if (m_serviceInterface && m_pFinishTip) {
            QString msg;
            int ret = m_serviceInterface->getOtpEmergMsg(m_username, msg);
            qDebug() << ret << msg;
            if (ret == 0 && !msg.isEmpty()) {
                m_pFinishTip->setText(msg);
            }
        }
    } else if (ret != 0 && m_pFinishLabel) {
        m_pFinishLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(64, 64)));
        m_pFinishMsg->setText(errorMsg);
    }
}

void OtpBindDialog::keyPressEvent(QKeyEvent * event)
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
        KDialog::keyPressEvent(event);
    }
}

