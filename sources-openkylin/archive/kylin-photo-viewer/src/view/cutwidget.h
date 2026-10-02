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
#ifndef CUTWIDGET_H
#define CUTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>
#include <QPushButton>
#include "controller/interaction.h"
#include "view/sizedate.h"
class CutWIdget : public QWidget
{
    Q_OBJECT
public:
    explicit CutWIdget(QWidget *parent = nullptr);
    void sizeChange(QSize currSize);
    bool touchBeginEventProcess(QEvent *event);
    bool touchUpdateEventProcess(QEvent *event);
    bool touchEndEventProcess(QEvent *event);

private:
    void initInteraction();
    void cutRegion(QPoint start, QPoint end); //裁剪框的起点和终点-裁剪区域
    void changeMouseType(QPoint currpos);     //改变鼠标样式
    void resetMouseType(int type);            //重设鼠标样式
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event); //鼠标离开，按钮、两栏隐藏
    void enterEvent(QEvent *event); //鼠标进入

    bool m_isPress = false;         //检测按下事件-默认不在-false
    bool m_isCutRegion = false;     //检测在裁剪框边界-默认不在-false
    bool m_paintProcessing = false; //默认裁剪绘制完毕
    QPoint m_startPoint = QPoint(-1, -1);
    QPoint m_endPoint = QPoint(-1, -1);
    enum MousePosType {
        OUTSIDE = -1,
        INSIDE = 0,
        TOP = 1,
        LEFT = 2,
        LEFT_TOP = 3,
        RIGHT_BOTTOM = 4,
        LEFT_BOTTOM = 5,
        RIGHT_TOP = 6,
        BOTTOM = 7,
        RIGHT = 8
    };
    MousePosType m_type = OUTSIDE; //裁剪框释放后-拉伸类型
    bool m_fixType = false;        //固定状态
    //记录按下和释放的鼠标位置
    QPoint m_enter;
    QPoint m_release;
    QPoint m_cutEnterPos = QPoint(-1, -1); //鼠标释放之后，再次按住某一点进行裁剪框调整的起始点
    int m_xOffest = 0;
    int m_yOffest = 0;
    QPoint m_startEnterPoint = QPoint(-1, -1); //第一次开始绘制裁剪框的起始点
    bool m_isCutRelease = false; //记录鼠标是不是裁剪框画好之后，没有进行裁剪，确定鼠标形状


Q_SIGNALS:
    void showBtnWid(bool isShow);
public Q_SLOTS:
    void cutFinish();
};

#endif // CUTWIDGET_H
