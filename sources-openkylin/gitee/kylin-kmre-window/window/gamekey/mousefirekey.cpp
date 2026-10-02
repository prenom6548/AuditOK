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

#include "mousefirekey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"

#include <QDebug>

MouseFireKey::MouseFireKey(KmreWindow* window) 
    : JoystickSingleBaseKey(window, FIRE)
{ 
    mCloseBtn = new QPushButton(this);
    mCloseBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mCloseBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                              "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    mCloseBtn->move(this->width() - DEFAULT_DELETE_KEY, 0);
    connect(mCloseBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameKey(FIRE, mIndex);
    });

    mBgBtn = new QPushButton(this);
    mBgBtn->resize(this->width()-mCloseBtn->width(),this->height()-mCloseBtn->height());
    mBgBtn->move(0,mCloseBtn->height());
    mBgBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/fire_hover.svg);}"
                          "QPushButton:focus{outline: none;}");
    mBgBtn->setMouseTracking(true);
    mBgBtn->installEventFilter(this);

    this->installEventFilter(this);
}

void MouseFireKey::enableEdit(bool enable){
    mBgBtn->setStyleSheet(QString("QPushButton{background:transparent; border-image: url(:/new/res/%1);}"
                            "QPushButton:focus{outline: none;}")
                            .arg(enable ? "fire_hover.svg" : "fire_normal.svg"));
    mCloseBtn->setVisible(enable);
    m_enableEdit = enable;
}
void MouseFireKey::updateSize(bool update_panel){
    Q_UNUSED(update_panel);
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);

    this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
    mBgBtn->setGeometry(0,mCloseBtn->height(),this->width()-mCloseBtn->width(),this->height()-mCloseBtn->height());
}

void MouseFireKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+mBgBtn->width()/2;
    event_coordinate_y = pos.y()+mCloseBtn->height()+mBgBtn->height()/2;
}


