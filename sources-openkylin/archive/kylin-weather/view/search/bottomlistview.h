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
#ifndef BOTTOMLISTVIEW_H
#define BOTTOMLISTVIEW_H

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
#include "bottomlistdelegate.h"
#include "model/titlesearchmodel.h"
class QListView;
class BottomListView : public QListView
{
    Q_OBJECT
    friend class TestBottomListView;
public:
    explicit BottomListView(QWidget *parent = nullptr);
    ~BottomListView();

private:
    TitleSearchModel *m_model = nullptr; 
    BottomListDelegate *m_delegate = nullptr;

    void setWidgetStyle();  // 设置listview中需要的属性
    void initConnect();     //信号槽
    void setModelforView(); //设置模型
    bool m_initFirst = true;
    
public slots:
    void slotChangeCityFromTitleSearchList(QModelIndex index); // 点击列表中的item，转发信号到界面
    void slotShowBottomList(int itemSize);
    
signals:
    void sigTitleSearchChangeCityToMain(); // 点击list图标发出信号，切换收藏的某个城市
};

#endif // BOTTOMLISTVIEW_H
