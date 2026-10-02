/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "crop.h"

#include <QDebug>

/**
 * 拉伸裁剪框todo list
 * 1. 得到裁剪区域后，计算【拉伸鼠标样式】的识别区域，包含八个区域：上下左右，四个角，其他区域恢复【箭头鼠标样式】
 * 2. 当鼠标按下并移动，且初始点在识别区域内，根据所在的识别区域以及鼠标位置，更新矩形框的坐标，触发paintEvent重绘裁剪框
 * 3. 接收到鼠标release事件，回到步骤1
 * */

CropLabel::CropLabel(QLabel *parent)
    : QLabel(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    initCropSettings();
    loadAllPixmaps();

}

CropLabel::~CropLabel()
{

}

void CropLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event); // draw label background

    QPainter painter(this);
    painter.begin(this);
    QPen pen(QColor(55, 144, 250), 1);
    painter.setPen(pen);

    QPainterPath borderPath;
    borderPath.setFillRule(Qt::WindingFill);
    borderPath.addRect(0, 0, this->width(), this->height()); // need judge

    int W = m_endX - m_beginX;
    int H = m_endY - m_beginY;
    m_beginPoint.setX(m_beginX);
    m_beginPoint.setY(m_beginY);
    m_endPoint.setX(m_endX);
    m_endPoint.setY(m_endY);

    m_currentSelectRect = getRect(m_beginPoint, m_endPoint);

    QPainterPath cropPath;
    cropPath.addRect(m_currentSelectRect);

    // draw different pixmap
    painter.drawPixmap(m_currentSelectRect.x()-m_leftTopPixmapSize.width()/2, m_currentSelectRect.y()-m_leftTopPixmapSize.height()/2,
                       m_leftTopPixmapSize.width(), m_leftTopPixmapSize.height(),  m_leftTopPixmap);

    painter.drawPixmap(m_currentSelectRect.x()+W/2-m_topPixmapSize.width()/2, m_currentSelectRect.y()-m_topPixmapSize.height()/2,
                       m_topPixmapSize.width(), m_topPixmapSize.height(), m_topPixmap);

    painter.drawPixmap(m_currentSelectRect.x()+W-m_rightTopPixmapSize.width()/2, m_currentSelectRect.y()-m_rightTopPixmapSize.height()/2,
                       m_rightTopPixmapSize.width(), m_rightTopPixmapSize.height(), m_rightTopPixmap);

    painter.drawPixmap(m_currentSelectRect.x()+W-m_rightPixmapSize.width()/2, m_currentSelectRect.y()+H/2-m_rightPixmapSize.height()/2,
                       m_rightPixmapSize.width(), m_rightPixmapSize.height(), m_rightPixmap);

    painter.drawPixmap(m_currentSelectRect.x()+W-m_rightBottomPixmapSize.width()/2, m_currentSelectRect.y()+H-m_rightBottomPixmapSize.height()/2,
                       m_rightBottomPixmapSize.width(), m_rightBottomPixmapSize.height(), m_rightBottomPixmap);

    painter.drawPixmap(m_currentSelectRect.x()+W/2-m_bottomPixmapSize.width()/2, m_currentSelectRect.y()+H-m_bottomPixmapSize.height()/2,
                       m_bottomPixmapSize.width(), m_bottomPixmapSize.height(), m_bottomPixmap);

    painter.drawPixmap(m_currentSelectRect.x()-m_leftBottomPixmapSize.width()/2, m_currentSelectRect.y()+H-m_leftBottomPixmapSize.height()/2,
                       m_leftBottomPixmapSize.width(), m_leftBottomPixmapSize.height(), m_leftBottomPixmap);

    painter.drawPixmap(m_currentSelectRect.x()-m_leftPixmapSize.width()/2, m_currentSelectRect.y()+H/2-m_leftPixmapSize.height()/2,
                       m_leftPixmapSize.width(), m_leftPixmapSize.height(), m_leftPixmap);

    // draw crop rect path
    painter.drawPath(cropPath);

    // draw the background of the area outside the clipping area
    QPainterPath outPath;
    outPath = borderPath.subtracted(cropPath);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(outPath, QColor(0, 0, 0, 100));

    if(!(H == 0 && W == 0)){
        emit paintCompleteSignal();
    }
    if(H == 0 && W == 0){
        emit hideCancelOkWidget();
    }
}

void CropLabel::initCropRegion(){

}
void CropLabel::calculateDetectRegion(){
    //更新不同鼠标样式识别区域
    //重新计算八个描点的区域
    int width = m_endX - m_beginX;
    int height = m_endY - m_beginY;
    //左侧描点区域
    m_rectLeft = QRect(m_beginX, m_beginY + m_padding, m_padding, height - m_padding * 2);
    //上侧描点区域
    m_rectTop = QRect(m_beginX + m_padding, m_beginY, width - m_padding * 2, m_padding);
    //右侧描点区域
    m_rectRight = QRect(m_beginX + width - m_padding, m_beginY + m_padding, m_padding, height - m_padding * 2);
    //下侧描点区域
    m_rectBottom = QRect(m_beginX + m_padding, m_beginY + height - m_padding, width - m_padding * 2, m_padding);
    //左上角描点区域
    m_rectLeftTop = QRect(m_beginX, m_beginY, m_padding, m_padding);
    //右上角描点区域
    m_rectRightTop = QRect(m_beginX + width - m_padding, m_beginY, m_padding, m_padding);
    //左下角描点区域
    m_rectLeftBottom = QRect(m_beginX, m_beginY + height - m_padding, m_padding, m_padding);
    //右下角描点区域
    m_rectRightBottom = QRect(m_beginX + width - m_padding, m_beginY + height - m_padding, m_padding, m_padding);
    //中间区域
    m_center = QRect(m_beginX + m_padding, m_beginY + m_padding, width - m_padding * 2, height - m_padding * 2);
}

void CropLabel::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    if (event->button() == Qt::LeftButton) {
        //判断按下的手柄的区域位置
        QPoint lastPos = event->pos();
        if (m_rectLeft.contains(lastPos)) {
            m_pressedLeft = true;
        } else if (m_rectRight.contains(lastPos)) {
            m_pressedRight = true;
        } else if (m_rectTop.contains(lastPos)) {
            m_pressedTop = true;
        } else if (m_rectBottom.contains(lastPos)) {
            m_pressedBottom = true;
        } else if (m_rectLeftTop.contains(lastPos)) {
            m_pressedLeftTop = true;
        } else if (m_rectRightTop.contains(lastPos)) {
            m_pressedRightTop = true;
        } else if (m_rectLeftBottom.contains(lastPos)) {
            m_pressedLeftBottom = true;
        } else if (m_rectRightBottom.contains(lastPos)) {
            m_pressedRightBottom = true;
        }else if (m_center.contains(lastPos)){
            m_pressCenter = true;
            m_startPos = event->globalPos();
        }
    }
}

void CropLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isPressed){
        m_pressedLeft = false;               //鼠标按下左侧
        m_pressedRight = false;              //鼠标按下右侧
        m_pressedTop = false;                //鼠标按下上侧
        m_pressedBottom = false;             //鼠标按下下侧
        m_pressedLeftTop = false;            //鼠标按下左上侧
        m_pressedRightTop = false;           //鼠标按下右上侧
        m_pressedLeftBottom = false;         //鼠标按下左下侧
        m_pressedRightBottom = false;        //鼠标按下右下侧
        m_pressCenter = false;

        m_isPressed = false;
    }
}

void CropLabel::mouseMoveEvent(QMouseEvent *event)
{
    //添加裁剪框边边的悬浮鼠标样式
    QHoverEvent *hoverEvent = (QHoverEvent *)event;
    QPoint point = hoverEvent->pos();
    if (m_rectLeft.contains(point)) {
        setCursor(Qt::SizeHorCursor);
    } else if (m_rectRight.contains(point)) {
        setCursor(Qt::SizeHorCursor);
    } else if (m_rectTop.contains(point)) {
        setCursor(Qt::SizeVerCursor);
    } else if (m_rectBottom.contains(point)) {
        setCursor(Qt::SizeVerCursor);
    } else if (m_rectLeftTop.contains(point)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (m_rectRightTop.contains(point)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (m_rectLeftBottom.contains(point)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (m_rectRightBottom.contains(point)) {
        setCursor(Qt::SizeFDiagCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

    if (m_isPressed && (event->buttons() & Qt::LeftButton)) {
        QPoint pos = event->pos();
        if(m_origRect.contains(pos)){
            if(m_pressedLeft){
                m_beginX = pos.x();
            }else if(m_pressedTop){
                m_beginY = pos.y();
            }else if(m_pressedRight){
                m_endX = pos.x();
            }else if(m_pressedBottom){
                m_endY = pos.y();
            }else if(m_pressedLeftTop){
                m_beginX = pos.x();
                m_beginY = pos.y();
            }else if(m_pressedRightTop){
                m_endX = pos.x();
                m_beginY = pos.y();
            }else if(m_pressedRightBottom){
                m_endX = pos.x();
                m_endY = pos.y();
            }else if(m_pressedLeftBottom){
                m_beginX = pos.x();
                m_endY = pos.y();
            }else if(m_pressCenter){
            }
            m_beginX = judgePosition(m_beginX,0,m_origRect.width());
            m_beginY = judgePosition(m_beginY,0,m_origRect.height());
            m_endX = judgePosition(m_endX,0,m_origRect.width());
            m_endY = judgePosition(m_endY,0,m_origRect.height());

            m_beginPoint.setX(m_beginX);
            m_beginPoint.setY(m_beginY);
            m_endPoint.setX(m_endX);
            m_endPoint.setY(m_endY);
            calculateDetectRegion();
            update();
        }
        if(m_pressCenter){
            int tmp_x = event->globalPos().x() - m_startPos.x();
            int tmp_y = event->globalPos().y() - m_startPos.y();

            m_startPos = event->globalPos();

            int m_beginXTmp = m_beginX + tmp_x;
            int m_beginYTmp = m_beginY + tmp_y;
            int m_endXTmp = m_endX + tmp_x;
            int m_endYTmp = m_endY + tmp_y;

            m_beginXTmp = judgePosition(m_beginXTmp,0,m_origRect.width());
            m_beginYTmp = judgePosition(m_beginYTmp,0,m_origRect.height());
            m_endXTmp = judgePosition(m_endXTmp,0,m_origRect.width());
            m_endYTmp = judgePosition(m_endYTmp,0,m_origRect.height());

            int x_dis = m_endXTmp - m_beginXTmp;
            int y_dis = m_endYTmp - m_beginYTmp;
            int width = m_endX - m_beginX;
            int length = m_endY - m_beginY;

            if(y_dis < length || x_dis < width){
                return;
            }else{
                m_beginX += tmp_x;
                m_beginY += tmp_y;
                m_endX += tmp_x;
                m_endY += tmp_y;
                qDebug() << "pos:" << tmp_x << tmp_y;
            }
        }else{
            return;
        }
        m_beginX = judgePosition(m_beginX,0,m_origRect.width());
        m_beginY = judgePosition(m_beginY,0,m_origRect.height());
        m_endX = judgePosition(m_endX,0,m_origRect.width());
        m_endY = judgePosition(m_endY,0,m_origRect.height());
        m_beginPoint.setX(m_beginX);
        m_beginPoint.setY(m_beginY);
        m_endPoint.setX(m_endX);
        m_endPoint.setY(m_endY);
        calculateDetectRegion();
        update();
        return;
    }
}

int CropLabel::judgePosition(int origin, int min, int max)
{
    qDebug() << "origin = " << origin
             << "min = " << min
             << "max = " << max;

    if (origin < min) {
        // left border
        return min;
    }
    if (origin > max) {
        // right border: In order to show green border, so need max-1
        return max-1;
    }

    return origin;
}

bool CropLabel::isPressPointInCropRegion(QPoint mousePressPoint)
{
    return true;
}

QRect CropLabel::getRect(const QPoint &m_beginPoint, const QPoint &m_endPoint)
{
    int W = m_endPoint.x() - m_beginPoint.x();
    int H = m_endPoint.y() - m_beginPoint.y();

    QRect rect(m_beginPoint.x(), m_beginPoint.y(),  W, H);

    return rect;

}


void CropLabel::initCropSettings(int width, int height)
{
    // Not show crop box in first crop operation
    m_origRect.setRect(0,0,this->width(),this->height());
    m_beginX = 0;
    m_beginY = 0;
    if(width == 0 || height == 0){
        m_endX = m_origRect.width()-1;
        m_endY = m_origRect.height()-1;
    }else{
        m_endX = width-1;
        m_endY = height-1;
    }
    m_beginPoint = QPoint(m_beginX, m_beginY);
    m_endPoint = QPoint(m_endX, m_endY);
    update();
    calculateDetectRegion();

    m_hasCropRegion = false;

}

void CropLabel::loadAllPixmaps()
{
    m_leftTopPixmap.load(":/crop-left-top.svg");
    m_leftTopPixmapSize = QSize(10, 10);

    m_topPixmap.load(":/crop-top.svg");
    m_topPixmapSize = QSize(18, 10);

    m_rightTopPixmap.load(":/crop-right-top.svg");
    m_rightTopPixmapSize = QSize(10, 10);

    m_rightPixmap.load(":/crop-right.svg");
    m_rightPixmapSize = QSize(10, 18);

    m_rightBottomPixmap.load(":/crop-right-bottom.svg");
    m_rightBottomPixmapSize = QSize(10, 10);

    m_bottomPixmap.load(":/crop-bottom.svg");
    m_bottomPixmapSize = QSize(18, 10);

    m_leftBottomPixmap.load(":/crop-left-bottom.svg");
    m_leftBottomPixmapSize = QSize(10, 10);

    m_leftPixmap.load(":/crop-left.svg");
    m_leftPixmapSize = QSize(10, 18);
}

void CropLabel::testCropLabelEvents()
{
    // 初始化裁剪区域
    initCropSettings(200, 150);
    loadAllPixmaps();

    // 模拟鼠标按下事件（左上角）
    QPoint pressPoint(m_beginX, m_beginY);
    QMouseEvent pressEvent(QMouseEvent::MouseButtonPress, pressPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&pressEvent);

    // 模拟鼠标移动事件（调整左上角位置）
    QPoint movePoint(m_beginX + 20, m_beginY + 20);
    QMouseEvent moveEvent(QMouseEvent::MouseMove, movePoint, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    mouseMoveEvent(&moveEvent);

    // 模拟鼠标释放事件
    QMouseEvent releaseEvent(QMouseEvent::MouseButtonRelease, movePoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mouseReleaseEvent(&releaseEvent);

    // 模拟鼠标按下事件（右下角）
    QPoint pressPointBottom(m_endX, m_endY);
    QMouseEvent pressEventBottom(QMouseEvent::MouseButtonPress, pressPointBottom, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&pressEventBottom);

    // 模拟鼠标移动事件（调整右下角位置）
    QPoint movePointBottom(m_endX - 20, m_endY - 20);
    QMouseEvent moveEventBottom(QMouseEvent::MouseMove, movePointBottom, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    mouseMoveEvent(&moveEventBottom);

    // 模拟鼠标释放事件
    QMouseEvent releaseEventBottom(QMouseEvent::MouseButtonRelease, movePointBottom, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mouseReleaseEvent(&releaseEventBottom);

    // 模拟鼠标按下事件（中间区域拖动）
    QPoint pressPointCenter((m_beginX + m_endX) / 2, (m_beginY + m_endY) / 2);
    QMouseEvent pressEventCenter(QMouseEvent::MouseButtonPress, pressPointCenter, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&pressEventCenter);

    // 模拟鼠标移动事件（拖动整个裁剪框）
    QPoint movePointCenter((m_beginX + m_endX) / 2 + 10, (m_beginY + m_endY) / 2 + 10);
    QMouseEvent moveEventCenter(QMouseEvent::MouseMove, movePointCenter, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    mouseMoveEvent(&moveEventCenter);

    // 模拟鼠标释放事件
    QMouseEvent releaseEventCenter(QMouseEvent::MouseButtonRelease, movePointCenter, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mouseReleaseEvent(&releaseEventCenter);

    // 模拟鼠标悬浮事件，改变鼠标样式
    QPoint hoverPoint((m_beginX + m_endX) / 2, m_beginY); // 上侧
    QHoverEvent hoverEvent(QHoverEvent::HoverMove, hoverPoint, hoverPoint);
    mouseMoveEvent(reinterpret_cast<QMouseEvent *>(&hoverEvent));

    // 验证裁剪区域逻辑
    QRect cropRegion = getRect(QPoint(m_beginX, m_beginY), QPoint(m_endX, m_endY));
    qDebug() << "Final Crop Region:" << cropRegion;
}

