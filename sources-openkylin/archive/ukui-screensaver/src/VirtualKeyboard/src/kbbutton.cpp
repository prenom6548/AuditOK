/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "kbbutton.h"
#include "commondef.h"
#include <QDebug>
#include <QEvent>

KBButton::KBButton(QWidget *parent/* = nullptr*/)
    : QPushButton(parent)
{
    this->installEventFilter(this);
    this->setIconSize(KEYBOARD_FIXED_DEFAULT_ICONSIZE);
    this->setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, [&,this]() {
        if (m_charId.isLetter()) {
            if (m_isShift) {
                if (m_isCaps) {
                    m_charId = m_charId.toLower();
                } else {
                    m_charId = m_charId.toUpper();
                }
            } else {
                if (m_isCaps) {
                    m_charId = m_charId.toUpper();
                } else {
                    m_charId = m_charId.toLower();
                }
            }
        }
        Q_EMIT clicked(m_charId);
    });
    // 保存原始文本
    if (m_originalText.isEmpty()) {
        m_originalText = text();
    }
    updateElidedText();
}

KBButton::~KBButton()
{

}

bool KBButton::eventFilter(QObject *watched, QEvent *event)
{
    if (m_clrNormal == KEYBOARD_LETTER_COLOR_NORMAL)
        return QPushButton::eventFilter(watched, event);
    if (event->type() == QEvent::MouseButtonPress) {
        QPixmap pressIcon =  this->icon().pixmap(QSize(24, 24));
        pressIcon = drawSymbolicColoredPixmap(pressIcon, "white");
        this->setIcon(pressIcon);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QPixmap releaseIcon =  this->icon().pixmap(QSize(24, 24));
        releaseIcon = drawSymbolicColoredPixmap(releaseIcon, "black");
        this->setIcon(releaseIcon);
    }
    return QPushButton::eventFilter(watched, event);
}

void KBButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    updateElidedText();
}

void KBButton::setCharId(QChar charId)
{
    m_charId = charId;
    if (charId == '&') {
        setText("&&");
    } else {
        setText(m_charId);
    }
}

void KBButton::updateElidedText()
{
    if (m_originalText.isEmpty()) {
        m_originalText = text();
    }

    // 计算可用文本宽度（按钮宽度减去边距）
    int textWidth = width() - 20; // 左右各留10像素边距

    if (textWidth <= 0) {
        m_elidedText = "...";
        m_isTextElided = true;
        return;
    }

    QFontMetrics fm(font());
    QString elided = fm.elidedText(m_originalText, Qt::ElideRight, textWidth);

    m_elidedText = elided;
    m_isTextElided = (elided != m_originalText);

    // 如果文本被截断，设置工具提示；否则清除工具提示
    if (m_isTextElided) {
        setText(m_elidedText);
        setToolTip(m_originalText);
    } else {
        setText(m_originalText);
        setToolTip("");
    }
}

void KBButton::updateStyleSheet(QString clrNormal, QString clrHover, QString clrChecked, QString clrBoard, QString clrBoardPress, QString clrFontPress, int radius, bool is_lock)
{
    QString strBorderRadius = QString("border-top-left-radius: %1px; border-top-right-radius: %2px; border-bottom-right-radius: %3px;border-bottom-left-radius: %4px;")
                            .arg((radius&BORDER_RADIUS_LT)?8:0)
                            .arg((radius&BORDER_RADIUS_TR)?8:0)
                            .arg((radius&BORDER_RADIUS_RB)?8:0)
                            .arg((radius&BORDER_RADIUS_LB)?8:0);
    if (!is_lock) {
        setStyleSheet(QString("QPushButton{%1 border: 2px solid %2; border-width: 0px 0px 2px 0px; background: %3; color: #262626}"
                                    // "QPushButton:hover{border: 2px solid %4; border-width: 2px 0px 0px 0px; background: %5;}"
                                     "QPushButton:pressed{border: 2px solid %4; border-width: 2px 0px 0px 0px; background: %5; color: %6}"
                                     "QPushButton:checked{border: 2px solid %7; border-width: 2px 0px 0px 0px; background: %8; color: %9}")
                                .arg(strBorderRadius).arg(clrBoard).arg(clrNormal).arg(clrBoardPress).arg(clrHover).arg(clrFontPress).arg(clrBoardPress).arg(clrChecked).arg(clrFontPress)
                            );
    } else {
        setStyleSheet(QString("QPushButton{%1 border: 2px solid %2; border-width: 0px 0px 2px 0px; background: %3; color: #FFFFFF}"
                                    // "QPushButton:hover{border: 2px solid %4; border-width: 2px 0px 0px 0px; background: %5;}"
                                     "QPushButton:pressed{border: 2px solid %4; border-width: 2px 0px 0px 0px; background: %5; color: %6}"
                                     "QPushButton:checked{border: 2px solid %7; border-width: 2px 0px 0px 0px; background: %8; color: %9}")
                                .arg(strBorderRadius).arg(clrBoard).arg(clrNormal).arg(clrBoardPress).arg(clrHover).arg(clrFontPress).arg(clrBoardPress).arg(clrChecked).arg(clrFontPress)
                            );
    }
    m_clrBoard = clrBoard;
    m_clrNormal = clrNormal;
    m_clrBoardPress = clrBoardPress;
    m_clrHover = clrHover;
    m_clrChecked = clrChecked;
    if (m_clrNormal == KEYBOARD_LETTER_COLOR_NORMAL) {
        sysFont.setPixelSize(32);
        this->setFont(sysFont);
        this->setIconSize(QSize(32, 32));
    } else {
        sysFont.setPixelSize(24);
        this->setFont(sysFont);
        this->setIconSize(QSize(24, 24));
    }
}

QPixmap KBButton::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ( "white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if( "black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray"== cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void KBButton::setShiftState(bool isShift)
{
    if (m_isShift != isShift) {
        if (m_charId.isLetter()) {
            if (isShift) {
                setText(m_charId.toUpper());
            } else {
                setText(m_charId.toLower());
            }
        }
        m_isShift = isShift;
    }
}

void KBButton::setCapsStatus(bool isCaps)
{
    m_isCaps = isCaps;
    setShiftState(m_isCaps);
}

void KBButton::setCtrlState(bool isCtrl)
{

}

void KBButton::setAltState(bool isAlt)
{

}
