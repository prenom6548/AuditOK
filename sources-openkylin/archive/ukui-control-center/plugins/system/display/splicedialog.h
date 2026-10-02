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
#ifndef SPLICEDIALOG_H
#define SPLICEDIALOG_H

#include <QDialog>
#include "./declarative/qmlscreen.h"
#include <QFrame>
#include <QLabel>
#include <QComboBox>

namespace Ui {
class SpliceDialog;
}

class SpliceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpliceDialog(QWidget *parent = nullptr);
    ~SpliceDialog();
    void createScreenFrame(int r, int c);
    void spliceComboBoxChanged(int selectId);
    void initSpliceComboBox();
    void createScreenLabel();
    void setScreenLabelPos();
    void init(KScreen::OutputList connectedOutputList);
    void getRowsAndColumnsList();

private:
    Ui::SpliceDialog *ui;
    QMLScreen *mScreen = nullptr;
    KScreen::OutputList mConnectedOutputList;
    QList<QPoint> rowsAndColumnsList;
    QList<QFrame *>screenFrameList;
    QList<QLabel *>screenLabelList;

Q_SIGNALS:
    void screenComboBoxChanged(QComboBox *comboBox, int currentIndex);
    void spliceScreenChanged(QString spliceMethodText);
};

#endif // SPLICEDIALOG_H
