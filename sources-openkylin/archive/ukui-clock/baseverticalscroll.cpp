/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "baseverticalscroll.h"
#include <QWheelEvent>
#include <QDebug>
#include <QThread>
#include <QStyleOption>
#include <QFontMetrics>
#include "theme.h"
#include <QPainterPath>
BaseVerticalScroll::BaseVerticalScroll(int currentValue, int minRange, int maxRange, BaseVerticalScroll_TYPE scrollType, QWidget *parent) : QWidget(parent),
  m_currentValue(currentValue),
    m_minRange(minRange),
    m_maxRange(maxRange),
    m_scrollType(scrollType),
    isDragging(false),
    m_deviation(0),   //默认偏移量为0   // The default offset is 0
    m_numSize(TIME_SCROLL_NUM_SIZE),
    interval(1),      //间隔默认1      // Interval default 1
    devide(4)         //默认分成4格    // Divided into 4 grids by default
{
    m_priManager = new PrimaryManager();
    m_priManager->setObjectName("ukui-clock_PrimaryManager_m_priManager");
    settingsStyle();
}

void BaseVerticalScroll::setWheelSpeed(int wheelSpeed)
{
    m_wheelSpeed = wheelSpeed;
}

void BaseVerticalScroll::settingsStyle()
{
    subject = GsettingSubject::getInstance();
    connect(subject,&GsettingSubject::mouseWheelChanged, this,[=](int speed){
        if(m_priManager->checkWayland()){
            this->setWheelSpeed(speed);
        }
    });

    subject->iniMouseWheel();
}
void BaseVerticalScroll::homing()
{
    if ( m_deviation > height() / 10) {
        homingAni->setStartValue( ( height() - 1 ) / 8 - m_deviation);
        homingAni->setEndValue(0);
        m_currentValue = calculateCurrentValue(m_currentValue,-interval);
    } else if ( m_deviation > -height() / 10 ) {
        homingAni->setStartValue(m_deviation);
        homingAni->setEndValue(0);
    } else if ( m_deviation < -height() / 10 ) {
        homingAni->setStartValue(-(height() - 1) / 8 - m_deviation);
        homingAni->setEndValue(0);
        m_currentValue = calculateCurrentValue(m_currentValue,interval);
    }
    emit currentValueChanged(m_currentValue);
    homingAni->start();
}
int BaseVerticalScroll::calculateCurrentValue(int current,int offsetValue)
{
    if ( current == m_minRange && offsetValue<0 ) {
        current = m_maxRange+offsetValue+1;
    } else if( current == m_maxRange && offsetValue>0 ) {
        current = m_minRange+offsetValue-1;
    }else{
        current+=offsetValue;
    }
    return current;
}
void BaseVerticalScroll::paintNum(QPainter &painter, int num, int deviation)
{
    int Width = width() - 1;
    int Height = height() - 1;
    int size = (Height - qAbs(deviation)) / (m_numSize*1.7); //偏移量越大，数字越小
    //The larger the offset, the smaller the number
    int transparency = Utils::handelColorRange(255 - 255 * qAbs(deviation) / Height);
    int height = Height / devide;
    int y = Height / 2 + deviation - height / 2;

    QFont font;
    QString str;
    if(m_scrollType==BaseVerticalScroll_TYPE::NUM_SCROLL){
        font.setPixelSize(size);
        str = change_NUM_to_str(num);
    }else{
        if(num==0){
            str = "";
        }else if(num==1){
            str = tr("AM");
        }else{
            str = tr("PM");
        }
        if(deviation!=0){
            font.setPixelSize(18);
        }else{
            font.setPixelSize(20);
        }
    }
    painter.setFont(font);
    painter.setPen(QColor(255,255,255,transparency));


    QStyleOption opt;
    opt.init(this);

    //偏移量越大颜色越浅
    if(theme::themetype==0)
    {
        painter.setPen(QColor(Utils::handelColorRange(34+(qAbs(deviation)*2)),
                              Utils::handelColorRange(34+(qAbs(deviation)*2)),
                              Utils::handelColorRange(34+(qAbs(deviation)*2)),transparency));
    }else{
        painter.setPen(QColor(Utils::handelColorRange(255-(qAbs(deviation)*2)),
                              Utils::handelColorRange(255-(qAbs(deviation)*2)),
                              Utils::handelColorRange(255-(qAbs(deviation)*2)),transparency));
    }
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient linearGradient(QPointF(110, -30), QPointF(110, 114));
    if(theme::themetype==0)
    {
        linearGradient.setColorAt(0.5, Qt::white);
        linearGradient.setColorAt(0.8, Qt::gray);
        linearGradient.setColorAt(1.0, Qt::black);
    }else{ // 深色模式
        linearGradient.setColorAt(0.55, QColor(30, 30, 30));
        linearGradient.setColorAt(0.8, Qt::gray);
        linearGradient.setColorAt(1.0, Qt::white);
    }
    //指定区域外的扩散方式
    linearGradient.setSpread(QGradient::ReflectSpread);
    if ( y >= 0 && y + height < Height) {
//        painter.drawText(QRectF(0, y, Width, height),
//                         Qt::AlignCenter,
//                         change_NUM_to_str(num));
        QFont font = painter.font();
        QFontMetrics fm(font);
//        QString str = change_NUM_to_str(num);
        QRect rect = fm.boundingRect(0, y, Width, height,Qt::AlignCenter,str);
        QPainterPath path;
        path.addText(rect.x(),rect.y()+ fm.ascent() ,font,str);
        painter.fillPath(path, linearGradient);
    }
    painter.setBrush(QBrush(linearGradient));

}

QString BaseVerticalScroll::change_NUM_to_str(int alarmHour)
{
    QString str;
    if (alarmHour < 10) {
        QString hours_str = QString::number(alarmHour);
        str = "0"+hours_str;
    } else {
        str = QString::number(alarmHour);
    }
    return str;
}
void BaseVerticalScroll::commonCalcValue(int height)
{
    if ( m_deviation >= height / devide  ) {
        m_mouseSrcPos += height / devide;
        m_deviation -= height / devide;
        m_currentValue = calculateCurrentValue(m_currentValue,-interval);
    }

    if ( m_deviation <= -height / devide  ) {
        m_mouseSrcPos -= height / devide;
        m_deviation += height / devide;
        m_currentValue = calculateCurrentValue(m_currentValue,interval);
    }
}
//鼠标滑轮滚动，数值滚动
//  Mouse wheel scrolling, numerical scrolling
void BaseVerticalScroll::wheelEvent(QWheelEvent *event)
{
    //wayland下没有初次进入，触发两次wheelEvent问题
    if(m_priManager->checkWayland()){
        m_isFirstFocus=false;
    }
    if(m_isFirstFocus){
        event->ignore();
        m_isFirstFocus = false;
    }else{
        for (int i=0;i<m_wheelSpeed;i++) {
            if (event->delta() > 0){
                if(m_scrollType==BaseVerticalScroll_TYPE::NUM_SCROLL){
                    if (m_currentValue <= m_minRange)
                        m_currentValue = m_maxRange;
                    else
                        m_currentValue-=1;
                }else{
                    if(m_currentValue==2)
                        m_currentValue=1;
                    else
                        event->ignore();
                }
            } else {
                if(m_scrollType==BaseVerticalScroll_TYPE::NUM_SCROLL){
                    if (m_currentValue >= m_maxRange)
                        m_currentValue = m_minRange;
                    else
                        m_currentValue+=1;
                }else{
                    if(m_currentValue==1)
                        m_currentValue=2;
                    else
                        event->ignore();
                }
            }
//            update();
            repaint();
            QThread::msleep(20);
        }
        event->accept();
    }

}
void BaseVerticalScroll::mouseMoveEvent(QMouseEvent *e)
{
    if (isDragging) {
//        if ( m_currentValue == m_minRange && e->pos().y() >= m_mouseSrcPos ) {
//            m_currentValue = m_maxRange;
//        } else if( m_currentValue == m_maxRange && e->pos().y() <= m_mouseSrcPos ) {
//            m_currentValue = m_minRange;
//        }

        m_deviation = e->pos().y() - m_mouseSrcPos;
        //若移动速度过快，则进行限制
        // If the movement speed is too fast, limit it
        if (m_deviation > (height() - 1) / devide) {
            m_deviation = (height() - 1) / devide;
        }  else if (m_deviation < -(height() - 1) / devide) {
            m_deviation = -( height() - 1) / devide;
        }
//        emit deviationChange((int)m_deviation / ((height() - 1) / devide));//这个的信号值一直为0
        emit deviationChange(m_deviation);
        repaint();
    }
}

void BaseVerticalScroll::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit srcollDoubleClicked();
    QWidget::mouseDoubleClickEvent(event);
}
