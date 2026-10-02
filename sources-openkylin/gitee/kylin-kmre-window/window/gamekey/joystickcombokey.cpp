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

#include "joystickcombokey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"

#include <QDebug>

JoystickComboKey::JoystickComboKey(KmreWindow* window):
    JoystickSingleBaseKey(window, COMBOKEY)
{
    mCloseBtn = new QPushButton(this);
    mCloseBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mCloseBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                              "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
    connect(mCloseBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameKey(COMBOKEY,mIndex);
    });

    mInputEdit = new QLineEdit(this);
    int editRectLength = this->height()-DEFAULT_DELETE_KEY-DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT-DEFAULT_COMBO_KEY_MARGIN;
    mInputEdit->resize(editRectLength,editRectLength);
    mInputEdit->move((this->width()-mInputEdit->width())/2,mCloseBtn->height());
    mInputEdit->setFocusPolicy(Qt::ClickFocus);
    mInputEdit->setAlignment(Qt::AlignCenter);
    mInputEdit->setContextMenuPolicy(Qt::NoContextMenu);
    mInputEdit->setAttribute(Qt::WA_InputMethodEnabled,false);
    mInputEdit->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/combo_key_hover.svg);color:rgba(255,255,255,1);}");

    mSensiWidget = new SensitivityWidget(this);
    mSensiWidget->resize(this->width(),DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT);
    mSensiWidget->move(0,this->height()-DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT);
    connect(mSensiWidget,SIGNAL(currentSliderValue(int)),this,SLOT(updateComboFreq(int)));

    this->installEventFilter(this);
    mInputEdit->installEventFilter(this);
}

void JoystickComboKey::enableEdit(bool enable){
    if(enable){
        mInputEdit->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/combo_key_hover.svg);color:rgba(255,255,255,1);}");
    }else{
        mInputEdit->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/combo_key_normal.svg);color:rgba(255,255,255,1);}");
    }
    mCloseBtn->setVisible(enable);
    mSensiWidget->setVisible(enable);
    mInputEdit->setEnabled(enable);
    mInputEdit->setSelection(mKeyString.length(), mKeyString.length());// 取消文本选中状态
    m_enableEdit = enable;
}
void JoystickComboKey::updateSize(bool update_panel) {
    Q_UNUSED(update_panel);
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);

    this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
    int editRectLength = this->height()-DEFAULT_DELETE_KEY-DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT-DEFAULT_COMBO_KEY_MARGIN;
    mInputEdit->setGeometry((this->width()-editRectLength)/2,mCloseBtn->height(),editRectLength,editRectLength);
    mSensiWidget->setGeometry(0,this->height()-DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT,this->width(),DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT);
}

void JoystickComboKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+this->width()/2;
    event_coordinate_y = pos.y()+mCloseBtn->height()+mInputEdit->height()/2;
}

void JoystickComboKey::updateComboFreq(int freq){
    this->mComboFreq = freq;
}

void JoystickComboKey::setComboFreq(int freq){
    mSensiWidget->setSliderLabelDefault(freq);
}


