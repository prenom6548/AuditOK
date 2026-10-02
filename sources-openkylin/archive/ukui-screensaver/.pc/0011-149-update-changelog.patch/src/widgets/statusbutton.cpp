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
#include "statusbutton.h"
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>

StatusButton::StatusButton(QWidget *parent, int type) : QPushButton(parent)
{
    buttonType = type;
    if (buttonType == BIOBUTTON || type == BOTBUTTON) {
        setFixedSize(48, 48);
        setIconSize(QSize(24, 24));
    } else if (buttonType == POWERBUTTON) {
        setFixedSize(130, 130);
        setIconSize(QSize(48, 48));
    } else if (buttonType == NORMALBTN) {
        setFixedHeight(48);
        setFocusPolicy(Qt::TabFocus);
    } else if (buttonType == PWDTIPBTN) {
        setFixedSize(48, 48);
        setFocusPolicy(Qt::TabFocus);
    }
    if (buttonType != NORMALBTN)
        this->setFocusPolicy(Qt::NoFocus);
}

void StatusButton::setClickedStatus(int status)
{
    curStatus = status;
    update();
}

void StatusButton::setDrivedId(int drivedId)
{
    m_drivedId = drivedId;
}

void StatusButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionButton option;
    initStyleOption(&option);

    if (curStatus == NORMAL) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        switch (buttonType) {
            case BOTBUTTON:
            case PWDTIPBTN:
                painter.setOpacity(0);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case BIOBUTTON:
                painter.setOpacity(0.1);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case POWERBUTTON:
                painter.setOpacity(0.1);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 65, 65);
                break;
            case NORMALBTN:
                painter.setOpacity(0);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            case SIDEBARBTN:
                painter.setOpacity(0);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 0, 0);
                break;
            default:
                painter.setOpacity(0);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
    }

    if (option.state & QStyle::State_MouseOver) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.15);
        switch (buttonType) {
            case BOTBUTTON:
            case BIOBUTTON:
            case PWDTIPBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case POWERBUTTON:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 65, 65);
                break;
            case NORMALBTN:
                painter.setOpacity(0.15);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            case SIDEBARBTN:
                painter.setOpacity(0.15);
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 0, 0);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
    }

    if (curStatus == CLICKED) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.21);
        switch (buttonType) {
            case BOTBUTTON:
            case BIOBUTTON:
            case PWDTIPBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case POWERBUTTON:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 65, 65);
                break;
            case NORMALBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            case SIDEBARBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 0, 0);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
    }

    if (curStatus == SELECT) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.25);
        switch (buttonType) {
            case BOTBUTTON:
            case BIOBUTTON:
            case PWDTIPBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case POWERBUTTON:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 65, 65);
                break;
            case NORMALBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            case SIDEBARBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 0, 0);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
        painter.save();
        QStyleOption opt;
        QColor color = opt.palette.color(QPalette::Highlight);
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        switch (buttonType) {
            case BOTBUTTON:
            case BIOBUTTON:
            case PWDTIPBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
            case POWERBUTTON:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 65, 65);
                break;
            case NORMALBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            case SIDEBARBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 0, 0);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
    }

    if (option.state & QStyle::State_HasFocus) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.25);
        switch (buttonType) {
            case NORMALBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
        painter.save();
        QStyleOption opt;
        QColor color = opt.palette.color(QPalette::Highlight);
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        switch (buttonType) {
            case NORMALBTN:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 24, 24);
                break;
            default:
                painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
                break;
        }
        painter.restore();
    }

    // 绘制文字
    if (buttonType == NORMALBTN && !text().isEmpty() && text() != "") {
        painter.setPen(QColor(255, 255, 255, 255));
        painter.setFont(font());
        painter.drawText(this->rect(), Qt::AlignHCenter | Qt::AlignVCenter, this->text());
    }

    // 绘制图片
    int buttonWidget = this->width();
    int buttonHeight = this->height();
    QRect buttonRect(0, 0, buttonWidget, buttonHeight);

    QPixmap pixmap = option.icon.pixmap(option.iconSize, QIcon::Active);
    int pixmapWidth = static_cast<int>(pixmap.width() / qApp->devicePixelRatio());
    int pixmapHeight = static_cast<int>(pixmap.height() / qApp->devicePixelRatio());
    QRect pixmapRect(0, 0, pixmapWidth, pixmapHeight);

    int deltaX = 0;
    int deltaY = 0;
    if (pixmapRect.width() < buttonRect.width())
        deltaX = buttonRect.width() - pixmapRect.width();
    else
        deltaX = pixmapRect.width() - buttonRect.width();
    if (pixmapRect.height() < buttonRect.height())
        deltaY = buttonRect.height() - pixmapRect.height();
    else
        deltaY = pixmapRect.height() - buttonRect.height();

    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.translate(deltaX / 2, deltaY / 2);
    painter.drawPixmap(pixmapRect, pixmap);
    painter.restore();
}

void StatusButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (buttonType == NORMALBTN || buttonType == SIDEBARBTN)
        Q_EMIT clicked();
    e->ignore();
}

void StatusButton::mousePressEvent(QMouseEvent *e)
{
    if (buttonType == NORMALBTN || buttonType == SIDEBARBTN)
        Q_EMIT clicked();
    e->ignore();
}
