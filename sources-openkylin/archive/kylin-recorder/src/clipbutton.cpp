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
 *
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "clipbutton.h"
#include "clipwidget.h"
#include <QApplication>

ClipButton::ClipButton(QString normalPicPath, QWidget *parent) : QPushButton(parent)
{
    QPixmap pixmap(normalPicPath);
    qDebug() << pixmap.size();
    resize(16,24);
    normalPix = pixmap;
}

void ClipButton::paintEvent(QPaintEvent *)
{
    QPixmap drawPix;
    drawPix = normalPix;
    QPainter painter(this);
    int x = this->size().width() / 2 - drawPix.width()/2;
    int y = this->size().height() / 2  - drawPix.height()/2;
    painter.fillRect(QRect(0, 0, 16, 24), QApplication::palette().highlight().color());
    painter.drawPixmap(x, y, drawPix);
}

void ClipButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);
    qDebug() << "按压le";
    if (event->button() == Qt::LeftButton) {
        //        this->raise(); //将此按钮移动到顶层显示
        this->pressPoint = event->pos();
    }
}

void ClipButton::mouseReleaseEvent(QMouseEvent *event)
{
    int leftButton_absolutePos = 0;
    int rightButton_absolutePos = 0;

    //    qDebug()<<"ClipButton的个数:"<<this->parent()->findChildren<ClipButton*>().count();
    int padding; //滑块内距
    if (event->button() == Qt::LeftButton) {

        if (this->objectName() == "leftBtn") {
            qDebug() << "左按钮的相对位置:" << this->pos().rx();
            leftButton_absolutePos = this->pos().rx();
            qDebug() << "左滑块的左边界的位置:" << leftButton_absolutePos << "两滑块内距离:" << padding;
            //发送左滑块左边界的相对位置的信号
            emit leftBtn_ReleaseStartPlayer_Signal(leftButton_absolutePos);
        } else if (this->objectName() == "rightBtn") {
            rightButton_absolutePos = this->pos().rx() + this->width();
            qDebug() << "右滑块的右边界的位置:" << rightButton_absolutePos << "两滑块内距离:" << padding;
            qDebug() << "当前按钮的位置:" << this->pos().rx();
            emit rightBtn_ReleaseGetEndPositon_Signal(rightButton_absolutePos);
        } else {
            qDebug() << "游标";
        }
    }
}

void ClipButton::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {

        if (this->objectName() == "leftBtn") {
            posAx = this->mapToParent(event->pos() - this->pressPoint).rx();
            emit btn_MoveSignal(posAx); //左
            qDebug() << "左按钮的相对位置:" << this->pos().rx();
            emit leftBtn_ReleaseStartPlayer_Signal(this->pos().rx());
        } else if (this->objectName() == "rightBtn") {
            posBx = this->mapToParent(event->pos() - this->pressPoint).rx();
            emit btn_MoveSignal(posBx); //右
            qDebug() << "右按钮的相对位置:" << this->pos().rx();
            emit rightBtn_ReleaseGetEndPositon_Signal(this->pos().rx());
        }
        //防止按钮移出父窗口
        if (this->mapToParent(this->rect().topLeft()).x() <= 0) {
            // qDebug()<<this->pos().y();
            this->move(1, this->pos().y());
            emit leftBtn_ReleaseStartPlayer_Signal(1);
        }
        if (this->mapToParent(this->rect().bottomRight()).x() >= this->parentWidget()->rect().width()) {
            this->move(this->parentWidget()->rect().width() - this->width(), this->pos().y());
        }
        if (this->mapToParent(this->rect().topLeft()).y() <= 0) {
            this->move(this->pos().x(), 0);
        }
        if (this->mapToParent(this->rect().bottomRight()).y() >= this->parentWidget()->rect().height()) {
            this->move(this->pos().x(), this->parentWidget()->rect().height() - this->height());
        }
    }
}
