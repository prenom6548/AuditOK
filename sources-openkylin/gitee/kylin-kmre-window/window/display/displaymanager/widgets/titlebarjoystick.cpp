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

#include "titlebarjoystick.h"

#include <QHBoxLayout>

TitleBarJoystick::TitleBarJoystick(KmreWindow* window) 
    : TitleBar(window)
{
    initUI();
}
TitleBarJoystick::~TitleBarJoystick(){

}


void TitleBarJoystick::initUI(){
    QHBoxLayout* mLayout = new QHBoxLayout(this);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);

    QWidget* w = new QWidget;
    QHBoxLayout* mLeftLayout = new QHBoxLayout(w);
    mLeftLayout->setContentsMargins(8, 0, 0, 0);
    mLeftLayout->setSpacing(8);

    m_iconLabel = new QLabel;
    mLeftLayout->addWidget(m_iconLabel);
    m_iconLabel->setFixedSize(24, 24);

    m_titleLabel = new QLabel;
    mLeftLayout->addWidget(m_titleLabel);

    m_exitBtn = new QPushButton;
    m_exitBtn->setFlat(true);
    m_exitBtn->setText(tr("Exit"));
    connect(m_exitBtn,&QPushButton::clicked,this,[this](){
        emit exitJoystickSettings();
    });

    //    mExitBtn->setText(tr("退出设置"));
    //    mExitBtn->setFont(QFont("宋体",10));
    //    mExitBtn->setFlat(true);
    //    mExitBtn->setFixedHeight(DEFAULT_TITLEBAR_HEIGHT);
    //    mExitBtn->setFixedWidth(100);

    mLayout->addWidget(w, 1, Qt::AlignLeft);
    mLayout->addStretch();
    mLayout->addWidget(m_exitBtn);

    this->setLayout(mLayout);
}

void TitleBarJoystick::initConnect(){

}

void TitleBarJoystick::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void TitleBarJoystick::setIcon(const QString &iconName)
{
    //const QIcon &icon = QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    const QIcon &icon = QIcon(iconName);
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(m_iconLabel->size()));
    }
}


