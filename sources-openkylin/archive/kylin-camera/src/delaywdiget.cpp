#include "delaywdiget.h"
#include <QPalette>
#include <QPushButton>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#define BUTTONWIDTH 80
#define BUTTONHEIGHT 35

DelayWdiget::DelayWdiget()
{
    width = 352;
    height = 45;
    delayLayout = new QHBoxLayout;

    close = new CustomButtonStyle;
    close->setFlat(true);
    close->setAutoExclusive(true);
    close->setCheckable(true);
    close->setChecked(true);
    close->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);

    delay1 = new CustomButtonStyle;
    delay1->setFlat(true);
    delay1->setAutoExclusive(true);
    delay1->setCheckable(true);
    delay1->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);


    delay2 = new CustomButtonStyle;
    delay2->setFlat(true);
    delay2->setAutoExclusive(true);
    delay2->setCheckable(true);
    delay2->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);

    delay3 = new CustomButtonStyle;
    delay3->setFlat(true);
    delay3->setAutoExclusive(true);
    delay3->setCheckable(true);
    delay3->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);

    delayLayout->setMargin(5);
    delayLayout->addWidget(close);
    delayLayout->addWidget(delay1);
    delayLayout->addWidget(delay2);
    delayLayout->addWidget(delay3);
    close->setText(tr("close"));
    delay1->setText(tr("3s"));
    delay2->setText(tr("6s"));
    delay3->setText(tr("9s"));
    this->setLayout(delayLayout);
    this->setFixedSize(width,height);

    connect(close,&QPushButton::clicked,this,&DelayWdiget::time0SecSlot);
    connect(delay1,&QPushButton::clicked,this,&DelayWdiget::time3SecSlot);
    connect(delay2,&QPushButton::clicked,this,&DelayWdiget::time6SecSlot);
    connect(delay3,&QPushButton::clicked,this,&DelayWdiget::time9SecSlot);
}
DelayWdiget::~DelayWdiget(){

}
void DelayWdiget::time0SecSlot(){
    Q_EMIT delay_emit(0);
}
void DelayWdiget::time3SecSlot(){
    Q_EMIT delay_emit(1);
}
void DelayWdiget::time6SecSlot(){
    Q_EMIT delay_emit(2);
}
void DelayWdiget::time9SecSlot(){
    Q_EMIT delay_emit(3);
}
//圆角及背景色特效
void DelayWdiget::paintEvent(QPaintEvent *event)   //毛玻璃及圆角特效
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
   QPainterPath rectPath;
   rectPath.addRoundedRect(this->rect(), 10, 10); // 左上右下
   QStyleOption opt;
   opt.init(this);
   QColor mainColor = Qt::black;
   mainColor.setAlpha(77);
   painter.fillPath(rectPath,QBrush(mainColor));
   QWidget::paintEvent(event);
}


