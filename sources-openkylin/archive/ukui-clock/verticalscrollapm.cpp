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
#include "verticalscrollapm.h"

#include <QMouseEvent>
#include <QProcess>

VerticalScroll_APM::VerticalScroll_APM(QWidget *parent)
    :     BaseVerticalScroll(0,0,2,BaseVerticalScroll_TYPE::TEXT_SCROLL,parent)
{
    setupUi(this);

    homingAni = new QPropertyAnimation(this, "deviation");
    homingAni->setObjectName("ukui-clock_QPropertyAnimation_homingAni");
    homingAni->setDuration(300);
    homingAni->setEasingCurve(QEasingCurve::OutQuad);
    //默认设置为上午，1-上午，2-下午
    m_currentValue=1;
    connect(this,&BaseVerticalScroll::deviationChange,this,[=](int flag){
        scrollDragslot(flag);
    });
}

VerticalScroll_APM::~VerticalScroll_APM()
{
    delete homingAni;
}
/*
 * 设置范围  set range
 * int min 最小值
 * int max 最大值
*/
void VerticalScroll_APM::setRange(int min, int max)
{
    m_minRange = min;
    m_maxRange = max;
    if (m_currentValue < min)
        m_currentValue = min;

    if (m_currentValue > max)
        m_currentValue = max;
    repaint();
}
//获取当前值
//Get current value
int VerticalScroll_APM::readValue()
{
    return m_currentValue;
}

void VerticalScroll_APM::mousePressEvent(QMouseEvent *e)
{
    qDebug()<<"mouse pressed on vertical scroll";
    homingAni->stop();
    isDragging = true;
    m_mouseSrcPos = e->pos().y();
    QWidget::mousePressEvent(e);
}

void VerticalScroll_APM::mouseReleaseEvent(QMouseEvent *)
{
    isDragging = false;
    homing();
}

void VerticalScroll_APM::paintEvent(QPaintEvent *)
{
    QProcess process;
    process.start("gsettings get org.ukui.control-center.panel.plugins hoursystem");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output;

    if (str_output.compare("'24'\n") == 0) {
        this->hide();
    } else {
        this->show();
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int Height = height() - 1;

    commonCalcValue(Height);

    // 中间数
    //middle number
    paintNum(painter, m_currentValue, m_deviation);

    //两侧数字
    // Numbers on both sides
    if (m_currentValue != m_minRange) {
        paintNum(painter, m_currentValue - interval, m_deviation - Height / devide);
    } else {
        paintNum(painter, m_maxRange, m_deviation - Height / devide);
    }

    if (m_currentValue != m_maxRange) {
        paintNum(painter, m_currentValue + interval, m_deviation + Height / devide);
    } else {
        paintNum(painter, m_minRange, m_deviation + Height / devide);
    }

    for (int i=2; i <= devide/2; ++i) {
        if (m_currentValue - interval * i >= m_minRange)
            paintNum(painter, m_currentValue - interval * i, m_deviation - Height / devide * i);
        if (m_currentValue + interval * i <= m_maxRange)
            paintNum(painter, m_currentValue + interval * i, m_deviation + Height / devide * i);
    }
}

void VerticalScroll_APM::enterEvent(QEvent *event)
{
    m_isFirstFocus = true;
    event->ignore();
}

void VerticalScroll_APM::leaveEvent(QEvent *event)
{
    m_isFirstFocus = false;
    event->ignore();
}

int VerticalScroll_APM::readDeviation()
{
    return m_deviation;
}

void VerticalScroll_APM::setDeviation(int n)
{
    m_deviation = n;
    repaint();
}

void VerticalScroll_APM::scrollDragslot(int flag)
{
    //小于0是往上拽的，大于0是往下拽的
    if(flag>0){
        if(m_currentValue==0){
            isDragging=false;
            m_currentValue=1;
            homing();
        }else if(m_currentValue==1){
            isDragging=false;
        }else{
            isDragging=true;
        }
    }else if(flag==0){
        //避免出现需要多次拖拽才能拖拽
        isDragging=true;
    }else{
        if(m_currentValue==0){
            isDragging=false;
            m_currentValue=2;
            homing();
        }else if(m_currentValue==1){
            isDragging=true;
        }else{
            isDragging=false;
        }
    }
}

void VerticalScroll_APM::setupUi(QWidget *VerticalScroll_APM)
{
    if (VerticalScroll_APM->objectName().isEmpty())
        VerticalScroll_APM->setObjectName(QString::fromUtf8("VerticalScroll_APM"));
    VerticalScroll_APM->resize(53, 200);

    retranslateUi(VerticalScroll_APM);

    QMetaObject::connectSlotsByName(VerticalScroll_APM);
} // setupUi

void VerticalScroll_APM::retranslateUi(QWidget *VerticalScroll_APM)
{
    VerticalScroll_APM->setWindowTitle(QApplication::translate("VerticalScroll_APM", "VerticalScroll_APM", nullptr));
} // retranslateUi
