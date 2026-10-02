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
#ifndef __TROUBLESHOOTING_DIALOG_H__
#define __TROUBLESHOOTING_DIALOG_H__

#include "custom_dialog.h"
#include "troubleshooting_info_widget.h"
#include "ukui_printer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TroubleshootingDialog; }
QT_END_NAMESPACE

class TroubleshootingDialog : public CustomDialog
{
    Q_OBJECT
public:
    TroubleshootingDialog(const UkuiUsbPrinter &printer = UkuiUsbPrinter(), QWidget *parent = nullptr);
    ~TroubleshootingDialog();

    void onShow();

private:
    Ui::TroubleshootingDialog *ui;
    void startCheck();
    void check1();
    void check2();
    void check3();
    void check4();
    void doRepair();

    TroubleshootingInfoWidget *checkitem1;
    TroubleshootingInfoWidget *checkitem2;
    TroubleshootingInfoWidget *checkitem3;
    TroubleshootingInfoWidget *checkitem4;

    bool checkSuccess = true;
    UkuiUsbPrinter m_printer;
};

#endif // __TROUBLESHOOTING_DIALOG_H__