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
#include "onclosedialog.h"
#include <QApplication>
#include <kysdk/applications/klabel.h>

#include "macro.h"

#define ICON_EDIT_SIZE 16,16

OnCloseDialog::OnCloseDialog(QWidget *parent) : KDialog(parent)
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

OnCloseDialog::~OnCloseDialog()
{

}

void OnCloseDialog::initUI()
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

    m_pRememberCheckBox = new QCheckBox(tr("Do not show again"));
    m_pCancelBtn = new KPushButton();
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton();
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    m_pConfirmBtn->setEnabled(true);
    QHBoxLayout *bottomBtnsHorLayout= new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(8);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addWidget(m_pRememberCheckBox);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
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

void OnCloseDialog::initStatus()
{
    bool directlyExit = m_pSystemMonitorGSetting->get(DIRECTLY_EXIT).toBool();
    m_pExitBtn->setChecked(directlyExit);
    m_pMinimizeBtn->setChecked(!directlyExit);
    // 获取不再提示是否勾选，默认勾选，根据用户配置修改
    bool rememberCheckBoxSelectedStatus = m_pSystemMonitorGSetting->get(INQUIRIESSELECTEDSTATUS).toBool();
    m_pRememberCheckBox->setChecked(rememberCheckBoxSelectedStatus);
}

void OnCloseDialog::setConnect()
{
    connect(m_pCancelBtn, &KPushButton::clicked, this, [=](){
        this->close();
    });

    connect(m_pConfirmBtn, &KPushButton::clicked, this, [=](){
        int checkedBtnId = m_pRadioBtnGroup->checkedId();
        m_pSystemMonitorGSetting->set(DIRECTLY_EXIT, checkedBtnId == 1 ? true : false);
        if (checkedBtnId == 1) {
            Q_EMIT needSaveSetting();
            qApp->exit();
        } else {
            Q_EMIT closeMainWindow();
        }

        bool inquiries = m_pRememberCheckBox->isChecked();
        // 设置是否不再提示
        m_pSystemMonitorGSetting->set(INQUIRIES, !inquiries);

        // 记住不再提示复选框是否勾选
        m_pSystemMonitorGSetting->set(INQUIRIESSELECTEDSTATUS, inquiries);

        this->close();
    });

}

void OnCloseDialog::keyPressEvent(QKeyEvent * event)
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

