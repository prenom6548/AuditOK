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
#include "allowlocation.h"
#include "global/corevar.h"
#include <windowmanage.hpp>
#include "ukuiwindowhelper.h"
AllowLocation::AllowLocation(QWidget *parent) : QDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    //    this->setWindowFlag(Qt::Tool);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    //    this->setWindowFlag(Qt::FramelessWindowHint);

    //主题框架
    UkuiWindowHelper *helper = new UkuiWindowHelper(this);
    helper->removeTitleBar();

    this->setFixedSize(380, 142);
    m_label = new QLabel(this);
    m_label->setAccessibleName("kylin-weather_QLabel_AllowLocation_label");
    m_label->setFixedSize(24, 24);
    QPixmap m_doubt = QPixmap(":/res/contorl_icons/location_doubt.png");
    m_doubt = m_doubt.scaled(m_label->width(), m_label->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_label->setPixmap(m_doubt);

    m_text = new QLabel(this);
    m_text->setAccessibleName("kylin-weather_QLabel_AllowLocation_text");
    m_text->setFixedSize(284, 24);
    m_text->setText(tr("Whether to enable application locating?")); //是否开启应用定位

    //    m_allowLocation = new QButtonGroup(this);

    //    m_allow = new QRadioButton(this);
    //    m_allow->setFixedSize(16,16);

    //    m_allowText = new QLabel(this);
    //    m_allowText->setText("允许");

    //    m_noAllow = new QRadioButton(this);
    //    m_noAllow->setFixedSize(16,16);

    //    m_noAllowText = new QLabel(this);
    //    m_noAllowText->setText("不允许");

    //    m_allowLocation->addButton(m_allow);
    //    m_allowLocation->addButton(m_noAllow);
    //    m_noAllow->setChecked(true);

    //    m_allowLocation->addButton()

    m_cancel = new QPushButton(this);
    m_cancel->setAccessibleName("kylin-weather_QPushButton_AllowLocation_cancel");
    m_cancel->setFixedSize(96, 36);
    m_cancel->setText(tr("Cancel"));

    m_certain = new QPushButton(this);
    m_certain->setAccessibleName("kylin-weather_QPushButton_AllowLocation_certain");
    m_certain->setFixedSize(96, 36);
    m_certain->setText(tr("OK"));

    initLayout();
    initConnect();
}

void AllowLocation::initLayout()
{
    m_label->move(24, 32);
    m_text->move(56, 32);
    //    m_allow->move(57,68);
    //    m_allowText->move(81,68);
    //    m_noAllow->move(161,68);
    //    m_noAllowText->move(185,63);
    m_cancel->move(154, 82);
    m_certain->move(260, 82);
}

void AllowLocation::initConnect()
{
    connect(m_cancel, &QPushButton::clicked, this, &AllowLocation::rejectLocate);
    connect(m_certain, &QPushButton::clicked, this, &AllowLocation::allowLocate);
    connect(this, &AllowLocation::allowLocationYes, Interaction::getInstance(),
            &Interaction::allowLocationYes); // 允许定位信号
}

void AllowLocation::rejectLocate()
{
    CoreVar::setSettings("locate-widget", "1"); //更改配置文件值为非“0”
    this->setVisible(false);
}

void AllowLocation::allowLocate()
{
    CoreVar::setSettings("locate-widget", "1"); //更改配置文件值为非“0”
    this->setVisible(false);
    emit allowLocationYes();
    emit setMenuModuleAllowLocationYes();
}
