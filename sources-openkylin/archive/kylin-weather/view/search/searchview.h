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
#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QDialog>
#include <QPainter>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "hotcity.h"
#include "controller/Interaction.h"
#include "searchcitylist.h"
#include "collectcity.h"
#include <buried_point.hpp>
#include "ksearchlineedit.h"
class SearchView : public QWidget
{
    Q_OBJECT
    friend class TestSearchView;
public:
    explicit SearchView(QWidget *parent = nullptr);
    void changeUiShow(); //切换主界面城市时的ui处理
    void searchReset();  //重置界面
protected:
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
private:
    void initWidget();
    void initStyle();
    void initInteraction();
    HotCity *m_hotCity = nullptr;
    QWidget *m_centerWid = nullptr;
    SearchCityLIst *m_searchCityList = nullptr;
    CollectCity *m_collectCity = nullptr;
    QPushButton *m_searchIcon;
    QLabel *m_searchText;
    QLabel *m_backLabel;
    kdk::KSearchLineEdit *m_searchLineEdit = nullptr;
    QString m_backInputText;

    QStackedWidget *m_stackedWidget = nullptr; // 堆栈窗口
    QWidget *m_searchWidget = nullptr;         // 搜索并收藏界面
    QVBoxLayout *m_vCollectLayout = nullptr;   // 收藏城市界面纵向布局             【删除】
    QHBoxLayout *m_hSearchLayout = nullptr;    // 搜索城市界面横向布局
    QVBoxLayout *m_vSearchLayout = nullptr;    // 搜索城市界面纵向布局

    void changePostion(); //确定热门城市和收藏城市位置
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);       //做searchBox的失焦处理
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeCity(QString cityId); //改变当前城市
    void initConnect();


signals:
    void searchWidState(); //发送信号，处理点击切换城市后的界面显示

private slots:
    void sendSearchInput();                        //发送搜索内容
    void addCollectCity(QModelIndex index);        //添加城市至收藏城市列表
    void changeCityFromSearch(QModelIndex index);  //从搜索界面切换主界面城市
    void changeCityFromCollect(QModelIndex index); //从收藏城市列表切换主界面城市
    void searchListUi(bool hasCity);               //搜索列表界面控件变化
    void changeStackedWidget();
    void onBackLabelClicked();
};

#endif // SEARCHVIEW_H
