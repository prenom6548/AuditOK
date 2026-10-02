#include "restorefailure.h"

restorefailure::restorefailure(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void restorefailure::initui()
{
    mainlayout = new QVBoxLayout();
    toplayout = new QVBoxLayout();
    midlayout = new QVBoxLayout();

    topwid = new QWidget();
//    topwid->setStyleSheet("background: #BDD7FD");
    midwid = new QWidget();
//    midwid->setStyleSheet("background: red");


    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/dialog-error.png").scaled(QSize(48,48)));
    title = new QLabel();
    title->setText("还原失败");
    confirm = new QPushButton();
    confirm->setText("确定");

    mainlayout->addSpacing(100);
    mainlayout->addWidget(topwid);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(midwid);
    mainlayout->addWidget(confirm);

    toplayout->addWidget(iconlabel,0,Qt::AlignHCenter);
    toplayout->addWidget(title,0,Qt::AlignHCenter);

    topwid->setLayout(toplayout);
    midwid->setLayout(midlayout);

//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    confirm->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
//    title->setStyleSheet("QLabel{font-size:20px;line-height: 24px;background: #FFFFFF}");
    this->setLayout(mainlayout);


}

void restorefailure::dbusconnection()
{
    connect(confirm,&QPushButton::clicked,this,&restorefailure::exitapp);
}

void restorefailure::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
