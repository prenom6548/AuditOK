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
#ifndef __RENAME_PRINTER_DIALOG_H__
#define __RENAME_PRINTER_DIALOG_H__

#include "custom_dialog.h"
#include "ukui_printer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RenamePrinterDialog; }
QT_END_NAMESPACE

class RenamePrinterDialog : public CustomDialog
{
    Q_OBJECT
public:
    RenamePrinterDialog(QString printerName = QString(), QWidget *parent = nullptr);
    ~RenamePrinterDialog();
    QString getPrinterName() const;
private:
    Ui::RenamePrinterDialog *ui;
    QString m_printerName{QString()};
    QList<UkuiUsbPrinter> m_printers;

    void acceptButtonClicked();
};

#endif // __RENAME_PRINTER_DIALOG_H__