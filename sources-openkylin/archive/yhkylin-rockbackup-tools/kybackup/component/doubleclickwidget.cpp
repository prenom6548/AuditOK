/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "doubleclickwidget.h"

DoubleClickWidget::DoubleClickWidget(QWidget *parent) :
    QWidget(parent)
{

}

void DoubleClickWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (event->button() == Qt::LeftButton){  // 左键双击事件才发射双击信号
        emit this->doubleClicked();
    }
}

