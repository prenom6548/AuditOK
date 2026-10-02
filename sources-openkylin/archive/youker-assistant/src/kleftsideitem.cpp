/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kleftsideitem.h"

#include <QIcon>
#include <QPixmap>
#include <QVariant>
#include <QEvent>

#include "commondef.h"
#include "utils.h"
#include "imageutil.h"

KLeftSideItem::KLeftSideItem(QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = "";
    m_strText = "";
    this->setFlat(true);
    initUI();
}

KLeftSideItem::KLeftSideItem(QString strText, QString strIcon, QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = strIcon;
    m_strText = strText;
    this->setFlat(true);
    initUI();
}

KLeftSideItem::~KLeftSideItem()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void KLeftSideItem::initUI()
{
    installEventFilter(this);
    setProperty("useButtonPalette", true);
    m_layoutMain = new QHBoxLayout();
    m_layoutMain->setContentsMargins(16,6,16,6);
    m_layoutMain->setSpacing(8);

    m_labelIcon = new QLabel();
    m_layoutMain->addWidget(m_labelIcon);

    m_labelText = new QLabel();
    setText(m_strText);
    m_layoutMain->addWidget(m_labelText);

    m_layoutMain->addStretch();
    this->setLayout(m_layoutMain);
    initStyleTheme();
    setIcon(m_strIcon);

    connect(this, &QPushButton::toggled, this, [=](bool checked) {
        if (checked) {
            setStyleSheet("QPushButton:checked{background-color: palette(highlight);border-radius: 6px;}");
            m_labelText->setStyleSheet("color:white");
            m_isNormal = false;
        } else {
            m_labelText->setStyleSheet("color:palette(windowText)");
            if (!m_isHover) {
                m_isNormal = true;
            }
        }
        setIcon(m_strIcon);
    });
}

void KLeftSideItem::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                auto styleNameValue = m_styleSettings->get("styleName");
                if (styleNameValue.isValid()) {
                    m_strThemeName = styleNameValue.toString();
                    setIcon(m_strIcon);
                }
            } else if (key == "systemFontSize" || key == "systemFont") {
                auto styleFontSizeValue = m_styleSettings->get("systemFontSize");
                if (styleFontSizeValue.isValid()) {
                    this->setText(m_strText);
                }
            }
        });
        auto styleNameValue = m_styleSettings->get("styleName");
        if (styleNameValue.isValid()) {
            m_strThemeName = styleNameValue.toString();
        }
        auto styleFontSizeValue = m_styleSettings->get("systemFontSize");
        if (styleFontSizeValue.isValid()) {
            this->setText(m_strText);
        }
    }
}

void KLeftSideItem::setText(QString &strText)
{
    m_strText = strText;
    if (!m_strText.isEmpty()) {
        QString ShowValue = getElidedText(m_labelText->font(), m_strText, width()-60); // 16+16+8+16 + 4
        m_labelText->setText(ShowValue);
        if (ShowValue != m_strText) {
            setToolTip(m_strText);
        } else {
            setToolTip("");
        }
        m_labelText->show();
    } else {
        m_labelText->hide();
    }
}

void KLeftSideItem::setIcon(QString &strIcon)
{
    m_strIcon = strIcon;
    QPixmap pixmap;
    if (m_strThemeName == "ukui-dark" || m_strThemeName == "ukui-black") {
        if (m_isNormal) {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        } else {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        }
    } else { // "ukui-default" "ukui-white" "ukui-light" "ukui"
        if (m_isNormal) {
            pixmap = ImageUtil::loadSvg(m_strIcon, "black", 16);
        } else {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        }
    }
    if (!pixmap.isNull()) {
        m_labelIcon->setPixmap(pixmap);
        m_labelIcon->show();
    } else {
        m_labelIcon->hide();
    }
}

bool KLeftSideItem::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        switch (event->type()) {
        case QEvent::HoverEnter:
            m_isHover = true;
            setIcon(m_strIcon);
            break;
        case QEvent::HoverLeave:
            m_isHover = false;
            setIcon(m_strIcon);
            break;
        default:
            break;
        }
    }
    return QPushButton::eventFilter(obj, event);
}
