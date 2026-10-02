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

#include "settingdatas.h"
#include "ui_settingdatas.h"
#include "src/frame/configuration.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/model/logicmain.h"

SettingDatas::SettingDatas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingDatas)
{
    this->setObjectName("kylin-burner_SettingDatas");
    ui->setupUi(this);
    connect(this, SIGNAL(title(QString)), ui->title, SLOT(setText(QString)));

    ui->Addwidget->setAccessibleName("kylin-burner_QWidget_SettingDatas_Addwidget");
    ui->checkBoxAddAll->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxAddAll");
    ui->checkBoxAddCurrent->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxAddCurrent");
    ui->checkBoxAddSuffix->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxAddSuffix");
    ui->checkBoxCleanAll->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxCleanAll");
    ui->checkBoxCleanCurrent->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxCleanCurrent");
    ui->checkBoxDeleteAll->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxDeleteAll");
    ui->checkBoxDeleteCurrent->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxDeleteCurrent");
    ui->checkBoxRename->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxRename");
    ui->checkBoxRepleace->setAccessibleName("kylin-burner_QRadioButton_SettingDatas_checkBoxRepleace");
    ui->Cleanwidget->setAccessibleName("kylin-burner_QWidget_SettingDatas_Cleanwidget");
    ui->Deletewidget->setAccessibleName("kylin-burner_QWidget_SettingDatas_Deletewidget");
    ui->labelReplceData->setAccessibleName("kylin-burner_QLabel_SettingDatas_labelDuplicateData");
    ui->labelAddData->setAccessibleName("kylin-burner_QLabel_SettingDatas_labelAddData");
    ui->labelCleanData->setAccessibleName("kylin-burner_QLabel_SettingDatas_labelCleanData");
    ui->labelDeleteData->setAccessibleName("kylin-burner_QLabel_SettingDatas_labelDeleteData");
    ui->layoutWidget->setAccessibleName("kylin-burner_QWidget_SettingDatas_layoutWidget");
    ui->layoutWidget1->setAccessibleName("kylin-burner_QWidget_SettingDatas_layoutWidget1");
    ui->layoutWidget2->setAccessibleName("kylin-burner_QWidget_SettingDatas_layoutWidget2");
    ui->layoutWidget3->setAccessibleName("kylin-burner_QWidget_SettingDatas_layoutWidget3");
    ui->title->setAccessibleName("kylin-burner_QLabel_SettingDatas_title");
    ui->widget->setAccessibleName("kylin-burner_QWidget_SettingDatas_Duplicatewidget");

    ui->labelAddData->setText(_("Add Data"));

    ui->checkBoxAddAll->setText(_("All"));

    ui->checkBoxAddCurrent->setText(_("Current"));

    ui->checkBoxAddAll->setCheckable(true);
    ui->checkBoxAddAll->setAutoExclusive(true);
    ui->checkBoxAddCurrent->setCheckable(true);
    ui->checkBoxAddCurrent->setAutoExclusive(true);

    ui->labelDeleteData->setText(_("Delete Data"));

    ui->checkBoxDeleteAll->setText(_("All"));

    ui->checkBoxDeleteCurrent->setText(_("Current"));

    ui->checkBoxDeleteAll->setCheckable(true);
    ui->checkBoxDeleteAll->setAutoExclusive(true);
    ui->checkBoxDeleteCurrent->setCheckable(true);
    ui->checkBoxDeleteCurrent->setAutoExclusive(true);

    ui->labelCleanData->setText(_("Clean Data"));

    ui->checkBoxCleanAll->setText(_("All"));

    ui->checkBoxCleanCurrent->setText(_("Current"));

    ui->checkBoxCleanAll->setCheckable(true);
    ui->checkBoxCleanAll->setAutoExclusive(true);
    ui->checkBoxCleanCurrent->setCheckable(true);
    ui->checkBoxCleanCurrent->setAutoExclusive(true);


    ui->labelReplceData->setText(_("Duplicate Data"));
    ui->checkBoxRename->setText(_("Rename"));
    ui->checkBoxAddSuffix->setText(_("Add Suffix"));
    ui->checkBoxRepleace->setText(_("Replace"));

    ui->checkBoxRename->setCheckable(true);
    ui->checkBoxRename->setAutoExclusive(true);
    ui->checkBoxAddSuffix->setCheckable(true);
    ui->checkBoxAddSuffix->setAutoExclusive(true);
    ui->checkBoxRepleace->setCheckable(true);
    ui->checkBoxRepleace->setAutoExclusive(true);

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &SettingDatas::slotChangeUIFontSize);

    connect(ui->checkBoxAddAll, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxAddCurrent, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxDeleteAll, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxDeleteCurrent, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxCleanAll, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxCleanCurrent, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxRename, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxRepleace, SIGNAL(clicked(bool)), this, SLOT(check()));
    connect(ui->checkBoxAddSuffix, SIGNAL(clicked(bool)), this, SLOT(check()));
}

void SettingDatas::load()
{
    title(_("Data Setting"));
    m_iDuplicate = CFG->get("DataSetting", "Duplicate Data", 0).toInt();
    switch (m_iDuplicate)
    {
    case 0:
        ui->checkBoxRename->setChecked(true);
        break;
    case 1:
        ui->checkBoxAddSuffix->setChecked(true);
        break;
    default:
        ui->checkBoxRepleace->setChecked(true);
        if (m_iDuplicate > 2) m_iDuplicate = 2;
        break;
    }
    m_bAdd = CFG->get("DataSetting", "Add Data", true).toBool();
    ui->checkBoxAddAll->setChecked(m_bAdd);
    ui->checkBoxAddCurrent->setChecked(!m_bAdd);
    m_bDelete = CFG->get("DataSetting", "Delete Data", false).toBool();
    ui->checkBoxDeleteAll->setChecked(m_bDelete);
    ui->checkBoxDeleteCurrent->setChecked(!m_bDelete);
    m_bClear = CFG->get("DataSetting", "Clean Data", false).toBool();
    ui->checkBoxCleanAll->setChecked(m_bClear);
    ui->checkBoxCleanCurrent->setChecked(!m_bClear);
    check();
    updateLabelText();
}

void SettingDatas::accept()
{
    int duplicate;

    if (ui->checkBoxRename->isChecked()) duplicate = 0;
    else if (ui->checkBoxAddSuffix->isChecked()) duplicate = 1;
    else duplicate = 2;

    CFG->set("DataSetting", "Duplicate Data", duplicate);
    CFG->set("DataSetting", "Add Data", ui->checkBoxAddAll->isChecked());
    CFG->set("DataSetting", "Delete Data", ui->checkBoxDeleteAll->isChecked());
    CFG->set("DataSetting", "Clean Data", ui->checkBoxCleanAll->isChecked());
}

void SettingDatas::resetText(QLabel *item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != ""
            && -1 != item_name.indexOf("…")
            && -1 != item->toolTip().indexOf(item_name.mid(0, item_name.length() - 3)))
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, item->width() - 3);
    item->setText(show_name);
    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}

void SettingDatas::updateLabelText()
{
    resetText(ui->labelAddData);
    resetText(ui->labelDeleteData);
    resetText(ui->labelCleanData);
    resetText(ui->labelReplceData);
}

void SettingDatas::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font = ui->title->font();
    font.setPixelSize(16 + scaleRatio);
    font.setBold(true);
    ui->title->setFont(font);

    updateLabelWidth(font);

    font = ui->labelAddData->font();
    font.setPixelSize(14 + scaleRatio);
    font.setWeight(60);
    ui->labelAddData->setFont(font);
    ui->labelDeleteData->setFont(font);
    ui->labelCleanData->setFont(font);
    ui->labelReplceData->setFont(font);

    ui->checkBoxDeleteAll->setFont(font);
    ui->checkBoxAddCurrent->setFont(font);
    ui->checkBoxDeleteCurrent->setFont(font);
    ui->checkBoxCleanAll->setFont(font);
    ui->checkBoxCleanCurrent->setFont(font);
    ui->labelReplceData->setFont(font);
    ui->checkBoxRename->setFont(font);
    ui->checkBoxAddSuffix->setFont(font);
    ui->checkBoxRepleace->setFont(font);
    ui->checkBoxAddAll->setFont(font);

    updateLabelText();
}

void SettingDatas::updateLabelWidth(QFont font)
{
    QList<QLabel*> labelList;
    labelList << ui->labelAddData << ui->labelDeleteData << ui->labelCleanData << ui->labelReplceData;
    for(QLabel* index : labelList){
        QString labelText = index->text();
        QFontMetrics fontMetrics(font);
        int textWidth = fontMetrics.width(labelText);
        index->setFixedWidth(textWidth);
    }
}

void SettingDatas::check()
{
    int duplicate;

    if (ui->checkBoxRename->isChecked()) duplicate = 0;
    else if (ui->checkBoxAddSuffix->isChecked()) duplicate = 1;
    else duplicate = 2;

    do
    {
        if (duplicate != m_iDuplicate) break;
        if (m_bAdd != ui->checkBoxAddAll->isChecked()) break;
        if (m_bDelete != ui->checkBoxDeleteAll->isChecked()) break;
        if (m_bClear != ui->checkBoxCleanAll->isChecked()) break;
        change(false);
        return;
    } while (0);
    change(true);
}

SettingDatas::~SettingDatas()
{
    delete ui;
}
