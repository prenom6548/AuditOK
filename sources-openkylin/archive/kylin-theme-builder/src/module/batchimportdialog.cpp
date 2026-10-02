/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "batchimportdialog.h"

BatchImportDialog::BatchImportDialog(QWidget *parent) :
    kdk::KDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setFixedHeight(170);
    this->setFocus();

    QWidget*mainwindow = mainWidget();

    QVBoxLayout *vlayout = new QVBoxLayout(mainwindow);
    vlayout->setContentsMargins(23,0,23,23);
    QHBoxLayout *vhlayout = new QHBoxLayout();
    QHBoxLayout *hlayout = new QHBoxLayout();

    QIcon icon = QIcon::fromTheme("emblem-danger");
    QPixmap pixmap = icon.pixmap(16, 16);
    QLabel *tip = new QLabel();
    tip->setPixmap(pixmap);

    QLabel *tip1 = new QLabel();
    tip1->setText(tr("Before importing the ICONS in batches, ensure that all ICONS are named according to the specifications."));

    QPushButton *specificationBtn = new QPushButton(this);
//    specificationBtn->setFixedSize(120,36);
    specificationBtn->setText(tr("View icon specification"));
    connect(specificationBtn,&QPushButton::clicked,this,[=](){
        QUrl Url("https://gitee.com/openkylin/kylin-theme-builder/blob/openkylin/nile/README.md");
        QDesktopServices::openUrl(Url);
    });

    QPushButton *importBtn = new QPushButton(this);
    importBtn->setFixedSize(96,36);
    importBtn->setText(tr("Batch import"));
    connect(importBtn,&QPushButton::clicked,this,[=](){
        emit startimport();
    });

    vhlayout->addWidget(tip);
    vhlayout->addWidget(tip1);
    vhlayout->addStretch(1);
    vlayout->addLayout(vhlayout);
    hlayout->addWidget(specificationBtn);
    hlayout->addStretch(1);
    hlayout->addWidget(importBtn);
    vlayout->addLayout(hlayout);
}
