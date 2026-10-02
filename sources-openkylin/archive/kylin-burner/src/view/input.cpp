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

#include "input.h"
#include "ui_input.h"
#include "src/view/execdialog.h"

Input::Input(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Input)
{
    this->setObjectName("kylin-burner_Input");
    QFont f;
    ui->setupUi(this);
    f = ui->labelContent->font();
    f.setPixelSize(14);

    ui->labelContent->setAccessibleName("kylin-burner_QLabel_Input_labelContent");
    ui->lineEditResult->setAccessibleName("kylin-burner_QLineEdit_Input_labelContent");

    ui->labelContent->setFont(f);
    //ui->labelContent->setGeometry(0,0,288,22);
    ui->labelContent->setGeometry(0,0,1,1);
    ui->lineEditResult->setGeometry(0,30,288,36);
	ui->labelContent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->lineEditResult->setFont(f);
	ui->verticalLayout->setContentsMargins(24,0,24,15);;

    connect(this, SIGNAL(content(QString)), ui->labelContent, SLOT(setText(QString)));
    connect(this, SIGNAL(result(QString)), ui->lineEditResult, SLOT(setText(QString)));
}

QString Input::result()
{
    return ui->lineEditResult->text();
}

Input::~Input()
{
    delete ui;
}
