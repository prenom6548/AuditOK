#include "npixcapwidget.h"
#include <QPalette>
#include <QPushButton>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#define BUTTONWIDTH 80
#define BUTTONHEIGHT 35

NPixCapWidget::NPixCapWidget()
{
    width = 440;
    height = 45;
    captureLayout = new QHBoxLayout;

    close = new CustomButtonStyle;
    close->setAccessibleName("kylin-camera_QPushButton_close");
    close->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    close->setAutoExclusive(true);
    close->setCheckable(true);
    close->setChecked(true);
    close->setFlat(true);
    capture3 = new CustomButtonStyle;
    capture3->setAccessibleName("kylin-camera_QPushButton_capture3");
    capture3->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    capture3->setAutoExclusive(true);
    capture3->setCheckable(true);
    capture3->setFlat(true);


    capture5 = new CustomButtonStyle;
    capture5->setAccessibleName("kylin-camera_QPushButton_capture5");
    capture5->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    capture5->setAutoExclusive(true);
    capture5->setCheckable(true);
    capture5->setFlat(true);

    capture10 = new CustomButtonStyle;
    capture10->setAccessibleName("kylin-camera_QPushButton_capture10");
    capture10->setAutoExclusive(true);
    capture10->setCheckable(true);
    capture10->setFlat(true);
    capture10->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);

    capture20 = new CustomButtonStyle;
    capture20->setAccessibleName("kylin-camera_QPushButton_capture20");
    capture20->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    capture20->setAutoExclusive(true);
    capture20->setCheckable(true);
    capture20->setFlat(true);

    captureLayout->setMargin(5);
    captureLayout->addWidget(close);
    captureLayout->addWidget(capture3);
    captureLayout->addWidget(capture5);
    captureLayout->addWidget(capture10);
    captureLayout->addWidget(capture20);
    close->setText(tr("close"));
    capture3->setText(tr("3p"));
    capture5->setText(tr("5p"));
    capture10->setText(tr("10p"));
    capture20->setText(tr("20p"));

    this->setLayout(captureLayout);
    this->setFixedSize(width,height);

    connect(close,&QPushButton::clicked,this,&NPixCapWidget::capCloseSlot);
    connect(capture3,&QPushButton::clicked,this,&NPixCapWidget::cap3Slot);
    connect(capture5,&QPushButton::clicked,this,&NPixCapWidget::cap5Slot);
    connect(capture10,&QPushButton::clicked,this,&NPixCapWidget::cap10Slot);
    connect(capture20,&QPushButton::clicked,this,&NPixCapWidget::cap20Slot);
}
NPixCapWidget::~NPixCapWidget(){
    delete close;
    delete capture3;
    delete capture5;
    delete capture10;
    delete capture20;

}
void NPixCapWidget::capCloseSlot(){
    Q_EMIT cap_emit(1);
}
void NPixCapWidget::cap3Slot(){
    Q_EMIT cap_emit(3);
}
void NPixCapWidget::cap5Slot(){
    Q_EMIT cap_emit(5);
}
void NPixCapWidget::cap10Slot(){
    Q_EMIT cap_emit(10);
}
void NPixCapWidget::cap20Slot(){
    Q_EMIT cap_emit(20);
}
//圆角及背景色特效
void NPixCapWidget::paintEvent(QPaintEvent *event)   //毛玻璃及圆角特效
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


