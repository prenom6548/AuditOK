/*
 * Copyright (C) 2018 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "dragwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "commondef.h"

DragWidget::DragWidget(QWidget *parent/* = nullptr*/)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    initUI();
}

DragWidget::~DragWidget()
{

}

void DragWidget::initUI()
{
    setFixedHeight(KEYBOARD_DRAG_DEFAULT_HEIGHT);

    m_labelDrag = new QLabel(this);
    m_labelDrag->setPixmap(QPixmap(":/images/images/drag.svg"));
    m_labelDrag->setGeometry((KEYBOARD_PARENT_DEFAULT_WIDTH - KEYBOARD_TITLEBTN_DEFAULT_WIDTH)/2,
                           (this->height() - KEYBOARD_TITLEBTN_DEFAULT_HEIGHT)/2,
                           KEYBOARD_TITLEBTN_DEFAULT_WIDTH, KEYBOARD_TITLEBTN_DEFAULT_HEIGHT);
    m_mapSubGeometrys[m_labelDrag] = m_labelDrag->geometry();
}

void DragWidget::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical/* = false*/, bool floatStatus)
{
    setFixedHeight(KEYBOARD_DRAG_DEFAULT_HEIGHT*lfHeightScale);
    QMap<QLabel*, QRect>::iterator itGeometry = m_mapSubGeometrys.begin();
    for (; itGeometry != m_mapSubGeometrys.end(); itGeometry ++) {
        QLabel *label = itGeometry.key();
        if (label) {
            QRect oldGeometry = itGeometry.value();
            QRect newGeometry = oldGeometry;
            if (floatStatus) {
                newGeometry.setX(oldGeometry.x()*lfWidthScale*KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setY(oldGeometry.y()*lfHeightScale);
                newGeometry.setWidth(oldGeometry.width()*lfWidthScale*KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setHeight(oldGeometry.height()*lfHeightScale);
            } else {
                newGeometry.setX(oldGeometry.x()*lfWidthScale);
                newGeometry.setY(oldGeometry.y()*lfHeightScale);
                newGeometry.setWidth(oldGeometry.width()*lfWidthScale);
                newGeometry.setHeight(oldGeometry.height()*lfHeightScale);
            }
            label->setGeometry(newGeometry);
        }
    }
}
