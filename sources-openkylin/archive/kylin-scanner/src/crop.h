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
#ifndef TARLOR_H
#define TARLOR_H

#include <QLabel>
#include <QMouseEvent>
#include <QString>
#include <QShowEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRect>
#include <QCursor>
#include <QApplication>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QSize>

#define EPLISION 1e-6

class CropLabel  :   public QLabel
{
    Q_OBJECT

public:
    CropLabel(QLabel *parent = nullptr);
    ~CropLabel();

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void testCropLabelEvents();

    int judgePosition(int origin, int min, int max);
    bool isPressPointInCropRegion(QPoint mousePressPoint);
    QRect getRect(const QPoint &m_beginPoint, const QPoint &m_endPoint);
    void calculateDetectRegion();
    void initCropRegion();

    inline int getBeginX() const {
        return m_beginX;
    }

    inline int getBeginY() const {
        return m_beginY;
    }

    inline int getEndX() const {
        return m_endX;
    }

    inline int getEndY() const {
        return m_endY;
    }


    void initCropSettings(int width=0, int height=0);
    void loadAllPixmaps();

    enum CropStates {
        initCrop = 0,
        beginCropImage,
        finishCropArea,
        beginMoveCropArea,
        finishMoveCropArea
    };

private:
    QRect m_rectLeft;                 //左侧区域
    QRect m_rectRight;                //右侧区域
    QRect m_rectTop;                  //上侧区域
    QRect m_rectBottom;               //下侧区域
    QRect m_rectLeftTop;              //左上侧区域
    QRect m_rectRightTop;             //右上侧区域
    QRect m_rectLeftBottom;           //左下侧区域
    QRect m_rectRightBottom;          //右下侧区域
    QRect m_center;
    const int m_padding = 8;

    bool m_pressedLeft = false;               //鼠标按下左侧
    bool m_pressedRight = false;              //鼠标按下右侧
    bool m_pressedTop = false;                //鼠标按下上侧
    bool m_pressedBottom = false;             //鼠标按下下侧
    bool m_pressedLeftTop = false;            //鼠标按下左上侧
    bool m_pressedRightTop = false;           //鼠标按下右上侧
    bool m_pressedLeftBottom = false;         //鼠标按下左下侧
    bool m_pressedRightBottom = false;        //鼠标按下右下侧
    bool m_pressCenter = false;               //鼠标按下中间区域

    QPoint m_startPos;

    bool m_isPressed;
    bool m_hasCropRegion;

    int m_beginX;
    int m_beginY;
    int m_endX;
    int m_endY;

    CropStates m_currentCropState;
    QRect m_currentSelectRect;

    QPoint m_beginPoint;
    QPoint m_endPoint;
    QPoint m_movePoint;

    QRect m_origRect;

    QPainter painter;

    QPixmap m_cropPixmap;

    QPixmap m_leftTopPixmap;
    QSize m_leftTopPixmapSize;

    QPixmap m_topPixmap;
    QSize m_topPixmapSize;

    QPixmap m_rightTopPixmap;
    QSize m_rightTopPixmapSize;

    QPixmap m_rightPixmap;
    QSize m_rightPixmapSize;

    QPixmap m_rightBottomPixmap;
    QSize m_rightBottomPixmapSize;

    QPixmap m_bottomPixmap;
    QSize m_bottomPixmapSize;

    QPixmap m_leftBottomPixmap;
    QSize m_leftBottomPixmapSize;

    QPixmap m_leftPixmap;
    QSize m_leftPixmapSize;

signals:
    void completeCropPixmap(QPixmap cropPixmap);
    void paintCompleteSignal();
    void hideCancelOkWidget();
};

#endif // TARLOR_H
