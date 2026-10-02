/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "button.h"
#include "style_helper.h"
#include <QDebug>
#include <QApplication>

Button::Button(const QString &text, const QString &img, QWidget *parent)
    : QPushButton(parent)
    , m_text(text)
    , m_modeImage(img)
    , m_mode(-1)
{
    if (qApp->property("kcontain-alpha-hover").isValid() && qApp->property("kcontain-alpha-hover").canConvert<QBrush>()) {
        QBrush hoverBrush = qApp->property("kcontain-alpha-hover").value<QBrush>();
        this->setProperty("setHoverBrush", hoverBrush);
    }

    if (qApp->property("kcontain-alpha-click").isValid() && qApp->property("kcontain-alpha-click").canConvert<QBrush>()) {
        QBrush clickBrush = qApp->property("kcontain-alpha-click").value<QBrush>();
        this->setProperty("setClickBrush", clickBrush);
        this->setProperty("setCheckBrush", clickBrush);
        this->setProperty("setCheckHoverBrush", clickBrush);
        this->setProperty("setCheckClickBrush", clickBrush);
    }
    setFocusPolicy(Qt::NoFocus);
    initButtonUi();
}

void Button::setText(const QString &text)
{
    m_text = text;
    if (m_textLabel) {
        m_textLabel->setText(text);
        m_textLabel->adjustSize();
    }
}

void Button::setStatus(bool state)
{
    m_statusLabel->setVisible(state);
}

void Button::setMode(int mode)
{
    m_mode = mode;
}

int Button::getMode()
{
    return m_mode;
}

QPixmap Button::drawPixmap(const QPixmap &source)
{
    QColor baseColor = this->palette().brightText().color();
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setRed(baseColor.red());
                color.setGreen(baseColor.green());
                color.setBlue(baseColor.blue());
            }
            img.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(img);
}

void Button::setModeImage(const QString &modeImage)
{
    m_modeImage = modeImage;
    if (m_imageLabel) {
        m_imageLabel->setPixmap(drawPixmap(QPixmap(m_modeImage)).scaled(60, 33, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        m_imageLabel->adjustSize();
    }
}

void Button::initButtonUi()
{
    this->setFlat(true);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setAlignment(Qt::AlignVCenter);
    m_mainLayout->setContentsMargins(24, 0, 27, 0);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(60, 40);
    m_imageLabel->setPixmap(drawPixmap(QPixmap(m_modeImage)).scaled(60, 33, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    m_imageLabel->adjustSize();

    m_textLabel = new QLabel(this);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setText(m_text);
    m_textLabel->adjustSize();

    //选中状态
    m_statusLabel = new QLabel(this);
    m_statusLabel->setFixedSize(17, 12);
    m_statusLabel->setPixmap(drawPixmap(QPixmap(":/img/selected.png")));
    m_statusLabel->setScaledContents(true);
    m_statusLabel->setVisible(false);

    //控件布局
    m_mainLayout->addWidget(m_imageLabel);
    m_mainLayout->addSpacing(16);
    m_mainLayout->addWidget(m_textLabel);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_statusLabel);
}
