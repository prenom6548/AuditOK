/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "addinputmethoddialog.h"
#include "ui_addinputmethoddialog.h"

const QList<QPair<QString, QString>>AddInputMethodDialog::inputMethodList_bo_CN = {
{"fcitx-keyboard-cn-tib" ,          tr("keyboard") + "-" + tr("Tibetan")},
{"fcitx-keyboard-cn-tib_asciinum",  tr("keyboard") + "-" + tr("Tibetan") +
                                    "  (" + tr("With ASCII numbers") + ")"}
};

AddInputMethodDialog::AddInputMethodDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddInputMethodDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Input Method"));
    ui->tableView->setStyleSheet("QTableView{border: 1px solid palette(midlight);}");
    mode = new QStandardItemModel(this);
    ui->tableView->setModel(mode);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setMouseTracking(true);
    ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->tableView->verticalHeader()->setVisible(false);//隐藏垂直表头
    ui->tableView->horizontalHeader()->setVisible(false);//隐藏水平表头
    ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableView->setShowGrid(false);//不显示表格线
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//水平表格自动拉伸
    ui->tableView->verticalHeader()->setDefaultSectionSize(36);//行高默认为36
    this->setWindowModality(Qt::ApplicationModal); //模态

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    mode->setColumnCount(1);

    connect(ui->yesBtn,&QPushButton::clicked,this,[=]() {
        QModelIndexList indexList =  ui->tableView->selectionModel()->selectedRows();
        QStringList selectedMethodList = {};
        for (QModelIndex a : indexList) {
            selectedMethodList.append(mode->data(a, Qt::UserRole).toString());
        }
        Q_EMIT inputMethodAdded(selectedMethodList);
        this->hide();
    });
    connect(ui->noBtn,&QPushButton::clicked,this,[=]() {
        this->hide();
    });
}

AddInputMethodDialog::~AddInputMethodDialog()
{
    delete ui;
}

void AddInputMethodDialog::addInputMethod(QString languageName)
{
    mode->clear();
    if (languageName == "bo_CN" && mode) {
        for (int i = 0; i < inputMethodList_bo_CN.count(); ++i) {
            mode->appendRow(new QStandardItem(inputMethodList_bo_CN.at(i).second));
            mode->setData(mode->index(mode->rowCount() - 1, 0),
                          inputMethodList_bo_CN.at(i).first,
                          Qt::UserRole);
        }
    }
}
