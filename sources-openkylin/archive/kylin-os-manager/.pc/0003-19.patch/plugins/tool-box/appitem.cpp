/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

#include <gsettingmonitor.h>

#include "appitem.h"

AppItem::AppItem(QWidget *parent) : QWidget(parent)
{
    m_theme = Utils::getTheme();
    m_fontSize = Utils::getFontSize();

    // m_icon = new QLabel(this);
    m_icon = new QPushButton(this);
    m_icon->setFixedSize(48, 48);
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_icon->setStyleSheet(btnStyle);

    m_text = new QLabel(this);

    m_describe = new QLabel(this);
    m_describe->setFixedWidth(260);
    QPalette pal(m_describe->palette());
    pal.setColor(QPalette::Text, QColor("#8F9399"));
    m_describe->setPalette(pal);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addStretch();
    vLayout->addWidget(m_text);
    vLayout->addWidget(m_describe);
    vLayout->addStretch();
    vLayout->setAlignment(m_text, Qt::AlignLeft);
    vLayout->setAlignment(m_describe, Qt::AlignLeft);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(20, 20, 20, 20);
    hLayout->setSpacing(0);
    hLayout->addWidget(m_icon);
    hLayout->addSpacing(12);
    hLayout->addLayout(vLayout);

    setStyle();
    setLayout(hLayout);

    initGsettings();
}

AppItem::~AppItem() = default;

void AppItem::setIcon(QIcon icon)
{
    m_icon->setIcon(icon);
    m_icon->setIconSize(QSize(48, 48));

    return;
}

void AppItem::setText(QString text)
{
    QFont font;
    font.setPixelSize(16 * m_fontSize / 11);
    // font.setBold(true);
    m_text->setFont(font);
    m_text->setText(text);

    return;
}

void AppItem::setDescribe(QString descri)
{
    QFont font;
    font.setPixelSize(14 * m_fontSize / 11);
    m_describe->setFont(font);
    m_describe->setWhatsThis(descri);
    QString adaptionText = Utils::getSelfAdaptionText(m_describe, descri);
    if (adaptionText.isEmpty()) {
        m_describe->setText(descri);
    } else {
        m_describe->setText(adaptionText);
        m_describe->setToolTip(descri);
    }

    return;
}

void AppItem::setStayHope(void)
{
    m_icon->setStyleSheet("QLabel{background:rgba(191,191,191,1);border-radius:6px;}");
    setIcon(QIcon(QPixmap(":/res/ubuntukylin-software-center.png")));
    setText(tr("coming soon"));
    setDescribe(tr("More tools are coming soon"));

    return;
}

void AppItem::initGsettings(void)
{
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        m_theme = Utils::getTheme();
        setStyle();
    });

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        m_fontSize = Utils::getFontSize();

        QFont font;
        font.setPixelSize(16 * m_fontSize / 11);
        m_text->setFont(font);

        QFont font1;
        font1.setPixelSize(14 * m_fontSize / 11);
        m_describe->setFont(font1);
        QString adaptionText = Utils::getSelfAdaptionText(m_describe, m_describe->whatsThis());
        if (adaptionText.isEmpty()) {
            m_describe->setText(m_describe->whatsThis());
            m_describe->setToolTip("");
        } else {
            m_describe->setText(adaptionText);
            m_describe->setToolTip(m_describe->whatsThis());
        }
    });

    return;
}

void AppItem::setStyle()
{
    if (m_theme == Theme::Dark) {

    } else {
    }

    return;
}
