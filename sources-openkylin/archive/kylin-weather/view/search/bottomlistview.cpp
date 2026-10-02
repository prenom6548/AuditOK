/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "bottomlistview.h"
#include "global/viewvar.h"
#include <QDebug>
#include <window_management.hpp>
#include "windowmanage.hpp"

const int LIST_ITEM_WIDTH_SIZE = 30;

BottomListView::BottomListView(QWidget *parent) : QListView(parent)
{
    setWidgetStyle();
}

BottomListView::~BottomListView()
{

}

void BottomListView::setWidgetStyle()
{
    this->setFocusPolicy(Qt::NoFocus);

    // m_model = new QStandardItemModel(this);
    m_model = new TitleSearchModel();
    m_delegate = new BottomListDelegate(this);

    this->setFixedHeight(50);                 //设置view大小
    this->setItemDelegate(m_delegate);           //添加代理--用来绘制
    this->setSpacing(0);
    this->setDragEnabled(false);
    this->setFrameShape(QListView::NoFrame);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    this->setFlow(QListView::LeftToRight);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           //设置无水平滑动条
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false); //设置滑动条无滑槽
    QPalette scrollarea = this->palette();
    scrollarea.setColor(QPalette::Base, ViewVar::s_colorWid);
    this->setPalette(scrollarea);
    //设置无边框
    // this->setFrameShape(NoFrame);

    // setModelforView();
    initConnect(); //信号槽
    return;
}

//设置模型model
void BottomListView::setModelforView()
{
    if (m_initFirst) {
        this->setModel(Interaction::getInstance()->getBottomCollectModel()); //设置model
        this->setFixedWidth( this->model()->rowCount() * LIST_ITEM_WIDTH_SIZE);
        m_initFirst = false;
    }
}

void BottomListView::initConnect()
{
    //点击搜索列表其他部分，切换主界面城市
    connect(this, &BottomListView::clicked, this, &BottomListView::slotChangeCityFromTitleSearchList);
    connect(Interaction::getInstance(), &Interaction::sigShowBottomList, this, &BottomListView::slotShowBottomList);
    connect(Interaction::getInstance(), &Interaction::sigBottomModeGet, this, &BottomListView::setModelforView);
    
}

void BottomListView::slotChangeCityFromTitleSearchList(QModelIndex index)
{
    if (!CoreVar::GET_CITY_INFO) {
        return;
    }
    QString cityId = index.data(TitleSearchModel::CityId).toString();
    emit sigTitleSearchChangeCityToMain();
    Interaction::getInstance()->changeCityFromTitleSearch(cityId);
    
    // 更新全局变量 当前城市类别
    int style = index.data(TitleSearchModel::CityStyle).toInt();
    if (style == TitleSearchModel::CityType::Location) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Location;
    } else if (style == TitleSearchModel::CityType::Collection) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
    } else {
        CoreVar::CURRENT_CITY_TYPE = CityType::Other;
    }
    return;
}

void BottomListView::slotShowBottomList(int itemSize)
{
    this->setModel(Interaction::getInstance()->getBottomCollectModel()); //设置model
    this->setFixedWidth( itemSize * LIST_ITEM_WIDTH_SIZE);

    return;
}
