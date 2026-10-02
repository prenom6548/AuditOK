/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2020 KYLINOS Information Technology Co., Ltd.
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

#ifndef CREATEGROUPDIALOG_H
#define CREATEGROUPDIALOG_H

#include <QPainter>
#include <QPainterPath>
#include <QCheckBox>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class changeUserGroup;
namespace Ui {
class CreateGroupDialog;
}

class CreateGroupDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CreateGroupDialog(QWidget *parent = nullptr);
    ~CreateGroupDialog();

public:
    void limitInput();
    void refreshCertainBtnStatus();
    KPushButton * certainBtnComponent();
    QLineEdit * lineNameComponent();
    QLineEdit * lineIdComponent();

private:
    QVBoxLayout *vlayout = nullptr;
    changeUserGroup *cgDialog = nullptr;

    QFrame *mGroupNameFrame;
    QFrame *mGroupIdFrame;
    QFrame *mConfirmFrame;

    KLabel *mGroupNameLabel;
    KLabel *mGroupIdLabel;

    QLineEdit *mGroupNameEdit;
    QLineEdit *mGroupIdEdit;

    KPushButton *mCancelBtn;
    KPushButton *mConfirmBtn;

    QHBoxLayout *mGroupNameHLayout;
    QHBoxLayout *mGroupIdHLayout;
    QHBoxLayout *mConfirmHLaout;
    QHBoxLayout *mGroupNameTipHLayout;

    bool _nameHasModified;
    bool _idHasModified;

    QString oldGroupName;
    KLabel *mGroupNameTipLabel;
    QString mGroupNameTip;

    void initUI();
    void signalsBind();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // CREATEGROUPDIALOG_H
