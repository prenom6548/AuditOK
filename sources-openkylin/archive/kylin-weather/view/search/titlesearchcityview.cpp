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
#include "titlesearchcityview.h"
#include "global/viewvar.h"
#include <QDebug>
#include <window_management.hpp>
#include "windowmanage.hpp"

const int WIDGET_HEIGHT = 293;
const int LIST_WIDTH = 292;
const int LIST_HEIGHT = 288;
const QSize ITEM_SIZE = QSize(292,32);
const int MAX_DISPLAY_ITEMS = 5;  // 最多显示5个item
const int MARGIN_TOP_BOTTOM = 8;  // 上下边距
const int MARGIN_LEFT_RIGHT = 8;   // 左右边距
const int ITEM_SPACING = 2;        // item间隔
static QGSettings * m_gsettings = nullptr;

TitleSearchCityView::TitleSearchCityView(QWidget *parent) : QListView(parent)
{
    m_gsettings = new QGSettings("org.ukui.style");
    initInteraction();
    setWidgetStyle();
    updateRadius("widgetThemeName");
}

TitleSearchCityView::~TitleSearchCityView()
{

}

void TitleSearchCityView::setWidgetStyle()
{
    this->setAttribute(Qt::WA_TranslucentBackground, false);

    this->setFocusPolicy(Qt::NoFocus);

    // m_model = new QStandardItemModel(this);
    m_model = new TitleSearchModel();
    m_delegate = new TitleSearchListDelegate(this);
    m_nullTip = new QLabel(this); //无匹配城市文字提示
    m_nullTip->setAccessibleName("kylin-weather_QLabel_TitleSearchCityView_nullTip");
    m_nullTip->setFixedHeight(48);
    m_nullTip->hide();
    m_nullTip->setText(tr("there is no search city!"));
    m_nullTip->setMargin(8);
    m_nullTip->setAlignment(Qt::AlignCenter);

    this->setItemDelegate(m_delegate);           //添加代理--用来绘制
    this->setSpacing(0);
    this->setDragEnabled(false);
    this->setFrameShape(QListView::NoFrame);       //设置无边框
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           //设置无水平滑动条
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false); //设置滑动条无滑槽
    QPalette scrollarea = this->palette();
    scrollarea.setColor(QPalette::Base, ViewVar::s_colorWid);
    this->setPalette(scrollarea);
    this->setObjectName("listview");
    this->verticalScrollBar()->setObjectName("scrollBar");

    setModelforView();
    initConnect(); //信号槽
    setStyle("");
    return;
}

//搜索有结果和搜索输入为空的界面显示
void TitleSearchCityView::searchUiChange(bool hasCity)
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

void TitleSearchCityView::setViewModel(bool needSetModel)
{
    if (needSetModel) {
        // 计算显示的item数量，最多显示5个
        int itemCount = CoreVar::TITLE_SEARCH_LIST_NUM;
        int displayCount = (itemCount > MAX_DISPLAY_ITEMS) ? MAX_DISPLAY_ITEMS : itemCount;
        
        // 计算高度：item高度 * 显示数量 + item间隔 * (显示数量-1) + 上下边距
        int height = ITEM_SIZE.height() * displayCount + ITEM_SPACING * (displayCount - 1) + MARGIN_TOP_BOTTOM * 2;
        
        // 当item数量<=5时，隐藏垂直滚动条；否则显示滚动条
        if (itemCount <= MAX_DISPLAY_ITEMS) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            // 当滚动条显示时，增加高度以补偿滚动条占用的空间
            height += 16;
        }
        
        this->setFixedHeight(height);

        if (itemCount > 1) {
            this->setSpacing(ITEM_SPACING);
        }

        // 使用setViewportMargins设置视口边距，确保item不紧贴边框
        this->setViewportMargins(MARGIN_LEFT_RIGHT, MARGIN_TOP_BOTTOM, MARGIN_LEFT_RIGHT, MARGIN_TOP_BOTTOM);

        if (this->isHidden()) {
            this->show();
        }
        if (this->model() == Interaction::getInstance()->getTitleModel()) {
            return;
        }
        this->setModel(Interaction::getInstance()->getTitleModel());
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
void TitleSearchCityView::setModelforView()
{
    //判断文字提示是否显示，若显示则隐藏
    if (m_nullTip != nullptr && !(m_nullTip->isHidden())) {
        m_nullTip->hide();
    }
    this->setModel(Interaction::getInstance()->getTitleModel()); //设置model
}

void TitleSearchCityView::initConnect()
{
    //点击搜索列表其他部分，切换主界面城市
    // connect(m_delegate, &TitleSearchListDelegate::changeCityFromSearch, this, &TitleSearchCityView::slotChangeCityFromTitleSearchList);
    connect(this, &TitleSearchCityView::clicked, this, &TitleSearchCityView::slotChangeCityFromTitleSearchList);
    connect(ViewVar::getInstance(), &ViewVar::monitorThemeChanged, this, &TitleSearchCityView::setStyle);
    connect(m_gsettings,&QGSettings::changed, this, &TitleSearchCityView::updateRadius);
}

void TitleSearchCityView::initInteraction()
{
    //搜索为空或无输入时，处理界面展示
    connect(Interaction::getInstance(), &Interaction::searchNull, this, &TitleSearchCityView::slotSearchNull);
}

//无匹配城市时显示--文字提示
void TitleSearchCityView::slotSearchNull()
{
    this->setFixedSize(300, 50);                 //设置view大小
    // 判断当当前的搜索结果是否返回给标题栏的搜索
    if (!ViewVar::m_isTitleSearch) {
        return;
    }
    emit cityListNull(true);
    m_nullTip->show();
    m_nullTip->setText(tr("there is no search city!"));
}

void TitleSearchCityView::slotChangeCityFromTitleSearchList(QModelIndex index)
{
    QString cityId = index.data(TitleSearchModel::CityId).toString();

    // 更新 全局变量 当前城市类别 名称 和 ID
    CoreVar::CURRENT_CITY_NAME = index.data(TitleSearchModel::CityName).toString();
    CoreVar::CURRENT_CITY_ID = cityId;
    if (CoreVar::CURRENT_COLLECT_LIST.contains(cityId)) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
    } else {
        CoreVar::CURRENT_CITY_TYPE = CityType::Other;
    }
    if (CoreVar::LOCATION_CITY_ID == cityId) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Location;
    }

    emit sigTitleSearchChangeCityToMain();

    Interaction::getInstance()->changeCityFromTitleSearch(cityId);

    return;
}

void TitleSearchCityView::setStyle(QString themeStyle)
{
    if (themeStyle == "") {
        if (ViewVar::s_themeStyle) {
            lightListView();
        } else {
            // 改下拉列表属性
            darkListview();
        }
        return;
    }
    if ("ukui-dark" == themeStyle || "ukui-black" == themeStyle ) {
        darkListview();
        updateRadius("widgetThemeName");

    } else {
        lightListView();
        updateRadius("widgetThemeName");
    }
}

void TitleSearchCityView::darkListview()
{
    QColor color = this->palette().color(QPalette::Base);
    this->setStyleSheet("#listview{margin:0px;padding:0px;border:none;border-radius:6px;outline:none;background-color:rgba(0,0,0,1);}"
                            "#listview::item:selected:!active{background-color:rgba(0,0,0,1);border-radius:6px;}"
                            "#listview::item:selected:active{background-color:rgba(0,0,0,1);border-radius:6px;}"
                            "#listview::item:hover{background-color:rgba(0,0,0,1);border-radius:6px;}");

//    this->verticalScrollBar()->setStyleSheet("#scrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:1px;}" \
//                                                 "#scrollBa::sub-line:vertical{subcontrol-origin:margin;border:none;height:6px;}" \
//                                                 "#scrollBa::up-arrow:vertical{subcontrol-origin:margin;background-color:transparent;height:6px;}" \
//                                                 "#scrollBa::sub-page:vertical{background-color:transparent;}" \
//                                                 "#scrollBa::handle:vertical{background-color:transparent;width:1px;}" \
//                                                 "#scrollBa::handle:vertical:hover{background-color:transparent;width:1px;}" \
//                                                 "#scrollBa::handle:vertical:pressed{background-color:transparent;width:1px;}" \
//                                                 "#scrollBa::add-page:vertical{background-color:transparent;}" \
//                                                 "#scrollBa::down-arrow:vertical{background-color:transparent;}" \
//                                                 "#scrollBa::add-line:vertical{subcontrol-origin:margin;border:none;height:6px;}");
    return;
}

void TitleSearchCityView::lightListView()
{
    QColor color = this->palette().color(QPalette::Base);
    this->setStyleSheet("#listview{margin:0px;padding:0px;border:none;border-radius:6px;outline:none;background-color:rgba(255,255,255,1);}"
                            "#listview::item:selected:!active{background-color:rgba(0,0,0,1);border-radius:6px;}"
                            "#listview::item:selected:active{background-color:rgba(0,0,0,1);border-radius:6px;}"
                            "#listview::item:hover{background-color:rgba(0,0,0,1);border-radius:6px;}");
//    this->verticalScrollBar()->setStyleSheet("#scrollBa:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:1px;}" \
//                                                "#scrollBa::sub-line:vertical{subcontrol-origin:margin;border:none;height:6px;}" \
//                                                "#scrollBa::up-arrow:vertical{subcontrol-origin:margin;background-color:transparent;height:6px;}" \
//                                                "#scrollBa::sub-page:vertical{background-color:transparent;}" \
//                                                "#scrollBa::handle:vertical{background-color:transparent;width:1px;}" \
//                                                "#scrollBa::handle:vertical:hover{background-color:transparent;width:1px;}" \
//                                                "#scrollBa::handle:vertical:pressed{background-color:transparent;width:1px;}" \
//                                                "#scrollBa::add-page:vertical{background-color:transparent;}" \
//                                                "#scrollBa::down-arrow:vertical{background-color:transparent;}" \
//                                                "#scrollBa::add-line:vertical{subcontrol-origin:margin;border:none;height:6px;}");
    return;
}

void TitleSearchCityView::updateRadius(QString key)
{
    if(key == "widgetThemeName"){
        QString themeData = m_gsettings->get("widgetThemeName").toString();
        QString themeStyle = m_gsettings->get("styleName").toString();
        if (themeData == "classical"){
            if (("ukui-dark" == themeStyle || "ukui-black" == themeStyle )){
            this->setStyleSheet("#listview{margin:0px;padding:0px;border:none;border-radius:0px;outline:none;background-color:rgba(0,0,0,1);}"
                                    "#listview::item:selected:!active{background-color:rgba(0,0,0,1);border-radius:0px;}"
                                    "#listview::item:selected:active{background-color:rgba(0,0,0,1);border-radius:0px;}"
                                    "#listview::item:hover{background-color:rgba(0,0,0,1);border-radius:0px;}");
            }else {
                this->setStyleSheet("#listview{margin:0px;padding:0px;border:none;border-radius:0px;outline:none;background-color:rgba(255,255,255,1);}"
                                        "#listview::item:selected:!active{background-color:rgba(0,0,0,1);border-radius:0px;}"
                                        "#listview::item:selected:active{background-color:rgba(0,0,0,1);border-radius:0px;}"
                                        "#listview::item:hover{background-color:rgba(0,0,0,1);border-radius:0px;}");
            }
        }
    }
    else setStyle("");
}
