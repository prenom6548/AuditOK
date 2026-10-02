#include "prerestoresucess.h"

prerestoresucess::prerestoresucess(QWidget *parent) : QWidget(parent)
{
    initui();
    connection();
}

void prerestoresucess::initui()
{
    mainlayout = new QVBoxLayout();
    title1 = new QLabel();
    title2 = new QLabel();
    title2->setText("检测成功，即将自动进入还原");
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


void prerestoresucess::connection()
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

void prerestoresucess::nextpage()
{
    qDebug()<<"next page to upgrade";
    emit nextpagesignal();
}
