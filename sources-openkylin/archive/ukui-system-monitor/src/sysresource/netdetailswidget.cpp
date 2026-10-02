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
#include "netdetailswidget.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

NetDetailsWidget::NetDetailsWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

NetDetailsWidget::~NetDetailsWidget()
{

}

void NetDetailsWidget::initUI()
{
    setTitle(tr("Network"));

    mNetGraphScrollArea =  new ChartViewScrollArea(ChartItemWidget::ChartViewTypes::NET_CHART);

    // 详情列表
    mDetailModel = new NetDetailsModel();
    NetInfoDetailItemDelegate *netItemDelegate = new NetInfoDetailItemDelegate();
    mDetailList =  new DetailListTable(mDetailModel, netItemDelegate);
    KDK_EXTEND_ALL_INFO_FORMAT(mDetailList, "NetDetailsWidget", "", "");
    mDetailList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mCurrentNetName = mNetGraphScrollArea->getCurrentChartName();
    mDetailModel->refreshNetifInfo(mCurrentNetName);

    mListHLayout = new QHBoxLayout();
    mListHLayout->addWidget(mDetailList);

    mMainLayout->addWidget(mNetGraphScrollArea);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(mListHLayout);

    connect(mNetGraphScrollArea, &ChartViewScrollArea::chartItemClicked, this, [=](const QString &mac){
        mDetailModel->refreshNetifInfo(mac);
        mCurrentNetName = mac;
    });
}

void NetDetailsWidget::onUpdateNetChartView()
{
    mNetGraphScrollArea->onUpdateNetChartData();
}

void NetDetailsWidget::onUpdateData()
{
    mDetailModel->refreshNetifInfo(mCurrentNetName);
}
