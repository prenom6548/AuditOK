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
#ifndef COLLECTCITY_H
#define COLLECTCITY_H

#include <QObject>
#include <QTableView>
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QScrollBar>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>

#include "collectitemdelegate.h"
#include "controller/Interaction.h"
#include "hotcity.h"
class CollectCity : public QWidget
{
    Q_OBJECT
    friend class TestCollectCity;
public:
    CollectCity(QWidget *parent = nullptr);
    QLabel *m_noneCity;     //列表无城市
    QListView *m_cityList; //收藏城市列表
    void setModelforView(); //设置模型
    void changeEditBtnStyle(); // 城市收藏后，收藏界面按钮变化
    void changeCollectWidgetStyle(); // 收藏界面隐藏后界面变化

private:
    QLabel *m_cityTitle;                       //收藏城市标题
    QPushButton *m_button = nullptr;           // 编辑按钮
    QHBoxLayout *m_hLayout = nullptr;   // 收藏城市界面横向布局
    QVBoxLayout *m_vLayout = nullptr;   // 收藏城市界面纵向布局
    CollectItemDelegate *m_delegate = nullptr; // model/view代理
    HotCity *m_hotcity = nullptr;       // 热门城市

    void initConnect();                        //信号槽
    void initInteraction();                    //信号槽--后端信号
    void initWidget();
    void initStyle();

private slots:
    void changeItem(); //收藏城市为空，收藏列表隐藏
    void changeShow(); //判断不为空，两个控件的显隐状态
public slots:
    void slotClickButton();
    void changeToHotCity(QString code);

signals:
    void changeCityFromCollect(QModelIndex index); //从收藏城市改变当前城市
    void changeItemPos();                          //判断为空，改变收藏和热门的位置
    void sigChangWidget();
};

#endif // COLLECTCITY_H
