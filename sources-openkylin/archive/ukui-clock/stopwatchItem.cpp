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
#include "stopwatchItem.h"
#include<QApplication>
#include <QDebug>

stopwatch_item::stopwatch_item(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    this->setFixedSize(356, 58);
    stopwatch1 = new QLabel(this);//计次
    stopwatch1->setObjectName("ukui-clock_QLabel_stopwatch1");
    stopwatch1->move(13, 11);
    stopwatch1->setFixedSize(100, 17);
    stopwatch1->setText("计次");


    stopwatch2 = new QLabel( this);//间隔
    stopwatch2->setObjectName("ukui-clock_QLabel_stopwatch2");
    stopwatch2->move(13, 32);
    stopwatch2->setFixedSize(138, 19);
    stopwatch2->setText("工作日");
    stopwatch2->setVisible(true);

    stopwatch3 = new QLabel(this);//记次时间
    stopwatch3->setObjectName("ukui-clock_QLabel_stopwatch3");
    stopwatch3->move(183,13);
    stopwatch3->setFixedSize(145, 54);
    stopwatch3->setStyleSheet("font-size:24px;");
    stopwatch3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    stopwatch3->setText("00:00");
    stopwatch3->setVisible(true);

    tipLabel = new QLabel(this);//最长最短标记
    tipLabel->setObjectName("ukui-clock_QLabel_tipLabel");
    tipLabel->move(150,30);
    tipLabel->setFixedSize(32, 20);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setText(tr("max"));
    tipLabel->hide();

    settingsStyle();
}

stopwatch_item::~stopwatch_item()
{
}

void stopwatch_item::setupUi(QWidget *stopwatch_item)
{
    if (stopwatch_item->objectName().isEmpty())
        stopwatch_item->setObjectName(QString::fromUtf8("stopwatch_item"));
    stopwatch_item->resize(372, 58);

    retranslateUi(stopwatch_item);

    QMetaObject::connectSlotsByName(stopwatch_item);
} // setupUi

void stopwatch_item::retranslateUi(QWidget *stopwatch_item)
{
    stopwatch_item->setWindowTitle(QApplication::translate("stopwatch_item", "Form", nullptr));
} // retranslateUi

/*
*监听主题
*/
void stopwatch_item::settingsStyle()
{
    GsettingSubject * subject = GsettingSubject::getInstance();;
    connect(subject,&GsettingSubject::blackStyle, this,[=](){
        this->blackStyle();
    });
    connect(subject,&GsettingSubject::whiteStyle, this,[=](){
        this->whiteStyle();
    });
    subject->iniWidgetStyle();
}

//黑色主题
void stopwatch_item::blackStyle()
{
    stopwatch1->setStyleSheet("color:rgba(255, 255, 255, 0.9);font-size:12px;");
    stopwatch2->setStyleSheet("font-size:12px;color:rgba(255, 255, 255, 0.55);");
    stopwatch3->setStyleSheet("color: rgba(255, 255, 255, 0.9)");
}
//白色主题
void stopwatch_item::whiteStyle()
{
    stopwatch1->setStyleSheet("color:rgba(49, 66, 89, 1);font-size:12px;");
    stopwatch2->setStyleSheet("font-size:12px;color:rgba(49, 66, 89, 0.5);");
    stopwatch3->setStyleSheet("color: rgba(49, 66, 89, 0.9)");
}

void stopwatch_item::updateTipCommonStyle()
{
    tipLabel->hide();
}

void stopwatch_item::updateTipLongestStyle()
{
    tipLabel->setText(tr("max"));
    tipLabel->setToolTip(tr("max"));
    QString itemRadius = QString::number(ITEM_RADIUS);
    tipLabel->setStyleSheet("QLabel{background-color:rgba(114, 46, 209, 255);border-radius: "+itemRadius+"px;color: white;text-align: center;font-size:12px;}");
    tipLabel->show();
}

void stopwatch_item::updateTipShortestStyle()
{
    tipLabel->setText(tr("min"));
    tipLabel->setToolTip(tr("min"));
    QString itemRadius = QString::number(ITEM_RADIUS);
    tipLabel->setStyleSheet("QLabel{background-color:rgba(55, 144, 250, 255);border-radius: "+itemRadius+"px;color: white;text-align: center;font-size:12px;}");
    tipLabel->show();
}


