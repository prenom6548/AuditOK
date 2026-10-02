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

#include "tips.h"
#include "ui_tips.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/model/logicmain.h"

#define ICON_SIZE QSize(25, 25)

tips::tips(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tips)
{
    this->setObjectName("kylin-burner_tips");
    ui->setupUi(this);

    ui->checkBox->setAccessibleName("kylin-burner_QCheckBox_tips_checkBox");
    ui->labelContent->setAccessibleName("kylin-burner_QLabel_tips_labelContent");
    ui->labelLogo->setAccessibleName("kylin-burner_QLabel_tips_labelLogo");
    ui->widget->setAccessibleName("kylin-burner_QWidget_tips_widget");
    ui->widgetCheckBox->setAccessibleName("kylin-burner_QWidget_tips_widgetCheckBox");

    ui->labelLogo->setGeometry(25,7,30,30);
    ui->labelContent->setGeometry(65,0,210,50);
    ui->widgetCheckBox->hide();
    ui->verticalLayout->removeWidget(ui->widgetCheckBox);
    ui->checkBox->hide();
    connect(this, SIGNAL(success(QString)), this, SLOT(slotsSuccess(QString)));
    connect(this, SIGNAL(fail(QString)), this, SLOT(slotsFailed(QString)));
    connect(ui->checkBox, SIGNAL(toggled(bool)), this, SIGNAL(checked(bool)));

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &tips::slotChangeUIFontSize);
}

void tips::information(QString content, bool next)
{
    ui->labelContent->setText(content);
    ui->checkBox->setText(_("Donot show agian."));
    ui->checkBox->setChecked(false);
    ui->checkBox->setHidden(next);
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-information").pixmap(ICON_SIZE));

}

void tips::error(QString content, bool next)
{
   // ui->labelContent->setFixedWidth(300);
    ui->labelContent->setText(content);
    ui->checkBox->setText(_("Donot show agian."));
    ui->checkBox->setChecked(false);
    ui->checkBox->setHidden(next);
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-error").pixmap(ICON_SIZE));

    ui->labelContent->setToolTip("");
    ui->labelContent->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QFontMetrics metrics(ui->labelContent->font());
    QString eliedText = metrics.elidedText(ui->labelContent->text(),Qt::ElideRight, ui->labelContent->width()*2);
    ui->labelContent->setText(eliedText);
    if(eliedText != content){
       ui->labelContent->setToolTip(content);
    }
}

void tips::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font;
    font = ui->labelContent->font();
    font.setPixelSize(16 + scaleRatio);
    ui->labelContent->setFont(font);
    font = ui->checkBox->font();
    font.setPixelSize(16 + scaleRatio);
    ui->checkBox->setFont(font);

}

void tips::question(QString content, bool next)
{
    ui->labelContent->setFixedWidth(320);
    ui->labelContent->setText(content);
    ui->checkBox->setChecked(false);
    ui->checkBox->setText(_("Donot show agian."));
    ui->checkBox->setHidden(next);
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-question").pixmap(ICON_SIZE));
}



tips::~tips()
{
    delete ui;
}
