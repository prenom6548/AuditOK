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
#include <QFile>
#include <QHBoxLayout>
#include <QApplication>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <qrencode.h>
#include <QPainter>
#include <QLineEdit>

// 二维码生成
QImage encodeImage(const QString &s, int bulk)
{
    QImage ret;
    QRcode *qr = QRcode_encodeString(s.toUtf8(), 1, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (qr != nullptr) {
        int allBulk = (qr->width) * bulk;
        ret = QImage(allBulk, allBulk, QImage::Format_Mono);
        QPainter painter(&ret);
        QColor fg("black");
        QColor bg("white");
        painter.setBrush(bg);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, allBulk, allBulk);

        painter.setBrush(fg);
        for (int y = 0; y < qr->width; y++) {
            for (int x = 0; x < qr->width; x++) {
                if (qr->data[y * qr->width + x] & 1) {
                    QRectF r(x * bulk, y * bulk, bulk, bulk);
                    painter.drawRects(&r, 1);
                }
            }
        }
        QRcode_free(qr);
    }
    return ret;
}

BindDialog::BindDialog(int mod, QWidget *parent) : mode(mod), QWidget(parent)
{
    initUI();
    updateSize();

    QMetaObject::connectSlotsByName(this); // 自动绑定信号槽
}

BindDialog::~BindDialog() {}

void BindDialog::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget *titleWidget = new QWidget(this);
    titleWidget->setObjectName(QString::fromUtf8("titleWidget"));
    titleWidget->setMinimumSize(QSize(0, 36));
    titleWidget->setMaximumSize(QSize(16777215, 36));
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setSpacing(0);
    QSpacerItem *titleSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    btnClose = new QPushButton(titleWidget);
    titleLayout->addItem(titleSpacer);
    titleLayout->addWidget(btnClose, 0, Qt::AlignVCenter);

    btnClose->setProperty("isWindowButton", 0x2);
    btnClose->setObjectName("btnClose");
    btnClose->setProperty("useIconHighlightEffect", 0x8);
    btnClose->setFixedSize(30, 30);
    btnClose->setIconSize(QSize(16, 16));
    btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    btnClose->setFlat(true);
    btnClose->setToolTip(tr("Close"));

    layout->addWidget(titleWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    buttonLayout->setSpacing(16);
    buttonLayout->setContentsMargins(24, 0, 24, 24);
    QSpacerItem *buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(buttonSpacer);

    btnOk = new QPushButton(this);
    btnOk->setObjectName("btnOk");
    btnOk->setText(tr("Ok"));
    btnOk->setMinimumSize(QSize(88, 36));
    btnCancel = new QPushButton(this);
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
        QLabel *promptLabel = new QLabel(this);
        promptLabel->setText(tr("Mfa authentication is on, please bind OTP code"));
        promptLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        promptLayout->addWidget(warningLabel);
        promptLayout->addWidget(promptLabel, 1); // 添加拉伸因子
        promptLayout->setContentsMargins(24, 0, 24, 0);
        layout->addLayout(promptLayout, 1);
        btnOk->setText(tr("Go to Bind"));
    } else if (mode == BINDOTPDIALOG) {
        QVBoxLayout *promptLayout = new QVBoxLayout(this);
        otpPromptLabel = new QLabel(this);
        otpPromptLabel->setWordWrap(true);
        QHBoxLayout *qrcodeLayout = new QHBoxLayout(this);
        qrcodeLayout->setContentsMargins(0, 32, 0, 32);
        otpQrcodeLabel = new QLabel(this);
        otpQrcodeLabel->setFixedSize(150, 150);
        qrcodeLayout->setAlignment(Qt::AlignHCenter);
        qrcodeLayout->addWidget(otpQrcodeLabel);
        otpPromptEdit = new QLineEdit(this);
        otpPromptEdit->setObjectName("otpPromptEdit");
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
        finishMsg = new QLabel(this);
        finishMsg->setWordWrap(true);
        QFont finishMsgFont = finishMsg->font();
        finishMsgFont.setPointSize(16);
        finishMsg->setFont(finishMsgFont);
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
    QWidget *parentWidget = qobject_cast<QWidget *>(parent());
    if (parentWidget) {
        move((parentWidget->width() - width()) / 2, (parentWidget->height() - height()) / 2);
    }
    return QWidget::resizeEvent(event);
}

void BindDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

void BindDialog::on_btnClose_clicked()
{
    if (otpPromptEdit) {
        otpPromptEdit->setText("");
    }
    Q_EMIT rejected();
    Q_EMIT finished();
}

void BindDialog::on_btnOk_clicked()
{
    if (otpPromptEdit) {
        Q_EMIT accepted(otpPromptEdit->text());
        otpPromptEdit->setText("");
    } else {
        Q_EMIT accepted("");
    }
}

void BindDialog::on_btnCancel_clicked()
{
    if (otpPromptEdit) {
        otpPromptEdit->setText("");
    }
    Q_EMIT rejected();
    Q_EMIT finished();
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
        QImage img = encodeImage(text, 5);
        otpQrcodeLabel->setPixmap(QPixmap::fromImage(img).scaled(150, 150));
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

void BindDialog::setOtpEmergMsg(QString text)
{
    showFinishedPrompt(text);
}

void BindDialog::showFinishedPrompt(QString text)
{
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
    if (btnClose)
        btnClose->hide();

    if (finishWidget)
        finishWidget->show();

    btnOk->setText(tr("Ok"));
    btnOk->setEnabled(true);

    if (finishLabel) {
        finishLabel->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(64, 64)));
        finishMsg->setText(tr("OTP added successfully"));
        finishTip->setText(text);
    }
}

void BindDialog::on_otpPromptEdit_textChanged(QString text)
{
    if (!text.isEmpty()) {
        btnOk->setEnabled(true);
    } else {
        btnOk->setEnabled(false);
    }
}
