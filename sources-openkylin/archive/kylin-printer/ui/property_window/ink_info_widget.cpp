/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "ink_info_widget.h"
#include "ui_ink_info_widget.h"
#include "custom_color_button.h"

InkInfoWidget::InkInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InkInfoWidget)
{
    ui->setupUi(this);
    setFixedHeight(64);
    ui->inkColorButton->setButtonType(CustomColorButton::Circle);
    ui->inkColorButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

InkInfoWidget::~InkInfoWidget()
{
    delete ui;
}

void InkInfoWidget::setMainLabel(const QString &str)
{
    ui->mainLabel->setText(str);
}

void InkInfoWidget::setInformativeLabel(const QString &str)
{
    ui->informativeLabel->setText(str);
}

void InkInfoWidget::setColor(const QColor &color)
{
    ui->inkColorButton->setBackgroundColor(color);
}
