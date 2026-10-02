/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "maskwindow.h"
#include <QGuiApplication>
#include <QHBoxLayout>                                                                                                                                                       
#include <QVBoxLayout>                                                                                                                                                       
#include <QLabel>
#include <syslog.h>                                                                                                                                                         
                                                                                                                                                                             
class CornerMask : public QLabel                                                                                                                                             
{                                                                                                                                                                            
public:                                                                                                                                                                      
    CornerMask(int width, int height, QWidget *parent)                                                                                                                       
        : QLabel(parent)                                                                                                                                                     
    {                                                                                                                                                                        
        setFixedSize(width, height);                                                                                                                                         
        setStyleSheet("QLabel{background-color:rgb(0,255,0);}");                                                                                                             
    }                                                                                                                                                                        
};

MaskWindow::MaskWindow()                                                                                                                                                     
    : QWidget(nullptr)                                                                                                                                                       
{                                                                                                                                                                            
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);                                                                      
    setAttribute(Qt::WA_TranslucentBackground);                                                                                                                              
                                                                                                                                                                             
    QHBoxLayout *hLayout1 = new QHBoxLayout();                                                                                                                               
    hLayout1->setMargin(0);                                                                                                                                                  
    hLayout1->addWidget(new CornerMask(100, 5, this), 1, Qt::AlignLeft);                                                                                                     
    hLayout1->addStretch();                                                                                                                                                  
    hLayout1->addWidget(new CornerMask(100, 5, this), 1, Qt::AlignRight);                                                                                                    
                                                                                                                                                                             
    QHBoxLayout *hLayout2 = new QHBoxLayout();                                                                                                                               
    hLayout2->setMargin(0);                                                                                                                                                  
    hLayout2->addWidget(new CornerMask(5, 95, this), 1, Qt::AlignLeft);                                                                                                      
    hLayout2->addStretch();                                                                                                                                                  
    hLayout2->addWidget(new CornerMask(5, 95, this), 1, Qt::AlignRight);                                                                                                     
                                                                                                                                                                             
    QHBoxLayout *hLayout3 = new QHBoxLayout();                                                                                                                               
    hLayout3->setMargin(0);                                                                                                                                                  
    hLayout3->addWidget(new CornerMask(5, 95, this), 1, Qt::AlignLeft);                                                                                                      
    hLayout3->addStretch();                                                                                                                                                  
    hLayout3->addWidget(new CornerMask(5, 95, this), 1, Qt::AlignRight);                                                                                                     

    QHBoxLayout *hLayout4 = new QHBoxLayout();
    hLayout4->setMargin(0);
    hLayout4->addWidget(new CornerMask(100, 5, this), 1, Qt::AlignLeft);
    hLayout4->addStretch();
    hLayout4->addWidget(new CornerMask(100, 5, this), 1, Qt::AlignRight);

    QVBoxLayout *layout= new QVBoxLayout(); 
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(hLayout1);
    layout->addLayout(hLayout2);
    layout->addStretch();
    layout->addLayout(hLayout3);
    layout->addLayout(hLayout4);

    setLayout(layout);
}

MaskWindow::~MaskWindow()
{
}

void MaskWindow::show(int x, int y, int w, int h)
{
    if ((x < 0) || (y < 0) || (w <= 0) || (h <= 0)) {
        syslog(LOG_ERR, "[%s] Show MaskWindow failed! Invalid param!", __func__);
        return;
    }

    //showFullScreen();
    move(x, y);
    setFixedSize(w, h);

    static_cast<QWidget*>(this)->show();
}