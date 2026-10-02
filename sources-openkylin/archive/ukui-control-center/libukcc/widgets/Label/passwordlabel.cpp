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
#include "passwordlabel.h"
#include <QPainter>
#include <QIcon>
#include <QGSettings/QGSettings>
#include <QDebug>
#include <QRect>
#include <QApplication>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

EyeBtn::EyeBtn(QWidget *parent) :QLabel(parent)
{
    this->setPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(24,24));
    this->setProperty("useIconHighlightEffect", 0x2);
    this->setFixedSize(24, 24);
    this->setAlignment(Qt::AlignCenter);
}

EyeBtn::~EyeBtn()
{

}

void EyeBtn::setIconStatus(bool isHidden)
{
    if (isHidden) {
        this->setPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(24,24));
    } else {
        this->setPixmap(QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(24,24));
    }
    mIsHidden = isHidden;

}

void EyeBtn::mouseReleaseEvent(QMouseEvent *event)
{
    setIconStatus(!mIsHidden);

    Q_EMIT clicked(mIsHidden);
}

PasswordLabel::PasswordLabel(QWidget *parent) :QWidget(parent)
{
    mPassword  =  new QLineEdit(this);
    mPassword->setEnabled(false);
    mPassword->setProperty("setDisableBrush", QBrush(Qt::NoBrush));
    mPassword->setEchoMode(QLineEdit::Password);
    mPassword->installEventFilter(this);
    QPalette palette = mPassword->palette();
    QColor color = palette.color(QPalette::Active, QPalette::Text);
    palette.setColor(QPalette::Disabled, QPalette::Text, color);
    mPassword->setPalette(palette);

    mPassword->setContentsMargins(0,0,0,0);
    mPassword->setMinimumWidth(0);
    mEyesBtn = new EyeBtn(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mEyesBtn, "PasswordLabel", "", "password label");

    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(0,0,0,0);
    mainHLayout->setSpacing(0);
    mainHLayout->addWidget(mPassword);
    mainHLayout->addWidget(mEyesBtn);
    mainHLayout->addStretch();
    this->setLayout(mainHLayout);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QPalette textPal = mEyesBtn->palette();
            QColor color1 = textPal.color(QPalette::Active, QPalette::Text);
            textPal.setColor(QPalette::Disabled, QPalette::Button, Qt::transparent);
            textPal.setColor(QPalette::Disabled, QPalette::Text, color1);
            mPassword->setPalette(textPal);
        }
        if ("systemFont" == key || "systemFontSize" == key) {
            updateSize();
        }
    });

    connect(mEyesBtn, &EyeBtn::clicked, this, [=](bool isHidden){
        if (isHidden) {
            mPassword->setEchoMode(QLineEdit::Password);
        } else {
            mPassword->setEchoMode(QLineEdit::Normal);
        }
        updateSize();
    });
}

PasswordLabel::~PasswordLabel()
{

}

void PasswordLabel::setStatus(bool isPassword)
{
    if (isPassword && mPassword->echoMode() == QLineEdit::Normal) {
        mPassword->setEchoMode(QLineEdit::Password);
        mEyesBtn->setIconStatus(isPassword);
    }
    updateSize();
}

void PasswordLabel::setText(const QString &str)
{
    mPassword->setText(str);
    updateSize();
}

QString PasswordLabel::text() const
{
    return mPassword->text();
}

void PasswordLabel::updateSize()
{
    QFontMetrics fontMetrics(this->font());
    if (mPassword->echoMode() == QLineEdit::Password) {
        QFontInfo fInfo(this->font());
        int charWidth = fInfo.pixelSize();
        mPassword->setFixedWidth(charWidth*mPassword->text().size() + 16);
        qDebug() << "increase" << (charWidth*mPassword->text().size() + 16);
    } else {
        int fontSize = fontMetrics.width(mPassword->text());
        mPassword->setFixedWidth(fontSize+16);
        qDebug() << "shorten" << (fontSize + 16);
    }
}
