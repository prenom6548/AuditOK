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
#include "searchcitylist.h"

SearchCityLIst::SearchCityLIst(QWidget *parent) : QTableView(parent)
{
    initInteraction();                                        //信号槽--后端信号
    this->setFixedSize(ViewVar::SEARCH_LIST);                 //设置view大小
    this->setAttribute(Qt::WA_TranslucentBackground);         //设置背景透明
    this->setAlternatingRowColors(false);                     //设置无相邻行颜色区分
    this->setShowGrid(false);                                 //设置无网格线
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    //设置行高列宽
    this->verticalHeader()->setDefaultSectionSize(ViewVar::SEARCH_ITEMH);   //列高
    this->horizontalHeader()->setDefaultSectionSize(ViewVar::SEARCH_ITEMW); //行高
    //    this->horizontalHeader()->setDefaultSectionSize(20);                    //设置行间距
    //    this->horizontalHeader()->setMinimumSectionSize(20);                    //设置行间距
    m_delegate = new SearchDelegate(this);       //添加代理--用来绘制
    this->setItemDelegate(m_delegate);           //添加代理--用来绘制
    this->horizontalHeader()->setVisible(false); // 水平不可见
    this->verticalHeader()->setVisible(false);   // 垂直不可见

    this->setSelectionMode(QAbstractItemView::NoSelection);               //设置不可多选
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           //设置无水平滑动条
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false); //设置滑动条无滑槽
    QPalette scrollarea = this->palette();
    scrollarea.setColor(QPalette::Base, ViewVar::s_colorWid);
    this->setPalette(scrollarea);
    //设置无边框
    this->setFrameShape(NoFrame);
    m_nullTip = new QLabel(this); //无匹配城市文字提示
    m_nullTip->setAccessibleName("kylin-weather_QLabel_SearchCityList_nullTip");
    m_nullTip->setFixedWidth(ViewVar::SEARCH_NONEW);
    m_nullTip->hide();
    m_model = new QStandardItemModel(this);
    setModelforView();
    initConnect(); //信号槽
}
//搜索有结果和搜索输入为空的界面显示
void SearchCityLIst::searchUiChange(bool hasCity)
{
    if (hasCity) {
        if (!m_nullTip->isHidden()) {
            m_nullTip->hide();
        }
    } else {
        if (m_nullTip->isHidden()) {
            m_nullTip->show();
            m_nullTip->setText(tr("there is no search city!"));
        }
    }
}

void SearchCityLIst::setViewModel(bool needSetModel)
{
    if (needSetModel) {
        if (this->isHidden()) {
            this->show();
        }
        if (this->model() == Interaction::getInstance()->getCityModel()) {
            return;
        }
        this->setModel(Interaction::getInstance()->getCityModel());
    } else {
        if (!this->isHidden()) {
            this->hide();
        }
        if (this->model() == m_model) {
            return;
        }
    }
}
//设置模型model
void SearchCityLIst::setModelforView()
{
    //判断文字提示是否显示，若显示则隐藏
    if (m_nullTip != nullptr && !(m_nullTip->isHidden())) {
        m_nullTip->hide();
    }
    this->setModel(Interaction::getInstance()->getCityModel()); //设置model
    //为view的每一行设置行高，必须放在setModel后，不然不生效---处理偶现行宽改变问题
    for (int i = 0; i < this->model()->rowCount(); i++) {
        this->setRowHeight(i, ViewVar::SEARCH_ITEMH);
    }
    for (int j = 0; j < this->model()->columnCount(); j++) {
        this->setColumnWidth(j, ViewVar::SEARCH_ITEMW);
    }
}

void SearchCityLIst::initConnect()
{
    //点击"+"号，添加收藏城市
    connect(m_delegate, &SearchDelegate::addCollectCity, this, &SearchCityLIst::addCollectCity);
    //点击搜索列表其他部分，切换主界面城市
    connect(m_delegate, &SearchDelegate::changeCityFromSearch, this, &SearchCityLIst::changeCityFromSearch);
}

void SearchCityLIst::initInteraction()
{
    //搜索为空或无输入时，处理界面展示
    connect(Interaction::getInstance(), &Interaction::searchNull, this, &SearchCityLIst::searchNull);
}
//无匹配城市时显示--文字提示
void SearchCityLIst::searchNull()
{
    if (ViewVar::m_isTitleSearch) {
        return;
    }
    emit cityListNull(true);
    m_nullTip->show();
    m_nullTip->setText(tr("there is no search city!"));
}
