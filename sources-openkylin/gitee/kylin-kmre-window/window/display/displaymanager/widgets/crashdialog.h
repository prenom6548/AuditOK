/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CRASH_DIALOG_H_
#define _CRASH_DIALOG_H_

#include <QDialog>
#include <QString>
#include <QPushButton>
#include <QPaintEvent>

class CrashDialog : public QDialog
{
    Q_OBJECT
private:
    QPushButton *mRestartBtn;
    QPushButton *mCancelBtn;

signals:
     void sigCrashRestart(bool restart);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public:
    CrashDialog(QWidget *parent);
    ~CrashDialog();

    void showDialog();
};

#endif