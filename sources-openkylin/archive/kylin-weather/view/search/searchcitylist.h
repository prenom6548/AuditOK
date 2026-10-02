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
#ifndef SEARCHCITYLIST_H
#define SEARCHCITYLIST_H

#include <QWidget>
#include <QTableView>
#include <QScrollArea>
#include <QScrollBar>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include "controller/Interaction.h"
#include "searchdelegate.h"
class SearchCityLIst : public QTableView
{
    Q_OBJECT
    friend class TestSearchCityList;
public:
    explicit SearchCityLIst(QWidget *parent = nullptr);
    void searchUiChange(bool hasCity); //搜索输入为空和有搜索结果的界面显示
    void setViewModel(bool needSetModel);

private:
    void initConnect();     //信号槽
    void initInteraction(); //后端-信号槽
    void searchNull();      //搜索为空的界面处理
    void setModelforView(); //设置模型
    QLabel *m_nullTip;
    SearchDelegate *m_delegate = nullptr;
    QStandardItemModel *m_model = nullptr;

signals:
    void addCollectCity(QModelIndex m_index);       //增加收藏城市
    void changeCityFromSearch(QModelIndex m_index); //从搜索界面改变当前城市
    void cityListNull(bool needShow);               //搜索为空
};

#endif // SEARCHCITYLIST_H
