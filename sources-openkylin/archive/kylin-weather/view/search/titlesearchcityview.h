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
#ifndef TITLESEARCHCITYVIEW_H
#define TITLESEARCHCITYVIEW_H

#include <QWidget>
#include <QListView>
#include <QScrollArea>
#include <QScrollBar>
#include <QHeaderView>
#include <QLabel>
#include <QBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include "controller/Interaction.h"
#include "titlesearchlistdelegate.h"
#include "model/titlesearchmodel.h"
class QListView;
class TitleSearchCityView : public QListView
{
    Q_OBJECT
    friend class TestTitleSearchCityView;
public:
    explicit TitleSearchCityView(QWidget *parent = nullptr);
    ~TitleSearchCityView();
    void searchUiChange(bool hasCity); //搜索输入为空和有搜索结果的界面显示
    void setViewModel(bool needSetModel);

private:
    QLabel *m_nullTip; // 未搜索出结果提示
    // QStandardItemModel *m_model = nullptr; 
    TitleSearchModel *m_model = nullptr; 
    TitleSearchListDelegate *m_delegate = nullptr;

    void setWidgetStyle();  // 设置listview中需要的属性
    void initConnect();     //信号槽
    void initInteraction(); //后端-信号槽
    void setModelforView(); //设置模型
    void setStyle(QString themeStyle);
    void darkListview();
    void lightListView();
    void updateRadius(QString key);

public slots:
    void slotSearchNull();      //搜索为空的界面处理
    void slotChangeCityFromTitleSearchList(QModelIndex index); // 点击标题栏的搜索列表中的item，转发信号到界面
    
signals:
    void sigTitleSearchChangeCityToMain(); // 点击标题栏的搜索列表中的item，转发信号到界面
    void cityListNull(bool needShow);               //搜索为空
};

#endif // TITLESEARCHCITYVIEW_H
