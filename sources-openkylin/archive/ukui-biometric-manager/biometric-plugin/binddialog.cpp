/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "binddialog.h"
#include "fontsizeutil.h"
#include <ukuiwindowhelper.h>
#include <QFile>
#include <QHBoxLayout>
#include <QApplication>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QLineEdit>
#include <unistd.h>
#include <pwd.h>

BindDialog::BindDialog(PwdlessProxy *service, int mod, QWidget *parent)
    : m_serviceInterface(service), mode(mod), KDialog(parent)
{
    initUI();
    updateSize();

    m_uid = getuid();
    struct passwd *pwInfo = getpwuid(m_uid);
    if (pwInfo) {
        if (pwInfo->pw_name && pwInfo->pw_name[0] != '\0') {
            m_name = QString(pwInfo->pw_name);
        }
    }

    if (m_serviceInterface) {
        QString qrcode;
        if (m_serviceInterface->otpBind(m_name, 0, qrcode) == 0) {
            setOtpQrcodeText(qrcode);
        } else {
            showFinishedPrompt(false);
        }
    } else {
        showFinishedPrompt(false);
    }

    QMetaObject::connectSlotsByName(this); // 自动绑定信号槽
}

BindDialog::~BindDialog() {}

void BindDialog::initUI()
{
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();

    setWindowIcon("ukui-control-center");
    setWindowTitle(tr("OTP Key"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    buttonLayout->setSpacing(16);
    buttonLayout->setContentsMargins(24, 0, 24, 24);
    QSpacerItem *buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(buttonSpacer);

    btnOk = new KPushButton(this);
    btnOk->setObjectName("btnOk");
    btnOk->setText("ok");
    btnOk->setMinimumSize(QSize(88, 36));
    btnOk->setDefault(true);
    btnOk->setBackgroundColorHighlight(true);
    btnCancel = new KPushButton(this);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setText(tr("Cancel"));
    btnCancel->setMinimumSize(QSize(88, 36));
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnOk);

    if (mode == ENSUREDIALOG) {
        QHBoxLayout *promptLayout = new QHBoxLayout(this);
        QLabel *warningLabel = new QLabel(this);
        warningLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap((QSize(16, 16))));
        warningLabel->setAlignment(Qt::AlignCenter);
        warningLabel->setFixedSize(16, 16);
        promptLabel = new QLabel(this);
        promptLabel->setWordWrap(true);
        promptLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        promptLayout->addWidget(warningLabel);
        promptLayout->addWidget(promptLabel, 1); // 添加拉伸因子
        promptLayout->setContentsMargins(24, 0, 24, 0);
        layout->addLayout(promptLayout, 1);
        btnCancel->hide();
        btnOk->setText(tr("Ok"));
    } else if (mode == BINDOTPDIALOG) {
        QVBoxLayout *promptLayout = new QVBoxLayout(this);
        otpPromptLabel = new KLabel(this);
        otpPromptLabel->setWordWrap(true);
        otpPromptLabel->setText(tr("Please scan the QRCode and input the OTP in editor below"));
        QHBoxLayout *qrcodeLayout = new QHBoxLayout(this);
        qrcodeLayout->setContentsMargins(0, 16, 0, 16);
        otpQrcodeLabel = new QLabel(this);
        otpQrcodeLabel->setFixedSize(150, 150);
        qrcodeLayout->setAlignment(Qt::AlignHCenter);
        qrcodeLayout->addWidget(otpQrcodeLabel);
        otpPromptEdit = new QLineEdit(this);
        otpPromptEdit->setObjectName("otpPromptEdit");
        otpPromptEdit->setPlaceholderText(tr("Input OTP"));
        otpPromptEdit->setFocus();
        otpMessageLabel = new QLabel(this);
        QPalette pe;
        pe.setColor(QPalette::WindowText, Qt::red);
        otpMessageLabel->setPalette(pe);

        finishWidget = new QWidget(this);
        QVBoxLayout *finishLayout = new QVBoxLayout(finishWidget);
        finishLayout->addStretch();
        finishLabel = new QLabel(this);
        finishLabel->setFixedSize(64, 64);
        QHBoxLayout *finishHLayout = new QHBoxLayout(finishWidget);
        finishHLayout->setContentsMargins(0, 0, 0, 0);
        finishHLayout->setAlignment(Qt::AlignHCenter);
        finishHLayout->addWidget(finishLabel);
        finishMsg = new KLabel(this);
        finishMsg->setWordWrap(true);
        FontSizeUtil::setFontSize(finishMsg, 16.0);
        finishTip = new QLabel(this);
        finishTip->setWordWrap(true);
        finishLayout->addLayout(finishHLayout);
        finishLayout->addWidget(finishMsg);
        finishLayout->addWidget(finishTip);
        finishLayout->addStretch();
        finishMsg->setAlignment(Qt::AlignHCenter);

        promptLayout->addWidget(otpPromptLabel);
        promptLayout->addLayout(qrcodeLayout);
        promptLayout->addWidget(otpPromptEdit);
        promptLayout->addWidget(otpMessageLabel);
        promptLayout->addWidget(finishWidget);
        promptLayout->setContentsMargins(24, 0, 24, 0);
        layout->addLayout(promptLayout, 1);

        finishWidget->hide();
        btnOk->setText(tr("Bind"));
    }

    layout->addLayout(buttonLayout);
    this->mainWidget()->setLayout(layout);
    if (mode == BINDOTPDIALOG) {
        btnOk->setEnabled(false);
    }
}

void BindDialog::updateSize()
{
    if (mode == ENSUREDIALOG)
        setFixedSize(452, 172);
    else
        setFixedSize(380, 456);
}

void BindDialog::resizeEvent(QResizeEvent *event)
{
    return QWidget::resizeEvent(event);
}

void BindDialog::on_btnOk_clicked()
{
    if (mode == ENSUREDIALOG) {
        close();
        Q_EMIT accept();
        Q_EMIT finished();
        return;
    }

    if (isFinished) {
        close();
        Q_EMIT accept();
        Q_EMIT finished();
    }

    if (!m_serviceInterface)
        return;

    if (otpPromptEdit) {
        Q_EMIT accepted(otpPromptEdit->text());
        QString errmsg;
        if (m_serviceInterface->otpVerify(m_name, otpPromptEdit->text(), errmsg) == 0) {
            // token success
            showFinishedPrompt(true);
        } else {
            showFinishedPrompt(false, errmsg);
        }
    }
}

void BindDialog::on_btnCancel_clicked()
{
    Q_EMIT rejected();
    Q_EMIT finished();
    close();
}

void BindDialog::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QColor colorPal = pal.color(QPalette::Base);

    // 设置窗体为圆角
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(colorPal);
    painter.setPen(Qt::transparent);
    auto rect = this->rect();
    painter.drawRoundedRect(rect, 12, 12); // 窗口圆角

    return QWidget::paintEvent(event);
}

void BindDialog::setOtpPromptText(QString text)
{
    if (otpPromptLabel)
        otpPromptLabel->setText(text);
}

void BindDialog::setOtpQrcodeText(QString text)
{
    if (otpQrcodeLabel) {
        QPixmap qrcodePix;
        qrcodePix.loadFromData(QByteArray::fromBase64(text.toLocal8Bit()));
        otpQrcodeLabel->setPixmap(qrcodePix.scaled(150, 150));
    }
}
void BindDialog::setOtpPrpmptPromptText(QString text)
{
    if (otpPromptEdit)
        otpPromptEdit->setPlaceholderText(text);
}
void BindDialog::setOtpMessageText(QString text)
{
    if (otpMessageLabel)
        otpMessageLabel->setText(text);
}

void BindDialog::setPromptLabelText(QString text)
{
    if (promptLabel)
        promptLabel->setText(text);
}

void BindDialog::on_otpPromptEdit_textChanged(QString text)
{
    if (!text.isEmpty()) {
        btnOk->setEnabled(true);
    } else {
        btnOk->setEnabled(false);
    }
}

void BindDialog::showFinishedPrompt(bool ret, QString text)
{
    isFinished = true;

    if (otpMessageLabel)
        otpMessageLabel->hide();
    if (otpPromptEdit)
        otpPromptEdit->hide();
    if (otpQrcodeLabel)
        otpQrcodeLabel->hide();
    if (otpPromptLabel)
        otpPromptLabel->hide();
    if (btnCancel)
        btnCancel->hide();

    if (finishWidget)
        finishWidget->show();

    btnOk->setText(tr("Ok"));
    btnOk->setEnabled(true);

    if (ret && finishLabel) {
        finishLabel->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(64, 64)));
        finishMsg->setText(tr("OTP added successfully"));
        if (m_serviceInterface && finishTip) {
            QString msg;
            int ret = m_serviceInterface->getOtpEmergMsg(m_name, msg);
            if (ret == 0 && !msg.isEmpty()) {
                finishTip->setText(msg);
            }
        }
    } else if (!ret && finishLabel) {
        finishLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(64, 64)));
        if (!text.isEmpty()) {
            finishMsg->setText(text);
        } else {
            finishMsg->setText(tr("OTP addition failed"));
        }
    }
}

void BindDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Enter:
            if (btnOk->isEnabled())
                btnOk->clicked();
            break;
        case Qt::Key_Return:
            if (btnOk->isEnabled())
                btnOk->clicked();
            break;
        default:
            QDialog::keyPressEvent(event);
    }
}
