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
#include "collectcity.h"

CollectCity::CollectCity(QWidget *parent) : QWidget(parent)
{
    initInteraction(); //信号槽--后端信号

    initWidget();
    
    setModelforView();
    initConnect();
}

void CollectCity::initWidget()
{
    m_cityTitle = new QLabel(this);              //“收藏城市”
    m_button = new QPushButton(this);            // 编辑按钮
    m_noneCity = new QLabel(this);               //“暂无”
    // m_cityList = new QTableView(this);           // 收藏城市列表
    m_cityList = new QListView(this);           // 收藏城市列表
    m_delegate = new CollectItemDelegate(this);
    m_hotcity = new HotCity(this);

    m_hLayout = new QHBoxLayout();
    m_vLayout = new QVBoxLayout();

    m_cityTitle->setAccessibleName("kylin-weather_QLabel_CollectCity_cityTitle");
    m_button->setAccessibleName("kylin-weather_QPushButton_CollectCity_button");
    m_noneCity->setAccessibleName("kylin-weather_QLabel_CollectCity_noneCity");
    m_cityList->setAccessibleName("kylin-weather_QTableView_CollectCity_cityList");
    m_hotcity->setAccessibleName("kylin-weather_HotCity_hotcity");

    initStyle();
    return;
}

void CollectCity::initStyle()
{
    this->setFixedSize(ViewVar::COLLECT);
    this->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明

    m_cityTitle->setText(tr("Collect City"));
    m_cityTitle->setFixedWidth(this->width());
    m_cityTitle->move(ViewVar::COLLECT_TITLE);

    m_button->setText(tr("Edit"));

    m_noneCity->setText(tr("None"));
    m_noneCity->setFixedWidth(this->width());
    m_noneCity->hide();

    m_cityList->setProperty("useIconHighlightEffect", 0x2);
    QSize adjustSize = ViewVar::COLLECT_LIST;
    adjustSize.setHeight(ViewVar::COLLECT_LIST.height() - ViewVar::HOT.height() + 32);
    m_cityList->setFixedSize(adjustSize);
    //设置代理
    m_cityList->setItemDelegate(m_delegate);
    m_cityList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           //设置无水平滑动条
    m_cityList->verticalScrollBar()->setProperty("drawScrollBarGroove", false); //设置滑动条无滑槽
    m_cityList->setFrameShape(QListView::NoFrame);                                 //设置无边框
    m_cityList->setAttribute(Qt::WA_TranslucentBackground);                     //设置背景透明
    m_cityList->setSpacing(10);                   //为视图设置控件间距
    m_cityList->setViewMode(QListView::IconMode); //设置Item图标显示
    m_cityList->setDragEnabled(false);    
    m_cityList->setFlow(QListView::LeftToRight);     
    m_cityList->setResizeMode(QListView::Adjust);

    m_cityList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_cityList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_cityList->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    QPalette scrollarea = m_cityList->palette();
    scrollarea.setColor(QPalette::Base, ViewVar::s_colorWid);
    m_cityList->setPalette(scrollarea);

    m_hLayout->addWidget(m_cityTitle);
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_button);
    m_hLayout->setContentsMargins(15, 20, 35, 0);

    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addWidget(m_cityList);
    m_vLayout->addSpacing(15);
    m_vLayout->addWidget(m_hotcity);
    m_vLayout->addSpacing(5);
    m_vLayout->addStretch();
    m_vLayout->setMargin(0);
//    m_vLayout->setContentsMargins(10, 0, 0, 0);
    this->setLayout(m_vLayout);

    return;
}

//设置model
void CollectCity::setModelforView()
{
    m_cityList->setModel(Interaction::getInstance()->getCollectModel());
    if ((m_cityList->model()->rowCount() < 1 || m_cityList->model()->rowCount() == 1) && m_cityList->model()->columnCount() < 2) {
        m_button->setEnabled(false);
    } else {
        m_button->setEnabled(true);
    } 
}
//信号槽--后端信号
void CollectCity::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::collectNull, this, &CollectCity::changeItem); //判断收藏城市为空时，交换两个控件的显隐
    connect(Interaction::getInstance(), &Interaction::collectShow, this, &CollectCity::changeShow); //判断不为空，两个控件的显隐状态
}
//信号槽
void CollectCity::initConnect()
{
    connect(m_delegate, &CollectItemDelegate::delCollectCity, Interaction::getInstance(), &Interaction::delCollectCity); //删除收藏列表城市
    connect(m_delegate, &CollectItemDelegate::changeCityFromCollect, this, &CollectCity::changeCityFromCollect); //从收藏城市改变当前城市
    connect(m_delegate, &CollectItemDelegate::sigAddCityFromCollect, this, &CollectCity::sigChangWidget); // 添加收藏城市信号切换界面到收藏城市界面
    connect(m_button, &QPushButton::clicked, this, &CollectCity::slotClickButton);
    connect(m_hotcity, &HotCity::setHotCity, this, &CollectCity::changeToHotCity);
}
//收藏城市为空，收藏列表隐藏
void CollectCity::changeItem()
{
    if ((m_cityList->model()->rowCount() < 1 || m_cityList->model()->rowCount() == 1) && m_cityList->model()->columnCount() < 2) {
        m_button->setText(tr("Edit"));
        m_cityTitle->setText(tr("Collection City"));
        ViewVar::m_editStyle = false;
        this->update();
        m_button->setEnabled(false);
    }
}
//判断不为空，两个控件的显隐状态
void CollectCity::changeShow()
{
    m_button->setEnabled(true);
    if (!m_cityList->isHidden()) {
        return;
    }
    m_cityList->show();
    m_noneCity->hide();
    // emit changeItemPos();
}

void CollectCity::slotClickButton()
{
    if (ViewVar::m_editStyle == false) {
        m_button->setText(tr("Finish"));
        m_cityTitle->setText(tr("City Management"));
        ViewVar::m_editStyle = true;
        this->update();
    } else {
        m_button->setText(tr("Edit"));
        m_cityTitle->setText(tr("Collection City"));
        ViewVar::m_editStyle = false;
        this->update();
    }
    return;
}

void CollectCity::changeToHotCity(QString code)
{
    Interaction::getInstance()->changeCityFromTitleSearch(code);
}

void CollectCity::changeEditBtnStyle()
{
    m_button->setEnabled(true);
    return;
}

void CollectCity::changeCollectWidgetStyle()
{

    return;
}
