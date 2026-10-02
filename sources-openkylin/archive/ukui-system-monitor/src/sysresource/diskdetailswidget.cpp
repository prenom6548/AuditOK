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
#include "diskdetailswidget.h"

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

DiskDetailsWidget::DiskDetailsWidget(QWidget *parent): BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

DiskDetailsWidget::~DiskDetailsWidget()
{

}

void DiskDetailsWidget::initUI()
{
    setTitle(tr("Disk"));

    m_pDiskGraphScrollArea =  new ChartViewScrollArea(ChartItemWidget::ChartViewTypes::BLOCK_CHART);

    // 详情列表
    m_pDetailModel = new DiskDetailsModel();
    m_pDetailList =  new DetailListTable(m_pDetailModel);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pDetailList, "DiskDetailsWidget", "", "");
    m_pDetailList->setFixedHeight(108);
    mCurrentDiskName = m_pDiskGraphScrollArea->getCurrentChartName();
    m_pDetailModel->refreshDiskInfo(mCurrentDiskName);

    QHBoxLayout *listHLayout = new QHBoxLayout(this);
    listHLayout->addWidget(m_pDetailList);

    mMainLayout->addWidget(m_pDiskGraphScrollArea);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(listHLayout);

    connect(m_pDiskGraphScrollArea, &ChartViewScrollArea::chartItemClicked, this, [=](const QString &diskName){
        mCurrentDiskName = diskName;
    });
}

void DiskDetailsWidget::onUpdateDiskChartView()
{
    m_pDiskGraphScrollArea->onUpdateDiskChartData();
}

void DiskDetailsWidget::onUpdateData()
{
    m_pDetailModel->refreshDiskInfo(mCurrentDiskName);
    // 刷新数据列表
    m_pDetailModel->onModelUpdated();
}

