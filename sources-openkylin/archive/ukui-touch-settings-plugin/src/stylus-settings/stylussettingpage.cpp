/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "stylussettingpage.h"
#include "ui_stylussettingpage.h"

#include <QTranslator>
#include <QPushButton>
#include <QLabel>

namespace StylusSettings {

StylusSettingPage::StylusSettingPage(QWidget* parent) : QWidget(parent),
                                                  ui(new Ui::StylusSettingPage)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    initUi();
}

StylusSettingPage::~StylusSettingPage()
{
    delete ui;
}

void StylusSettingPage::initUi()
{
    // init widget in ui file
    //~ contents_path /Stylus/Stylus Settings
    ui->titleLabel->setText(tr("Stylus Settings"));
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->penImgLabel->setPixmap(QPixmap(":/img/img/setting-illus-pen_small.png"));
    ui->noteLabel->setEnabled(false);

    // init other widget
    m_sl_erase_btn = new QLabel(this);
    m_sl_erase_btn->setPixmap(QPixmap(":/img/img/open_inuse.png"));
    ui->slEraseLayout->addWidget(m_sl_erase_btn);
}

} // namespace
