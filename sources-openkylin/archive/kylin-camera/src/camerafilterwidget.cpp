#include "camerafilterwidget.h"
#include <QPalette>
#include <QPushButton>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

cameraFilterWidget::cameraFilterWidget()
{
    width = 320;
    height = 70;
    filterLayout = new QHBoxLayout;
    QPalette pal;
    pal.setColor(QPalette::Button,QColor(20, 20, 20,0));
    original = new QPushButton(this);
    original->setAccessibleName("kylin-camera_QPushButton_original");
    original->setFixedSize(64,64);
    original->setAutoExclusive(true);
    original->setCheckable(true);
    original->setChecked(true);
    original->setIcon(QIcon(":/image/pic_yt.png"));
    original->setIconSize(QSize(64,64));
    original->setToolTip(tr("original"));
    original->setPalette(pal);


    warm = new QPushButton;
    warm->setAccessibleName("kylin-camera_QPushButton_warm");
    warm->setFixedSize(64,64);
    warm->setIcon(QIcon(":/image/pic_ns.png"));
    warm->setIconSize(QSize(64,64));
    warm->setAutoExclusive(true);
    warm->setCheckable(true);
    warm->setToolTip(tr("warm"));
    warm->setPalette(pal);

    cool = new QPushButton;
    cool->setAccessibleName("kylin-camera_QPushButton_cool");
    cool->setFixedSize(64,64);
    cool->setIcon(QIcon(":/image/pic_ls.png"));
    cool->setIconSize(QSize(64,64));
//    cool->setStyleSheet("QPushButton{border:none;background-image:url(:/image/pic_ls.png);color:rgb(255,255,255);text-align:top;font-weight: bold;}"
//                        "QPushButton:checked{border:none;color:rgb(0, 0, 0);}");
    cool->setAutoExclusive(true);
    cool->setCheckable(true);
    cool->setToolTip(tr("cool"));
    cool->setPalette(pal);

    grayscale = new QPushButton;
    grayscale->setAccessibleName("kylin-camera_QPushButton_grayscale");
    grayscale->setFixedSize(64,64);
    grayscale->setIcon(QIcon(":/image/pic_hb.png"));
    grayscale->setIconSize(QSize(64,64));
    grayscale->setAutoExclusive(true);
    grayscale->setCheckable(true);
    grayscale->setToolTip(tr("black-and-white"));
    grayscale->setPalette(pal);

    filterLayout->setMargin(0);
    filterLayout->addWidget(original);
    filterLayout->addWidget(warm);
    filterLayout->addWidget(cool);
    filterLayout->addWidget(grayscale);

    this->setLayout(filterLayout);
    this->setFixedSize(width,height);

    connect(original,&QPushButton::clicked,this,&cameraFilterWidget::filterOriginalSlot);
    connect(warm,&QPushButton::clicked,this,&cameraFilterWidget::filterWarmSlot);
    connect(cool,&QPushButton::clicked,this,&cameraFilterWidget::filterCoolSlot);
    connect(grayscale,&QPushButton::clicked,this,&cameraFilterWidget::filterGrayscaleSlot);
}
cameraFilterWidget::~cameraFilterWidget(){
    delete original;
    delete warm;
    delete cool;
    delete grayscale;
}
void cameraFilterWidget::filterOriginalSlot(){
    Q_EMIT filter_emit(0);
}
void cameraFilterWidget::filterWarmSlot(){
    Q_EMIT filter_emit(1);
}
void cameraFilterWidget::filterCoolSlot(){
    Q_EMIT filter_emit(2);
}
void cameraFilterWidget::filterGrayscaleSlot(){
    Q_EMIT filter_emit(3);
}
//圆角及背景色特效
void cameraFilterWidget::paintEvent(QPaintEvent *event)   //毛玻璃及圆角特效
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 10, 10); // 左上右下
    QStyleOption opt;
    opt.init(this);
    QColor mainColor;
    mainColor = QColor(20, 20, 20,150);
    painter.fillPath(rectPath,QBrush(mainColor));
    QWidget::paintEvent(event);
}
