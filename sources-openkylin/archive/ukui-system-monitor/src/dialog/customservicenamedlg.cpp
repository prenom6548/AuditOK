/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "customservicenamedlg.h"

#include <QDebug>
#include <QMessageBox>
#include <QIcon>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "../controls/mydialog.h"
#include "../macro.h"
#include "../util.h"

CustomServiceNameDlg::CustomServiceNameDlg(const QString &title, const QString &message, QWidget *parent)
    : MyDialog(title, "", parent)
{
    this->setFixedSize(372,160);
    m_nameLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_nameLineEdit, "CustomServiceNameDlg", "", "custom name lineedit of CustomServiceNameDlg");
    addContent(m_nameLineEdit);

    addButton(tr("OK"), true);
    addButton(tr("Cancel"));

    connect(this,
            &CustomServiceNameDlg::buttonClicked,
            this,
            &CustomServiceNameDlg::onButtonClicked);
}

CustomServiceNameDlg::~CustomServiceNameDlg()
{
}

void CustomServiceNameDlg::onButtonClicked(int index, const QString &)
{
    if (index == 0) {
        m_strCustomName = m_nameLineEdit->text();
        setResult(QMessageBox::Ok);
    } else {
        setResult(QMessageBox::Cancel);
    }
}

// 显示事件
void CustomServiceNameDlg::showEvent(QShowEvent *)
{
    // 聚焦输入框
    m_nameLineEdit->setFocus();
}

QString CustomServiceNameDlg::getCustomServiceName()
{
    return m_strCustomName;
}

void CustomServiceNameDlg::setCustomServiceName(QString strSvcName)
{
    m_strCustomName = strSvcName;
}

int CustomServiceNameDlg::getResult()
{
    return m_nResult;
}

void CustomServiceNameDlg::setResult(int nRet)
{
    m_nResult = nRet;
}
