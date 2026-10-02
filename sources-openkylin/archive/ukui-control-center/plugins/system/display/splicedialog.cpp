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
#include "splicedialog.h"
#include "ui_splicedialog.h"
#include <QDebug>


SpliceDialog::SpliceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpliceDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Splice Screen"));
    ui->cancelBtn->setText(tr("Cancel"));
    ui->okBtn->setText(tr("Ok"));
    setWindowModality(Qt::WindowModal);
    ui->widget->setStyleSheet("background-color:palette(window)");
    ui->spliceLabel->setText(tr("Splicing Method"));

    connect(this, &SpliceDialog::screenComboBoxChanged, this, [=](QComboBox *comboBox, int currentIndex){
        bool noneFlag = false;
        if (currentIndex != 0) {
            for (int var = 0; var < screenFrameList.size(); ++var) {
                QComboBox *mComboBox = screenFrameList.at(var)->findChild<QComboBox*>();
                if (mComboBox != comboBox) {
                    if (mComboBox->currentIndex() == 0) {
                        noneFlag = true;
                    }
                    if (mComboBox->currentText() == comboBox->currentText()) {
                        mComboBox->blockSignals(true);
                        mComboBox->setCurrentIndex(0);
                        mComboBox->blockSignals(false);
                        noneFlag = true;
                    }
                }
            }
        } else {
             noneFlag = true;
        }
        if (!noneFlag) {
            ui->okBtn->setEnabled(true);
        } else {
            ui->okBtn->setEnabled(false);
        }
        setScreenLabelPos();
    });

    connect(ui->okBtn, &QPushButton::clicked, this, [=](){
        this->hide();
        Q_EMIT spliceScreenChanged(ui->spliceComboBox->currentText());
    });

    connect(ui->cancelBtn, &QPushButton::clicked, this, [=](){
        this->hide();
        Q_EMIT finished(0);
    });

    connect(this, &SpliceDialog::finished, this, [=](){
        //各项设置需要恢复之前配置
    });
}

SpliceDialog::~SpliceDialog()
{
    delete ui;
    ui = nullptr;
}

void SpliceDialog::getRowsAndColumnsList()
{
    rowsAndColumnsList.clear();
    int connectedOutputNum = mConnectedOutputList.size();
    rowsAndColumnsList.append(QPoint(1, connectedOutputNum));
    for (int i = 2; i < (connectedOutputNum - 1); ++i) {
        if (connectedOutputNum % i == 0) {
            int j = connectedOutputNum / i;
            rowsAndColumnsList.append(QPoint(i,j));
        }
    }
    rowsAndColumnsList.append(QPoint(connectedOutputNum, 1));
}


void SpliceDialog::initSpliceComboBox()
{
    ui->spliceComboBox->disconnect();
    ui->spliceComboBox->clear();
    getRowsAndColumnsList();

    for (int var = 0; var < rowsAndColumnsList.size(); ++var) {
        QString row = QString::number(rowsAndColumnsList.at(var).x());
        QString column = QString::number(rowsAndColumnsList.at(var).y());
        QString rowName = rowsAndColumnsList.at(var).x() < 2 ? tr("row"):tr("rows");
        QString columnName = rowsAndColumnsList.at(var).y() < 2 ? tr("column"):tr("columns");
        ui->spliceComboBox->addItem(tr("%1 %2, %3 %4").arg(row, rowName, column, columnName),rowsAndColumnsList.at(var));
    }

    connect(ui->spliceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int selectId) {
        spliceComboBoxChanged(selectId);
        setScreenLabelPos();
    });
}

void SpliceDialog::spliceComboBoxChanged(int selectId)
{
    Q_UNUSED(selectId);
    qDeleteAll(screenFrameList);
    screenFrameList.clear();

    QPoint point = ui->spliceComboBox->currentData().toPoint();
    for (int i = 1; i <= point.x(); ++i) {
        for (int j = 1; j <= point.y(); ++j) {
            createScreenFrame(i, j);
        }
    }

    for (int var = 0; var < screenFrameList.size(); ++var) {
        QComboBox *comboBox = screenFrameList.at(var)->findChild<QComboBox*>();
        comboBox->blockSignals(true);
        comboBox->setCurrentIndex(var + 1);
        comboBox->blockSignals(false);
        ui->screenFrame->layout()->addWidget(screenFrameList.at(var));
    }

    this->setFixedHeight(332 + screenFrameList.size() * 52);
}

void SpliceDialog::createScreenFrame(int r, int c)
{
    QString row = QString::number(r);
    QString column = QString::number(c);
    QString labelString = tr("Row %1, Column %2").arg(row, column);
    QFrame *frame = new QFrame(this);
    frame->setFixedHeight(52);
    QHBoxLayout *layout = new QHBoxLayout(frame);
    QLabel *label = new QLabel(frame);
    QComboBox *comboBox = new QComboBox(frame);
    comboBox->setFixedWidth(284);
    layout->setSpacing(24);
    layout->setContentsMargins(0,8,0,8);
    layout->addWidget(label);
    layout->addWidget(comboBox);
    label->setText(labelString);

    comboBox->addItem(tr("Select screen"), QPoint(0,0));
    for (KScreen::OutputPtr output : mConnectedOutputList) {
        comboBox->addItem(output->name(), QPoint(r, c));
    }
    screenFrameList.append(frame);
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int selectId) {
        Q_EMIT screenComboBoxChanged(comboBox, selectId);
    });
}

void SpliceDialog::createScreenLabel()
{
    qDeleteAll(screenLabelList);
    screenLabelList.clear();
    for (KScreen::OutputPtr output : mConnectedOutputList) {
        QLabel *screenLabel = new QLabel(ui->widget);
        screenLabel->setStyleSheet("background:palette(button);");
        screenLabel->setText(output->name());
        screenLabel->setAlignment(Qt::AlignCenter);
        screenLabelList.append(screenLabel);
    }
}

void SpliceDialog::setScreenLabelPos()
{
    QPoint point = ui->spliceComboBox->currentData().toPoint();
    int rows = point.x();
    int columns = point.y();
    int labelW = 86;
    int labelH = 48;

    if (screenLabelList.size() == 4 && rows == 4) {
        labelW = labelW/1.3;
        labelH = labelH/1.3;
    }

    int beginX = (ui->widget->width() - columns*labelW - 2*(columns - 1))/2;
    int beginY = (ui->widget->height() - rows*labelH - 2*(rows - 1))/2;

    for (QLabel *screenLabel : screenLabelList) {
        bool setPosFlag  = false;
        for (QFrame *frame : screenFrameList) {
            QComboBox *comboBox = frame->findChild<QComboBox*>();
            if (comboBox->currentText() == screenLabel->text()) {
                QPoint pos = comboBox->currentData().toPoint();
                int r = pos.x();
                int c = pos.y();
                if (r == 0 || c == 0) {
                    continue;
                }
                screenLabel->setGeometry(beginX + (c - 1)*labelW + (c - 1) * 2,
                                         beginY + (r - 1)*labelH + (r - 1) * 2,
                                         labelW,
                                         labelH);
                setPosFlag = true;
                screenLabel->setVisible(true);
                break;
            }
        }
        if (!setPosFlag) {
            screenLabel->setVisible(false);
        }
    }
}

void SpliceDialog::init(KScreen::OutputList connectedOutputList)
{
    this->mConnectedOutputList = connectedOutputList;
    initSpliceComboBox();
    spliceComboBoxChanged(0);
    createScreenLabel();
    setScreenLabelPos();
}


