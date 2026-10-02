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
#include "verticalScroll24.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "clock.h"
#include "ui_clock.h"
#include <QProcess>
VerticalScroll_24::VerticalScroll_24(QWidget *parent, Clock *clock) :
    BaseVerticalScroll(0,0,23,BaseVerticalScroll_TYPE::NUM_SCROLL,parent)
{
    setupUi(this);
    m_Pclock = clock;
    homingAni = new QPropertyAnimation(this, "deviation");
    homingAni->setObjectName("ukui-clock_QPropertyAnimation_homingAni");
    homingAni->setDuration(300);
    homingAni->setEasingCurve(QEasingCurve::OutQuad);
    timeEditStyle_24();
}

VerticalScroll_24::~VerticalScroll_24()
{
    delete homingAni;
    delete timeEdit_24;

    qDebug()<<"-------VerticalScroll_24---------";
    //delete ui;
}
/*
 * 设置范围 set range
 * int min 最小值
 * int max 最大值
*/
void VerticalScroll_24::setRange(int min, int max)
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
int VerticalScroll_24::readValue()
{
    return m_currentValue;
}

void VerticalScroll_24::timeEditStyle_24()
{
    timeEdit_24 = new QLineEdit(this);
    timeEdit_24->setObjectName("ukui-clock_QLineEdit_timeEdit_24");
    timeEdit_24->resize(52,54);
    timeEdit_24->move(3,height()/2-15);
    QRegExp rx = QRegExp("^(((0|1)[0-9])|20|21|22|23|[0-9]|"")$");
    QRegExpValidator* validator = new QRegExpValidator(rx);
    validator->setObjectName("ukui-clock_QRegExpValidator_validator");
    timeEdit_24->setValidator(validator);
    QFont font;
    font.setPixelSize(34);
    timeEdit_24->setFont(font);
    timeEdit_24->hide();
}

//鼠标按压选择当前值
// Press the mouse to select the current value
void VerticalScroll_24::mousePressEvent(QMouseEvent *e)
{
    qDebug()<<"mouse pressed on vertical scroll";

    homingAni->stop();
    isDragging = true;
    m_mouseSrcPos = e->pos().y();
    QWidget::mousePressEvent(e);
}



//鼠标释放，数值弹回正中间
//  Release the mouse, and the value will bounce back to the middle
void VerticalScroll_24::mouseReleaseEvent(QMouseEvent *)
{
    if (isDragging) {
        isDragging = false;
        homing();
    }
}




//绘制当前数值轮画面
// Draw the current numerical wheel screen
void VerticalScroll_24::paintEvent(QPaintEvent *)
{
    QProcess process;
    process.start("gsettings get org.ukui.control-center.panel.plugins hoursystem");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int Height = height() - 1;

    commonCalcValue(Height);
    if (str_output.compare("'24'\n") == 0) {
        paintNum_24( painter, Height);
    } else {
        paintNum_12( painter, Height);
    }
}

void VerticalScroll_24::enterEvent(QEvent *event)
{
    m_isFirstFocus = true;
    event->ignore();
}

void VerticalScroll_24::leaveEvent(QEvent *event)
{
    m_isFirstFocus = false;
    event->ignore();
}







void VerticalScroll_24::paintNum_24(QPainter &painter, int Height)
{
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
    m_Pclock->ui->timeFormatOnTimeWheel->hide();
}

void VerticalScroll_24::paintNum_12(QPainter &painter, int Height)
{
    int system_12_Value = m_currentValue;
    if (m_currentValue > 12) {
        system_12_Value = m_currentValue - 12;
        paintNum(painter, system_12_Value, m_deviation);
    } else {
        if (m_currentValue == 0) {
            system_12_Value = 12;
            paintNum(painter, system_12_Value, m_deviation);
        } else {
            system_12_Value = m_currentValue;
            paintNum(painter, system_12_Value, m_deviation);
        }
    }

    if (system_12_Value != m_minRange) {
        if (system_12_Value == 1) {
            paintNum(painter, 12, m_deviation - Height / devide);
        } else {
            paintNum(painter, system_12_Value - interval, m_deviation - Height / devide);
        }
    } else {
        paintNum(painter, m_maxRange, m_deviation - Height / devide);
    }


    if (system_12_Value != m_maxRange) {
        if (system_12_Value == 12) {
            paintNum(painter, 1, m_deviation + Height / devide);
        } else {
            paintNum(painter, system_12_Value + interval, m_deviation + Height / devide);
        }
    } else {
        paintNum(painter, m_minRange, m_deviation + Height / devide);
    }
    m_Pclock->ui->timeFormatOnTimeWheel->hide();
    //换用上下午滚轮
//    if (m_currentValue>=12) {
//        m_Pclock->ui->timeFormatOnTimeWheel->setText(tr("PM"));
//    } else {
//        m_Pclock->ui->timeFormatOnTimeWheel->setText(tr("AM"));
//    }
}




//鼠标移动偏移量，默认为0
// Mouse movement offset, default is 0
int VerticalScroll_24::readDeviation()
{
    return m_deviation;
}
//设置偏移量
// Set offset
void VerticalScroll_24::setDeviation(int n)
{
    m_deviation = n;
    repaint();
}


void VerticalScroll_24::setupUi(QWidget *VerticalScroll_24)
{
    if (VerticalScroll_24->objectName().isEmpty())
        VerticalScroll_24->setObjectName(QString::fromUtf8("VerticalScroll_24"));
    VerticalScroll_24->resize(53, 200);

    retranslateUi(VerticalScroll_24);

    QMetaObject::connectSlotsByName(VerticalScroll_24);
} // setupUi

void VerticalScroll_24::retranslateUi(QWidget *VerticalScroll_24)
{
    VerticalScroll_24->setWindowTitle(QApplication::translate("VerticalScroll_24", "VerticalScroll_24", nullptr));
} // retranslateUi


