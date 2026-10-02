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

#include "progress.h"
#include "ui_progress.h"
#include "src/view/modaldialog.h"
#include "src/model/logicmain.h"

#include <QDebug>

#define ICON_SIZE QSize(16, 16)

Progress::Progress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Progress)
{
    this->setObjectName("kylin-burner_Progress");
    ui->setupUi(this);

    ui->label->setAccessibleName("kylin-burner_QLabel_Progress_labelProgress");
    ui->labelContent->setAccessibleName("kylin-burner_MyTooltipLabel_Progress_labelContent");
    ui->labelTitle->setAccessibleName("kylin-burner_QLabel_Progress_labelTitle");
    ui->progressBar->setAccessibleName("kylin-burner_QProgressBar_Progress_progressBar");
    ui->widget->setAccessibleName("kylin-burner_QWidget_Progress_widget");
    ui->widget_2->setAccessibleName("kylin-burner_QWidget_Progress_widget2");

    ui->labelTitle->setFocusPolicy(Qt::NoFocus);
    ui->progressBar->setValue(0);

    connect(this, SIGNAL(title(QString)), ui->labelTitle, SLOT(setText(QString)));
    connect(this, SIGNAL(setRange(int,int)), this, SLOT(setsSetRange(int,int)));
    connect(this, &Progress::setValue, this, &Progress::slotsSetValue);
    connect(this, SIGNAL(addValue(int,QIcon,QString)), this, SLOT(slotsAddValue(int,QIcon,QString)));

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &Progress::slotChangeUIFontSize);
}

void Progress::slotsSetValue(int value, QIcon icon, QString str)
{
    if(value > ui->progressBar->maximum()){
        return;
    }

    ui->progressBar->setValue(value);
    ui->progressBar->setFormat(QString());
    ui->label->setText(QString("%1%").arg(value));

    ui->labelContent->setText(str);

    if (ui->progressBar->value() == ui->progressBar->maximum()) {
        MODAL->accept();
    }
}

void Progress::slotsAddValue(int value, QIcon icon, QString str)
{
    int current = ui->progressBar->value();
    current += value;
    ui->progressBar->setValue(current);
    ui->progressBar->setFormat(QString());

    int rate = (current*1.0) / (m_RangeEnd-m_RangeBegin) * 100 ;
    if(rate > 100) {
        rate = 100;
    }

    ui->label->setText(QString("%1%").arg(rate));


    QString labelContentShowText = str;
    ui->labelContent->setText(labelContentShowText);

    if (ui->progressBar->value() == ui->progressBar->maximum()) {
        MODAL->accept();
    }
}

void Progress::slotsSetValueWithoutClosing(int value,QString str)
{
    ui->progressBar->setValue(value);
    ui->progressBar->setFormat(QString());

    int rate = value;
    if(rate > 100) rate = 100;
    ui->label->setText(QString("%1%").arg(rate));

    ui->labelContent->setText(str);
}

Progress::~Progress()
{
    delete ui;
}

void Progress::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font = ui->labelTitle->font();
    font.setPixelSize(16 + scaleRatio);
    font.setBold(true);
    ui->labelTitle->setFont(font);

    font = ui->labelContent->font();
    font.setPixelSize(14 + scaleRatio);
    ui->labelContent->setFont(font);
    ui->progressBar->setFont(font);
    ui->label->setFont(font);
}

void Progress::resetProgressBar()
{
    if(ui->progressBar->value() != 0){
        ui->progressBar->setValue(0);
    }
}

void Progress::setsSetRange(int begin, int end)
{
    ui->progressBar->setRange(begin, end);
    m_RangeBegin = begin;
    m_RangeEnd = end;
    m_currnet = begin;
}
