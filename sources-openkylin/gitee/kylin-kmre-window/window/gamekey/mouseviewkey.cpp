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

#include "mouseviewkey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"

MouseViewKey::MouseViewKey(KmreWindow* window) :
    DirectTypeBaseKey(window, VIEW)
{
    mBgBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/view_hover.svg);}"
                           "QPushButton:focus{outline: none;}");
    connect(mCloseBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameKey(VIEW,0);
    });
    JoystickManager* manager = mMainWindow->getJoystickManager();
    if(!manager->isKeyMouseValueExist("↑")){
        mEditTop->setText("↑");
    }
    if(!manager->isKeyMouseValueExist("←")){
        mEditLeft->setText("←");
    }
    if(!manager->isKeyMouseValueExist("→")){
        mEditRight->setText("→");
    }
    if(!manager->isKeyMouseValueExist("↓")){
        mEditBottom->setText("↓");
    }
}

void MouseViewKey::enableEdit(bool enable)
{
    mBgBtn->setStyleSheet(QString("QPushButton{background:transparent; border-image: url(:/new/res/%1);}"
                            "QPushButton:focus{outline: none;}")
                            .arg(enable ? "view_hover.svg" : "view_normal.svg"));
    mEditTop->setEnabled(enable);
    mEditRight->setEnabled(enable);
    mEditBottom->setEnabled(enable);
    mEditLeft->setEnabled(enable);
    if(enable){
        mEditTop->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
        mEditRight->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
        mEditBottom->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
        mEditLeft->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
    }else{
        mEditTop->setStyleSheet("QLineEdit{background:transparent;color:rgba(255,255,255,1);}");
        mEditRight->setStyleSheet("QLineEdit{background:transparent;color:rgba(255,255,255,1);}");
        mEditBottom->setStyleSheet("QLineEdit{background:transparent;color:rgba(255,255,255,1);}");
        mEditLeft->setStyleSheet("QLineEdit{background:transparent;color:rgba(255,255,255,1);}");
    }

    mCloseBtn->setVisible(enable);
    m_enableEdit = enable;
}
