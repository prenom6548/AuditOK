#include "upgradesuccess.h"

upgradesuccess::upgradesuccess(QWidget *parent) : QWidget(parent)
{
    initui();
    connection();
}

void upgradesuccess::initui()
{
//    mainlayout = new QVBoxLayout();
//    btnlayout = new QHBoxLayout();
//    btnwid = new QWidget();
//    title1 = new QLabel();
//    title2 = new QLabel();
//    confirm = new QPushButton();
//    later = new QPushButton();
//    buttonbox = new QWidget();

//    title1->setText("在重启界面选择\"立即重启\"来完成本次升级");
////    title2->setText("建议立即重启，避免用户误操作导致升级失败");
//    confirm->setText("立即重启");
//    later->setText("稍后重启");

//    btnwid->setLayout(btnlayout);
//    btnlayout->addWidget(confirm);
//    btnlayout->addWidget(later);
//    mainlayout->addWidget(title1);
//    mainlayout->addWidget(title2);
//    mainlayout->addWidget(btnwid);
//    this->setLayout(mainlayout);

    mainlayout = new QVBoxLayout();
    title1 = new QLabel();
    title2 = new QLabel();
    title1->setFixedSize(40,40);
    title1->setPixmap(QPixmap(":/png/dialog-info.png").scaled(QSize(38,38)));
    mainlayout->addStretch();
    mainlayout->addWidget(title1,0,Qt::AlignHCenter);
    mainlayout->addWidget(title2,0,Qt::AlignHCenter);
    mainlayout->addStretch();

//    title1->setStyleSheet("QLabel{background: #FFFFFF}");
//    title2->setStyleSheet("QLabel{background: #FFFFFF}");
    this->setLayout(mainlayout);
}


void upgradesuccess::connection()
{
//    connect(confirm,&QPushButton::clicked,this,&upgradesuccess::reboot);
//    connect(later,&QPushButton::clicked,this,&upgradesuccess::exitapp);

    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(m_timer,&QTimer::timeout,this,[=]()mutable
    {
            if(value>0)
            {
                qDebug()<<"倒计时关机"<<value;
                value--;
                title2->setText(QString("%1").arg(value)+"秒后会自动进行重启");
            }
            else
            {
                qDebug()<<"system reboot!!!!!!!";
                system("reboot");
                m_timer->stop();
            }
    });
}

void upgradesuccess::exitapp()
{
    qDebug()<<"exit signal emit";
    emit exitappsignal();
}

void upgradesuccess::reboot()
{
    qDebug()<<"reboot signal emit";
    emit rebootsignal();
}
