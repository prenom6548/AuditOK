/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "searchview.h"
#include <QLineEdit>
#include "dbusclient.h"
#include "formatconversion.h"
#include "locationget.h"
namespace kmre_gps
{
SearchView::SearchView(QWidget *parent) : QWidget(parent)
{
    setFixedSize(670, 340);
    initWidget();
    initConnection();
}

void SearchView::initWidget()
{
    //搜索框
    m_searchBox = new SearchBox(this);

    //搜索按钮
    //    m_searchBtn = new QPushButton(this);
    //    m_searchBtn->resize(40, 40);
    //    m_searchBtn->move(526, 0);
    //    m_searchBtn->setIcon(QIcon(":/res/search.png"));
    //    m_searchBtn->setIconSize(QSize(40, 40));

    //当前位置显示
    m_nowLocationBtn = new QPushButton(this);
    m_nowLocationBtn ->resize(100,36);
    m_nowLocationBtn ->move(560,0);
    m_nowLocationBtn ->setText(tr("current-location"));

    //搜索结果列表
    m_locationlistWidget = new LocationListWidget(this);
    m_locationlistWidget->move(0, 40);

    //地图
    setFixedSize(671, 340);
    m_view = new QWebEngineView(this);
    m_view->resize(671, 340);
    m_view->move(0, 40);
    m_view->page()->load(QUrl("qrc:/res/map.html"));
    m_view->lower();
}

void SearchView::initConnection()
{
    connect(m_searchBox, &SearchBox::textChanged, this, &SearchView::searchClicked);
    connect(m_searchBox, &SearchBox::siglistHide, this, &SearchView::slotlistHide);
    // connect(m_searchBtn, &QPushButton::clicked, this, &SearchView::searchClicked);
    connect(m_locationlistWidget, &LocationListWidget::sigCurrentShow, this, &SearchView::slotcurrentShow);
    connect(m_nowLocationBtn, &QPushButton::clicked, this, &SearchView::slotNowClicked);
}

void SearchView::searchClicked()
{
    QString inputText = m_searchBox->text().trimmed().toLower();
    if (inputText != "") {
        if (LocationGet::getInstance()->getLocation(inputText)) {
            m_locationlistWidget->listShow();
        }
    } else {
        m_locationlistWidget->hide();
    }
}

bool SearchView::event(QEvent *event)
{
    //点击别处，使搜索框失去焦点,搜索按钮和文字提示居中显示
    if (event->type() == QEvent::MouseButtonPress) {
        m_searchBox->clearFocus();
        m_locationlistWidget->hide();
        if (m_searchBox->text() == "") {
            m_searchBox->m_searchBottom->move(200, (m_searchBox->height() - m_searchBox->m_searchBottom->height()) / 2);
            m_searchBox->m_searchText->show();
            m_searchBox->m_searchBottom->show();
        }
    }
    return QWidget::event(event);
}

void SearchView::slotcurrentShow()
{
    Q_EMIT sigcurrentShow();
}

void SearchView::slotlistHide()
{
    m_locationlistWidget->hide();
}

void SearchView::slotNowClicked(){
    Q_EMIT sigupdateCurrentShow();
}

SearchView::~SearchView() {}

} // namespace kmre_gps
