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
#include "itemNew.h"
#include<QApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QProcess>
#include <math.h>

item_new::item_new(QWidget *parent,int s) :
    QWidget(parent)
{
    setupUi(this);
    this->setFixedSize(340, 108);
    m_selectBtnUtil = new SelectBtnUtil();
    m_selectBtnUtil->setObjectName("ukui-clock_SelectBtnUtil_m_selectBtnUtil");

    alarmLabel_w0 = new QLabel(this);//闹钟用途//Alarm function
    alarmLabel_w0->setObjectName("ukui-clock_QLabel_alarmLabel_w0");
    alarmLabel_w0->move(27  ,6);
    alarmLabel_w0->setFixedSize(200, 25);
    alarmLabel_w0->setText("闹钟");

    alarmLabel_s0 = new QLabel( this);//闹钟重复//Repetition of alarm clock
    alarmLabel_s0->setObjectName("ukui-clock_QLabel_alarmLabel_s0");
    alarmLabel_s0->move(27, 74);
    //调整闹钟页面“重复项”可显示宽度
    alarmLabel_s0->setFixedSize(350, 23);
    alarmLabel_s0->setText("工作日");
    alarmLabel_s0->setVisible(true);

    alarmLabel0 = new QLabel(this);//闹钟//Alarm clock
    alarmLabel0->setObjectName("ukui-clock_QLabel_alarmLabel0");
    //    alarmLabel0->move(25,24);
    alarmLabel0->setFixedSize(125, 56);
    alarmLabel0->setText("00:00");
    alarmLabel0->setVisible(true);
    alarmLabel0->show();

    alarmLabel1 = new QLabel(this);//上下午//Last afternoon
    alarmLabel1->setObjectName("ukui-clock_QLabel_alarmLabel1");
    //    alarmLabel1->move(155,22);
    alarmLabel1->setFixedSize(36, 56);
    QFont f;
    f.setPixelSize(18);
    alarmLabel1->setFont(f);
    //    alarmLabel1->setStyleSheet("font: 10pt;background-color: rgb();");
    alarmLabel1->setText("上午");
    alarmLabel1->setVisible(true);

    //更新24/12小时制的文本位置
    updateLabelStyle();

    alarm_on_off0 = new KSwitchButton(this);//闹钟开关// Alarm switch
    alarm_on_off0->setObjectName("ukui-clock_KSwitchButton_alarm_on_off0");
    alarm_on_off0->move(263,43);
    alarm_on_off0->setFixedSize(50,24);
    alarm_on_off0->setFlat(true);
    alarm_on_off0->setVisible(true);
    printf("%d\n", s);
    if(s)
        alarm_on_off0->setChecked(false);
    else
        alarm_on_off0->setChecked(true);
    settingsStyle();
    bool onTablet = GsettingSubject::getInstance()->getOnTablet();
    updatePosition(onTablet);
}

item_new::~item_new()
{
}

void item_new::setupUi(QWidget *item_new)
{
    if (item_new->objectName().isEmpty())
        item_new->setObjectName(QString::fromUtf8("item_new"));
    item_new->resize(376, 56);
    //    item_new->setStyleSheet(QString::fromUtf8("border-radius:12px;\n"
    //                                              ""));
    retranslateUi(item_new);

    QMetaObject::connectSlotsByName(item_new);
} // setupUi

void item_new::retranslateUi(QWidget *item_new)
{
    item_new->setWindowTitle(QApplication::translate("item_new", "Form", nullptr));
} // retranslateUi

/*
*监听主题
*/
void item_new::settingsStyle()
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
void item_new::blackStyle()
{

}
//白色主题
void item_new::whiteStyle()
{

}

void item_new::openStyle(int size)
{
    m_itemClose=false;
    commonStyle(size);
}

void item_new::commonStyle(int size)
{
    alarmLabel0->setStyleSheet(getLabelStyleSheet(round(2.7*size)));//时间
    alarmLabel1->setStyleSheet(getLabelStyleSheet(round(1.1*size)));//上下午
    alarmLabel_w0->setStyleSheet(getLabelStyleSheet(round(1.1*size))); //名字
    alarmLabel_s0->setStyleSheet(getLabelStyleSheet(round(1.1*size)));//重复
}
/**
 * @brief 关闭置灰
 */
void item_new::closeStyle(int size)
{
    m_itemClose=true;
    commonStyle(size);
}

QString item_new::getLabelStyleSheet(int size)
{
    QString str = "font-size:";
    str = str.append(QString::number(size));
    str = str.append("px;");
    if(m_itemClose){
        str = str.append("color:rgba(172,172,172,255);");
    }
    return str;
}

QString item_new::id() const
{
    return m_id;
}

void item_new::setId(const QString &id)
{
    m_id = id;
}

void item_new::updateLabelStyle()
{
    QString systemLang = QLocale::system().name();
    int localeNum = m_selectBtnUtil->getCurrentLocale();
    QProcess process;
    process.start("gsettings get org.ukui.control-center.panel.plugins hoursystem");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output;
    if (str_output.compare("'24'\n") == 0) {
        if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
            alarmLabel0->move(235,24);
            alarmLabel1->move(310,22);
        }else{
            alarmLabel0->move(25,50);
            alarmLabel1->move(134,30);
        }
    } else {
        if(localeNum!=11){
            if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
                alarmLabel0->move(182,24);
                alarmLabel1->move(282,22);
            }else{
                alarmLabel0->move(70,50);
                alarmLabel1->move(28,56);
            }
        }else{
            if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
                alarmLabel0->move(0,24);
                alarmLabel1->move(310,22);
            }else{
                alarmLabel0->move(25,50);
                alarmLabel1->move(134,56);
            }
        }
    }
}

void item_new::updatePosition(bool onTbalet)
{
    if(onTbalet){
        this->setFixedSize(340, 160);
        alarmLabel_w0->move(27  ,16);
        alarmLabel_s0->move(27, 118);
        alarmLabel0->move(25,52);
        alarmLabel1->setFixedSize(36, 56);
        alarm_on_off0->move(277,43);
    }else{
        this->setFixedSize(340, 108);
        QString systemLang = QLocale::system().name();
        if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
            alarmLabel_w0->move(63+55,6);
            alarmLabel_s0->move(48, 74);
            alarm_on_off0->move(27,43);
        }else{
            alarmLabel_w0->move(27  ,6);
            alarmLabel_s0->move(27, 74);
            alarmLabel0->move(25,24);
            alarmLabel1->move(155,22);
            alarm_on_off0->move(263,43);
        }
    }
}

