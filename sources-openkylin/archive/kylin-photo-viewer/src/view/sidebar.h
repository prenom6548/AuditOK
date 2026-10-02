/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QGridLayout>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItem>
#include <QDebug>
#include <QScrollArea>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QMovie>
#include <QScrollBar>
#include "controller/interaction.h"
#include "sizedate.h"
#include "sidebardelegate.h"

class SideBar : public QListView
{
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr);
    void showItem();
    void setItemNum(double acturalHeight);

public Q_SLOTS:
    void getSelect(int type);
    void openEmptyFile(bool changeHigh); //判断是不是打开了图片，之后设置相册高亮
    void dealScrollbarMove(int moveValue);

private:
    QMovie *m_loadingMovie = nullptr; //没有处理完，则显示此动图
    SideBarDelegate *m_delegate = nullptr;
    QModelIndex m_modelIndexOld; //记录上一次的index
    int m_nextMove = 0;          //记录滑动条移动位置-下一次
    int m_backMove = 0;          //记录滑动条移动位置-上一次
    int m_imageNum = 0;
    QTimer *m_refreshTimer = nullptr; //延迟刷新定时器
    QModelIndex m_pendingIndex;       //待刷新的索引
    int m_pendingRefreshCount = 0;    //防抖计数器，记录待刷新请求次数

    void initConnect();
    void doReloadImage(); //执行实际的刷新
protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);

private Q_SLOTS:

    void setCurrIndex(QModelIndex modelIndex);

Q_SIGNALS:

    void changeImage(bool isChange); //解决从相册切换时动图没有刷新右键菜单内容
    void sizeChange();               //相册尺寸改变，位置改变
    void showLoadingImageWid();     //显示"载入图片中..."界面
};

#endif // SIDEBAR_H
