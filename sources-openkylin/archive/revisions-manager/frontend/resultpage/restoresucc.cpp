#include "restoresucc.h"

RestoreSucc::RestoreSucc(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void RestoreSucc::initui()
{
    mainlayout = new QVBoxLayout();

    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/ukui-selected.png").scaled(QSize(48,48)));

    warning = new QLabel();
    warning->setText("还原成功");
//    warning->setStyleSheet("font-size:20px;line-height: 24px;");

    btn = new QPushButton();
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setText("确定");
    btn->setFixedSize(120,48);

    mainlayout->addSpacing(155);
    mainlayout->addWidget(iconlabel,0,Qt::AlignHCenter);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(warning,0,Qt::AlignHCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(btn,0,Qt::AlignHCenter);
    mainlayout->addSpacing(40);

//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    warning->setStyleSheet("QLabel{background: #FFFFFF}");
//    btn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
    this->setLayout(mainlayout);

}

void RestoreSucc::dbusconnection()
{
    connect(btn,&QPushButton::clicked,this,&RestoreSucc::exitapp);
}

void RestoreSucc::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
