#include "restoresuccess.h"

restoresuccess::restoresuccess(QWidget *parent) : QWidget(parent)
{
    initui();
    connection();
}

void restoresuccess::initui()
{
//    mainlayout = new QVBoxLayout();
//    btnlayout = new QHBoxLayout();
//    btnwid = new QWidget();
//    title1 = new QLabel();
//    title2 = new QLabel();
//    confirm = new QPushButton();
//    later = new QPushButton();
//    buttonbox = new QWidget();

//    title1->setText("在重启界面选择\"立即重启\"来完成本次还原");
//    title2->setText("建议立即重启，避免用户误操作导致升级还原");
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


void restoresuccess::connection()
{
//    connect(confirm,&QPushButton::clicked,this,&restoresuccess::reboot);
//    connect(later,&QPushButton::clicked,this,&restoresuccess::exitapp);

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

void restoresuccess::exitapp()
{
    qDebug()<<"exit signal emit";
    emit exitappsignal();
}

void restoresuccess::reboot()
{
    qDebug()<<"reboot signal emit";
    emit rebootsignal();
}
