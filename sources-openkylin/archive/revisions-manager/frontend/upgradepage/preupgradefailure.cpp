#include "preupgradefailure.h"

preupgradefailure::preupgradefailure(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void preupgradefailure::initui()
{
    mainlayout = new QVBoxLayout();

    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/dialog-error.png").scaled(QSize(48,48)));

    warning = new QLabel();
    warning->setText("检测完成，不满足升级条件");

    content = new QTextEdit();
    content->setFrameShape(QFrame::NoFrame);
    content->setTextInteractionFlags(Qt::NoTextInteraction);
    content->viewport()->setAutoFillBackground(true);
    content->viewport()->setBackgroundRole(QPalette::Window);
    content->append("详情：");

    btn = new QPushButton();
    btn->setText("确定");
    btn->setFixedSize(120,48);

    mainlayout->addSpacing(155);
    mainlayout->addWidget(iconlabel,0,Qt::AlignHCenter);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(warning,0,Qt::AlignHCenter);
    mainlayout->addSpacing(8);
    mainlayout->addWidget(content,0,Qt::AlignHCenter);
    mainlayout->addStretch();
    mainlayout->addSpacing(8);
    mainlayout->addWidget(btn,0,Qt::AlignHCenter);
    mainlayout->addSpacing(40);

    this->setLayout(mainlayout);
}

void preupgradefailure::dbusconnection()
{
    connect(btn,&QPushButton::clicked,this,&preupgradefailure::exitapp);
}

void preupgradefailure::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
