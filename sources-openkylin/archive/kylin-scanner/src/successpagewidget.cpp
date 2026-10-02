/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "successpagewidget.h"
#include <QDebug>

SuccessPageWidget::SuccessPageWidget(QWidget *parent) :
    QWidget(parent),
    m_leftStackSuccessPageWidget(new QStackedWidget(this)),
    m_scanSettingsWidget(new ScanSettingsWidget(this)),
    m_scanSettingsSrollArea(new QScrollArea(this)),
    m_leftImageHandleSuccessPageWidget(new LeftImageHandleSuccessPageWidget(this)),
    m_leftSuccessPageWidget(new LeftSuccessPageWidget(this)),
    m_successPageHLayout(new QHBoxLayout(this))
{
    setupGui();
    initConnect();
}

void SuccessPageWidget::setupGui()
{
    m_leftStackSuccessPageWidget->addWidget(m_leftImageHandleSuccessPageWidget);
    m_leftStackSuccessPageWidget->addWidget(m_leftSuccessPageWidget);
    m_leftStackSuccessPageWidget->setCurrentWidget(m_leftSuccessPageWidget);

    m_scanSettingsSrollArea->setWidget(m_scanSettingsWidget);
    m_scanSettingsSrollArea->setWidgetResizable(true);
    m_scanSettingsSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scanSettingsSrollArea->setFrameShape(QFrame::NoFrame);
    m_scanSettingsWidget->adjustSize();
    m_scanSettingsSrollArea->setFixedWidth(ScanSettingsWidgetWidth);

    m_successPageHLayout->setSpacing(0);
    m_successPageHLayout->addWidget(m_leftStackSuccessPageWidget);
    m_successPageHLayout->addWidget(m_scanSettingsSrollArea);
    m_successPageHLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_successPageHLayout);
}

void SuccessPageWidget::initConnect()
{
    connect(g_user_signal, &GlobalUserSignal::changeToConnectuccessPageSignal, this, [=](){m_leftStackSuccessPageWidget->setCurrentWidget(m_leftSuccessPageWidget);});
}

void SuccessPageWidget::showLeftImageHandleSuccessPage()
{
    m_leftStackSuccessPageWidget->setCurrentWidget(m_leftImageHandleSuccessPageWidget);
}

void SuccessPageWidget::updateScanSettingsSlot(bool isWork)
{
    m_scanSettingsWidget->updateSettingsForDetectDevices();
}

