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
#include "rename_printer_dialog.h"
#include "ui_rename_printer_dialog.h"

RenamePrinterDialog::RenamePrinterDialog(QString printerName, QWidget *parent)
    : CustomDialog(parent)
    , m_printerName(printerName)
    , ui(new Ui::RenamePrinterDialog)
{
    ui->setupUi(this->mainWidget());
    setFixedSize(420, 184);
    setWindowTitle(tr("Printer Name"));
    ui->printerNameEdit->setText(m_printerName);
    ui->acceptButton->setEnabled(false);
    ui->statusLabel->setStyleSheet("color:red");
    m_printers.clear();
    UkuiPrinterManager::getInstance().enumPrinters(m_printers);

    connect(ui->acceptButton, &QPushButton::clicked, this, &RenamePrinterDialog::acceptButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &RenamePrinterDialog::reject);
    connect(ui->printerNameEdit, &QLineEdit::returnPressed, this, &RenamePrinterDialog::acceptButtonClicked);
    connect(ui->printerNameEdit, &QLineEdit::textChanged, [=](const QString &str){
        ui->statusLabel->setText("");
        auto res = UkuiPrinterManager::getInstance().printerNameCheck(str, false, m_printers, m_printerName);

        switch (res.status)
        {
        case (CommonStatusFlag::SUCCESS):
            ui->acceptButton->setEnabled(true);
            break;
        case (CommonStatusFlag::FAIL):
        case (CommonStatusFlag::ERROR_EXIST_SAME_PRINTER):
            ui->statusLabel->setText(res.message);
            ui->acceptButton->setEnabled(false);
            break;
        default:
            qDebug() << "RenamePrinterDialog::RenamePrinterDialog :undefined error!";
            break;
        }
    });

}

void RenamePrinterDialog::acceptButtonClicked()
{
    CommonStatus res =  UkuiPrinterManager::getInstance().printerNameCheck(getPrinterName());
    ui->statusLabel->setText("");
    switch (res.status)
    {
    case (CommonStatusFlag::SUCCESS):
        accept();
        break;
    case (CommonStatusFlag::FAIL):
    case (CommonStatusFlag::ERROR_EXIST_SAME_PRINTER):
        ui->statusLabel->setText(res.message);
        ui->acceptButton->setEnabled(false);
        break;
    default:
        qDebug() << "RenamePrinterDialog::acceptButtonClicked :undefined error!";
        break;
    }
}

RenamePrinterDialog::~RenamePrinterDialog()
{
    delete ui;
}

QString RenamePrinterDialog::getPrinterName() const
{
    return ui->printerNameEdit->text();
}
