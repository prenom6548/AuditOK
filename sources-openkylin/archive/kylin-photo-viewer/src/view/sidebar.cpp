/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "sidebar.h"
#include "global/interactiveqml.h"
SideBar::SideBar(QWidget *parent) : QListView(parent)
{
    this->setObjectName("Kylin-photo-viewer_QListView_sideBar");
    this->setAccessibleName("Kylin-photo-viewer_QListView_sideBar");
    this->viewport()->setAccessibleName("Kylin-photo-viewer_QWidget_sideBarViewport");
    this->viewport()->setAccessibleDescription("sideBar viewport");

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           //设置无水平滑动条
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false); //设置滑动条无滑槽
    this->verticalScrollBar()->setAccessibleName("Kylin-photo-viewer_QScrollBar_sideBarVerticalScrollBar");
    this->verticalScrollBar()->setAccessibleDescription("sideBar vertical scrollBar");
    this->setDragEnabled(false);                                          //设置不可拖动
    this->setSpacing(8);                                                  //为视图设置控件间距
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);             //设置不可输入文字
    this->setIconSize(SIDEITEM_SIZE);                                     //
    this->setViewMode(QListView::IconMode);

    m_delegate = new SideBarDelegate();
    this->setItemDelegate(m_delegate);
    this->showItem();
    //绘制阴影
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(ALBUM_COLOR);       //设置阴影颜色，也可以setColor(QColor(220,220,220))
    effect->setBlurRadius(ALBUM_RADIUS); //设定阴影的模糊半径，数值越大越模糊
    this->setGraphicsEffect(effect);
    //响应拖拽事件
    this->setAcceptDrops(false);
    this->setFocusPolicy(Qt::NoFocus);
    this->initConnect();

    // 初始化防抖计数器
    m_pendingRefreshCount = 0;

    //图片没处理完毕时，显示转圈圈图
    m_loadingMovie = new QMovie(":/res/res/loadgif.gif"); //暂不需要此动图
    m_loadingMovie->setParent(this);
}

void SideBar::showItem()
{

    this->setModel(Interaction::getInstance()->getAlbumModel());
}

void SideBar::setItemNum(double acturalHeight)
{
    if (acturalHeight <= 0) {
        return;
    }
    if (SIDEITEM_SIZE.height() <= 0) {
        return;
    }
    if (m_imageNum <= 0) {
        return;
    }
    int itemNum = acturalHeight / (SIDEITEM_SIZE.height());

    if (itemNum >= m_imageNum) {
        Variable::LOADIMAGE_NUM = m_imageNum;
        Variable::g_needChangeAlbumPos = true;
        this->resize(SIDEBAR_SIZE.width(), (56 + 6 + 8) * m_imageNum + 10);
    } else {
        Variable::LOADIMAGE_NUM = itemNum;
        Variable::g_needChangeAlbumPos = false;
        this->resize(SIDEBAR_SIZE.width(), acturalHeight);
    }
}

void SideBar::getSelect(int type)
{
    m_imageNum = type;
    setItemNum(Variable::g_acturalWidHeight);
    Q_EMIT sizeChange();
}

void SideBar::openEmptyFile(bool changeHigh)
{
    if (changeHigh == true) {
        setCurrIndex(m_modelIndexOld);
    }
}

void SideBar::initConnect()
{
    connect(Interaction::getInstance(), &Interaction::changeAlbumHighLight, this, &SideBar::setCurrIndex);
    connect(Interaction::getInstance(), &Interaction::updateSideLength, this, &SideBar::getSelect);

    // 创建延迟刷新定时器(50ms)，用于滚动节流
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    m_refreshTimer->setInterval(50);
    connect(m_refreshTimer, &QTimer::timeout, this, &SideBar::doReloadImage);

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &SideBar::dealScrollbarMove);
}

void SideBar::mouseReleaseEvent(QMouseEvent *e)
{
    g_nextOrPreviousTiff = false;
    //处理滑动，或拖出相册时，相册选中态和主界面显示的图片保持一致
    if (m_modelIndexOld == this->currentIndex()){
        return;
    }
    Variable::g_isShowOrigImage = false;
    Interaction::getInstance()->changeImageFromClick(this->currentIndex());
    this->setCurrentIndex(this->currentIndex());
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    QListView::mouseReleaseEvent(e);
}

void SideBar::setCurrIndex(QModelIndex modelIndex)
{
    //当为第一张图时，回到顶部
    if (modelIndex.row() == 1) {
        this->verticalScrollBar()->setValue(0);
    }
    //存上一个，为了点击加号，但不打开时，能切换到正确的显示
    if (modelIndex.row() != 0) {
        Q_EMIT changeImage(true);
        m_modelIndexOld = modelIndex;
    } else {
        Q_EMIT changeImage(false);
    }
    this->setCurrentIndex(modelIndex);
}

void SideBar::dealScrollbarMove(int moveValue)
{
//    QPoint centerPoint = this->viewport()->contentsRect().center();
//    if (this->indexAt(this->viewport()->contentsRect().center()).row() == -1) {
//        if (this->indexAt(QPoint(centerPoint.x(), centerPoint.y() + 8)).row() == -1) {
//            Interaction::getInstance()->reloadImage(this->indexAt(QPoint(centerPoint.x(), centerPoint.y() - 8)));
//        } else {
//            Interaction::getInstance()->reloadImage(this->indexAt(QPoint(centerPoint.x(), centerPoint.y() + 8)));
//        }
//    } else {
//        Interaction::getInstance()->reloadImage(this->indexAt(centerPoint));
//    }

    Q_UNUSED(moveValue);

    QPoint centerPoint = this->viewport()->contentsRect().center();
    QModelIndex index = this->indexAt(centerPoint);

    if (index.row() == -1) {
        // 中心点没有命中有效item，尝试向下偏移
        index = this->indexAt(QPoint(centerPoint.x(), centerPoint.y() + 8));
        if (index.row() == -1) {
            // 向下8像素也没命中，尝试更大的偏移
            index = this->indexAt(QPoint(centerPoint.x(), centerPoint.y() + 16));
            if (index.row() == -1) {
                // 还没命中，尝试向上偏移
                index = this->indexAt(QPoint(centerPoint.x(), centerPoint.y() - 8));
            }
        }
    }

    // 防抖优化：只有当索引真正变化时，才更新并重启定时器
    // 避免频繁调用 start() 导致的性能损耗
    if (index.isValid() && index != m_pendingIndex) {
        m_pendingIndex = index;
        m_pendingRefreshCount++;
        m_refreshTimer->start();
    } else if (index.isValid() && m_pendingIndex.isValid()) {
        // 相同索引但定时器未运行，说明已经设置过，无需重启
        // 仅增加计数用于调试
        m_pendingRefreshCount++;
    }
}

void SideBar::doReloadImage()
{
    if (m_pendingIndex.isValid()) {
        // 调试信息：显示防抖效果
        if (m_pendingRefreshCount > 1) {
            qDebug() << "[SideBar::doReloadImage] 防抖生效: 合并了" << m_pendingRefreshCount << "次滚动请求";
        }

        Interaction::getInstance()->reloadImage(m_pendingIndex);
        m_pendingIndex = QModelIndex();
        m_pendingRefreshCount = 0;  // 重置计数器
    }
}

