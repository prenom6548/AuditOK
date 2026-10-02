/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#include "toast.h"

#include <QDebug>

Toast::Toast(QWidget* parent):QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);

    mTimer = new QTimer(this);
    connect(mTimer, &QTimer::timeout, this, &Toast::OnTimer);
}

Toast::~Toast(){
    deleteLater();
}

Toast* Toast::getToast(){
    static Toast toast;
    return &toast;
}

void Toast::setParentAttr(QWidget* w){
    setParent(w);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void Toast::setMessage(const QString str){

    QFontMetrics fm1(font());
    this->setFixedSize(fm1.width(str) + 30, 40);

    QWidget* w = parentWidget();
    if(width()>w->width()){
        setFixedSize(w->width()-60,80);
        label->setWordWrap(true);
    }else{
        label->setWordWrap(false);
    }
    label->setFixedSize(this->width(),this->height());
    label->setText(str);

    QRect rect = w->geometry();

    move(rect.x()+(w->width()-this->width())/2,rect.y()+(w->height()-this->height())/2);
    setVisible(true);
    mnTransparent = 200;
    mTimer->start(30);
}

void Toast::OnTimer()
{
    mnTransparent -= 2;
    if (mnTransparent > 0)
    {
        setStyleSheet(QString("color:rgba(0, 0, 0, %1);font:16px \"Microsoft YaHei\";border-radius:5px;background-color:rgba(164, 164, 164, %2);").arg(mnTransparent).arg(mnTransparent));
    }
    else
    {
        mTimer->stop();
        setVisible(false);
    }
}




