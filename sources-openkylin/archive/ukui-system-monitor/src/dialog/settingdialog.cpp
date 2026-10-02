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
#include "settingdialog.h"

#include <QFrame>
#include <QPainter>
#include <QVariant>
#include <QApplication>

#include <kysdk/applications/klabel.h>

#include "macro.h"

SettingDialog::SettingDialog(QWidget *parent) : KDialog(parent)
{
    setWindowIcon(QIcon::fromTheme("ukui-system-monitor"));
    setWindowTitle(tr("System Monitor"));

    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid)) {
        m_pSystemMonitorGSetting = new QGSettings(ifid);
    }

    initUI();
    initStatus();
    setConnect();
}

SettingDialog::~SettingDialog()
{

}

void SettingDialog::initUI()
{
    setFixedSize(424, 225);
    KLabel *exitSettingTitleLabel = new KLabel();
    exitSettingTitleLabel->setLabelType(KLabelType::NormalType);
    exitSettingTitleLabel->setText(tr("Select the action to be executed when closing the window"));
    QFont ft = exitSettingTitleLabel->font();
    ft.setBold(true);
    exitSettingTitleLabel->setFont(ft);

    m_pMinimizeBtn = new QRadioButton(tr("Minimize to System Tray"));
    m_pExitBtn = new QRadioButton(tr("Exit the program"));
    m_pRadioBtnGroup = new QButtonGroup();
    m_pRadioBtnGroup->addButton(m_pMinimizeBtn, 0);
    m_pRadioBtnGroup->addButton(m_pExitBtn, 1);

    m_pCancelBtn = new KPushButton();
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton();
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    m_pConfirmBtn->setEnabled(true);
    QHBoxLayout *bottomBtnsHorLayout= new QHBoxLayout;
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addSpacing(8);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    m_pMainVerLayout = new QVBoxLayout;
    m_pMainVerLayout->setSpacing(0);
    m_pMainVerLayout->setContentsMargins(24, 8, 24, 24);
    m_pMainVerLayout->addWidget(exitSettingTitleLabel);
    m_pMainVerLayout->addWidget(m_pMinimizeBtn);
    m_pMainVerLayout->addWidget(m_pExitBtn);
    m_pMainVerLayout->addSpacing(24);
    m_pMainVerLayout->addLayout(bottomBtnsHorLayout);

    mainWidget()->setLayout(m_pMainVerLayout);
}

void SettingDialog::initStatus()
{
    bool directlyExit = m_pSystemMonitorGSetting->get(DIRECTLY_EXIT).toBool();
    m_pExitBtn->setChecked(directlyExit);
    m_pMinimizeBtn->setChecked(!directlyExit);
}

void SettingDialog::setConnect()
{
    connect(m_pCancelBtn, &KPushButton::clicked, this, [=](){
        this->close();
    });

    connect(m_pConfirmBtn, &KPushButton::clicked, this, [=](){
        int checkedBtnId = m_pRadioBtnGroup->checkedId();
        m_pSystemMonitorGSetting->set(DIRECTLY_EXIT, checkedBtnId == 1 ? true : false);

        this->close();
    });
}

void SettingDialog::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}
