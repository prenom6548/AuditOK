/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include <QFont>
#include <QPixmap>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include "basicbutton.h"

#include "data_warehouse.h"
#include "widgetstyle.h"

// 设置按钮初始样式
BasicButton::BasicButton(QWidget *parent) : QPushButton(parent) {}

// 设置按钮内容
void BasicButton::setText(const QString &text)
{
    this->btnText = text;
}

// 获取按钮内容
QString BasicButton::getText()
{
    return this->btnText;
}

// 获取按钮内容
QString BasicButton::text()
{
    return this->btnText;
}

IntelModeButton::IntelModeButton(QWidget *parent) : QPushButton(parent)
{
    this->setFlat(true);
    this->m_status = false;
    this->setStyleSheet("QPushButton{background-color:transparent;border-radius:10px;border:0px}"
                        "QPushButton:hover{background-color:#FB7054;}"
                        "QPushButton:pressed{background-color:#FB7054;}");
}

IntelModeButton::~IntelModeButton() {}

void IntelModeButton::init(QString text)
{
    this->setFixedHeight(50);

    this->m_text = new QLabel();
    m_text->setAccessibleName("kylin-calculator_QLabel_m_text");
    QFont font;
    font.setPixelSize(16);
    this->m_text->setFont(font);
    this->m_text->setText(text);

    this->m_icon = new QLabel();
    m_icon->setAccessibleName("kylin-calculator_QLabel_m_icon");
    this->m_icon->setFixedSize(14, 12);

    this->m_hlayout = new QHBoxLayout();
    this->m_hlayout->setMargin(0);
    this->m_hlayout->addStretch(0);
    this->m_hlayout->addWidget(this->m_text);
    this->m_hlayout->addStretch(0);
    this->m_hlayout->addWidget(this->m_icon);
    this->m_hlayout->addStretch(0);

    this->setLayout(this->m_hlayout);

    return;
}

void IntelModeButton::select(void)
{
    if (this->m_status == false) {
        QPixmap icon(":/image/intelStandLight/selected.png");
        icon.scaled(14, 12);
        this->m_icon->setScaledContents(true);
        this->m_icon->setPixmap(icon);

        this->setStyleSheet("QPushButton{background-color:#FB7054;border-radius:10px;}");
        this->m_text->setStyleSheet("color:white");

        this->m_status = true;
    }

    return;
}

void IntelModeButton::deselect(void)
{
    if (this->m_status == true) {
        QPixmap icon;
        this->m_icon->setPixmap(icon);
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            this->setStyleSheet("QPushButton{background-color:transparent;border-radius:10px;}");
            this->m_text->setStyleSheet("color:black");
        } else {
            this->setStyleSheet("QPushButton{background-color:transparent;border-radius:10px;}");
            this->m_text->setStyleSheet("color:white");
        }

        this->m_status = false;
    }

    return;
}

bool IntelModeButton::getStatus()
{
    return this->m_status;
}

void IntelModeButton::setTextColor(QString color)
{
    this->m_text->setStyleSheet("color:" + color);
}

IntelModeList::IntelModeList(QWidget *parent) : QWidget(parent)
{
    this->init();
    this->establishInterrupt();
}

IntelModeList::~IntelModeList() {}

void IntelModeList::init(void)
{
    this->setFixedSize(120, 110);

    this->m_standard = new IntelModeButton(this);
    m_standard->setAccessibleName("kylin-calculator_IntelModeButton_m_standard");
    this->m_standard->init(tr("standard"));

    this->m_science = new IntelModeButton(this);
    m_science->setAccessibleName("kylin-calculator_IntelModeButton_m_science");
    this->m_science->init(tr("scientific"));

    this->m_vlayout = new QVBoxLayout();
    this->m_vlayout->setMargin(4);
    this->m_vlayout->addWidget(this->m_standard);
    this->m_vlayout->addStretch(0);
    this->m_vlayout->addWidget(this->m_science);
    this->m_vlayout->addSpacing(4);

    this->setLayout(this->m_vlayout);

    return;
}

void IntelModeList::establishInterrupt(void)
{
    connect(this->m_standard, &IntelModeButton::clicked, this, &IntelModeList::slotStandardClick);
    connect(this->m_science, &IntelModeButton::clicked, this, &IntelModeList::slotScienceClick);

    return;
}

void IntelModeList::setWidgetStyle()
{
    if (this->m_standard == nullptr || this->m_science == nullptr) {
        return;
    }
    if (this->m_standard->getStatus()) {
        this->m_standard->setStyleSheet("QPushButton{background-color:#FB7054;border-radius:10px;}");
        this->m_standard->setTextColor("white");
        this->m_science->setStyleSheet("QPushButton{background-color:transparent;border-radius:10px;}");
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            this->m_science->setTextColor("black");
        } else {
            this->m_science->setTextColor("white");
        }
    }
    if (this->m_science->getStatus()) {
        this->m_science->setStyleSheet("QPushButton{background-color:#FB7054;border-radius:10px;}");
        this->m_science->setTextColor("white");
        this->m_standard->setStyleSheet("QPushButton{background-color:transparent;border-radius:10px;}");
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            this->m_standard->setTextColor("black");
        } else {
            this->m_standard->setTextColor("white");
        }
    }
}

void IntelModeList::slotStandardClick(void)
{
    this->m_standard->select();
    this->m_science->deselect();

    return;
}

void IntelModeList::slotScienceClick(void)
{
    this->m_science->select();
    this->m_standard->deselect();

    return;
}

void IntelModeList::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 12, 12);

    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        p.fillPath(rectPath, QColor("#FFFFFF"));
    } else {
        p.fillPath(rectPath, QColor("#3A3A3D"));
    }

    return;
}
