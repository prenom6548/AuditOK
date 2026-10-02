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
#include "modeButton.h"

#include <QIcon>
#include <QPixmap>
#include <QVariant>
#include <QEvent>
#include <QDebug>

ModeButton::ModeButton(QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = "";
    m_strText = "";
    this->setFlat(true);
    initUI();
    initStyleTheme();
}

ModeButton::ModeButton(QString strIcon, QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = strIcon;
    this->setFlat(true);
    initUI();
    initStyleTheme();
}

ModeButton::~ModeButton()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void ModeButton::initUI()
{
    installEventFilter(this);
    setModeIcon();
    setFixedSize(QSize(24, 24));
    setModeIcon();
    setCursor(Qt::ArrowCursor);
    setStyleSheet("QPushButton::pressed{border:none;background-color:transparent}"
                                    "QPushButton::hover::!pressed{border:none;background-color:transparent}");

    connect(this, &QPushButton::clicked, this, [=](){
        if (!pwdShow) {
            pwdShow = true;
            setModeIcon();
        } else {
            pwdShow = false;
            setModeIcon();
        }
    });
}

void ModeButton::initStyleTheme()
{
    const QByteArray idd("org.ukui.style");
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                auto styleNameValue = m_styleSettings->get("styleName");
                if (styleNameValue.isValid()) {
                    m_strThemeName = styleNameValue.toString();
                    setModeIcon();
                }
            }
        });
        auto styleNameValue = m_styleSettings->get("styleName");
        if (styleNameValue.isValid()) {
            m_strThemeName = styleNameValue.toString();
        }
    }
}

void ModeButton::setModeIcon()
{
    QIcon icon;
    QPixmap pixmap;
    if(pwdShow){
        icon = QIcon::fromTheme("ukui-eye-display-symbolic");
        pixmap = icon.pixmap(QSize(24,24));
    } else{
        icon = QIcon::fromTheme("ukui-eye-hidden-symbolic");
        pixmap = icon.pixmap(QSize(24,24));
    }
    if(m_isHover)
        pixmap = drawSymbolicColoredPixmap(pixmap, "white");
    else
        pixmap = drawSymbolicColoredPixmap(pixmap, "gray");
    if(m_strThemeName == "ukui-dark" || m_strThemeName == "ukui-black")
        this->setIcon(pixmap);
    else
        this->setIcon(icon);
}

bool ModeButton::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this){
        if(event->type() == QEvent::Enter){
            m_isHover = true;
            setModeIcon();
        } else if(event->type() == QEvent::Leave){
            m_isHover = false;
            setModeIcon();
        }
    }
    return QPushButton::eventFilter(obj, event);
}

QPixmap ModeButton::drawSymbolicColoredPixmap(QPixmap &source, QString cgColor)
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

bool ModeButton::isShowPwd()
{
    return pwdShow;
}
