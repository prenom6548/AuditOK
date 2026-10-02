/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "myprobar.h"
#include <QVBoxLayout>
#include <QImage>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIcon>
myprobar::myprobar(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lay=new QVBoxLayout;
    this->setLayout(lay);
    lay->setContentsMargins(0,20,0,20);
    lay->setMargin(0);

    percent=new QLabel;
    percent->setObjectName("percent");
    percent->setMaximumSize(64,37);
    percent->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    percentlay=new QHBoxLayout;
    percentlay->setContentsMargins(0,0,0,0);
    percentlay->setMargin(0);
    perspace=new QSpacerItem(1,10, QSizePolicy::Minimum, QSizePolicy::Preferred);
    percentlay->addSpacerItem(perspace);
    percentlay->addWidget(percent,0,Qt::AlignBottom);
    percentlay->addSpacerItem(new QSpacerItem(1,10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    lay->addLayout(percentlay);
    percent->setVisible(true);

    bar=new QProgressBar;
    bar->setObjectName("mybar");
    bar->setMaximumHeight(10);
    bar->setMinimumHeight(10);
    bar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
//    bar->setFixedWidth(this->width());
    bar->setTextVisible(false);

    QHBoxLayout* barlayout=new QHBoxLayout;
    //barlayout->addSpacerItem(new QSpacerItem(percent->width()/2,10, QSizePolicy::Fixed, QSizePolicy::Fixed));
    barlayout->addSpacerItem(new QSpacerItem(0,10, QSizePolicy::Fixed, QSizePolicy::Fixed));
    barlayout->addWidget(bar,0,Qt::AlignTop);
    lay->addLayout( barlayout);
    this->show();

    bar->setStyleSheet("QProgressBar::chunk {\
                       border-radius: 5px;\
                       background:qlineargradient(spread:pad, x1:0,y1:0,x2:1,y2:0,stop:0 #01FAFF,stop:1 #26B4FF);\
                        }"
                       " QProgressBar {\
                        height: 10px;\
                        border-radius: 5px;\
                        background-color: #C2CCD0;}");


}

void myprobar::setValue(int Value)
{
    percent->setVisible(true);
    //perspace->changeSize(bar->width()*Value/100,10);
    if( bar->width()*Value/100 >= perspace->sizeHint().width() && bar->width()*Value/100 >=percent->width()/2 )
    {
        perspace->changeSize(bar->width()*Value/100 - percent->width()/2,10);
    }

    QIcon pg(":/data/png/percent.svg");
    QPixmap pix=pg.pixmap(64,37)/*pg.scaled(QSize(64,37),Qt::KeepAspectRatio)*/;
    QPainter painter(&pix);
    painter.begin(&pix);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(12);
    font.setFamily("Noto Sans CJK SC");
    painter.setFont(font);
    QString display=QString("%1%").arg(Value);
    QRect rect(0,0,64,28);
    painter.drawText(rect, Qt::AlignCenter, display.toStdString().c_str());
    percent->setPixmap(pix);
    bar->setValue(Value);
    update();
}

void myprobar::resizeEvent(QResizeEvent *event)
{
    bar->setFixedWidth(this->width());//bug161672
}
