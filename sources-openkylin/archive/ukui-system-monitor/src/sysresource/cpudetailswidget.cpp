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
#include "cpudetailswidget.h"

#include <libkycpu.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

CpuDetailsWidget::CpuDetailsWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

CpuDetailsWidget::~CpuDetailsWidget()
{
}

void CpuDetailsWidget::initUI()
{
    setTitle(tr("Processor"));
    setDetail(kdk_cpu_get_model());

    m_pCpuGraphScrollArea = new ChartViewScrollArea(ChartItemWidget::ChartViewTypes::CPU_CHART);

    mCpuGraphVLayout =  new QVBoxLayout();
    mCpuGraphVLayout->setContentsMargins(0,0,0,0);
    mCpuGraphVLayout->setSpacing(0);
    mCpuGraphVLayout->addWidget(m_pCpuGraphScrollArea);

    mDetailModel = new CpuDetailsModel();
    mDetailList =  new DetailListTable(mDetailModel);
    KDK_EXTEND_ALL_INFO_FORMAT(mDetailList, "CpuDetailsWidget", "", "");

    mListHLayout = new QHBoxLayout();
    mListHLayout->addWidget(mDetailList);

    mMainLayout->addLayout(mCpuGraphVLayout);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(mListHLayout);
}

void CpuDetailsWidget::onUpdateCpuChartView(QVector<double> usage)
{
    m_pCpuGraphScrollArea->onUpdateCpuChartData(usage);
}

void CpuDetailsWidget::onUpdateData()
{
    mDetailModel->onModelUpdated();
}
