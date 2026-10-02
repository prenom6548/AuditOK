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
#include "washcar.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
washcar::washcar(QWidget *parent) : QWidget(parent)
{
    m_washcarLayout = new QVBoxLayout(this);

    m_washcarIcon = new QLabel(this);
    m_washcarIcon->setAccessibleName("kylin-weather_QLabel_washcarIcon");
    m_str = new QLabel(this);
    m_str->setAccessibleName("kylin-weather_QLabel_washcarStr");
    m_degree = new QLabel(this);
    m_degree->setAccessibleName("kylin-weather_QLabel_washcarDegree");

    QPixmap washcarpixmap(":/res/contorl_icons/index_cash_wash.png");
    m_washcarIcon->setPixmap(washcarpixmap);
    m_washcarIcon->setAlignment(Qt::AlignCenter);
    m_str->setText(tr("Wash Car"));
    m_str->setAlignment(Qt::AlignCenter);
    m_degree->setText("N/A");
    m_degree->setAlignment(Qt::AlignCenter);
    initLayout();
    //    initQss();
}

void washcar::setItemContent(QString text)
{
    m_degree->setText(text);
}

void washcar::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(14.0);
}

void washcar::initLayout()
{
    m_washcarLayout->addWidget(m_washcarIcon);
    m_washcarLayout->addWidget(m_str);
    m_washcarLayout->addWidget(m_degree);
}

void washcar::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = 11;                              //魔鬼数字，自行处理
    double nowFontSize = lable2BaseFontSize * double(size) / 11; // 11为系统默认大小，魔鬼数字，自行处理
    //    QFont font;
    //    font.setPointSizeF(nowFontSize);
    //    ui->label2->setFont(font);
    initQss(nowFontSize);
}

void washcar::initQss(double nowFontSize)
{
    m_washcarIcon->setAlignment(Qt::AlignCenter);
   
    m_washcarIcon->resize(30, 30);
    QPixmap washcarpixmap(washcarpath);
    m_washcarIcon->setPixmap(washcarpixmap);

    m_str->setStyleSheet("color: white;");
    QFont m_font;
    m_str->setFont(m_font);
    m_str->setAlignment(Qt::AlignCenter);
    //    设置字体透明度
    QGraphicsOpacityEffect *washcarOpacityEffect = new QGraphicsOpacityEffect;
    washcarOpacityEffect->setOpacity(0.7);
    m_str->setGraphicsEffect(washcarOpacityEffect);
    m_degree->setStyleSheet("color: white;");
    m_degree->setAlignment(Qt::AlignCenter);
    m_degree->setFont(m_font);

}
