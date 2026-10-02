#include "preupgradesuccess.h"

preupgradesuccess::preupgradesuccess(QWidget *parent) : QWidget(parent)
{
    initui();
    connection();
}

void preupgradesuccess::initui()
{
    mainlayout = new QVBoxLayout();
//    title1 = new QLabel();
    title2 = new QLabel();
    progress = new KProgressCircle();
    progress->setValue(100);
    title2->setText("检测成功，即将自动进入升级");
//    title1->setFixedSize(40,40);
//    title1->setPixmap(QPixmap(":/png/dialog-info.png").scaled(QSize(38,38)));
    mainlayout->addStretch();
    mainlayout->addWidget(progress,0,Qt::AlignHCenter);
    mainlayout->addWidget(title2,0,Qt::AlignHCenter);
    mainlayout->addStretch();

//    title1->setStyleSheet("QLabel{background: #FFFFFF}");
//    title2->setStyleSheet("QLabel{background: #FFFFFF}");
    this->setLayout(mainlayout);
}


void preupgradesuccess::connection()
{
    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(m_timer,&QTimer::timeout,this,[=]()mutable
    {
            if(value>0)
            {
                qDebug()<<"倒计时升级"<<value;
                value--;
//                title2->setText(QString("%1").arg(value)+"秒后会自动进行更新");
            }
            else
            {
                qDebug()<<"upgrade!!!!!!";
//                system("reboot");
                title2->setText("nextpagesignal");
                emit nextpagesignal();
                m_timer->stop();
            }
    });
}

void preupgradesuccess::nextpage()
{
    qDebug()<<"next page to upgrade";
    emit nextpagesignal();
}

