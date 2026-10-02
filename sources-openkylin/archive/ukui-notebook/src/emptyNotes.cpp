/*
* Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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

#include <QBitmap>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

#include "emptyNotes.h"
#include "ui_emptyNotes.h"
emptyNotes::emptyNotes(QWidget *parent) :
    QMessageBox(parent),
    ui(new Ui::emptyNotes),
    m_isDontShow(false)
{
//    setWindowTitle(tr("Empty Note"));
    setIcon(QMessageBox::Question);
    setText(tr("Are you sure empty notebook?"));
    checkbtn = new QCheckBox(this);
    checkbtn->setText(tr("No Tips"));
    setCheckBox(checkbtn);
    QPushButton *cancelBtn = new QPushButton(tr("cancel"));
    QPushButton *yesBtn = new QPushButton(tr("yes"));
    cancelBtn->setFocusPolicy(Qt::NoFocus);
    yesBtn->setFocusPolicy(Qt::NoFocus);

    addButton(cancelBtn,QMessageBox::NoRole);
    addButton(yesBtn,QMessageBox::YesRole);
    setDefaultButton(yesBtn);


    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(on_pushButton_clicked()) );
    connect(yesBtn, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()) );
    connect(checkbtn, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)) );
}

emptyNotes::~emptyNotes()
{
    delete ui;
    delete checkbtn;
}

void emptyNotes::on_pushButton_clicked()
{

    checkbtn->setCheckState(Qt::Unchecked);
    this->close();
}

void emptyNotes::on_pushButton_2_clicked()
{

    if(checkbtn->isChecked())
    {
        m_isDontShow = true;
    }
    else
    {
        m_isDontShow = false;
    }

    emit requestEmptyNotes();
    this->close();
}
