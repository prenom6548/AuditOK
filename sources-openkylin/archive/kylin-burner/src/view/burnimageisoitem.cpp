/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "burnimageisoitem.h"
#include "ui_burnimageisoitem.h"

BurnImageISOItem::BurnImageISOItem(QString path, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BurnImageISOItem)
{
    this->setObjectName("kylin-burner_BurnImageISOItem");
    QFont f;
    ui->setupUi(this);
    ui->label->setAccessibleName("kylin-burner_QLabel_BurnImageISOItem_label");
    ui->pushButton->setAccessibleName("kylin-burner_QPushButton_BurnImageISOItem_pushButton");
    f = ui->label->font();
    f.setPixelSize(14);
    ui->label->setText(path);
    setToolTip(path);
    ui->label->setFont(f);
    ui->pushButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->pushButton->setProperty("isWindowButton", 0x2);
    ui->pushButton->setProperty("useIconHighlightEffect", 0x8);
    ui->pushButton->hide();
}

BurnImageISOItem::~BurnImageISOItem()
{
    delete ui;
}

void BurnImageISOItem::setText(QString path)
{
    ui->label->setText(path);
}

QFont BurnImageISOItem::getFont()
{
    return ui->label->font();
}
