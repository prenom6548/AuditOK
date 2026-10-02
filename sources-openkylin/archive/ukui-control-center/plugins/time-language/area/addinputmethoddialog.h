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
#ifndef ADDINPUTMETHODDIALOG_H
#define ADDINPUTMETHODDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class AddInputMethodDialog;
}

class AddInputMethodDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddInputMethodDialog(QWidget *parent = nullptr);
    ~AddInputMethodDialog();
    void addInputMethod(QString languageName);

public:
    static const QList<QPair<QString, QString>> inputMethodList_bo_CN;

private:
    Ui::AddInputMethodDialog *ui;
    QStandardItemModel *mode = nullptr;

Q_SIGNALS:
    void inputMethodAdded(const QStringList inputMethodList);
};

#endif // ADDINPUTMETHODDIALOG_H
