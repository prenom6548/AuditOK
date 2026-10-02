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
#include "verticalScroll99.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "constant_class.h"
#include "theme.h"
VerticalScroll_99::VerticalScroll_99(QWidget *parent) :
    BaseVerticalScroll(0,0,23,BaseVerticalScroll_TYPE::NUM_SCROLL,parent)
{
    setupUi(this);

    homingAni = new QPropertyAnimation(this, "deviation");
    homingAni->setObjectName("ukui-clock_QPropertyAnimation_homingAni");
    homingAni->setDuration(300);
    homingAni->setEasingCurve(QEasingCurve::OutQuad);

    timer_21111 = new QTimer();
    timer_21111->setObjectName("ukui-clock_QTimer_timer_21111");
    connect(timer_21111, SIGNAL(timeout()), this, SLOT(listClickslot()));
    timer_21111->setInterval(1000);
    timeEditStyle_99();
    qDebug() << m_currentValue;
}

VerticalScroll_99::~VerticalScroll_99()
{
    delete timer_21111;
    delete homingAni;
    delete timeEdit_99;
    qDebug()<<"-------VerticalScroll_99---------";

}
/*
 * 设置范围
 * set range
 * int min 最小值
 * int max 最大值
*/
void VerticalScroll_99::setRange(int min, int max)
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
int VerticalScroll_99::readValue()
{
    return m_currentValue;
}

void VerticalScroll_99::timeEditStyle_99()
{
    timeEdit_99 = new QLineEdit(this);
    timeEdit_99->setObjectName("ukui-clock_QLineEdit_timeEdit_99");
    timeEdit_99->resize(52,54);
    timeEdit_99->move(3,height()/2-15);
    QRegExp rx = QRegExp("^(((0|1)[0-9])|20|21|22|23|[0-9]|"")$");
    QRegExpValidator* validator = new QRegExpValidator(rx);
    validator->setObjectName("ukui-clock_QRegExpValidator_validator");
    timeEdit_99->setValidator(validator);
    QFont font;
    font.setPixelSize(34);
    timeEdit_99->setFont(font);
    timeEdit_99->hide();
}

void VerticalScroll_99::listClickslot()
{
    qDebug() << m_currentValue;
}

void VerticalScroll_99::mousePressEvent(QMouseEvent *e)
{
    qDebug()<<"mouse pressed on vertical scroll";
    homingAni->stop();
    isDragging = true;
    m_mouseSrcPos = e->pos().y();
    QWidget::mousePressEvent(e);
}



void VerticalScroll_99::mouseReleaseEvent(QMouseEvent *)
{
    if (isDragging) {
        isDragging = false;
        homing();
    }
}



void VerticalScroll_99::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int Height = height() - 1;

    commonCalcValue(Height);

    // 中间数
    //middle number
    paintNum(painter, m_currentValue, m_deviation);

    //两侧数字
    // Numbers on both sides
    if (m_currentValue != m_minRange)
        paintNum(painter, m_currentValue - interval, m_deviation - Height / devide);
    else
        paintNum(painter, m_maxRange, m_deviation - Height / devide);

    if (m_currentValue != m_maxRange)
        paintNum(painter, m_currentValue + interval, m_deviation + Height / devide);
    else
        paintNum(painter, m_minRange, m_deviation + Height / devide);


    for (int i=2; i <= devide/2; ++i) {
        if (m_currentValue - interval * i >= m_minRange)
            paintNum(painter, m_currentValue - interval * i, m_deviation - Height / devide * i);
        if (m_currentValue + interval * i <= m_maxRange)
            paintNum(painter, m_currentValue + interval * i, m_deviation + Height / devide * i);
    }

}

void VerticalScroll_99::enterEvent(QEvent *event)
{
    m_isFirstFocus = true;
    event->ignore();
}

void VerticalScroll_99::leaveEvent(QEvent *event)
{
    m_isFirstFocus = false;
    event->ignore();
}




//鼠标移动偏移量，默认为0
// Mouse movement offset, default is 0
int VerticalScroll_99::readDeviation()
{
    return m_deviation;
}

//设置偏移量
// Set offset
void VerticalScroll_99::setDeviation(int n)
{
    m_deviation = n;
    repaint();
}

void VerticalScroll_99::setupUi(QWidget *VerticalScroll_99)
{
    if (VerticalScroll_99->objectName().isEmpty())
        VerticalScroll_99->setObjectName(QString::fromUtf8("VerticalScroll_99"));
    VerticalScroll_99->resize(53, 200);

    retranslateUi(VerticalScroll_99);

    QMetaObject::connectSlotsByName(VerticalScroll_99);
} // setupUi

void VerticalScroll_99::retranslateUi(QWidget *VerticalScroll_99)
{
    VerticalScroll_99->setWindowTitle(QApplication::translate("VerticalScroll_99", "VerticalScroll_99", nullptr));
} // retranslateUi
