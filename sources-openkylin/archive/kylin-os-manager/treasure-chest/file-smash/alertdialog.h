/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef ALERTDIALOG_H
#define ALERTDIALOG_H

#include <QWidget>
#include <QtGui>
#include <QStyleOption>

namespace Ui
{
class AlertDialog;
}

class AlertDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AlertDialog(QWidget *parent = 0);
    ~AlertDialog();
    void paintEvent(QPaintEvent *);
    void setMSG(QString);

private:
    Ui::AlertDialog *ui;
};

#endif // ALERTDIALOG_H
