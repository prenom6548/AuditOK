/*
 * Copyright (c) KylinSoft Co.,Ltd. 2025 .All rights reserved.
 * License: Non open source,The copyright belongs to KylinSoft Co.,Ltd.
*/
#include "line.h"
#include <QComboBox>
line::line()
{
    this->setMouseTracking(true);

}
void line::leaveEvent(QEvent *event)
{
}
void line::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton){
    }
    QLineEdit::mousePressEvent(event);
}
void line::mouseReleaseEvent(QMouseEvent *event)
{

    QLineEdit::mouseReleaseEvent(event);

}
void line::mouseDoubleClickEvent(QMouseEvent *event)
{

}
void line::mouseMoveEvent(QMouseEvent *e)
{
    QLineEdit::mouseMoveEvent(e);
}

