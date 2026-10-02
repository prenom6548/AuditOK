/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "flu.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
flu::flu(QWidget *parent) : QWidget(parent)
{
    m_fluLayout = new QVBoxLayout(this);

    m_fluIcon = new QLabel(this);
    m_fluIcon->setAccessibleName("kylin-weather_QLabel_fluIcon");
    m_str = new QLabel(this);
    m_str->setAccessibleName("kylin-weather_QLabel_fluStr");
    m_degree = new QLabel(this);
    m_degree->setAccessibleName("kylin-weather_QLabel_fluDegree");

    QPixmap flupixmap(":/res/contorl_icons/index_flu.png");
    m_fluIcon->setPixmap(flupixmap);
    m_fluIcon->setAlignment(Qt::AlignCenter);
    m_str->setText(tr("Flu"));
    m_str->setAlignment(Qt::AlignCenter);
    m_degree->setText("N/A");
    m_degree->setAlignment(Qt::AlignCenter);
    initLayout();
    //    initQss();
}

void flu::setItemContent(QString text)
{
    m_degree->setText(text);
}

void flu::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(14.0);
}

void flu::initLayout()
{
    m_fluLayout->addWidget(m_fluIcon);
    m_fluLayout->addWidget(m_str);
    m_fluLayout->addWidget(m_degree);
}

void flu::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = 11;                              //魔鬼数字，自行处理
    double nowFontSize = lable2BaseFontSize * double(size) / 11; // 11为系统默认大小，魔鬼数字，自行处理
    //    QFont font;
    //    font.setPointSizeF(nowFontSize);
    //    ui->label2->setFont(font);
    initQss(nowFontSize);
}
void flu::initQss(double nowFontSize)
{
    m_fluIcon->setAlignment(Qt::AlignCenter);

    m_fluIcon->resize(30, 30);
    QPixmap flupixmap(flupath);
    m_fluIcon->setPixmap(flupixmap);

    m_str->setStyleSheet("color: white;");
    QFont m_font;
    m_str->setFont(m_font);
    m_str->setAlignment(Qt::AlignCenter);
    //    设置字体透明度
    QGraphicsOpacityEffect *fluOpacityEffect = new QGraphicsOpacityEffect;
    fluOpacityEffect->setOpacity(0.7);
    m_str->setGraphicsEffect(fluOpacityEffect);
    m_degree->setStyleSheet("color: white;");
    m_degree->setAlignment(Qt::AlignCenter);
    m_degree->setFont(m_font);
}
