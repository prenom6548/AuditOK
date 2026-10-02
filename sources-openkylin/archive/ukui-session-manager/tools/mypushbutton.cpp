/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/

#include "mypushbutton.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QGSettings/qgsettings.h>

MyPushButton::MyPushButton(QString iconPath, QString buttonLable, QString objName, QWidget *parent, int x, int y,
                           int width, int height, int iconWidth, int labelWidth) : QWidget(parent)
{
    this->setFixedSize(labelWidth, height);
    this->setObjectName(objName);
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->setContentsMargins(0, 0, 0, 10);
    m_iconLabel = new MyIconLabel(labelWidth, iconWidth, iconPath);
    m_iconLabel->setObjectName(objName + "_button");
    m_iconLabel->installEventFilter(this);

    QString fakeText = "·" + buttonLable;
    QFontMetrics fontMetric(fakeText);
    int fontSize = fontMetric.width(fakeText);
    int fontHeight = fontMetric.height();
    qDebug() << "objName : " << objName << " labelWidth : " << labelWidth << " fontSize : " << fontSize << " fakeText : " << fakeText;
    //在buttonlable的宽度大于labelWidth - 20的时候，将其省略显示，为什么不是大于labelWidth，因为在藏文的环境中，buttonLabel的宽度和
    //labelWidth的宽度只差几个像素，但实际画出来的效果仍然会超出控件，所以多留点余地
    QString elideLabelText = fontMetric.elidedText(buttonLable, Qt::ElideRight, labelWidth - 35 );

//    if (objName == "upgradethenreboot" || objName == "upgradeThenShutdown") {
//        QString themeColor = "sss";
//        QString themeColorRGB = "#3D6BE5";
//        if (themeColor == "jamPurple") {
//            themeColorRGB = "#3D6BE5";
//        } else if (themeColor == "magenta") {
//            themeColorRGB = "#EB3096";
//        } else if (themeColor == "sunRed") {
//            themeColorRGB = "#F3222D";
//        } else if (themeColor == "sunsetOrange") {
//            themeColorRGB = "#F68C27";
//        } else if (themeColor == "dustGold") {
//            themeColorRGB = "#F8C53D";
//        } else if (themeColor == "polarGreen") {
//            themeColorRGB = "#52C429";
//        } else {
//            themeColorRGB = "#3D6BE5";
//        }

//        elideLabelText = "<font style = 'font-size:20px' color = " + themeColorRGB + ">·</font>" + elideLabelText;
//    }
    m_buttonLabel = new QLabel();
    m_buttonLabel->setText(elideLabelText);
    m_buttonLabel->setToolTip(buttonLable);
    double fsize = 10;
    if (QGSettings::isSchemaInstalled("org.ukui.style")){
        QGSettings *gset = new QGSettings("org.ukui.style");
        fsize = gset->get("system-font-size").toDouble();
    }
    fsize = fsize + 8;
    const QString font_size= "QLabel{font-size: " + QString::number(fsize) + "px}";
    m_buttonLabel->setStyleSheet(font_size +
                                 "QLabel{color: white;}");
    m_buttonLabel->setAlignment(Qt::AlignCenter);

    //newHeight是QWidget的新高度，由icon的高度labelWidth，字体的高度fontHeight再加上一个30的余量组成
    qDebug() << "height is " << height << " fontHeight is " << fontHeight;
    int newHeight = fontHeight + labelWidth + 30;
    if (newHeight > height) {
        //高度加大，避免某些藏文字体会出现的超出现象
        this->setFixedSize(labelWidth, newHeight + 40);
    }
    vBoxLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    vBoxLayout->addWidget(m_buttonLabel, 0, Qt::AlignCenter);
    this->setLayout(vBoxLayout);
    connect(m_iconLabel, &MyIconLabel::mouseEventSignals, this, &MyPushButton::iconLabelMouseEvent);
}

MyPushButton::~MyPushButton()
{
}

void MyPushButton::changeIconBackColor(bool isChoose, bool isKeySelect)
{
    if (m_isMouseSelect && m_isKeySelect) {
        if ((isChoose && isKeySelect) || (isChoose && !isKeySelect)) {
            m_isKeySelect = true;
            m_isMouseSelect = true;
        } else if (!isChoose && !isKeySelect) {
            m_isKeySelect = true;
            m_isMouseSelect = false;
        } else if (!isChoose && isKeySelect) {
            m_isKeySelect = false;
            m_isMouseSelect = true;
        }
    } else if (m_isMouseSelect && !m_isKeySelect) {
        if ((isChoose && isKeySelect) || (!isChoose && !isKeySelect)) {
            m_isKeySelect = isKeySelect;
            m_isMouseSelect = isChoose;
        } else if ((isChoose && !isKeySelect) || (!isChoose && isKeySelect)) {
            m_isKeySelect = false;
            m_isMouseSelect = true;
        }
    } else if (!m_isMouseSelect && !m_isKeySelect) {
        if (isChoose && isKeySelect) {
            m_isKeySelect = true;
            m_isMouseSelect = false;
        } else if (isChoose && !isKeySelect) {
            m_isKeySelect = false;
            m_isMouseSelect = true;
        } else if ((!isChoose && !isKeySelect) || (!isChoose && isKeySelect)) {
            m_isKeySelect = false;
            m_isMouseSelect = false;
        }
    } else if (!m_isMouseSelect && m_isKeySelect) {
        if ((isChoose && isKeySelect) || (!isChoose && !isKeySelect)) {
            m_isKeySelect = true;
            m_isMouseSelect = false;
        } else if ((isChoose && !isKeySelect) || (!isChoose && isKeySelect)) {
            m_isKeySelect = !isKeySelect;
            m_isMouseSelect = isChoose;
        }
    }
//    qDebug() << "isKeySelect..." << objectName() << m_isKeySelect << m_isMouseSelect;

    QString str;
    if ((m_isKeySelect && m_isMouseSelect) || (m_isKeySelect && !m_isMouseSelect)) {
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border: 1px solid #296CD9; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    } else if(!m_isKeySelect && m_isMouseSelect){
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    } else if(!m_isKeySelect && !m_isMouseSelect){
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,40);border: 0px; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    }

    m_iconLabel->setStyleSheet(str);
    m_iconLabel->setAttribute(Qt::WA_StyledBackground);

}

MyIconLabel* MyPushButton::getIconLabel()
{
    return m_iconLabel;
}

void MyPushButton::iconLabelMouseEvent(QEvent *event)
{
    emit mouseRelase(event, m_iconLabel->objectName());
}

bool MyPushButton::getIsKeySelect()
{
    return m_isKeySelect;
}

bool MyPushButton::getIsMouseSelect()
{
    return m_isMouseSelect;
}

void MyPushButton::setIsKeySelect(bool isKeySelect)
{
    if(m_isKeySelect != isKeySelect)
        m_isKeySelect = isKeySelect;
}

void MyPushButton::setIsMouseSelect(bool isMouseSelect)
{
    if(m_isMouseSelect != isMouseSelect)
        m_isMouseSelect = isMouseSelect;
}
