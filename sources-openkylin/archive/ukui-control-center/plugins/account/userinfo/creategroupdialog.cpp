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

#include "creategroupdialog.h"
#include "userinfo.h"
#include "changeusergroup.h"

#define GROUPNAME_LENGTH 32

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

CreateGroupDialog::CreateGroupDialog(QWidget *parent) :
    KDialog(parent),
    cgDialog(new changeUserGroup),
    _nameHasModified(false),
    _idHasModified(false)
{
    vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(22, 0, 25, 0);
    initUI();
    signalsBind();
}

CreateGroupDialog::~CreateGroupDialog()
{
    delete cgDialog;
    cgDialog = nullptr;
}

void CreateGroupDialog::initUI()
{
    this->setWindowTitle(tr("Add User Group"));
    this->setWindowIcon("ukui-control-center");
    this->setFixedSize(480, 252);
    setAttribute(Qt::WA_DeleteOnClose);

    mGroupNameLabel = new KLabel(this);
    mGroupNameLabel->setText(tr("Name"));
    mGroupNameLabel->setFixedSize(62, 36);
    mGroupNameEdit = new QLineEdit(this);
    mGroupNameEdit->installEventFilter(this);
    mGroupNameEdit->setFixedSize(370, 36);
    mGroupNameEdit->setAccessibleName(tr("Name"));

    mGroupNameHLayout = new QHBoxLayout(this);
    mGroupNameHLayout->setContentsMargins(0,0,0,0);
    mGroupNameHLayout->setSpacing(2);
    mGroupNameHLayout->addWidget(mGroupNameLabel);
    mGroupNameHLayout->addWidget(mGroupNameEdit);

    mGroupNameTipLabel = new KLabel(this);
    mGroupNameTipLabel->setFixedSize(QSize(370, 24));
    QFont ft;
    ft.setPixelSize(12);
    mGroupNameTipLabel->setFont(ft);
    mGroupNameTipLabel->setFontColor(Qt::red);
    mGroupNameTipHLayout = new QHBoxLayout();
    mGroupNameTipHLayout->setContentsMargins(0,0,0,0);
    mGroupNameTipHLayout->addStretch();
    mGroupNameTipHLayout->addWidget(mGroupNameTipLabel);

    mGroupIdLabel = new KLabel(this);
    mGroupIdLabel->setText(tr("ID"));
    mGroupIdLabel->setFixedSize(62, 36);
    mGroupIdEdit = new QLineEdit(this);
    mGroupIdEdit->setFixedSize(370, 36);
    mGroupIdEdit->setAccessibleName(tr("ID"));

    mGroupIdHLayout = new QHBoxLayout(this);
    mGroupIdHLayout->setContentsMargins(0,0,0,0);
    mGroupIdHLayout->setSpacing(2);
    mGroupIdHLayout->addWidget(mGroupIdLabel);
    mGroupIdHLayout->addWidget(mGroupIdEdit);

    mCancelBtn = new KPushButton(this);
    mCancelBtn->setText(tr("Cancel"));
    mCancelBtn->setFixedSize(96, 36);
    mConfirmBtn = new KPushButton(this);
    mConfirmBtn->setText(tr("Confirm"));
    mConfirmBtn->setMinimumSize(96, 36);
    mConfirmBtn->setBackgroundColorHighlight(true);

    mConfirmHLaout = new QHBoxLayout(this);
    mConfirmHLaout->setContentsMargins(0,0,0,0);
    mConfirmHLaout->addStretch();
    mConfirmHLaout->addWidget(mCancelBtn);
    mConfirmHLaout->addWidget(mConfirmBtn);

    vlayout->addStretch();
    vlayout->addSpacing(20);
    vlayout->addLayout(mGroupNameHLayout);
    vlayout->addLayout(mGroupNameTipHLayout);
    vlayout->addLayout(mGroupIdHLayout);
    vlayout->addSpacing(40);
    vlayout->addLayout(mConfirmHLaout);
    vlayout->addSpacing(24);
    vlayout->addStretch();

    mainWidget()->setLayout(vlayout);

    // 设置确认按钮
    refreshCertainBtnStatus();
    // 限制组名输入规则
    limitInput();
}


void CreateGroupDialog::limitInput()
{
    QIntValidator *intValidator = new QIntValidator;
    // QRegExp rx("^[a-zA-z]+$");// 首字符为字母
    QRegExp rx("^[a-zA-Z][a-zA-Z0-9_-]*${32}");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    // intValidator->setRange(0, 65535);
    intValidator->setBottom(0);
    // 整形输入限制
    mGroupIdEdit->setValidator(intValidator);
    // 字母输入限制
    mGroupNameEdit->setValidator(regValidator);
    // 字符长度限制
    // ui->lineEdit_name->setMaxLength(4);
}

void CreateGroupDialog::refreshCertainBtnStatus()
{
    if (mGroupNameEdit->text().isEmpty()
        || mGroupIdEdit->text().isEmpty())
        mConfirmBtn->setEnabled(false);
    else
        mConfirmBtn->setEnabled(_nameHasModified || _idHasModified);
}

KPushButton *CreateGroupDialog::certainBtnComponent()
{
    return mConfirmBtn;
}

QLineEdit *CreateGroupDialog::lineNameComponent()
{
    return mGroupNameEdit;
}

QLineEdit *CreateGroupDialog::lineIdComponent()
{
    return mGroupIdEdit;
}

void CreateGroupDialog::signalsBind()
{
    connect(mCancelBtn, &KPushButton::clicked, [=](){
        close();
    });
    connect(mGroupNameEdit, &QLineEdit::textChanged, [=](QString txt){
        Q_UNUSED(txt);
        refreshCertainBtnStatus();
    });
    connect(mGroupIdEdit, &QLineEdit::textChanged, [=](QString txt){
        Q_UNUSED(txt);
        refreshCertainBtnStatus();
    });
    connect(mGroupIdEdit, &QLineEdit::textEdited, [=](){
        for (int j=0; j < cgDialog->groupList.size(); j++) {
            if (mGroupIdEdit->text() == cgDialog->groupList.at(j)->groupid) {
                _idHasModified = false;
                break;
            } else {
                if (j == cgDialog->groupList.size() - 1) {
                    _idHasModified = true;
                }
            }
        }
    });
    connect(mGroupNameEdit, &QLineEdit::textEdited, [=](QString txt){
        for (int j=0; j < cgDialog->groupList.size(); j++) {
            if (mGroupNameEdit->text() == cgDialog->groupList.at(j)->groupname) {
                _nameHasModified = false;
                break;
            } else {
                if (j == cgDialog->groupList.size() - 1) {
                    _nameHasModified = true;
                }
            }
        }
        if (mGroupNameEdit->text().length() > GROUPNAME_LENGTH) {
            mGroupNameEdit->setText(oldGroupName);
        } else {
            oldGroupName = txt;
        }
        if(mGroupNameEdit->text().isEmpty()) {
            mGroupNameTip = tr("GroupName's length must be between 1 and %1 characters!").arg(GROUPNAME_LENGTH);
        } else {
            mGroupNameTip = "";
        }

        mGroupNameTipLabel->setText(mGroupNameTip);
    });
}

bool CreateGroupDialog::eventFilter(QObject *watched, QEvent *event){

    if (event->type() == QEvent::FocusOut) {
        if (watched == mGroupNameEdit) {
            if (mGroupNameEdit->text().isEmpty()) {
                mGroupNameTip = tr("GroupName's length must be between 1 and %1 characters!").arg(GROUPNAME_LENGTH);
                mGroupNameTipLabel->setText(mGroupNameTip);
            }
        }
    }
    return KDialog::eventFilter(watched, event);
}
